#!/usr/bin/env python3
"""
LogGenerator.py

Log and Route Generator for WB-MELKENS Integration

This tool generates comprehensive logs and route data for analysis
and visualization of WB-MELKENS integration testing. It provides:
- Route generation with track and bay configurations
- Performance logging and analysis
- CSV/JSON export for visualization tools
- Statistical analysis and reporting
- Integration test data generation

Author: MOOver Integration Team
Created: 2024-12-19
Phase: 3 - Testing and Emulation
"""

import sys
import time
import json
import csv
import math
import random
import argparse
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Tuple, Any
from dataclasses import dataclass, asdict, field
from pathlib import Path
import numpy as np

# ============================================================================
# DATA STRUCTURES
# ============================================================================

@dataclass
class TrackPoint:
    """Single point on a track route"""
    x: float
    y: float
    heading: float
    timestamp: float
    speed: float = 0.0
    magnetic_position: int = 16  # Center position
    track_id: int = 0
    bay_id: int = 0

@dataclass
class RouteSegment:
    """Route segment between two points"""
    start_point: TrackPoint
    end_point: TrackPoint
    segment_type: str  # "straight", "curve", "approach", "feeding"
    duration: float = 0.0
    distance: float = 0.0
    max_speed: float = 1.0

@dataclass
class PerformanceMetric:
    """Performance measurement data"""
    timestamp: float
    robot_position: Tuple[float, float, float]  # x, y, heading
    target_position: Tuple[float, float, float]
    speed_actual: float
    speed_target: float
    battery_level: float
    error_magnitude: float
    algorithm_type: str  # "MELKENS" or "WB"
    command_latency: float = 0.0
    cpu_usage: float = 0.0
    memory_usage: float = 0.0

@dataclass
class LogEntry:
    """Generic log entry"""
    timestamp: float
    level: str  # DEBUG, INFO, WARN, ERROR
    component: str  # System component name
    message: str
    data: Dict[str, Any] = field(default_factory=dict)
    error_code: int = 0

@dataclass
class TestScenario:
    """Test scenario configuration"""
    name: str
    description: str
    duration: float
    tracks: List[int]
    bays: List[int]
    algorithm: str  # "MELKENS", "WB", or "BOTH"
    stress_level: str  # "LOW", "MEDIUM", "HIGH"
    error_injection: bool = False

# ============================================================================
# LOG GENERATOR CORE CLASS
# ============================================================================

class LogGenerator:
    """Main log and route generator"""
    
    def __init__(self, config: Dict):
        """Initialize log generator"""
        self.config = config
        self.start_time = time.time()
        
        # Configuration
        self.farm_layout = config.get('farm_layout', {})
        self.track_spacing = config.get('track_spacing', 5.0)  # meters
        self.bay_depth = config.get('bay_depth', 10.0)  # meters
        self.robot_speed_max = config.get('robot_speed_max', 2.0)  # m/s
        
        # Data storage
        self.routes: List[RouteSegment] = []
        self.performance_data: List[PerformanceMetric] = []
        self.log_entries: List[LogEntry] = []
        self.track_points: List[TrackPoint] = []
        
        # Statistics
        self.stats = {
            'total_distance': 0.0,
            'total_time': 0.0,
            'average_speed': 0.0,
            'max_speed': 0.0,
            'min_speed': float('inf'),
            'battery_consumed': 0.0,
            'total_errors': 0,
            'melkens_performance': {},
            'wb_performance': {}
        }
        
        # Scenario data
        self.current_scenario: Optional[TestScenario] = None
        self.scenario_start_time = 0.0
        
    def generate_farm_layout(self, num_tracks: int = 20, num_bays: int = 15) -> Dict:
        """Generate farm layout with tracks and bays"""
        layout = {
            'tracks': {},
            'bays': {},
            'dimensions': {
                'width': num_tracks * self.track_spacing,
                'height': self.bay_depth * 2,
                'origin': {'x': 0.0, 'y': 0.0}
            }
        }
        
        # Generate tracks
        for track_id in range(1, num_tracks + 1):
            x_position = track_id * self.track_spacing
            layout['tracks'][track_id] = {
                'id': track_id,
                'name': f"Track_{track_id:02d}",
                'position': {'x': x_position, 'y': 0.0},
                'direction': 0,  # degrees
                'length': self.bay_depth * 2,
                'active': True,
                'speed_limit': self.robot_speed_max * 0.8
            }
        
        # Generate bays (alternating sides of tracks)
        bay_id = 1
        for track_id in range(1, num_tracks + 1):
            if bay_id > num_bays:
                break
                
            track_x = track_id * self.track_spacing
            
            # Bay on positive Y side
            if bay_id <= num_bays:
                layout['bays'][bay_id] = {
                    'id': bay_id,
                    'name': f"Bay_{bay_id:02d}",
                    'track_id': track_id,
                    'side': 'positive',
                    'entry_near': {'x': track_x - 0.5, 'y': 1.0},
                    'entry_far': {'x': track_x - 0.5, 'y': 0.5},
                    'exit_near': {'x': track_x + 0.5, 'y': 1.0},
                    'exit_far': {'x': track_x + 0.5, 'y': 0.5},
                    'feed_position': {'x': track_x, 'y': self.bay_depth / 2},
                    'capacity': random.randint(50, 200),  # animals
                    'active': True
                }
                bay_id += 1
            
            # Bay on negative Y side
            if bay_id <= num_bays:
                layout['bays'][bay_id] = {
                    'id': bay_id,
                    'name': f"Bay_{bay_id:02d}",
                    'track_id': track_id,
                    'side': 'negative',
                    'entry_near': {'x': track_x - 0.5, 'y': -1.0},
                    'entry_far': {'x': track_x - 0.5, 'y': -0.5},
                    'exit_near': {'x': track_x + 0.5, 'y': -1.0},
                    'exit_far': {'x': track_x + 0.5, 'y': -0.5},
                    'feed_position': {'x': track_x, 'y': -self.bay_depth / 2},
                    'capacity': random.randint(50, 200),  # animals
                    'active': True
                }
                bay_id += 1
        
        self.farm_layout = layout
        return layout
    
    def generate_route(self, start_track: int, end_track: int, bays: List[int] = None) -> List[RouteSegment]:
        """Generate route from start track to end track with optional bay visits"""
        if not self.farm_layout:
            self.generate_farm_layout()
            
        route_segments = []
        current_position = self.farm_layout['tracks'][start_track]['position']
        current_time = time.time()
        
        # Add bay visits if specified
        if bays:
            for bay_id in bays:
                if bay_id in self.farm_layout['bays']:
                    bay = self.farm_layout['bays'][bay_id]
                    
                    # Navigate to bay track
                    bay_track = bay['track_id']
                    if bay_track != start_track:
                        segment = self._create_track_to_track_segment(
                            current_position, 
                            self.farm_layout['tracks'][bay_track]['position'],
                            current_time
                        )
                        route_segments.append(segment)
                        current_position = segment.end_point
                        current_time += segment.duration
                    
                    # Approach bay
                    approach_segment = self._create_bay_approach_segment(
                        current_position, bay, current_time
                    )
                    route_segments.append(approach_segment)
                    current_position = approach_segment.end_point
                    current_time += approach_segment.duration
                    
                    # Feeding operation
                    feeding_segment = self._create_feeding_segment(
                        current_position, bay, current_time
                    )
                    route_segments.append(feeding_segment)
                    current_position = feeding_segment.end_point
                    current_time += feeding_segment.duration
        
        # Navigate to final track if different
        if end_track != start_track:
            final_segment = self._create_track_to_track_segment(
                current_position,
                self.farm_layout['tracks'][end_track]['position'],
                current_time
            )
            route_segments.append(final_segment)
        
        self.routes.extend(route_segments)
        return route_segments
    
    def _create_track_to_track_segment(self, start_pos: Dict, end_pos: Dict, start_time: float) -> RouteSegment:
        """Create route segment between two tracks"""
        distance = math.sqrt((end_pos['x'] - start_pos['x'])**2 + (end_pos['y'] - start_pos['y'])**2)
        heading = math.atan2(end_pos['y'] - start_pos['y'], end_pos['x'] - start_pos['x'])
        speed = self.robot_speed_max * 0.7  # 70% of max speed for track navigation
        duration = distance / speed
        
        start_point = TrackPoint(
            x=start_pos['x'], y=start_pos['y'], heading=heading,
            timestamp=start_time, speed=speed
        )
        
        end_point = TrackPoint(
            x=end_pos['x'], y=end_pos['y'], heading=heading,
            timestamp=start_time + duration, speed=speed
        )
        
        return RouteSegment(
            start_point=start_point,
            end_point=end_point,
            segment_type="straight",
            duration=duration,
            distance=distance,
            max_speed=speed
        )
    
    def _create_bay_approach_segment(self, start_pos: TrackPoint, bay: Dict, start_time: float) -> RouteSegment:
        """Create bay approach segment"""
        entry_pos = bay['entry_near']
        distance = math.sqrt((entry_pos['x'] - start_pos.x)**2 + (entry_pos['y'] - start_pos.y)**2)
        heading = math.atan2(entry_pos['y'] - start_pos.y, entry_pos['x'] - start_pos.x)
        speed = self.robot_speed_max * 0.3  # Slow approach speed
        duration = distance / speed
        
        start_point = TrackPoint(
            x=start_pos.x, y=start_pos.y, heading=heading,
            timestamp=start_time, speed=speed, bay_id=bay['id']
        )
        
        end_point = TrackPoint(
            x=entry_pos['x'], y=entry_pos['y'], heading=heading,
            timestamp=start_time + duration, speed=0.0, bay_id=bay['id']
        )
        
        return RouteSegment(
            start_point=start_point,
            end_point=end_point,
            segment_type="approach",
            duration=duration,
            distance=distance,
            max_speed=speed
        )
    
    def _create_feeding_segment(self, start_pos: TrackPoint, bay: Dict, start_time: float) -> RouteSegment:
        """Create feeding operation segment"""
        feed_pos = bay['feed_position']
        feed_duration = bay['capacity'] * 0.1  # 0.1 seconds per animal
        
        start_point = TrackPoint(
            x=start_pos.x, y=start_pos.y, heading=start_pos.heading,
            timestamp=start_time, speed=0.0, bay_id=bay['id']
        )
        
        end_point = TrackPoint(
            x=feed_pos['x'], y=feed_pos['y'], heading=start_pos.heading,
            timestamp=start_time + feed_duration, speed=0.0, bay_id=bay['id']
        )
        
        return RouteSegment(
            start_point=start_point,
            end_point=end_point,
            segment_type="feeding",
            duration=feed_duration,
            distance=0.0,  # Stationary feeding
            max_speed=0.0
        )
    
    # ========================================================================
    # PERFORMANCE DATA GENERATION
    # ========================================================================
    
    def generate_performance_comparison(self, scenario: TestScenario) -> List[PerformanceMetric]:
        """Generate performance comparison between MELKENS and WB algorithms"""
        self.current_scenario = scenario
        self.scenario_start_time = time.time()
        
        performance_data = []
        
        # Generate route for scenario
        route_segments = []
        for i, track_id in enumerate(scenario.tracks):
            if i < len(scenario.tracks) - 1:
                next_track = scenario.tracks[i + 1]
                bays_for_segment = [bay for bay in scenario.bays if bay % 2 == i % 2]  # Distribute bays
                segments = self.generate_route(track_id, next_track, bays_for_segment[:2])
                route_segments.extend(segments)
        
        # Simulate both algorithms if required
        algorithms = ["MELKENS", "WB"] if scenario.algorithm == "BOTH" else [scenario.algorithm]
        
        for algorithm in algorithms:
            alg_performance = self._simulate_algorithm_performance(route_segments, algorithm, scenario)
            performance_data.extend(alg_performance)
        
        self.performance_data.extend(performance_data)
        return performance_data
    
    def _simulate_algorithm_performance(self, route_segments: List[RouteSegment], 
                                      algorithm: str, scenario: TestScenario) -> List[PerformanceMetric]:
        """Simulate performance for specific algorithm"""
        performance_data = []
        current_time = self.scenario_start_time
        
        # Algorithm-specific parameters
        if algorithm == "MELKENS":
            update_rate = 20  # Hz
            position_accuracy = 0.05  # meters
            response_time_base = 0.02  # 20ms base latency
        else:  # WB
            update_rate = 10  # Hz
            position_accuracy = 0.02  # meters  
            response_time_base = 0.01  # 10ms base latency
        
        update_interval = 1.0 / update_rate
        
        for segment in route_segments:
            # Simulate segment execution
            segment_time = 0.0
            while segment_time < segment.duration:
                # Calculate current position along segment
                progress = segment_time / segment.duration if segment.duration > 0 else 1.0
                current_x = segment.start_point.x + (segment.end_point.x - segment.start_point.x) * progress
                current_y = segment.start_point.y + (segment.end_point.y - segment.start_point.y) * progress
                current_heading = segment.start_point.heading
                
                # Add algorithm-specific noise and errors
                if algorithm == "MELKENS":
                    # MELKENS has discrete step-based positioning
                    current_x += random.gauss(0, position_accuracy)
                    current_y += random.gauss(0, position_accuracy)
                    actual_speed = segment.max_speed * random.uniform(0.9, 1.1)
                else:  # WB
                    # WB has smoother positioning but occasional glitches
                    if random.random() < 0.02:  # 2% chance of position glitch
                        current_x += random.gauss(0, position_accuracy * 5)
                        current_y += random.gauss(0, position_accuracy * 5)
                    actual_speed = segment.max_speed * random.uniform(0.95, 1.05)
                
                # Calculate target position (perfect following)
                target_x = segment.start_point.x + (segment.end_point.x - segment.start_point.x) * progress
                target_y = segment.start_point.y + (segment.end_point.y - segment.start_point.y) * progress
                target_heading = segment.start_point.heading
                
                # Calculate error magnitude
                error_magnitude = math.sqrt((current_x - target_x)**2 + (current_y - target_y)**2)
                
                # Simulate battery consumption
                battery_consumption_rate = 1.0 + actual_speed * 0.5  # %/hour base consumption
                battery_level = 100 - (current_time - self.scenario_start_time) / 3600 * battery_consumption_rate
                battery_level = max(0, battery_level)
                
                # Simulate command latency with stress level impact
                stress_multiplier = {"LOW": 1.0, "MEDIUM": 1.5, "HIGH": 2.0}[scenario.stress_level]
                command_latency = response_time_base * stress_multiplier * random.uniform(0.8, 1.2)
                
                # Create performance metric
                metric = PerformanceMetric(
                    timestamp=current_time,
                    robot_position=(current_x, current_y, current_heading),
                    target_position=(target_x, target_y, target_heading),
                    speed_actual=actual_speed,
                    speed_target=segment.max_speed,
                    battery_level=battery_level,
                    error_magnitude=error_magnitude,
                    algorithm_type=algorithm,
                    command_latency=command_latency,
                    cpu_usage=random.uniform(10, 80),  # Simulated CPU usage
                    memory_usage=random.uniform(20, 60)  # Simulated memory usage
                )
                
                performance_data.append(metric)
                
                # Update time
                segment_time += update_interval
                current_time += update_interval
        
        return performance_data
    
    # ========================================================================
    # LOG GENERATION
    # ========================================================================
    
    def generate_system_logs(self, duration: float, components: List[str] = None) -> List[LogEntry]:
        """Generate realistic system logs"""
        if components is None:
            components = ["WB_Compatibility", "MotorManager", "Navigation", "CANHandler", "BatteryManager"]
        
        log_entries = []
        start_time = time.time()
        current_time = start_time
        
        while current_time - start_time < duration:
            # Generate log entry for random component
            component = random.choice(components)
            level, message, data = self._generate_log_entry_content(component, current_time)
            
            entry = LogEntry(
                timestamp=current_time,
                level=level,
                component=component,
                message=message,
                data=data
            )
            
            log_entries.append(entry)
            
            # Variable interval between log entries
            interval = random.expovariate(2.0)  # Average 0.5 seconds between entries
            current_time += interval
        
        self.log_entries.extend(log_entries)
        return log_entries
    
    def _generate_log_entry_content(self, component: str, timestamp: float) -> Tuple[str, str, Dict]:
        """Generate realistic log entry content for component"""
        
        # Component-specific log patterns
        patterns = {
            "WB_Compatibility": [
                ("DEBUG", "Processing Butler command 0x{:04X}", {"command_id": random.randint(1000, 9999)}),
                ("INFO", "Status response sent, seq={}", {"sequence": random.randint(1, 1000)}),
                ("WARN", "Command processing delayed by {}ms", {"delay": random.randint(10, 100)}),
                ("ERROR", "Protocol violation detected", {"error_code": random.randint(1, 10)})
            ],
            "MotorManager": [
                ("DEBUG", "Motor speeds set: L={}, R={}, T={}", 
                 {"left": random.randint(-1000, 1000), "right": random.randint(-1000, 1000), "thumble": random.randint(-500, 500)}),
                ("INFO", "Motor manager initialized successfully", {}),
                ("WARN", "Motor temperature high: {}°C", {"temperature": random.randint(60, 80)}),
                ("ERROR", "Motor fault detected on {}", {"motor": random.choice(["left", "right", "thumble"])})
            ],
            "Navigation": [
                ("DEBUG", "Position update: ({:.2f}, {:.2f})", {"x": random.uniform(-50, 50), "y": random.uniform(-10, 10)}),
                ("INFO", "Route segment completed", {"segment_id": random.randint(1, 100)}),
                ("WARN", "Position accuracy degraded", {"accuracy": random.uniform(0.1, 0.5)}),
                ("ERROR", "GPS signal lost", {"duration": random.randint(1, 30)})
            ],
            "CANHandler": [
                ("DEBUG", "CAN message received: ID=0x{:03X}", {"can_id": random.randint(0x100, 0x7FF)}),
                ("INFO", "CAN bus initialized at {} baud", {"baud_rate": random.choice([500000, 1000000])}),
                ("WARN", "CAN bus utilization high: {}%", {"utilization": random.randint(70, 95)}),
                ("ERROR", "CAN bus error: {}", {"error": random.choice(["Bus off", "Error passive", "RX overflow"])})
            ],
            "BatteryManager": [
                ("DEBUG", "Battery level: {}%", {"level": random.randint(20, 100)}),
                ("INFO", "Battery monitoring started", {}),
                ("WARN", "Battery voltage low: {:.2f}V", {"voltage": random.uniform(22.0, 24.0)}),
                ("ERROR", "Battery critical: {}%", {"level": random.randint(1, 10)})
            ]
        }
        
        # Select random pattern for component
        component_patterns = patterns.get(component, [("INFO", "Generic log message", {})])
        level, message_template, data = random.choice(component_patterns)
        
        # Format message with random data
        try:
            if "{}" in message_template or "{:" in message_template:
                if data:
                    message = message_template.format(**data)
                else:
                    message = message_template.format(random.randint(1, 100))
            else:
                message = message_template
        except:
            message = message_template
            
        # Add timestamp to data
        data["timestamp"] = timestamp
        
        return level, message, data
    
    # ========================================================================
    # STATISTICAL ANALYSIS
    # ========================================================================
    
    def calculate_statistics(self) -> Dict:
        """Calculate comprehensive statistics from generated data"""
        stats = {
            'route_statistics': self._calculate_route_stats(),
            'performance_statistics': self._calculate_performance_stats(),
            'log_statistics': self._calculate_log_stats(),
            'comparison_statistics': self._calculate_comparison_stats()
        }
        
        self.stats.update(stats)
        return stats
    
    def _calculate_route_stats(self) -> Dict:
        """Calculate route-related statistics"""
        if not self.routes:
            return {}
        
        total_distance = sum(segment.distance for segment in self.routes)
        total_time = sum(segment.duration for segment in self.routes)
        avg_speed = total_distance / total_time if total_time > 0 else 0
        
        speeds = [segment.max_speed for segment in self.routes if segment.max_speed > 0]
        
        return {
            'total_segments': len(self.routes),
            'total_distance_m': total_distance,
            'total_time_s': total_time,
            'average_speed_ms': avg_speed,
            'max_speed_ms': max(speeds) if speeds else 0,
            'min_speed_ms': min(speeds) if speeds else 0,
            'feeding_segments': len([s for s in self.routes if s.segment_type == "feeding"]),
            'approach_segments': len([s for s in self.routes if s.segment_type == "approach"]),
            'straight_segments': len([s for s in self.routes if s.segment_type == "straight"])
        }
    
    def _calculate_performance_stats(self) -> Dict:
        """Calculate performance statistics"""
        if not self.performance_data:
            return {}
        
        errors = [metric.error_magnitude for metric in self.performance_data]
        latencies = [metric.command_latency for metric in self.performance_data]
        speeds = [metric.speed_actual for metric in self.performance_data]
        
        return {
            'total_measurements': len(self.performance_data),
            'average_error_m': np.mean(errors) if errors else 0,
            'max_error_m': max(errors) if errors else 0,
            'error_std_m': np.std(errors) if errors else 0,
            'average_latency_s': np.mean(latencies) if latencies else 0,
            'max_latency_s': max(latencies) if latencies else 0,
            'average_speed_ms': np.mean(speeds) if speeds else 0,
            'speed_variance': np.var(speeds) if speeds else 0
        }
    
    def _calculate_log_stats(self) -> Dict:
        """Calculate log statistics"""
        if not self.log_entries:
            return {}
        
        levels = [entry.level for entry in self.log_entries]
        components = [entry.component for entry in self.log_entries]
        
        level_counts = {level: levels.count(level) for level in set(levels)}
        component_counts = {comp: components.count(comp) for comp in set(components)}
        
        return {
            'total_entries': len(self.log_entries),
            'level_distribution': level_counts,
            'component_distribution': component_counts,
            'error_rate': level_counts.get('ERROR', 0) / len(self.log_entries) * 100,
            'warning_rate': level_counts.get('WARN', 0) / len(self.log_entries) * 100
        }
    
    def _calculate_comparison_stats(self) -> Dict:
        """Calculate MELKENS vs WB comparison statistics"""
        melkens_data = [m for m in self.performance_data if m.algorithm_type == "MELKENS"]
        wb_data = [m for m in self.performance_data if m.algorithm_type == "WB"]
        
        if not melkens_data or not wb_data:
            return {}
        
        melkens_errors = [m.error_magnitude for m in melkens_data]
        wb_errors = [m.error_magnitude for m in wb_data]
        
        melkens_latencies = [m.command_latency for m in melkens_data]
        wb_latencies = [m.command_latency for m in wb_data]
        
        return {
            'melkens_avg_error': np.mean(melkens_errors),
            'wb_avg_error': np.mean(wb_errors),
            'error_improvement_percent': (np.mean(melkens_errors) - np.mean(wb_errors)) / np.mean(melkens_errors) * 100,
            'melkens_avg_latency': np.mean(melkens_latencies),
            'wb_avg_latency': np.mean(wb_latencies),
            'latency_improvement_percent': (np.mean(melkens_latencies) - np.mean(wb_latencies)) / np.mean(melkens_latencies) * 100
        }
    
    # ========================================================================
    # EXPORT FUNCTIONS
    # ========================================================================
    
    def export_to_json(self, filename: str) -> None:
        """Export all data to JSON format"""
        data = {
            'metadata': {
                'generation_time': datetime.now().isoformat(),
                'generator_version': '1.0.0',
                'config': self.config,
                'statistics': self.calculate_statistics()
            },
            'farm_layout': self.farm_layout,
            'routes': [asdict(segment) for segment in self.routes],
            'performance_data': [asdict(metric) for metric in self.performance_data],
            'log_entries': [asdict(entry) for entry in self.log_entries],
            'track_points': [asdict(point) for point in self.track_points]
        }
        
        try:
            with open(filename, 'w') as f:
                json.dump(data, f, indent=2, default=str)
            print(f"Data exported to JSON: {filename}")
        except Exception as e:
            print(f"Error exporting to JSON: {e}")
    
    def export_to_csv(self, base_filename: str) -> None:
        """Export data to multiple CSV files"""
        try:
            # Export routes
            routes_file = f"{base_filename}_routes.csv"
            with open(routes_file, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow([
                    'segment_id', 'start_x', 'start_y', 'start_heading', 'start_timestamp',
                    'end_x', 'end_y', 'end_heading', 'end_timestamp',
                    'segment_type', 'duration', 'distance', 'max_speed'
                ])
                
                for i, segment in enumerate(self.routes):
                    writer.writerow([
                        i, segment.start_point.x, segment.start_point.y, segment.start_point.heading, segment.start_point.timestamp,
                        segment.end_point.x, segment.end_point.y, segment.end_point.heading, segment.end_point.timestamp,
                        segment.segment_type, segment.duration, segment.distance, segment.max_speed
                    ])
            
            # Export performance data
            performance_file = f"{base_filename}_performance.csv"
            with open(performance_file, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow([
                    'timestamp', 'robot_x', 'robot_y', 'robot_heading',
                    'target_x', 'target_y', 'target_heading',
                    'speed_actual', 'speed_target', 'battery_level',
                    'error_magnitude', 'algorithm_type', 'command_latency',
                    'cpu_usage', 'memory_usage'
                ])
                
                for metric in self.performance_data:
                    writer.writerow([
                        metric.timestamp, metric.robot_position[0], metric.robot_position[1], metric.robot_position[2],
                        metric.target_position[0], metric.target_position[1], metric.target_position[2],
                        metric.speed_actual, metric.speed_target, metric.battery_level,
                        metric.error_magnitude, metric.algorithm_type, metric.command_latency,
                        metric.cpu_usage, metric.memory_usage
                    ])
            
            # Export log entries
            logs_file = f"{base_filename}_logs.csv"
            with open(logs_file, 'w', newline='') as f:
                writer = csv.writer(f)
                writer.writerow(['timestamp', 'level', 'component', 'message', 'data', 'error_code'])
                
                for entry in self.log_entries:
                    writer.writerow([
                        entry.timestamp, entry.level, entry.component, entry.message,
                        json.dumps(entry.data), entry.error_code
                    ])
            
            print(f"Data exported to CSV files:")
            print(f"  Routes: {routes_file}")
            print(f"  Performance: {performance_file}")
            print(f"  Logs: {logs_file}")
            
        except Exception as e:
            print(f"Error exporting to CSV: {e}")

# ============================================================================
# COMMAND LINE INTERFACE
# ============================================================================

def main():
    """Main function for command line usage"""
    parser = argparse.ArgumentParser(description='WB-MELKENS Log and Route Generator')
    parser.add_argument('--config', '-c', default='log_generator_config.json',
                       help='Configuration file path')
    parser.add_argument('--output', '-o', default='wb_melkens_data',
                       help='Output file base name')
    parser.add_argument('--format', choices=['json', 'csv', 'both'], default='both',
                       help='Output format')
    parser.add_argument('--scenario', choices=['basic', 'feeding', 'stress', 'comparison'], 
                       default='basic', help='Test scenario to generate')
    parser.add_argument('--duration', type=int, default=300,
                       help='Scenario duration in seconds')
    parser.add_argument('--tracks', type=int, default=20,
                       help='Number of tracks to generate')
    parser.add_argument('--bays', type=int, default=15,
                       help='Number of bays to generate')
    
    args = parser.parse_args()
    
    # Load configuration
    config = {
        'farm_layout': {},
        'track_spacing': 5.0,
        'bay_depth': 10.0,
        'robot_speed_max': 2.0
    }
    
    try:
        with open(args.config, 'r') as f:
            file_config = json.load(f)
            config.update(file_config)
    except FileNotFoundError:
        print(f"Config file {args.config} not found, using defaults")
    except Exception as e:
        print(f"Error loading config file: {e}")
        return 1
    
    # Create log generator
    generator = LogGenerator(config)
    
    print("Generating farm layout...")
    generator.generate_farm_layout(args.tracks, args.bays)
    
    # Generate scenario data
    print(f"Generating {args.scenario} scenario data...")
    
    if args.scenario == 'basic':
        # Basic navigation scenario
        scenario = TestScenario(
            name="Basic Navigation",
            description="Simple track-to-track navigation",
            duration=args.duration,
            tracks=[1, 5, 10, 15, 20],
            bays=[2, 7, 12],
            algorithm="MELKENS",
            stress_level="LOW",
            error_injection=False
        )
        generator.generate_performance_comparison(scenario)
        
    elif args.scenario == 'feeding':
        # Feeding scenario
        scenario = TestScenario(
            name="Feeding Sequence",
            description="Multi-bay feeding operation",
            duration=args.duration,
            tracks=[3, 8, 12, 16],
            bays=[1, 4, 7, 10, 13],
            algorithm="WB",
            stress_level="MEDIUM",
            error_injection=False
        )
        generator.generate_performance_comparison(scenario)
        
    elif args.scenario == 'stress':
        # Stress test scenario
        scenario = TestScenario(
            name="Stress Test",
            description="High-frequency operations",
            duration=args.duration,
            tracks=list(range(1, 21)),
            bays=list(range(1, 16)),
            algorithm="BOTH",
            stress_level="HIGH",
            error_injection=True
        )
        generator.generate_performance_comparison(scenario)
        
    elif args.scenario == 'comparison':
        # Algorithm comparison scenario
        scenario = TestScenario(
            name="Algorithm Comparison",
            description="Direct MELKENS vs WB comparison",
            duration=args.duration,
            tracks=[2, 6, 10, 14, 18],
            bays=[3, 8, 11, 14],
            algorithm="BOTH",
            stress_level="MEDIUM",
            error_injection=False
        )
        generator.generate_performance_comparison(scenario)
    
    # Generate system logs
    print("Generating system logs...")
    generator.generate_system_logs(args.duration)
    
    # Calculate statistics
    print("Calculating statistics...")
    stats = generator.calculate_statistics()
    
    # Display summary
    print("\n=== Generation Summary ===")
    if 'route_statistics' in stats:
        route_stats = stats['route_statistics']
        print(f"Routes: {route_stats.get('total_segments', 0)} segments, "
              f"{route_stats.get('total_distance_m', 0):.1f}m total")
    
    if 'performance_statistics' in stats:
        perf_stats = stats['performance_statistics']
        print(f"Performance: {perf_stats.get('total_measurements', 0)} measurements, "
              f"{perf_stats.get('average_error_m', 0):.3f}m avg error")
    
    if 'log_statistics' in stats:
        log_stats = stats['log_statistics']
        print(f"Logs: {log_stats.get('total_entries', 0)} entries, "
              f"{log_stats.get('error_rate', 0):.1f}% error rate")
    
    # Export data
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    output_base = f"{args.output}_{timestamp}"
    
    if args.format in ['json', 'both']:
        print(f"\nExporting to JSON...")
        generator.export_to_json(f"{output_base}.json")
    
    if args.format in ['csv', 'both']:
        print(f"\nExporting to CSV...")
        generator.export_to_csv(output_base)
    
    print(f"\nLog generation completed successfully!")
    return 0

if __name__ == '__main__':
    sys.exit(main())

"""
USAGE EXAMPLES:

1. Basic scenario with default settings:
   python3 LogGenerator.py --scenario basic --duration 300

2. Feeding scenario with custom output:
   python3 LogGenerator.py --scenario feeding --output farm_feeding_test --format json

3. Stress test with large farm:
   python3 LogGenerator.py --scenario stress --tracks 30 --bays 25 --duration 600

4. Algorithm comparison:
   python3 LogGenerator.py --scenario comparison --duration 900 --format both

CONFIGURATION FILE (log_generator_config.json):
{
    "farm_layout": {
        "name": "Test Farm Alpha",
        "location": "Test Site"
    },
    "track_spacing": 5.0,
    "bay_depth": 10.0,
    "robot_speed_max": 2.0,
    "default_bay_capacity": 100,
    "magnetic_sensor_accuracy": 0.02
}

OUTPUT FILES:
- JSON: Complete data structure for web visualization
- CSV: Separate files for routes, performance, and logs
- Statistics included in all formats for analysis

INTEGRATION:
1. Use with WB_Simulator.py for complete test environment
2. Export data for web dashboard visualization
3. Import into analysis tools for performance comparison
4. Use for generating test scenarios and validation data
"""