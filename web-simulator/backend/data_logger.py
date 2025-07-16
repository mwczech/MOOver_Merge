#!/usr/bin/env python3
"""
MELKENS Data Logger
Logs all IMU data and robot position for HIL testing analysis
"""

import csv
import os
import time
from datetime import datetime
from typing import Dict, List, Optional, Any
from pathlib import Path
import threading
import structlog

logger = structlog.get_logger()

class DataLogger:
    """Data logger for HIL IMU testing"""
    
    def __init__(self, log_file_path: str = "../../logs/hil_imu_test.csv"):
        self.log_file_path = Path(log_file_path)
        self.log_file_path.parent.mkdir(parents=True, exist_ok=True)
        
        self.csv_file = None
        self.csv_writer = None
        self.lock = threading.Lock()
        
        # In-memory log buffer for recent logs
        self.log_buffer = []
        self.max_buffer_size = 1000
        
        # Initialize CSV file
        self._initialize_csv()
        
        logger.info("Data logger initialized", log_file=str(self.log_file_path))
    
    def _initialize_csv(self):
        """Initialize CSV file with headers"""
        try:
            # Check if file exists and has data
            file_exists = self.log_file_path.exists() and self.log_file_path.stat().st_size > 0
            
            self.csv_file = open(self.log_file_path, 'a', newline='')
            self.csv_writer = csv.writer(self.csv_file)
            
            # Write headers if file is new
            if not file_exists:
                headers = [
                    'timestamp',
                    'iso_time',
                    'data_source',  # 'hardware' or 'simulated'
                    'imu_connected',
                    'fault_injection_enabled',
                    'fault_type',
                    # Raw IMU data
                    'acc_x', 'acc_y', 'acc_z',
                    'gyro_x', 'gyro_y', 'gyro_z', 
                    'mag_x', 'mag_y', 'mag_z',
                    'temperature',
                    # Processed data
                    'quat_w', 'quat_x', 'quat_y', 'quat_z',
                    'roll', 'pitch', 'yaw',
                    'pos_x', 'pos_y', 'pos_z',
                    'vel_x', 'vel_y', 'vel_z',
                    # Robot state
                    'robot_route',
                    'robot_pos_x', 'robot_pos_y', 'robot_angle',
                    'motor_left_speed', 'motor_right_speed',
                    # Statistics
                    'packet_count', 'error_count', 'data_rate'
                ]
                self.csv_writer.writerow(headers)
                self.csv_file.flush()
                logger.info("CSV headers written")
        
        except Exception as e:
            logger.error("Failed to initialize CSV file", error=str(e))
            raise
    
    def log_imu_data(self, 
                     imu_data: Dict[str, Any], 
                     imu_status: Dict[str, Any],
                     robot_data: Optional[Dict[str, Any]] = None,
                     fault_config: Optional[Dict[str, Any]] = None):
        """Log IMU data and robot state"""
        
        if not imu_data:
            return
        
        with self.lock:
            try:
                timestamp = time.time()
                iso_time = datetime.now().isoformat()
                
                # Extract IMU data
                acc = imu_data.get('accelerometer', {})
                gyro = imu_data.get('gyroscope', {})
                mag = imu_data.get('magnetometer', {})
                quat = imu_data.get('quaternion', {})
                euler = imu_data.get('euler_angles', {})
                pos = imu_data.get('position', {})
                
                # Extract robot data
                robot_pos_x = robot_data.get('x', 0.0) if robot_data else 0.0
                robot_pos_y = robot_data.get('y', 0.0) if robot_data else 0.0
                robot_angle = robot_data.get('angle', 0.0) if robot_data else 0.0
                robot_route = robot_data.get('current_route', 'None') if robot_data else 'None'
                motor_left = robot_data.get('motor_left_speed', 0) if robot_data else 0
                motor_right = robot_data.get('motor_right_speed', 0) if robot_data else 0
                
                # Extract fault injection data
                fault_enabled = fault_config.get('enabled', False) if fault_config else False
                fault_type = fault_config.get('fault_type', 'none') if fault_config else 'none'
                
                # Prepare row data
                row_data = [
                    timestamp,
                    iso_time,
                    imu_status.get('mode', 'unknown'),
                    imu_status.get('hardware_connected', False),
                    fault_enabled,
                    fault_type,
                    # Raw IMU
                    acc.get('x', 0.0), acc.get('y', 0.0), acc.get('z', 0.0),
                    gyro.get('x', 0.0), gyro.get('y', 0.0), gyro.get('z', 0.0),
                    mag.get('x', 0.0), mag.get('y', 0.0), mag.get('z', 0.0),
                    imu_data.get('temperature', 0.0),
                    # Processed
                    quat.get('w', 0.0), quat.get('x', 0.0), quat.get('y', 0.0), quat.get('z', 0.0),
                    euler.get('roll', 0.0), euler.get('pitch', 0.0), euler.get('yaw', 0.0),
                    pos.get('x', 0.0), pos.get('y', 0.0), pos.get('z', 0.0),
                    0.0, 0.0, 0.0,  # Velocity (not currently calculated in simple version)
                    # Robot
                    robot_route,
                    robot_pos_x, robot_pos_y, robot_angle,
                    motor_left, motor_right,
                    # Statistics
                    imu_status.get('packet_count', 0),
                    imu_status.get('error_count', 0),
                    imu_status.get('data_rate', 0.0)
                ]
                
                # Write to CSV
                if self.csv_writer:
                    self.csv_writer.writerow(row_data)
                    self.csv_file.flush()
                
                # Add to in-memory buffer
                log_entry = {
                    'timestamp': timestamp,
                    'iso_time': iso_time,
                    'imu_data': imu_data,
                    'imu_status': imu_status,
                    'robot_data': robot_data,
                    'fault_config': fault_config
                }
                
                self.log_buffer.append(log_entry)
                
                # Trim buffer if too large
                if len(self.log_buffer) > self.max_buffer_size:
                    self.log_buffer = self.log_buffer[-self.max_buffer_size:]
                
            except Exception as e:
                logger.error("Error logging data", error=str(e))
    
    def log_event(self, event_type: str, description: str, data: Optional[Dict] = None):
        """Log special events (errors, mode changes, etc.)"""
        with self.lock:
            try:
                timestamp = time.time()
                iso_time = datetime.now().isoformat()
                
                # Create a special event row
                event_row = [
                    timestamp,
                    iso_time,
                    f"EVENT_{event_type}",
                    description,
                    str(data) if data else ""
                ] + [''] * 20  # Fill remaining columns
                
                if self.csv_writer:
                    self.csv_writer.writerow(event_row)
                    self.csv_file.flush()
                
                # Add to buffer as well
                self.log_buffer.append({
                    'timestamp': timestamp,
                    'iso_time': iso_time,
                    'type': 'event',
                    'event_type': event_type,
                    'description': description,
                    'data': data
                })
                
            except Exception as e:
                logger.error("Error logging event", error=str(e))
    
    def get_recent_logs(self, limit: int = 100) -> List[Dict]:
        """Get recent log entries from memory buffer"""
        with self.lock:
            return self.log_buffer[-limit:] if self.log_buffer else []
    
    def get_log_statistics(self) -> Dict[str, Any]:
        """Get logging statistics"""
        try:
            file_size = self.log_file_path.stat().st_size if self.log_file_path.exists() else 0
            
            # Count lines in file
            line_count = 0
            if self.log_file_path.exists():
                with open(self.log_file_path, 'r') as f:
                    line_count = sum(1 for _ in f)
            
            return {
                'log_file_path': str(self.log_file_path),
                'file_size_bytes': file_size,
                'file_size_mb': round(file_size / (1024 * 1024), 2),
                'total_records': max(0, line_count - 1),  # Subtract header
                'buffer_size': len(self.log_buffer),
                'created_time': datetime.fromtimestamp(
                    self.log_file_path.stat().st_ctime
                ).isoformat() if self.log_file_path.exists() else None
            }
        except Exception as e:
            logger.error("Error getting log statistics", error=str(e))
            return {'error': str(e)}
    
    def export_analysis_data(self, start_time: Optional[float] = None, 
                           end_time: Optional[float] = None) -> Dict[str, Any]:
        """Export data for analysis (filtered by time range)"""
        try:
            import pandas as pd
            
            # Read CSV file
            df = pd.read_csv(self.log_file_path)
            
            # Filter by time range if specified
            if start_time:
                df = df[df['timestamp'] >= start_time]
            if end_time:
                df = df[df['timestamp'] <= end_time]
            
            # Calculate statistics
            analysis = {
                'time_range': {
                    'start': df['timestamp'].min() if not df.empty else None,
                    'end': df['timestamp'].max() if not df.empty else None,
                    'duration_seconds': df['timestamp'].max() - df['timestamp'].min() if not df.empty else 0
                },
                'data_quality': {
                    'total_samples': len(df),
                    'hardware_samples': len(df[df['data_source'] == 'hardware']),
                    'simulated_samples': len(df[df['data_source'] == 'simulated']),
                    'error_rate': df['error_count'].max() / len(df) if not df.empty else 0
                },
                'sensor_stats': {}
            }
            
            # Calculate sensor statistics
            if not df.empty:
                for sensor in ['acc', 'gyro', 'mag']:
                    for axis in ['x', 'y', 'z']:
                        col = f"{sensor}_{axis}"
                        if col in df.columns:
                            analysis['sensor_stats'][col] = {
                                'mean': float(df[col].mean()),
                                'std': float(df[col].std()),
                                'min': float(df[col].min()),
                                'max': float(df[col].max())
                            }
            
            return analysis
            
        except ImportError:
            return {'error': 'pandas not available for analysis'}
        except Exception as e:
            return {'error': str(e)}
    
    def close(self):
        """Close the logger and cleanup resources"""
        with self.lock:
            if self.csv_file:
                try:
                    self.csv_file.flush()
                    self.csv_file.close()
                    logger.info("Data logger closed")
                except Exception as e:
                    logger.error("Error closing data logger", error=str(e))
                finally:
                    self.csv_file = None
                    self.csv_writer = None