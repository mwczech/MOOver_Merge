#!/usr/bin/env python3
"""
MOOver MELKENS Local Simulator with Real IMU Support
Compatible with Railway web simulator
"""

import asyncio
import json
import logging
import time
from dataclasses import dataclass, asdict
from typing import Dict, List, Optional, Tuple
import serial
import serial.tools.list_ports
from flask import Flask, render_template, request, jsonify
from flask_socketio import SocketIO, emit
import numpy as np
import threading

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

@dataclass
class IMUData:
    """IMU sensor data structure"""
    gyro_x: float = 0.0
    gyro_y: float = 0.0
    gyro_z: float = 0.0
    accel_x: float = 0.0
    accel_y: float = 0.0
    accel_z: float = 0.0
    mag_x: float = 0.0
    mag_y: float = 0.0
    mag_z: float = 0.0
    timestamp: float = 0.0

@dataclass
class RobotState:
    """Robot simulation state - compatible with Railway simulator"""
    motorRightSpeed: int = 0
    motorLeftSpeed: int = 0
    augerSpeed: int = 50
    batteryVoltage: int = 12400
    currentConsumption: int = 250
    magnetometer: List[int] = None
    systemErrors: List[str] = None
    connectionStatus: str = "connected"
    autonomousMode: bool = False
    currentRoute: str = "none"
    position: Dict[str, float] = None
    
    def __post_init__(self):
        if self.magnetometer is None:
            self.magnetometer = [0] * 16
        if self.systemErrors is None:
            self.systemErrors = []
        if self.position is None:
            self.position = {"x": 0.0, "y": 0.0, "heading": 0.0}

class IMUInterface:
    """Interface for real IMU communication via USB/Serial"""
    
    def __init__(self):
        self.serial_port: Optional[serial.Serial] = None
        self.is_connected = False
        self.latest_data = IMUData()
        self.data_callback = None
        
    def find_imu_device(self) -> Optional[str]:
        """Auto-detect IMU device"""
        ports = serial.tools.list_ports.comports()
        
        # Try to find STM32/ESP32 devices
        for port in ports:
            if any(keyword in port.description.lower() for keyword in 
                  ['stm32', 'esp32', 'usb serial', 'arduino']):
                logger.info(f"Found potential IMU device: {port.device}")
                return port.device
                
        # Fallback to common USB serial devices
        for port in ports:
            if 'usb' in port.device.lower():
                logger.info(f"Trying USB device: {port.device}")
                return port.device
                
        return None
    
    async def connect(self, port: str = None, baudrate: int = 115200) -> bool:
        """Connect to IMU device"""
        try:
            if port is None:
                port = self.find_imu_device()
                
            if port is None:
                logger.error("No IMU device found")
                return False
                
            self.serial_port = serial.Serial(
                port=port,
                baudrate=baudrate,
                timeout=1.0,
                write_timeout=1.0
            )
            
            await asyncio.sleep(2)  # Wait for device initialization
            
            # Test communication
            if await self.test_communication():
                self.is_connected = True
                logger.info(f"IMU connected successfully on {port}")
                return True
            else:
                self.serial_port.close()
                return False
                
        except Exception as e:
            logger.error(f"Failed to connect to IMU: {e}")
            return False
    
    async def test_communication(self) -> bool:
        """Test if IMU is responding"""
        try:
            # Send ping command
            self.serial_port.write(b"PING\n")
            response = self.serial_port.readline().decode().strip()
            return "PONG" in response or len(response) > 0
        except:
            return False
    
    def parse_imu_data(self, line: str) -> Optional[IMUData]:
        """Parse IMU data from serial line"""
        try:
            # Expected format: "IMU,gx,gy,gz,ax,ay,az,mx,my,mz"
            if line.startswith("IMU,"):
                parts = line.strip().split(",")
                if len(parts) >= 10:
                    return IMUData(
                        gyro_x=float(parts[1]),
                        gyro_y=float(parts[2]),
                        gyro_z=float(parts[3]),
                        accel_x=float(parts[4]),
                        accel_y=float(parts[5]),
                        accel_z=float(parts[6]),
                        mag_x=float(parts[7]),
                        mag_y=float(parts[8]),
                        mag_z=float(parts[9]),
                        timestamp=time.time()
                    )
            return None
        except ValueError as e:
            logger.warning(f"Failed to parse IMU data: {e}")
            return None
    
    async def read_data_loop(self):
        """Continuous data reading loop"""
        while self.is_connected and self.serial_port:
            try:
                if self.serial_port.in_waiting > 0:
                    line = self.serial_port.readline().decode().strip()
                    data = self.parse_imu_data(line)
                    
                    if data and self.data_callback:
                        self.latest_data = data
                        self.data_callback(data)
                        
                await asyncio.sleep(0.01)  # 100Hz max
                
            except Exception as e:
                logger.error(f"Error reading IMU data: {e}")
                await asyncio.sleep(1.0)
    
    def disconnect(self):
        """Disconnect from IMU"""
        self.is_connected = False
        if self.serial_port:
            self.serial_port.close()
            self.serial_port = None

class RouteEngine:
    """Physics-based route simulation engine"""
    
    def __init__(self):
        self.routes = {}
        self.current_route = None
        self.route_progress = 0.0
        self.load_routes()
        
    def load_routes(self):
        """Load route definitions"""
        # Default routes compatible with Railway simulator
        self.routes = {
            "A": {"waypoints": [(0, 0), (100, 0), (100, 100)], "speed": 50},
            "B": {"waypoints": [(0, 0), (0, 100), (100, 100)], "speed": 40},
            "C": {"waypoints": [(0, 0), (50, 50), (100, 0)], "speed": 60},
            # Add more routes as needed
        }
        
        # Try to load from file
        try:
            with open('routes/routes.json', 'r') as f:
                file_routes = json.load(f)
                self.routes.update(file_routes)
        except FileNotFoundError:
            logger.info("No custom routes file found, using defaults")
    
    def start_route(self, route_name: str) -> bool:
        """Start autonomous route execution"""
        if route_name in self.routes:
            self.current_route = route_name
            self.route_progress = 0.0
            logger.info(f"Started route {route_name}")
            return True
        return False
    
    def update_route_progress(self, robot_state: RobotState, imu_data: IMUData) -> RobotState:
        """Update robot state based on route progress and IMU feedback"""
        if not self.current_route:
            return robot_state
            
        route = self.routes[self.current_route]
        waypoints = route["waypoints"]
        
        # Simple physics simulation with IMU feedback
        if len(waypoints) > 1:
            progress_step = 0.01  # Adjust based on real IMU data
            self.route_progress = min(1.0, self.route_progress + progress_step)
            
            # Calculate position along route
            total_segments = len(waypoints) - 1
            segment_progress = self.route_progress * total_segments
            segment_index = int(segment_progress)
            
            if segment_index < total_segments:
                local_progress = segment_progress - segment_index
                start_point = waypoints[segment_index]
                end_point = waypoints[segment_index + 1]
                
                # Interpolate position
                x = start_point[0] + (end_point[0] - start_point[0]) * local_progress
                y = start_point[1] + (end_point[1] - start_point[1]) * local_progress
                
                # Calculate heading from IMU magnetometer
                heading = np.arctan2(imu_data.mag_y, imu_data.mag_x) * 180 / np.pi
                
                robot_state.position = {"x": x, "y": y, "heading": heading}
                
                # Set motor speeds based on route requirements
                base_speed = route["speed"]
                robot_state.motorLeftSpeed = base_speed
                robot_state.motorRightSpeed = base_speed
                
                # Route completed
                if self.route_progress >= 1.0:
                    robot_state.autonomousMode = False
                    robot_state.currentRoute = "completed"
                    logger.info(f"Route {self.current_route} completed")
                    
        return robot_state

class LocalSimulator:
    """Main local simulator class"""
    
    def __init__(self):
        self.app = Flask(__name__)
        self.socketio = SocketIO(self.app, cors_allowed_origins="*")
        self.robot_state = RobotState()
        self.imu = IMUInterface()
        self.route_engine = RouteEngine()
        self.setup_routes()
        self.setup_socketio()
        
    def setup_routes(self):
        """Setup Flask routes"""
        
        @self.app.route('/')
        def index():
            return render_template('dashboard.html')
            
        @self.app.route('/api/status')
        def status():
            return jsonify({
                "robot": asdict(self.robot_state),
                "imu": asdict(self.imu.latest_data),
                "imu_connected": self.imu.is_connected
            })
            
        @self.app.route('/api/routes')
        def get_routes():
            return jsonify(self.route_engine.routes)
            
        @self.app.route('/api/start_route', methods=['POST'])
        def start_route():
            route_name = request.json.get('route')
            success = self.route_engine.start_route(route_name)
            if success:
                self.robot_state.autonomousMode = True
                self.robot_state.currentRoute = route_name
            return jsonify({"success": success})
    
    def setup_socketio(self):
        """Setup SocketIO events"""
        
        @self.socketio.on('connect')
        def handle_connect():
            logger.info("Client connected")
            emit('robot_status', asdict(self.robot_state))
            
        @self.socketio.on('disconnect')
        def handle_disconnect():
            logger.info("Client disconnected")
            
        @self.socketio.on('control_robot')
        def handle_control(data):
            if not self.robot_state.autonomousMode:
                self.robot_state.motorLeftSpeed = data.get('leftSpeed', 0)
                self.robot_state.motorRightSpeed = data.get('rightSpeed', 0)
                self.broadcast_state()
                
        @self.socketio.on('emergency_stop')
        def handle_emergency_stop():
            self.robot_state.motorLeftSpeed = 0
            self.robot_state.motorRightSpeed = 0
            self.robot_state.autonomousMode = False
            self.robot_state.currentRoute = "stopped"
            self.broadcast_state()
    
    def on_imu_data(self, data: IMUData):
        """Callback for new IMU data"""
        # Update magnetometer array for compatibility
        if hasattr(data, 'mag_x'):
            # Convert real magnetometer data to 16-element array format
            mag_strength = np.sqrt(data.mag_x**2 + data.mag_y**2 + data.mag_z**2)
            # Simulate 16-sensor array based on single magnetometer
            self.robot_state.magnetometer = [int(mag_strength)] * 16
            
        # Update route progress if in autonomous mode
        if self.robot_state.autonomousMode:
            self.robot_state = self.route_engine.update_route_progress(
                self.robot_state, data
            )
            
        self.broadcast_state()
    
    def broadcast_state(self):
        """Broadcast robot state to all connected clients"""
        self.socketio.emit('robot_status', asdict(self.robot_state))
    
    async def start_simulator(self, port: str = None):
        """Start the local simulator"""
        logger.info("Starting MOOver MELKENS Local Simulator...")
        
        # Connect to IMU
        if await self.imu.connect(port):
            self.imu.data_callback = self.on_imu_data
            # Start IMU reading loop in background
            asyncio.create_task(self.imu.read_data_loop())
        else:
            logger.warning("Running in simulation mode without real IMU")
            
        # Start web server
        logger.info("Starting web interface on http://localhost:5000")
        self.socketio.run(self.app, host='0.0.0.0', port=5000, debug=False)

async def main():
    """Main entry point"""
    simulator = LocalSimulator()
    await simulator.start_simulator()

if __name__ == "__main__":
    asyncio.run(main())