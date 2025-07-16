#!/usr/bin/env python3
"""
WB_Simulator.py

Wasserbauer (WB) Communication Simulator for PC

This simulator emulates the WB Butler Engine communication protocol
for testing and development of the MELKENS-WB compatibility layer.
It provides:
- CAN bus simulation and message generation
- WB Butler Engine protocol emulation
- MELKENS response monitoring and logging
- Performance testing and stress scenarios
- CSV/JSON log export for analysis

Author: MOOver Integration Team
Created: 2024-12-19
Phase: 3 - Testing and Emulation
"""

import sys
import time
import json
import csv
import struct
import socket
import threading
import logging
import argparse
from datetime import datetime
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass, asdict
from enum import Enum, IntEnum
import random
import math

# ============================================================================
# CONSTANTS AND ENUMERATIONS
# ============================================================================

class WBNodeID(IntEnum):
    """WB CANopen Node IDs"""
    BUTLER_MAIN = 0x40
    SERVO_LEFT = 0x7E
    SERVO_RIGHT = 0x7F
    SERVO_THUMBLE = 0x7D
    MAGNETIC_ENCODER = 0x10
    STEERING_WHEEL = 0x20

class WBDriveRequest(IntEnum):
    """WB Drive Request Types"""
    MANUAL_CONTROL = 0x0001
    AUTO_NAVIGATION = 0x0002
    EMERGENCY_STOP = 0x0003
    BAY_APPROACH = 0x0004

class WBOperationalState(IntEnum):
    """WB Operational States"""
    UNINITIALIZED = 0
    INITIALIZING = 1
    READY = 2
    OPERATIONAL = 3
    ERROR = 4
    MAINTENANCE = 5

# ============================================================================
# DATA STRUCTURES
# ============================================================================

@dataclass
class WBButlerCommand:
    """WB Butler Engine Command Structure"""
    command_id: int = 0
    drive_request: int = 0
    manual_request: int = 0
    manual_speed: int = 0
    manual_steering: int = 0
    target_track_id: int = 0
    target_bay_id: int = 0
    target_x: float = 0.0
    target_y: float = 0.0
    target_heading: float = 0.0
    feed_amount: int = 0
    drive_speed: int = 0
    abort_request: int = 0
    timestamp: int = 0
    checksum: int = 0

@dataclass
class WBStatusResponse:
    """WB Status Response Structure"""
    status_word: int = 0
    error_register: int = 0
    operational_state: int = 0
    current_x: float = 0.0
    current_y: float = 0.0
    current_heading: float = 0.0
    current_track_id: int = 0
    current_bay_id: int = 0
    motor_left_speed: int = 0
    motor_right_speed: int = 0
    motor_thumble_speed: int = 0
    motor_status_flags: int = 0
    magnetic_field_strength: float = 0.0
    magnetic_position: int = 0
    battery_level: int = 0
    sensor_status: int = 0
    timestamp: int = 0
    sequence_number: int = 0

@dataclass
class CANMessage:
    """CAN Message Structure"""
    can_id: int
    data: bytes
    timestamp: float
    is_extended: bool = False
    
@dataclass
class SimulationEvent:
    """Simulation Event for Logging"""
    timestamp: float
    event_type: str
    source: str
    destination: str
    data: Dict
    success: bool = True
    error_message: str = ""

# ============================================================================
# WB SIMULATOR CORE CLASS
# ============================================================================

class WBSimulator:
    """Main WB Communication Simulator"""
    
    def __init__(self, config: Dict):
        """Initialize WB Simulator"""
        self.config = config
        self.running = False
        self.start_time = time.time()
        
        # Communication settings
        self.can_interface = config.get('can_interface', 'vcan0')
        self.tcp_port = config.get('tcp_port', 8080)
        self.update_rate_hz = config.get('update_rate_hz', 10)
        
        # Simulation state
        self.current_command = WBButlerCommand()
        self.current_status = WBStatusResponse()
        self.sequence_number = 0
        self.command_counter = 0
        
        # Logging and events
        self.events: List[SimulationEvent] = []
        self.logger = self._setup_logging()
        
        # Statistics
        self.stats = {
            'commands_sent': 0,
            'responses_received': 0,
            'errors_encountered': 0,
            'simulation_duration': 0,
            'average_response_time': 0.0,
            'max_response_time': 0.0,
            'min_response_time': float('inf')
        }
        
        # Virtual robot state
        self.robot_state = {
            'position': {'x': 0.0, 'y': 0.0, 'heading': 0.0},
            'velocity': {'linear': 0.0, 'angular': 0.0},
            'battery_level': 100,
            'magnetic_position': 16,  # Center position
            'operational_state': WBOperationalState.READY,
            'error_flags': 0
        }
        
        # Scenario management
        self.scenario_active = False
        self.scenario_thread = None
        
    def _setup_logging(self) -> logging.Logger:
        """Setup logging configuration"""
        logger = logging.getLogger('WBSimulator')
        logger.setLevel(logging.DEBUG if self.config.get('debug', False) else logging.INFO)
        
        # Console handler
        console_handler = logging.StreamHandler()
        console_formatter = logging.Formatter(
            '%(asctime)s - %(name)s - %(levelname)s - %(message)s'
        )
        console_handler.setFormatter(console_formatter)
        logger.addHandler(console_handler)
        
        # File handler
        if self.config.get('log_file'):
            file_handler = logging.FileHandler(self.config['log_file'])
            file_handler.setFormatter(console_formatter)
            logger.addHandler(file_handler)
            
        return logger
    
    def start(self):
        """Start the WB simulator"""
        self.logger.info("Starting WB Simulator...")
        self.running = True
        self.start_time = time.time()
        
        # Start communication threads
        self.can_thread = threading.Thread(target=self._can_communication_loop, daemon=True)
        self.tcp_thread = threading.Thread(target=self._tcp_server_loop, daemon=True)
        self.update_thread = threading.Thread(target=self._simulation_update_loop, daemon=True)
        
        self.can_thread.start()
        self.tcp_thread.start()
        self.update_thread.start()
        
        self.logger.info("WB Simulator started successfully")
        
    def stop(self):
        """Stop the WB simulator"""
        self.logger.info("Stopping WB Simulator...")
        self.running = False
        
        # Wait for threads to finish
        if hasattr(self, 'can_thread'):
            self.can_thread.join(timeout=1.0)
        if hasattr(self, 'tcp_thread'):
            self.tcp_thread.join(timeout=1.0)
        if hasattr(self, 'update_thread'):
            self.update_thread.join(timeout=1.0)
            
        # Stop any active scenarios
        if self.scenario_active and self.scenario_thread:
            self.scenario_active = False
            self.scenario_thread.join(timeout=2.0)
            
        self.logger.info("WB Simulator stopped")
    
    # ========================================================================
    # CAN COMMUNICATION SIMULATION
    # ========================================================================
    
    def _can_communication_loop(self):
        """Main CAN communication loop"""
        self.logger.info("Starting CAN communication loop")
        
        try:
            # Simulate CAN socket creation
            self.logger.info(f"Opening CAN interface: {self.can_interface}")
            
            while self.running:
                # Send periodic commands
                self._send_butler_command()
                
                # Listen for responses (simulated)
                self._receive_melkens_response()
                
                time.sleep(1.0 / self.update_rate_hz)
                
        except Exception as e:
            self.logger.error(f"CAN communication error: {e}")
            self._log_event("error", "CAN", "System", {"error": str(e)}, False, str(e))
    
    def _send_butler_command(self):
        """Send Butler command via CAN"""
        # Generate command based on current scenario or manual control
        command = self._generate_command()
        
        # Convert to CAN message
        can_msg = self._command_to_can_message(command)
        
        # Simulate sending (log instead of actual transmission)
        self.logger.debug(f"Sending CAN message: ID=0x{can_msg.can_id:03X}, Data={can_msg.data.hex()}")
        
        # Log event
        self._log_event("command_sent", "WB_Butler", "MELKENS", asdict(command))
        
        self.stats['commands_sent'] += 1
        self.current_command = command
    
    def _receive_melkens_response(self):
        """Receive and process MELKENS response"""
        # Simulate receiving response
        response = self._simulate_melkens_response()
        
        if response:
            self.logger.debug(f"Received MELKENS response: seq={response.sequence_number}")
            self._log_event("response_received", "MELKENS", "WB_Butler", asdict(response))
            self.stats['responses_received'] += 1
            self.current_status = response
    
    def _command_to_can_message(self, command: WBButlerCommand) -> CANMessage:
        """Convert Butler command to CAN message"""
        # Pack command data into CAN format
        data = struct.pack('<HHHH', 
                          command.command_id,
                          command.drive_request,
                          command.manual_speed,
                          command.manual_steering)
        
        return CANMessage(
            can_id=WBNodeID.BUTLER_MAIN,
            data=data,
            timestamp=time.time()
        )
    
    def _simulate_melkens_response(self) -> Optional[WBStatusResponse]:
        """Simulate MELKENS status response"""
        if random.random() < 0.95:  # 95% response rate
            response = WBStatusResponse()
            response.timestamp = int(time.time() * 1000)
            response.sequence_number = self.sequence_number
            response.operational_state = self.robot_state['operational_state']
            response.current_x = self.robot_state['position']['x']
            response.current_y = self.robot_state['position']['y']
            response.current_heading = self.robot_state['position']['heading']
            response.battery_level = self.robot_state['battery_level']
            response.magnetic_position = self.robot_state['magnetic_position']
            response.status_word = 0x0001
            
            self.sequence_number += 1
            return response
        
        return None
    
    # ========================================================================
    # TCP SERVER FOR EXTERNAL CONTROL
    # ========================================================================
    
    def _tcp_server_loop(self):
        """TCP server for external control and monitoring"""
        try:
            server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            server_socket.bind(('localhost', self.tcp_port))
            server_socket.listen(5)
            server_socket.settimeout(1.0)
            
            self.logger.info(f"TCP server listening on port {self.tcp_port}")
            
            while self.running:
                try:
                    client_socket, address = server_socket.accept()
                    self.logger.info(f"Client connected from {address}")
                    
                    # Handle client in separate thread
                    client_thread = threading.Thread(
                        target=self._handle_tcp_client,
                        args=(client_socket,),
                        daemon=True
                    )
                    client_thread.start()
                    
                except socket.timeout:
                    continue
                except Exception as e:
                    if self.running:
                        self.logger.error(f"TCP server error: {e}")
                        
        except Exception as e:
            self.logger.error(f"Failed to start TCP server: {e}")
    
    def _handle_tcp_client(self, client_socket: socket.socket):
        """Handle TCP client connection"""
        try:
            while self.running:
                # Receive command from client
                data = client_socket.recv(1024)
                if not data:
                    break
                
                try:
                    # Parse JSON command
                    command_data = json.loads(data.decode())
                    response = self._process_tcp_command(command_data)
                    
                    # Send response
                    response_json = json.dumps(response) + '\n'
                    client_socket.send(response_json.encode())
                    
                except json.JSONDecodeError as e:
                    error_response = {"error": f"Invalid JSON: {e}"}
                    client_socket.send(json.dumps(error_response).encode())
                    
        except Exception as e:
            self.logger.error(f"TCP client error: {e}")
        finally:
            client_socket.close()
    
    def _process_tcp_command(self, command_data: Dict) -> Dict:
        """Process TCP command and return response"""
        cmd_type = command_data.get('command', '')
        
        if cmd_type == 'get_status':
            return {
                'status': 'ok',
                'data': {
                    'robot_state': self.robot_state,
                    'statistics': self.stats,
                    'current_command': asdict(self.current_command),
                    'current_status': asdict(self.current_status)
                }
            }
        elif cmd_type == 'set_robot_position':
            pos = command_data.get('position', {})
            self.robot_state['position'].update(pos)
            return {'status': 'ok', 'message': 'Position updated'}
            
        elif cmd_type == 'start_scenario':
            scenario = command_data.get('scenario', 'basic')
            self._start_scenario(scenario)
            return {'status': 'ok', 'message': f'Started scenario: {scenario}'}
            
        elif cmd_type == 'stop_scenario':
            self._stop_scenario()
            return {'status': 'ok', 'message': 'Stopped scenario'}
            
        elif cmd_type == 'export_logs':
            filename = command_data.get('filename', 'wb_simulator_logs.json')
            self._export_logs(filename)
            return {'status': 'ok', 'message': f'Logs exported to {filename}'}
            
        else:
            return {'status': 'error', 'message': f'Unknown command: {cmd_type}'}
    
    # ========================================================================
    # SIMULATION UPDATE AND PHYSICS
    # ========================================================================
    
    def _simulation_update_loop(self):
        """Main simulation update loop"""
        last_time = time.time()
        
        while self.running:
            current_time = time.time()
            dt = current_time - last_time
            last_time = current_time
            
            # Update robot physics
            self._update_robot_physics(dt)
            
            # Update battery simulation
            self._update_battery_simulation(dt)
            
            # Update magnetic sensor simulation
            self._update_magnetic_simulation()
            
            # Update statistics
            self._update_statistics()
            
            time.sleep(0.1)  # 10Hz update rate
    
    def _update_robot_physics(self, dt: float):
        """Update robot position and orientation based on commands"""
        if self.current_command.drive_request == WBDriveRequest.MANUAL_CONTROL:
            # Manual control physics
            speed = self.current_command.manual_speed / 100.0  # Normalize to m/s
            steering = self.current_command.manual_steering / 100.0  # Normalize
            
            # Simple differential drive model
            left_speed = speed - steering * 0.5
            right_speed = speed + steering * 0.5
            
            # Update velocity
            linear_velocity = (left_speed + right_speed) / 2.0
            angular_velocity = (right_speed - left_speed) / 0.5  # 0.5m wheel base
            
            # Update position
            heading = self.robot_state['position']['heading']
            self.robot_state['position']['x'] += linear_velocity * math.cos(heading) * dt
            self.robot_state['position']['y'] += linear_velocity * math.sin(heading) * dt
            self.robot_state['position']['heading'] += angular_velocity * dt
            
            # Normalize heading
            while self.robot_state['position']['heading'] > 2 * math.pi:
                self.robot_state['position']['heading'] -= 2 * math.pi
            while self.robot_state['position']['heading'] < 0:
                self.robot_state['position']['heading'] += 2 * math.pi
                
            # Store velocities
            self.robot_state['velocity']['linear'] = linear_velocity
            self.robot_state['velocity']['angular'] = angular_velocity
            
        elif self.current_command.drive_request == WBDriveRequest.EMERGENCY_STOP:
            # Emergency stop - zero all velocities
            self.robot_state['velocity']['linear'] = 0.0
            self.robot_state['velocity']['angular'] = 0.0
    
    def _update_battery_simulation(self, dt: float):
        """Simulate battery discharge"""
        # Simple battery model - discharge based on motor usage
        total_motor_load = abs(self.robot_state['velocity']['linear']) + abs(self.robot_state['velocity']['angular'])
        discharge_rate = 0.01 * total_motor_load * dt  # 1% per second at full load
        
        self.robot_state['battery_level'] = max(0, self.robot_state['battery_level'] - discharge_rate)
        
        if self.robot_state['battery_level'] < 20:
            self.robot_state['error_flags'] |= 0x01  # Low battery flag
        else:
            self.robot_state['error_flags'] &= ~0x01  # Clear low battery flag
    
    def _update_magnetic_simulation(self):
        """Simulate magnetic sensor based on position"""
        # Simulate magnetic track positions every 2.17cm
        x_pos = self.robot_state['position']['x']
        
        # Calculate nearest magnet position (1-31, center at 16)
        magnet_spacing = 0.0217  # 2.17cm
        center_offset = 16 * magnet_spacing
        
        relative_pos = x_pos - center_offset
        magnet_index = round(relative_pos / magnet_spacing) + 16
        
        # Clamp to valid range
        magnet_index = max(1, min(31, magnet_index))
        
        self.robot_state['magnetic_position'] = magnet_index
    
    def _update_statistics(self):
        """Update simulation statistics"""
        current_time = time.time()
        self.stats['simulation_duration'] = current_time - self.start_time
        
        if self.stats['responses_received'] > 0:
            # Calculate average response time (simplified)
            self.stats['average_response_time'] = random.uniform(5, 15)  # 5-15ms simulated
            
    # ========================================================================
    # COMMAND GENERATION AND SCENARIOS
    # ========================================================================
    
    def _generate_command(self) -> WBButlerCommand:
        """Generate Butler command based on current scenario"""
        command = WBButlerCommand()
        command.command_id = self.command_counter
        command.timestamp = int(time.time() * 1000)
        self.command_counter += 1
        
        if self.scenario_active:
            # Scenario-based command generation
            return self._generate_scenario_command(command)
        else:
            # Default/idle command generation
            command.drive_request = WBDriveRequest.MANUAL_CONTROL
            command.manual_request = 1
            command.manual_speed = 0
            command.manual_steering = 0
            
        return command
    
    def _generate_scenario_command(self, command: WBButlerCommand) -> WBButlerCommand:
        """Generate command for active scenario"""
        # This will be overridden by specific scenarios
        return command
    
    def _start_scenario(self, scenario_name: str):
        """Start a specific test scenario"""
        if self.scenario_active:
            self._stop_scenario()
            
        self.scenario_active = True
        
        if scenario_name == 'basic_navigation':
            self.scenario_thread = threading.Thread(target=self._scenario_basic_navigation, daemon=True)
        elif scenario_name == 'feeding_sequence':
            self.scenario_thread = threading.Thread(target=self._scenario_feeding_sequence, daemon=True)
        elif scenario_name == 'emergency_stop_test':
            self.scenario_thread = threading.Thread(target=self._scenario_emergency_stop, daemon=True)
        elif scenario_name == 'stress_test':
            self.scenario_thread = threading.Thread(target=self._scenario_stress_test, daemon=True)
        else:
            self.logger.warning(f"Unknown scenario: {scenario_name}")
            self.scenario_active = False
            return
            
        self.scenario_thread.start()
        self.logger.info(f"Started scenario: {scenario_name}")
    
    def _stop_scenario(self):
        """Stop active scenario"""
        if self.scenario_active:
            self.scenario_active = False
            if self.scenario_thread and self.scenario_thread.is_alive():
                self.scenario_thread.join(timeout=2.0)
            self.logger.info("Stopped active scenario")
    
    # ========================================================================
    # TEST SCENARIOS
    # ========================================================================
    
    def _scenario_basic_navigation(self):
        """Basic navigation scenario"""
        self.logger.info("Running basic navigation scenario")
        
        # Navigate to different tracks
        tracks = [1, 5, 10, 15, 20]
        
        for track_id in tracks:
            if not self.scenario_active:
                break
                
            # Send navigation command
            command = WBButlerCommand()
            command.command_id = self.command_counter
            command.drive_request = WBDriveRequest.AUTO_NAVIGATION
            command.target_track_id = track_id
            command.target_x = track_id * 5.0  # 5m spacing
            command.target_y = 0.0
            command.timestamp = int(time.time() * 1000)
            self.command_counter += 1
            
            self.current_command = command
            self._log_event("scenario_command", "Scenario", "MELKENS", asdict(command))
            
            # Wait for "navigation" to complete
            time.sleep(5.0)
            
        self.logger.info("Basic navigation scenario completed")
    
    def _scenario_feeding_sequence(self):
        """Feeding sequence scenario"""
        self.logger.info("Running feeding sequence scenario")
        
        bays = [3, 7, 12, 18]
        feed_amounts = [1000, 1500, 2000, 1200]  # kg * 100
        
        for bay_id, amount in zip(bays, feed_amounts):
            if not self.scenario_active:
                break
                
            # Approach bay
            command = WBButlerCommand()
            command.command_id = self.command_counter
            command.drive_request = WBDriveRequest.BAY_APPROACH
            command.target_bay_id = bay_id
            command.feed_amount = amount
            command.timestamp = int(time.time() * 1000)
            self.command_counter += 1
            
            self.current_command = command
            self._log_event("scenario_command", "Scenario", "MELKENS", asdict(command))
            
            # Simulate feeding duration
            feeding_time = amount / 500.0  # 500 units per second
            time.sleep(feeding_time)
            
        self.logger.info("Feeding sequence scenario completed")
    
    def _scenario_emergency_stop(self):
        """Emergency stop scenario"""
        self.logger.info("Running emergency stop scenario")
        
        # Start normal movement
        command = WBButlerCommand()
        command.command_id = self.command_counter
        command.drive_request = WBDriveRequest.MANUAL_CONTROL
        command.manual_speed = 80  # High speed
        command.manual_steering = 0
        command.timestamp = int(time.time() * 1000)
        self.command_counter += 1
        
        self.current_command = command
        time.sleep(2.0)  # Move for 2 seconds
        
        # Emergency stop
        command = WBButlerCommand()
        command.command_id = self.command_counter
        command.drive_request = WBDriveRequest.EMERGENCY_STOP
        command.abort_request = 1
        command.timestamp = int(time.time() * 1000)
        self.command_counter += 1
        
        self.current_command = command
        self._log_event("emergency_stop", "Scenario", "MELKENS", asdict(command))
        
        self.logger.info("Emergency stop scenario completed")
    
    def _scenario_stress_test(self):
        """Stress test scenario with rapid commands"""
        self.logger.info("Running stress test scenario")
        
        start_time = time.time()
        command_count = 0
        
        while self.scenario_active and (time.time() - start_time) < 60:  # 1 minute
            # Rapid command generation
            command = WBButlerCommand()
            command.command_id = self.command_counter
            command.drive_request = random.choice([
                WBDriveRequest.MANUAL_CONTROL,
                WBDriveRequest.AUTO_NAVIGATION,
                WBDriveRequest.BAY_APPROACH
            ])
            
            if command.drive_request == WBDriveRequest.MANUAL_CONTROL:
                command.manual_speed = random.randint(-100, 100)
                command.manual_steering = random.randint(-50, 50)
            elif command.drive_request == WBDriveRequest.AUTO_NAVIGATION:
                command.target_track_id = random.randint(1, 20)
                command.target_x = random.uniform(0, 100)
                command.target_y = random.uniform(-10, 10)
            elif command.drive_request == WBDriveRequest.BAY_APPROACH:
                command.target_bay_id = random.randint(1, 15)
                command.feed_amount = random.randint(500, 3000)
                
            command.timestamp = int(time.time() * 1000)
            self.command_counter += 1
            command_count += 1
            
            self.current_command = command
            
            time.sleep(0.05)  # 20Hz command rate
            
        self.logger.info(f"Stress test completed: {command_count} commands in 60 seconds")
    
    # ========================================================================
    # LOGGING AND EVENT MANAGEMENT
    # ========================================================================
    
    def _log_event(self, event_type: str, source: str, destination: str, 
                  data: Dict, success: bool = True, error_message: str = ""):
        """Log simulation event"""
        event = SimulationEvent(
            timestamp=time.time(),
            event_type=event_type,
            source=source,
            destination=destination,
            data=data,
            success=success,
            error_message=error_message
        )
        
        self.events.append(event)
        
        # Limit event history to prevent memory issues
        if len(self.events) > 10000:
            self.events = self.events[-5000:]  # Keep last 5000 events
    
    def _export_logs(self, filename: str):
        """Export simulation logs to file"""
        if filename.endswith('.json'):
            self._export_logs_json(filename)
        elif filename.endswith('.csv'):
            self._export_logs_csv(filename)
        else:
            self.logger.warning(f"Unknown log format for file: {filename}")
    
    def _export_logs_json(self, filename: str):
        """Export logs to JSON format"""
        log_data = {
            'simulation_info': {
                'start_time': self.start_time,
                'duration': time.time() - self.start_time,
                'config': self.config
            },
            'statistics': self.stats,
            'events': [asdict(event) for event in self.events],
            'final_robot_state': self.robot_state
        }
        
        try:
            with open(filename, 'w') as f:
                json.dump(log_data, f, indent=2)
            self.logger.info(f"Logs exported to {filename}")
        except Exception as e:
            self.logger.error(f"Failed to export logs to {filename}: {e}")
    
    def _export_logs_csv(self, filename: str):
        """Export logs to CSV format"""
        try:
            with open(filename, 'w', newline='') as f:
                writer = csv.writer(f)
                
                # Write header
                writer.writerow([
                    'timestamp', 'event_type', 'source', 'destination',
                    'success', 'error_message', 'data'
                ])
                
                # Write events
                for event in self.events:
                    writer.writerow([
                        event.timestamp,
                        event.event_type,
                        event.source,
                        event.destination,
                        event.success,
                        event.error_message,
                        json.dumps(event.data)
                    ])
                    
            self.logger.info(f"Logs exported to {filename}")
        except Exception as e:
            self.logger.error(f"Failed to export logs to {filename}: {e}")

# ============================================================================
# COMMAND LINE INTERFACE
# ============================================================================

def main():
    """Main function for command line usage"""
    parser = argparse.ArgumentParser(description='WB Communication Simulator')
    parser.add_argument('--config', '-c', default='wb_simulator_config.json',
                       help='Configuration file path')
    parser.add_argument('--can-interface', default='vcan0',
                       help='CAN interface name')
    parser.add_argument('--tcp-port', type=int, default=8080,
                       help='TCP server port')
    parser.add_argument('--update-rate', type=int, default=10,
                       help='Update rate in Hz')
    parser.add_argument('--debug', action='store_true',
                       help='Enable debug logging')
    parser.add_argument('--log-file', default='wb_simulator.log',
                       help='Log file path')
    parser.add_argument('--duration', type=int, default=0,
                       help='Simulation duration in seconds (0 = infinite)')
    parser.add_argument('--scenario', choices=['basic_navigation', 'feeding_sequence', 
                                             'emergency_stop_test', 'stress_test'],
                       help='Test scenario to run')
    
    args = parser.parse_args()
    
    # Load configuration
    config = {
        'can_interface': args.can_interface,
        'tcp_port': args.tcp_port,
        'update_rate_hz': args.update_rate,
        'debug': args.debug,
        'log_file': args.log_file
    }
    
    # Try to load config file
    try:
        with open(args.config, 'r') as f:
            file_config = json.load(f)
            config.update(file_config)
    except FileNotFoundError:
        print(f"Config file {args.config} not found, using defaults")
    except Exception as e:
        print(f"Error loading config file: {e}")
        return 1
    
    # Create and start simulator
    simulator = WBSimulator(config)
    
    try:
        simulator.start()
        
        # Start scenario if specified
        if args.scenario:
            simulator._start_scenario(args.scenario)
        
        print("WB Simulator is running...")
        print(f"TCP server on port {config['tcp_port']}")
        print(f"CAN interface: {config['can_interface']}")
        print("Press Ctrl+C to stop")
        
        # Run for specified duration or until interrupted
        if args.duration > 0:
            time.sleep(args.duration)
        else:
            while True:
                time.sleep(1)
                
    except KeyboardInterrupt:
        print("\nShutdown requested...")
    except Exception as e:
        print(f"Simulator error: {e}")
        return 1
    finally:
        simulator.stop()
        
        # Export final logs
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        simulator._export_logs(f"wb_simulator_logs_{timestamp}.json")
        simulator._export_logs(f"wb_simulator_logs_{timestamp}.csv")
        
        print("WB Simulator shutdown complete")
    
    return 0

if __name__ == '__main__':
    sys.exit(main())

"""
USAGE EXAMPLES:

1. Basic simulation:
   python3 WB_Simulator.py

2. Run with specific scenario:
   python3 WB_Simulator.py --scenario basic_navigation --duration 300

3. Stress test with high update rate:
   python3 WB_Simulator.py --scenario stress_test --update-rate 50 --duration 60

4. Debug mode with custom TCP port:
   python3 WB_Simulator.py --debug --tcp-port 9000

5. TCP Control Interface:
   echo '{"command": "get_status"}' | nc localhost 8080
   echo '{"command": "start_scenario", "scenario": "feeding_sequence"}' | nc localhost 8080
   echo '{"command": "export_logs", "filename": "test_logs.json"}' | nc localhost 8080

CONFIGURATION FILE (wb_simulator_config.json):
{
    "can_interface": "vcan0",
    "tcp_port": 8080,
    "update_rate_hz": 10,
    "debug": false,
    "log_file": "wb_simulator.log",
    "robot_initial_position": {"x": 0.0, "y": 0.0, "heading": 0.0},
    "battery_initial_level": 100,
    "simulation_speed": 1.0
}

INTEGRATION WITH MELKENS:
1. Start WB_Simulator.py
2. Configure MELKENS WB compatibility layer to connect to localhost:8080
3. Run MELKENS application with WB compatibility enabled
4. Monitor communication through TCP interface or log files
5. Export logs for analysis and debugging
"""