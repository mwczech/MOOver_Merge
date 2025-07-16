#!/usr/bin/env python3
"""
MELKENS Advanced Scenario Runner and Fault Injection Framework
Comprehensive testing framework for IMU fault scenarios and system response validation
"""

import asyncio
import json
import csv
import time
import copy
import random
import math
from pathlib import Path
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Any, Tuple
from dataclasses import dataclass, asdict
from enum import Enum
import logging

import numpy as np

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

class FaultType(Enum):
    """Types of IMU faults that can be injected"""
    STUCK_VALUE = "stuck_value"           # Sensor value frozen
    AXIS_LOSS = "axis_loss"               # Complete axis failure
    BIAS_DRIFT = "bias_drift"             # Gradual bias introduction
    NOISE_INJECTION = "noise_injection"   # Added noise/interference
    PACKET_LOSS = "packet_loss"           # Missing data packets
    SCALE_ERROR = "scale_error"           # Scaling factor errors
    PERIODIC_GLITCH = "periodic_glitch"   # Periodic disturbances
    SATURATION = "saturation"             # Value saturation/clipping

class FaultSeverity(Enum):
    """Severity levels for fault injection"""
    LOW = "low"
    MEDIUM = "medium"
    HIGH = "high"
    CRITICAL = "critical"

@dataclass
class FaultScenario:
    """Definition of a single fault injection scenario"""
    id: str
    name: str
    description: str
    fault_type: FaultType
    severity: FaultSeverity
    target_sensor: str  # "accelerometer", "gyroscope", "magnetometer"
    target_axis: str    # "x", "y", "z", "all"
    start_time: float   # Seconds from scenario start
    duration: float     # Duration in seconds (0 = permanent)
    parameters: Dict[str, Any]  # Fault-specific parameters
    
    def to_dict(self) -> Dict[str, Any]:
        """Convert to dictionary for serialization"""
        data = asdict(self)
        data['fault_type'] = self.fault_type.value
        data['severity'] = self.severity.value
        return data
    
    @classmethod
    def from_dict(cls, data: Dict[str, Any]) -> 'FaultScenario':
        """Create from dictionary"""
        data = data.copy()
        data['fault_type'] = FaultType(data['fault_type'])
        data['severity'] = FaultSeverity(data['severity'])
        return cls(**data)

@dataclass
class TestResult:
    """Results from a single test execution"""
    scenario_id: str
    start_time: datetime
    end_time: datetime
    route_completed: bool
    completion_percentage: float
    fault_detected: bool
    detection_time: Optional[float]
    recovery_achieved: bool
    recovery_time: Optional[float]
    navigation_errors: List[Dict[str, Any]]
    performance_metrics: Dict[str, float]
    raw_data_file: str

class FaultInjector:
    """Core fault injection engine"""
    
    def __init__(self):
        self.active_faults = {}
        self.fault_history = []
        self.start_time = None
    
    def reset(self):
        """Reset fault injector state"""
        self.active_faults = {}
        self.fault_history = []
        self.start_time = time.time()
    
    def activate_fault(self, scenario: FaultScenario):
        """Activate a fault scenario"""
        fault_id = f"{scenario.id}_{time.time()}"
        self.active_faults[fault_id] = {
            'scenario': scenario,
            'start_time': time.time(),
            'active': True,
            'parameters': scenario.parameters.copy()
        }
        self.fault_history.append({
            'fault_id': fault_id,
            'scenario_id': scenario.id,
            'activated_at': time.time(),
            'fault_type': scenario.fault_type.value
        })
        logger.info(f"Activated fault: {scenario.name} ({scenario.fault_type.value})")
        return fault_id
    
    def deactivate_fault(self, fault_id: str):
        """Deactivate a specific fault"""
        if fault_id in self.active_faults:
            self.active_faults[fault_id]['active'] = False
            logger.info(f"Deactivated fault: {fault_id}")
    
    def apply_faults(self, imu_data: Dict[str, Any]) -> Dict[str, Any]:
        """Apply all active faults to IMU data"""
        if not self.active_faults:
            return imu_data
        
        # Create a copy to avoid modifying original data
        modified_data = copy.deepcopy(imu_data)
        current_time = time.time()
        
        # Apply each active fault
        faults_to_remove = []
        for fault_id, fault_info in self.active_faults.items():
            if not fault_info['active']:
                continue
            
            scenario = fault_info['scenario']
            
            # Check if fault should expire
            if scenario.duration > 0:
                elapsed = current_time - fault_info['start_time']
                if elapsed > scenario.duration:
                    fault_info['active'] = False
                    faults_to_remove.append(fault_id)
                    continue
            
            # Apply fault based on type
            modified_data = self._apply_single_fault(modified_data, scenario, fault_info)
        
        # Clean up expired faults
        for fault_id in faults_to_remove:
            logger.info(f"Fault expired: {fault_id}")
        
        return modified_data
    
    def _apply_single_fault(self, data: Dict[str, Any], scenario: FaultScenario, fault_info: Dict) -> Dict[str, Any]:
        """Apply a single fault to the data"""
        sensor = scenario.target_sensor
        axis = scenario.target_axis
        fault_type = scenario.fault_type
        params = fault_info['parameters']
        
        if sensor not in data:
            return data
        
        # Determine which axes to affect
        axes = [axis] if axis != "all" else ["x", "y", "z"]
        
        for target_axis in axes:
            if target_axis not in data[sensor]:
                continue
            
            original_value = data[sensor][target_axis]
            
            if fault_type == FaultType.STUCK_VALUE:
                # Freeze value at first occurrence or specified value
                if 'stuck_value' not in params:
                    params['stuck_value'] = original_value
                data[sensor][target_axis] = params['stuck_value']
            
            elif fault_type == FaultType.AXIS_LOSS:
                # Set to zero or NaN
                data[sensor][target_axis] = 0.0
            
            elif fault_type == FaultType.BIAS_DRIFT:
                # Add gradually increasing bias
                elapsed = time.time() - fault_info['start_time']
                bias_rate = params.get('bias_rate', 0.1)  # units per second
                current_bias = bias_rate * elapsed
                data[sensor][target_axis] = original_value + current_bias
            
            elif fault_type == FaultType.NOISE_INJECTION:
                # Add random noise
                noise_level = params.get('noise_level', 0.1)
                noise = random.gauss(0, noise_level)
                data[sensor][target_axis] = original_value + noise
            
            elif fault_type == FaultType.SCALE_ERROR:
                # Apply scaling factor
                scale_factor = params.get('scale_factor', 1.5)
                data[sensor][target_axis] = original_value * scale_factor
            
            elif fault_type == FaultType.PERIODIC_GLITCH:
                # Add periodic disturbances
                frequency = params.get('frequency', 1.0)  # Hz
                amplitude = params.get('amplitude', 1.0)
                elapsed = time.time() - fault_info['start_time']
                glitch = amplitude * math.sin(2 * math.pi * frequency * elapsed)
                data[sensor][target_axis] = original_value + glitch
            
            elif fault_type == FaultType.SATURATION:
                # Clip values to limits
                min_val = params.get('min_value', -10.0)
                max_val = params.get('max_value', 10.0)
                data[sensor][target_axis] = max(min_val, min(max_val, original_value))
        
        return data
    
    def should_drop_packet(self) -> bool:
        """Check if current packet should be dropped due to packet loss fault"""
        for fault_info in self.active_faults.values():
            if not fault_info['active']:
                continue
            
            scenario = fault_info['scenario']
            if scenario.fault_type == FaultType.PACKET_LOSS:
                loss_rate = scenario.parameters.get('loss_rate', 0.1)
                return random.random() < loss_rate
        
        return False

class ScenarioManager:
    """Manages test scenarios and execution"""
    
    def __init__(self):
        self.scenarios = {}
        self.test_results = []
        self.current_test = None
        self.fault_injector = FaultInjector()
    
    def load_scenarios_from_json(self, file_path: str) -> int:
        """Load scenarios from JSON file"""
        try:
            with open(file_path, 'r') as f:
                data = json.load(f)
            
            count = 0
            scenarios_data = data if isinstance(data, list) else data.get('scenarios', [])
            
            for scenario_data in scenarios_data:
                scenario = FaultScenario.from_dict(scenario_data)
                self.scenarios[scenario.id] = scenario
                count += 1
            
            logger.info(f"Loaded {count} scenarios from {file_path}")
            return count
        
        except Exception as e:
            logger.error(f"Failed to load scenarios from {file_path}: {e}")
            return 0
    
    def load_scenarios_from_csv(self, file_path: str) -> int:
        """Load scenarios from CSV file"""
        try:
            count = 0
            with open(file_path, 'r') as f:
                reader = csv.DictReader(f)
                
                for row in reader:
                    # Parse CSV row into scenario
                    scenario_data = {
                        'id': row['id'],
                        'name': row['name'],
                        'description': row.get('description', ''),
                        'fault_type': row['fault_type'],
                        'severity': row['severity'],
                        'target_sensor': row['target_sensor'],
                        'target_axis': row['target_axis'],
                        'start_time': float(row['start_time']),
                        'duration': float(row['duration']),
                        'parameters': json.loads(row.get('parameters', '{}'))
                    }
                    
                    scenario = FaultScenario.from_dict(scenario_data)
                    self.scenarios[scenario.id] = scenario
                    count += 1
            
            logger.info(f"Loaded {count} scenarios from {file_path}")
            return count
        
        except Exception as e:
            logger.error(f"Failed to load scenarios from {file_path}: {e}")
            return 0
    
    def add_scenario(self, scenario: FaultScenario):
        """Add a single scenario"""
        self.scenarios[scenario.id] = scenario
        logger.info(f"Added scenario: {scenario.name}")
    
    def get_scenario(self, scenario_id: str) -> Optional[FaultScenario]:
        """Get scenario by ID"""
        return self.scenarios.get(scenario_id)
    
    def list_scenarios(self) -> List[Dict[str, Any]]:
        """List all available scenarios"""
        return [scenario.to_dict() for scenario in self.scenarios.values()]
    
    def create_default_scenarios(self):
        """Create a set of default test scenarios"""
        default_scenarios = [
            FaultScenario(
                id="acc_stuck_x",
                name="Accelerometer X-Axis Stuck",
                description="X-axis accelerometer freezes at current value",
                fault_type=FaultType.STUCK_VALUE,
                severity=FaultSeverity.MEDIUM,
                target_sensor="accelerometer",
                target_axis="x",
                start_time=10.0,
                duration=30.0,
                parameters={}
            ),
            FaultScenario(
                id="gyro_noise_high",
                name="High Gyroscope Noise",
                description="Add high-level noise to all gyroscope axes",
                fault_type=FaultType.NOISE_INJECTION,
                severity=FaultSeverity.HIGH,
                target_sensor="gyroscope",
                target_axis="all",
                start_time=15.0,
                duration=20.0,
                parameters={"noise_level": 0.5}
            ),
            FaultScenario(
                id="mag_bias_drift",
                name="Magnetometer Bias Drift",
                description="Gradual bias drift in magnetometer Z-axis",
                fault_type=FaultType.BIAS_DRIFT,
                severity=FaultSeverity.MEDIUM,
                target_sensor="magnetometer",
                target_axis="z",
                start_time=20.0,
                duration=60.0,
                parameters={"bias_rate": 0.05}
            ),
            FaultScenario(
                id="packet_loss_critical",
                name="Critical Packet Loss",
                description="20% packet loss for IMU data",
                fault_type=FaultType.PACKET_LOSS,
                severity=FaultSeverity.CRITICAL,
                target_sensor="accelerometer",
                target_axis="all",
                start_time=5.0,
                duration=45.0,
                parameters={"loss_rate": 0.2}
            ),
            FaultScenario(
                id="acc_saturation",
                name="Accelerometer Saturation",
                description="Accelerometer values saturate at ±8g",
                fault_type=FaultType.SATURATION,
                severity=FaultSeverity.HIGH,
                target_sensor="accelerometer",
                target_axis="all",
                start_time=25.0,
                duration=15.0,
                parameters={"min_value": -8.0, "max_value": 8.0}
            )
        ]
        
        for scenario in default_scenarios:
            self.scenarios[scenario.id] = scenario
        
        logger.info(f"Created {len(default_scenarios)} default scenarios")

class SystemMonitor:
    """Monitors system response during fault injection tests"""
    
    def __init__(self):
        self.reset()
    
    def reset(self):
        """Reset monitoring state"""
        self.start_time = time.time()
        self.navigation_errors = []
        self.fault_detections = []
        self.performance_data = []
        self.route_progress = 0.0
        self.last_position = None
        self.total_distance = 0.0
        self.max_position_error = 0.0
        self.navigation_stability = []
    
    def update_robot_state(self, robot_data: Dict[str, Any]):
        """Update with current robot state"""
        current_time = time.time() - self.start_time
        
        # Track route progress
        if 'step_progress' in robot_data:
            self.route_progress = max(self.route_progress, robot_data['step_progress'])
        
        # Track position and navigation
        if 'x' in robot_data and 'y' in robot_data:
            current_pos = (robot_data['x'], robot_data['y'])
            
            if self.last_position is not None:
                # Calculate distance traveled
                dx = current_pos[0] - self.last_position[0]
                dy = current_pos[1] - self.last_position[1]
                distance = math.sqrt(dx*dx + dy*dy)
                self.total_distance += distance
                
                # Track navigation stability (velocity consistency)
                velocity = distance * 50  # Assuming 50Hz update rate
                self.navigation_stability.append(velocity)
            
            self.last_position = current_pos
        
        # Check for navigation errors
        if 'error' in robot_data or robot_data.get('status') == 'error':
            self.navigation_errors.append({
                'time': current_time,
                'error': robot_data.get('error', 'Unknown navigation error'),
                'position': self.last_position
            })
    
    def update_imu_state(self, imu_data: Dict[str, Any], fault_injector: FaultInjector):
        """Update with current IMU state and check for anomalies"""
        current_time = time.time() - self.start_time
        
        # Simple fault detection based on data characteristics
        detected_faults = self._detect_anomalies(imu_data)
        
        for fault_type in detected_faults:
            self.fault_detections.append({
                'time': current_time,
                'fault_type': fault_type,
                'data_snapshot': copy.deepcopy(imu_data)
            })
        
        # Record performance metrics
        self.performance_data.append({
            'time': current_time,
            'imu_data': copy.deepcopy(imu_data),
            'active_faults': len(fault_injector.active_faults)
        })
    
    def _detect_anomalies(self, imu_data: Dict[str, Any]) -> List[str]:
        """Simple anomaly detection for fault awareness"""
        detected = []
        
        # Check for stuck values (consecutive identical readings)
        # This would need historical data in a real implementation
        
        # Check for unrealistic values
        for sensor in ['accelerometer', 'gyroscope', 'magnetometer']:
            if sensor not in imu_data:
                continue
            
            for axis in ['x', 'y', 'z']:
                if axis not in imu_data[sensor]:
                    continue
                
                value = imu_data[sensor][axis]
                
                # Define reasonable ranges for each sensor
                ranges = {
                    'accelerometer': (-20.0, 20.0),  # ±20 m/s²
                    'gyroscope': (-10.0, 10.0),      # ±10 rad/s
                    'magnetometer': (-100.0, 100.0)  # ±100 µT (simplified)
                }
                
                min_val, max_val = ranges.get(sensor, (-1000, 1000))
                if not (min_val <= value <= max_val):
                    detected.append(f"{sensor}_{axis}_out_of_range")
        
        return detected
    
    def get_performance_metrics(self) -> Dict[str, float]:
        """Calculate performance metrics for the test"""
        if not self.performance_data:
            return {}
        
        metrics = {
            'route_completion_percentage': self.route_progress * 100,
            'total_distance_traveled': self.total_distance,
            'navigation_error_count': len(self.navigation_errors),
            'fault_detection_count': len(self.fault_detections),
            'test_duration': time.time() - self.start_time
        }
        
        # Calculate navigation stability
        if self.navigation_stability:
            velocities = np.array(self.navigation_stability)
            metrics['avg_velocity'] = float(np.mean(velocities))
            metrics['velocity_std'] = float(np.std(velocities))
            metrics['navigation_stability_score'] = float(1.0 / (1.0 + np.std(velocities)))
        
        return metrics

class AdvancedScenarioRunner:
    """Main class for running advanced fault injection scenarios"""
    
    def __init__(self, data_logger=None):
        self.scenario_manager = ScenarioManager()
        self.system_monitor = SystemMonitor()
        self.data_logger = data_logger
        self.is_running = False
        self.current_test_id = None
        
        # Create default scenarios
        self.scenario_manager.create_default_scenarios()
    
    async def run_scenario(self, scenario_id: str, route_name: str = "A") -> TestResult:
        """Run a single fault injection scenario"""
        scenario = self.scenario_manager.get_scenario(scenario_id)
        if not scenario:
            raise ValueError(f"Scenario not found: {scenario_id}")
        
        logger.info(f"Starting scenario: {scenario.name}")
        
        # Initialize test
        self.current_test_id = f"test_{scenario_id}_{int(time.time())}"
        self.is_running = True
        self.system_monitor.reset()
        self.scenario_manager.fault_injector.reset()
        
        start_time = datetime.now()
        
        try:
            # Schedule fault activation
            asyncio.create_task(self._schedule_fault_activation(scenario))
            
            # Monitor test execution (this would integrate with your robot simulator)
            await self._monitor_test_execution(scenario, route_name)
            
            end_time = datetime.now()
            
            # Generate test result
            result = TestResult(
                scenario_id=scenario_id,
                start_time=start_time,
                end_time=end_time,
                route_completed=self.system_monitor.route_progress >= 0.95,
                completion_percentage=self.system_monitor.route_progress * 100,
                fault_detected=len(self.system_monitor.fault_detections) > 0,
                detection_time=self.system_monitor.fault_detections[0]['time'] if self.system_monitor.fault_detections else None,
                recovery_achieved=self._assess_recovery(),
                recovery_time=self._calculate_recovery_time(),
                navigation_errors=self.system_monitor.navigation_errors,
                performance_metrics=self.system_monitor.get_performance_metrics(),
                raw_data_file=f"logs/scenario_{self.current_test_id}.json"
            )
            
            # Save detailed logs
            await self._save_test_logs(result)
            
            self.scenario_manager.test_results.append(result)
            logger.info(f"Scenario completed: {scenario.name} - Route completed: {result.route_completed}")
            
            return result
        
        finally:
            self.is_running = False
            self.current_test_id = None
    
    async def _schedule_fault_activation(self, scenario: FaultScenario):
        """Schedule fault activation at specified time"""
        await asyncio.sleep(scenario.start_time)
        
        if self.is_running:
            fault_id = self.scenario_manager.fault_injector.activate_fault(scenario)
            
            # Schedule deactivation if duration is specified
            if scenario.duration > 0:
                await asyncio.sleep(scenario.duration)
                if self.is_running:
                    self.scenario_manager.fault_injector.deactivate_fault(fault_id)
    
    async def _monitor_test_execution(self, scenario: FaultScenario, route_name: str):
        """Monitor test execution and collect data"""
        # This is a simulation of test monitoring
        # In real implementation, this would integrate with your robot simulator
        
        test_duration = max(60.0, scenario.start_time + scenario.duration + 10.0)
        update_interval = 0.02  # 50Hz
        
        for i in range(int(test_duration / update_interval)):
            if not self.is_running:
                break
            
            # Simulate robot state updates
            current_time = i * update_interval
            progress = min(1.0, current_time / test_duration)
            
            # Mock robot data
            robot_data = {
                'x': progress * 10.0 + random.gauss(0, 0.1),
                'y': math.sin(progress * 2 * math.pi) * 2.0 + random.gauss(0, 0.1),
                'step_progress': progress,
                'status': 'running'
            }
            
            # Mock IMU data
            imu_data = {
                'accelerometer': {
                    'x': random.gauss(0, 1.0),
                    'y': random.gauss(0, 1.0),
                    'z': random.gauss(9.8, 0.5)
                },
                'gyroscope': {
                    'x': random.gauss(0, 0.1),
                    'y': random.gauss(0, 0.1),
                    'z': random.gauss(0, 0.1)
                },
                'magnetometer': {
                    'x': random.gauss(20.0, 5.0),
                    'y': random.gauss(-10.0, 5.0),
                    'z': random.gauss(45.0, 5.0)
                }
            }
            
            # Apply fault injection
            if not self.scenario_manager.fault_injector.should_drop_packet():
                modified_imu_data = self.scenario_manager.fault_injector.apply_faults(imu_data)
                self.system_monitor.update_imu_state(modified_imu_data, self.scenario_manager.fault_injector)
            
            self.system_monitor.update_robot_state(robot_data)
            
            await asyncio.sleep(update_interval)
    
    def _assess_recovery(self) -> bool:
        """Assess if the system recovered from faults"""
        # Simple recovery assessment based on recent navigation stability
        if len(self.system_monitor.navigation_stability) < 10:
            return False
        
        # Check if navigation stabilized after fault detection
        recent_stability = self.system_monitor.navigation_stability[-10:]
        stability_score = 1.0 / (1.0 + np.std(recent_stability))
        
        return stability_score > 0.7  # Threshold for "recovered"
    
    def _calculate_recovery_time(self) -> Optional[float]:
        """Calculate time to recovery after fault detection"""
        if not self.system_monitor.fault_detections:
            return None
        
        first_detection = self.system_monitor.fault_detections[0]['time']
        
        # Find when navigation stabilized (simplified)
        stability_threshold = 0.7
        for i, data_point in enumerate(self.system_monitor.performance_data):
            if data_point['time'] > first_detection:
                # Check stability in a window around this point
                window_start = max(0, i - 5)
                window_end = min(len(self.system_monitor.performance_data), i + 5)
                window_velocities = []
                
                for j in range(window_start, window_end):
                    if j < len(self.system_monitor.navigation_stability):
                        window_velocities.append(self.system_monitor.navigation_stability[j])
                
                if window_velocities:
                    stability = 1.0 / (1.0 + np.std(window_velocities))
                    if stability > stability_threshold:
                        return data_point['time'] - first_detection
        
        return None
    
    async def _save_test_logs(self, result: TestResult):
        """Save detailed test logs"""
        logs_dir = Path("logs/scenarios")
        logs_dir.mkdir(parents=True, exist_ok=True)
        
        log_data = {
            'test_result': asdict(result),
            'performance_data': self.system_monitor.performance_data,
            'fault_history': self.scenario_manager.fault_injector.fault_history,
            'navigation_errors': self.system_monitor.navigation_errors,
            'fault_detections': self.system_monitor.fault_detections
        }
        
        log_file = logs_dir / f"scenario_{self.current_test_id}.json"
        with open(log_file, 'w') as f:
            json.dump(log_data, f, indent=2, default=str)
        
        logger.info(f"Test logs saved to: {log_file}")
    
    def generate_scenario_report(self, scenario_ids: List[str] = None) -> Dict[str, Any]:
        """Generate comprehensive report for specified scenarios"""
        results = self.scenario_manager.test_results
        
        if scenario_ids:
            results = [r for r in results if r.scenario_id in scenario_ids]
        
        if not results:
            return {"error": "No test results found"}
        
        report = {
            'generated_at': datetime.now().isoformat(),
            'total_tests': len(results),
            'summary': {
                'tests_passed': sum(1 for r in results if r.route_completed),
                'tests_failed': sum(1 for r in results if not r.route_completed),
                'faults_detected': sum(1 for r in results if r.fault_detected),
                'recoveries_achieved': sum(1 for r in results if r.recovery_achieved),
                'avg_completion_rate': np.mean([r.completion_percentage for r in results]),
                'avg_detection_time': np.mean([r.detection_time for r in results if r.detection_time is not None])
            },
            'detailed_results': [asdict(r) for r in results],
            'recommendations': self._generate_recommendations(results)
        }
        
        return report
    
    def _generate_recommendations(self, results: List[TestResult]) -> List[str]:
        """Generate recommendations based on test results"""
        recommendations = []
        
        # Analyze completion rates
        completion_rates = [r.completion_percentage for r in results]
        avg_completion = np.mean(completion_rates)
        
        if avg_completion < 80:
            recommendations.append("System shows poor fault tolerance - consider implementing redundant sensors")
        
        # Analyze fault detection
        detection_rate = sum(1 for r in results if r.fault_detected) / len(results)
        if detection_rate < 0.5:
            recommendations.append("Fault detection capabilities need improvement - implement better anomaly detection")
        
        # Analyze recovery
        recovery_rate = sum(1 for r in results if r.recovery_achieved) / len(results)
        if recovery_rate < 0.7:
            recommendations.append("System recovery mechanisms need enhancement")
        
        return recommendations
    
    def export_results_csv(self, output_file: str):
        """Export test results to CSV"""
        if not self.scenario_manager.test_results:
            logger.warning("No test results to export")
            return
        
        fieldnames = [
            'scenario_id', 'start_time', 'end_time', 'route_completed',
            'completion_percentage', 'fault_detected', 'detection_time',
            'recovery_achieved', 'recovery_time', 'navigation_error_count'
        ]
        
        with open(output_file, 'w', newline='') as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writeheader()
            
            for result in self.scenario_manager.test_results:
                row = {
                    'scenario_id': result.scenario_id,
                    'start_time': result.start_time.isoformat(),
                    'end_time': result.end_time.isoformat(),
                    'route_completed': result.route_completed,
                    'completion_percentage': result.completion_percentage,
                    'fault_detected': result.fault_detected,
                    'detection_time': result.detection_time,
                    'recovery_achieved': result.recovery_achieved,
                    'recovery_time': result.recovery_time,
                    'navigation_error_count': len(result.navigation_errors)
                }
                writer.writerow(row)
        
        logger.info(f"Results exported to: {output_file}")

# Example usage and testing
async def main():
    """Example usage of the advanced scenario runner"""
    runner = AdvancedScenarioRunner()
    
    # Run a test scenario
    try:
        result = await runner.run_scenario("acc_stuck_x", "A")
        print(f"Test completed - Route completed: {result.route_completed}")
        
        # Generate report
        report = runner.generate_scenario_report()
        print(f"Report generated with {report['total_tests']} tests")
        
        # Export results
        runner.export_results_csv("test_results.csv")
        
    except Exception as e:
        logger.error(f"Test failed: {e}")

if __name__ == "__main__":
    asyncio.run(main())