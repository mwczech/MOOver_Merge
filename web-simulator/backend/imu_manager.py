#!/usr/bin/env python3
"""
MELKENS IMU Manager
Handles both simulated and real hardware IMU data acquisition
"""

import asyncio
import struct
import time
import math
import random
from typing import Dict, Optional, Tuple, Any
from dataclasses import dataclass
from datetime import datetime

import serial
import serial.tools.list_ports
import crcmod
import structlog

logger = structlog.get_logger()

@dataclass
class IMUReading:
    """IMU sensor reading structure"""
    timestamp: float
    accelerometer: Tuple[float, float, float]  # x, y, z (m/s²)
    gyroscope: Tuple[float, float, float]      # x, y, z (rad/s)
    magnetometer: Tuple[float, float, float]   # x, y, z (gauss)
    temperature: float                         # °C

@dataclass
class ProcessedIMUData:
    """Processed IMU data with pose estimation"""
    raw: IMUReading
    quaternion: Tuple[float, float, float, float]  # w, x, y, z
    euler_angles: Tuple[float, float, float]       # roll, pitch, yaw (rad)
    position: Tuple[float, float, float]           # x, y, z (m)
    velocity: Tuple[float, float, float]           # x, y, z (m/s)

class MelkensProtocolHandler:
    """Handles MELKENS IMU protocol communication"""
    
    # Protocol constants based on MessageTypes.h
    PROTOCOL_VERSION = 1
    
    # Message types
    IMU_TO_PC_FRAME = 0x01
    PC_TO_IMU_FRAME = 0x02
    
    def __init__(self):
        # CRC16 function for MELKENS protocol
        self.crc16 = crcmod.mkCrcFun(0x18005, rev=False, initCrc=0xFFFF, xorOut=0x0000)
    
    def pack_imu_to_pc_frame(self, data: ProcessedIMUData) -> bytes:
        """Pack IMU data into Imu2PCFrame_t structure"""
        # Convert data to the format expected by MELKENS
        motor_right_speed = int(data.velocity[0] * 1000)  # Convert to int16
        motor_left_speed = int(data.velocity[1] * 1000)
        x_pos1 = int(data.position[0] * 1000) & 0xFFFF
        y_pos1 = int(data.position[1] * 1000) & 0xFFFF
        x_pos2 = int(data.position[2] * 1000) & 0xFFFF
        y_pos2 = 0  # Reserved
        angle = int(data.euler_angles[2] * 180 / math.pi * 100) & 0xFFFF  # Yaw in centidegrees
        motor_belt2_speed = 0  # Not used in simulation
        
        # Pack data without CRC first
        packed_data = struct.pack('<HHHHHHHH',
            motor_right_speed & 0xFFFF,
            motor_left_speed & 0xFFFF,
            x_pos1,
            y_pos1,
            x_pos2,
            y_pos2,
            angle,
            motor_belt2_speed
        )
        
        # Calculate CRC
        crc = self.crc16(packed_data)
        
        # Pack with CRC
        return packed_data + struct.pack('<H', crc)
    
    def unpack_pc_to_imu_frame(self, data: bytes) -> Optional[Dict]:
        """Unpack PC to IMU frame (commands)"""
        if len(data) < 10:  # Minimum frame size
            return None
        
        try:
            # Extract CRC
            payload = data[:-2]
            received_crc = struct.unpack('<H', data[-2:])[0]
            
            # Verify CRC
            calculated_crc = self.crc16(payload)
            if received_crc != calculated_crc:
                logger.warning("CRC mismatch in PC to IMU frame")
                return None
            
            # Unpack based on expected structure
            if len(payload) >= 8:
                values = struct.unpack('<HHHH', payload[:8])
                return {
                    'command_type': values[0],
                    'param1': values[1],
                    'param2': values[2],
                    'param3': values[3]
                }
        except Exception as e:
            logger.error("Error unpacking PC to IMU frame", error=str(e))
        
        return None
    
    def create_raw_imu_frame(self, reading: IMUReading) -> bytes:
        """Create raw IMU data frame (custom format for simulation)"""
        # Pack raw sensor data
        data = struct.pack('<ffffffffffff',
            reading.timestamp,
            reading.accelerometer[0], reading.accelerometer[1], reading.accelerometer[2],
            reading.gyroscope[0], reading.gyroscope[1], reading.gyroscope[2],
            reading.magnetometer[0], reading.magnetometer[1], reading.magnetometer[2],
            reading.temperature,
            0.0  # Reserved
        )
        
        # Add CRC
        crc = self.crc16(data)
        return data + struct.pack('<H', crc)

class IMUManager:
    """Main IMU Manager class"""
    
    def __init__(self, default_port: str = "/dev/ttyUSB0", default_baud: int = 115200):
        self.mode = "simulated"  # "simulated" or "hardware"
        self.serial_port = default_port
        self.baud_rate = default_baud
        self.serial_connection: Optional[serial.Serial] = None
        self.protocol_handler = MelkensProtocolHandler()
        
        # State tracking
        self.is_running = False
        self.hardware_connected = False
        self.last_error = None
        self.data_update_task = None
        
        # Data storage
        self.latest_data: Optional[ProcessedIMUData] = None
        self.data_lock = asyncio.Lock()
        
        # Fault injection
        self.fault_injection_enabled = False
        self.fault_config = {}
        
        # Simulation state
        self.sim_time = 0.0
        self.sim_position = [0.0, 0.0, 0.0]
        self.sim_velocity = [0.0, 0.0, 0.0]
        self.sim_acceleration = [0.0, 0.0, 9.81]  # Start with gravity
        
        # Statistics
        self.packet_count = 0
        self.error_count = 0
        self.last_packet_time = 0.0
    
    async def start(self):
        """Start the IMU manager"""
        self.is_running = True
        self.data_update_task = asyncio.create_task(self._data_update_loop())
        logger.info("IMU Manager started")
    
    async def stop(self):
        """Stop the IMU manager"""
        self.is_running = False
        
        if self.data_update_task:
            self.data_update_task.cancel()
            try:
                await self.data_update_task
            except asyncio.CancelledError:
                pass
        
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
        
        logger.info("IMU Manager stopped")
    
    async def set_mode(self, mode: str):
        """Set IMU mode (simulated or hardware)"""
        if mode not in ["simulated", "hardware"]:
            raise ValueError(f"Invalid mode: {mode}")
        
        old_mode = self.mode
        self.mode = mode
        
        logger.info("IMU mode changed", old_mode=old_mode, new_mode=mode)
        
        if mode == "hardware":
            await self._connect_hardware()
        else:
            await self._disconnect_hardware()
    
    async def configure_serial(self, port: str, baud_rate: int):
        """Configure serial connection parameters"""
        self.serial_port = port
        self.baud_rate = baud_rate
        
        if self.mode == "hardware":
            await self._connect_hardware()
    
    async def _connect_hardware(self):
        """Connect to hardware IMU"""
        try:
            if self.serial_connection and self.serial_connection.is_open:
                self.serial_connection.close()
            
            # Try to connect to the specified port
            self.serial_connection = serial.Serial(
                port=self.serial_port,
                baudrate=self.baud_rate,
                timeout=1.0,
                write_timeout=1.0
            )
            
            # Test connection
            await asyncio.sleep(0.1)  # Give it time to stabilize
            
            if self.serial_connection.is_open:
                self.hardware_connected = True
                self.last_error = None
                logger.info("Connected to hardware IMU", port=self.serial_port, baud=self.baud_rate)
            else:
                raise Exception("Failed to open serial port")
                
        except Exception as e:
            self.hardware_connected = False
            self.last_error = str(e)
            logger.error("Failed to connect to hardware IMU", error=str(e), port=self.serial_port)
    
    async def _disconnect_hardware(self):
        """Disconnect from hardware IMU"""
        if self.serial_connection and self.serial_connection.is_open:
            self.serial_connection.close()
        
        self.hardware_connected = False
        logger.info("Disconnected from hardware IMU")
    
    async def _data_update_loop(self):
        """Main data update loop"""
        while self.is_running:
            try:
                if self.mode == "hardware" and self.hardware_connected:
                    data = await self._read_hardware_data()
                else:
                    data = await self._generate_simulated_data()
                
                if data:
                    # Apply fault injection if enabled
                    if self.fault_injection_enabled:
                        data = self._apply_fault_injection(data)
                    
                    async with self.data_lock:
                        self.latest_data = data
                    
                    self.packet_count += 1
                    self.last_packet_time = time.time()
                
                await asyncio.sleep(0.02)  # 50 Hz update rate
                
            except Exception as e:
                self.error_count += 1
                logger.error("Error in data update loop", error=str(e))
                await asyncio.sleep(0.1)
    
    async def _read_hardware_data(self) -> Optional[ProcessedIMUData]:
        """Read data from hardware IMU"""
        try:
            if not self.serial_connection or not self.serial_connection.is_open:
                return None
            
            # Check for available data
            if self.serial_connection.in_waiting < 20:  # Minimum frame size
                return None
            
            # Read available data
            data = self.serial_connection.read(self.serial_connection.in_waiting)
            
            # Try to parse the data
            # This is a simplified parser - in real implementation,
            # you would need to handle frame synchronization
            if len(data) >= 48:  # Expected size for raw IMU frame
                parsed = self._parse_hardware_frame(data[-48:])  # Take last complete frame
                if parsed:
                    return parsed
            
            return None
            
        except Exception as e:
            logger.error("Error reading hardware data", error=str(e))
            self.last_error = str(e)
            return None
    
    def _parse_hardware_frame(self, data: bytes) -> Optional[ProcessedIMUData]:
        """Parse hardware IMU frame"""
        try:
            # Extract CRC
            payload = data[:-2]
            received_crc = struct.unpack('<H', data[-2:])[0]
            
            # Verify CRC
            calculated_crc = self.protocol_handler.crc16(payload)
            if received_crc != calculated_crc:
                logger.warning("CRC mismatch in hardware frame")
                return None
            
            # Unpack raw sensor data
            values = struct.unpack('<ffffffffffff', payload)
            
            raw_reading = IMUReading(
                timestamp=values[0],
                accelerometer=(values[1], values[2], values[3]),
                gyroscope=(values[4], values[5], values[6]),
                magnetometer=(values[7], values[8], values[9]),
                temperature=values[10]
            )
            
            # Process the raw data
            return self._process_raw_data(raw_reading)
            
        except Exception as e:
            logger.error("Error parsing hardware frame", error=str(e))
            return None
    
    async def _generate_simulated_data(self) -> ProcessedIMUData:
        """Generate simulated IMU data"""
        current_time = time.time()
        dt = 0.02  # 50 Hz
        self.sim_time += dt
        
        # Simulate some movement pattern
        # Simple circular motion with some noise
        omega = 0.1  # Angular velocity
        radius = 2.0
        
        # Position (circular motion)
        self.sim_position[0] = radius * math.cos(omega * self.sim_time)
        self.sim_position[1] = radius * math.sin(omega * self.sim_time)
        self.sim_position[2] = 0.1 * math.sin(2 * omega * self.sim_time)  # Small Z movement
        
        # Velocity
        self.sim_velocity[0] = -radius * omega * math.sin(omega * self.sim_time)
        self.sim_velocity[1] = radius * omega * math.cos(omega * self.sim_time)
        self.sim_velocity[2] = 0.2 * omega * math.cos(2 * omega * self.sim_time)
        
        # Acceleration
        centripetal_acc = radius * omega * omega
        self.sim_acceleration[0] = -centripetal_acc * math.cos(omega * self.sim_time)
        self.sim_acceleration[1] = -centripetal_acc * math.sin(omega * self.sim_time)
        self.sim_acceleration[2] = 9.81 - 0.4 * omega * omega * math.sin(2 * omega * self.sim_time)
        
        # Add noise
        noise_level = 0.01
        acc_noise = [random.gauss(0, noise_level) for _ in range(3)]
        gyro_noise = [random.gauss(0, noise_level * 0.1) for _ in range(3)]
        mag_noise = [random.gauss(0, noise_level * 10) for _ in range(3)]
        
        # Simulate LSM6DSR (accelerometer + gyroscope) and LIS3MDL (magnetometer)
        raw_reading = IMUReading(
            timestamp=current_time,
            accelerometer=(
                self.sim_acceleration[0] + acc_noise[0],
                self.sim_acceleration[1] + acc_noise[1],
                self.sim_acceleration[2] + acc_noise[2]
            ),
            gyroscope=(
                omega + gyro_noise[0],
                0.0 + gyro_noise[1],
                0.0 + gyro_noise[2]
            ),
            magnetometer=(
                25000 + mag_noise[0],  # Earth's magnetic field ~25-65 µT
                -2000 + mag_noise[1],
                40000 + mag_noise[2]
            ),
            temperature=25.0 + random.gauss(0, 0.5)
        )
        
        return self._process_raw_data(raw_reading)
    
    def _process_raw_data(self, raw: IMUReading) -> ProcessedIMUData:
        """Process raw IMU data to extract pose and position"""
        # Simple Madgwick-style quaternion integration
        # This is a simplified version - real implementation would be more complex
        
        # Convert gyroscope to quaternion delta
        gx, gy, gz = raw.gyroscope
        dt = 0.02
        
        # Simple integration for demonstration
        # In real implementation, you'd use proper sensor fusion
        yaw = math.atan2(raw.magnetometer[1], raw.magnetometer[0])
        pitch = math.asin(-raw.accelerometer[0] / 9.81) if abs(raw.accelerometer[0]) < 9.81 else 0
        roll = math.atan2(raw.accelerometer[1], raw.accelerometer[2])
        
        # Convert to quaternion (simplified)
        cy = math.cos(yaw * 0.5)
        sy = math.sin(yaw * 0.5)
        cp = math.cos(pitch * 0.5)
        sp = math.sin(pitch * 0.5)
        cr = math.cos(roll * 0.5)
        sr = math.sin(roll * 0.5)
        
        quaternion = (
            cr * cp * cy + sr * sp * sy,  # w
            sr * cp * cy - cr * sp * sy,  # x
            cr * sp * cy + sr * cp * sy,  # y
            cr * cp * sy - sr * sp * cy   # z
        )
        
        return ProcessedIMUData(
            raw=raw,
            quaternion=quaternion,
            euler_angles=(roll, pitch, yaw),
            position=tuple(self.sim_position),
            velocity=tuple(self.sim_velocity)
        )
    
    def _apply_fault_injection(self, data: ProcessedIMUData) -> ProcessedIMUData:
        """Apply fault injection to IMU data"""
        if not self.fault_injection_enabled or not self.fault_config:
            return data
        
        fault_type = self.fault_config.get('fault_type', 'none')
        axis = self.fault_config.get('axis', 'x')
        severity = self.fault_config.get('severity', 1.0)
        
        # Create a copy of the data to modify
        modified_raw = IMUReading(
            timestamp=data.raw.timestamp,
            accelerometer=list(data.raw.accelerometer),
            gyroscope=list(data.raw.gyroscope),
            magnetometer=list(data.raw.magnetometer),
            temperature=data.raw.temperature
        )
        
        axis_idx = {'x': 0, 'y': 1, 'z': 2}.get(axis, 0)
        
        if fault_type == 'drift':
            # Add drift to accelerometer
            drift = severity * 0.1 * math.sin(time.time() * 0.5)
            modified_raw.accelerometer[axis_idx] += drift
            
        elif fault_type == 'stuck_axis':
            # Stuck axis - freeze the value
            modified_raw.accelerometer[axis_idx] = 0.0
            modified_raw.gyroscope[axis_idx] = 0.0
            
        elif fault_type == 'noise':
            # Add excessive noise
            noise = random.gauss(0, severity * 0.5)
            modified_raw.accelerometer[axis_idx] += noise
            modified_raw.gyroscope[axis_idx] += noise * 0.1
            
        elif fault_type == 'missing_packets':
            # Randomly drop packets
            if random.random() < severity * 0.1:
                return data  # Return old data (simulating missing packet)
        
        # Reprocess the modified data
        return self._process_raw_data(modified_raw)
    
    async def configure_fault_injection(self, config: dict):
        """Configure fault injection"""
        self.fault_config = config
        self.fault_injection_enabled = config.get('enabled', False)
        logger.info("Fault injection configured", config=config)
    
    async def get_latest_data(self) -> Optional[dict]:
        """Get the latest processed IMU data"""
        async with self.data_lock:
            if not self.latest_data:
                return None
            
            data = self.latest_data
            return {
                'timestamp': data.raw.timestamp,
                'accelerometer': {
                    'x': data.raw.accelerometer[0],
                    'y': data.raw.accelerometer[1],
                    'z': data.raw.accelerometer[2]
                },
                'gyroscope': {
                    'x': data.raw.gyroscope[0],
                    'y': data.raw.gyroscope[1],
                    'z': data.raw.gyroscope[2]
                },
                'magnetometer': {
                    'x': data.raw.magnetometer[0],
                    'y': data.raw.magnetometer[1],
                    'z': data.raw.magnetometer[2]
                },
                'quaternion': {
                    'w': data.quaternion[0],
                    'x': data.quaternion[1],
                    'y': data.quaternion[2],
                    'z': data.quaternion[3]
                },
                'euler_angles': {
                    'roll': data.euler_angles[0],
                    'pitch': data.euler_angles[1],
                    'yaw': data.euler_angles[2]
                },
                'position': {
                    'x': data.position[0],
                    'y': data.position[1],
                    'z': data.position[2]
                },
                'hardware_connected': self.hardware_connected,
                'error_message': self.last_error
            }
    
    async def get_status(self) -> dict:
        """Get IMU system status"""
        return {
            'mode': self.mode,
            'hardware_connected': self.hardware_connected,
            'serial_port': self.serial_port,
            'baud_rate': self.baud_rate,
            'last_error': self.last_error,
            'packet_count': self.packet_count,
            'error_count': self.error_count,
            'data_rate': self._calculate_data_rate(),
            'fault_injection_enabled': self.fault_injection_enabled
        }
    
    def _calculate_data_rate(self) -> float:
        """Calculate current data rate in Hz"""
        current_time = time.time()
        if current_time - self.last_packet_time > 1.0:
            return 0.0
        return 50.0  # Nominal rate
    
    async def self_test(self) -> dict:
        """Perform comprehensive self-test"""
        logger.info("Starting IMU self-test")
        
        test_results = {
            'success': True,
            'tests': {},
            'timestamp': datetime.now().isoformat()
        }
        
        # Test 1: Check available serial ports
        available_ports = [port.device for port in serial.tools.list_ports.comports()]
        test_results['tests']['available_ports'] = {
            'passed': True,
            'ports': available_ports,
            'target_port': self.serial_port
        }
        
        # Test 2: Hardware connection test
        if self.mode == "hardware":
            try:
                await self._connect_hardware()
                test_results['tests']['hardware_connection'] = {
                    'passed': self.hardware_connected,
                    'error': self.last_error
                }
                if not self.hardware_connected:
                    test_results['success'] = False
            except Exception as e:
                test_results['tests']['hardware_connection'] = {
                    'passed': False,
                    'error': str(e)
                }
                test_results['success'] = False
        
        # Test 3: Data acquisition test
        test_start = time.time()
        data_samples = []
        
        for i in range(10):  # Collect 10 samples
            if self.mode == "hardware":
                data = await self._read_hardware_data()
            else:
                data = await self._generate_simulated_data()
            
            if data:
                data_samples.append(data)
            
            await asyncio.sleep(0.02)
        
        test_results['tests']['data_acquisition'] = {
            'passed': len(data_samples) >= 8,  # At least 80% success rate
            'samples_collected': len(data_samples),
            'expected_samples': 10,
            'test_duration': time.time() - test_start
        }
        
        if len(data_samples) < 8:
            test_results['success'] = False
        
        # Test 4: Data validation test
        if data_samples:
            last_sample = data_samples[-1]
            
            # Check accelerometer values (should be reasonable)
            acc_magnitude = math.sqrt(sum(x*x for x in last_sample.raw.accelerometer))
            acc_test_passed = 5.0 < acc_magnitude < 15.0  # Reasonable range
            
            # Check gyroscope values (should not be extremely large)
            gyro_magnitude = math.sqrt(sum(x*x for x in last_sample.raw.gyroscope))
            gyro_test_passed = gyro_magnitude < 10.0  # Reasonable range
            
            # Check magnetometer values
            mag_magnitude = math.sqrt(sum(x*x for x in last_sample.raw.magnetometer))
            mag_test_passed = mag_magnitude > 1000  # Should detect Earth's field
            
            validation_passed = acc_test_passed and gyro_test_passed and mag_test_passed
            
            test_results['tests']['data_validation'] = {
                'passed': validation_passed,
                'accelerometer_magnitude': acc_magnitude,
                'gyroscope_magnitude': gyro_magnitude,
                'magnetometer_magnitude': mag_magnitude
            }
            
            if not validation_passed:
                test_results['success'] = False
        else:
            test_results['tests']['data_validation'] = {
                'passed': False,
                'error': 'No data samples to validate'
            }
            test_results['success'] = False
        
        logger.info("IMU self-test completed", success=test_results['success'])
        return test_results