#!/usr/bin/env python3
"""
Robot Navigation Simulator
Comparison of MELKENS vs WB navigation algorithms

Author: MELKENS Integration Team
Created: 2024
"""

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.patches import Rectangle, Circle, Polygon
import json
import math
import time
from enum import Enum
from dataclasses import dataclass
from typing import List, Tuple, Optional

# Configuration
SIMULATION_TIME_STEP = 0.1  # seconds
ROBOT_WIDTH = 0.8  # meters
ROBOT_LENGTH = 1.2  # meters
WHEEL_BASE = 0.6  # meters
MAX_SPEED = 1.0  # m/s
MAX_STEERING_ANGLE = 30  # degrees

class NavigationMode(Enum):
    MELKENS_STEP_BASED = "MELKENS Step-based"
    WB_COORDINATE_BASED = "WB Coordinate-based"

class RobotState(Enum):
    IDLE = 0
    NAVIGATING = 1
    APPROACHING_BAY = 2
    FEEDING = 3
    ERROR = 4

@dataclass
class Position:
    x: float
    y: float
    heading: float  # degrees
    
    def distance_to(self, other: 'Position') -> float:
        dx = other.x - self.x
        dy = other.y - self.y
        return math.sqrt(dx*dx + dy*dy)
    
    def angle_to(self, other: 'Position') -> float:
        dx = other.x - self.x
        dy = other.y - self.y
        return math.degrees(math.atan2(dy, dx))

@dataclass
class RouteStep:
    operation_type: str  # NORM, TU_L, TU_R, L_90, R_90
    dx: float  # meters
    dy: float  # meters
    speed: float
    angle: float  # degrees
    magnet_correction: float

@dataclass
class TrackPosition:
    track_id: int
    pos_x: float
    pos_y: float
    direction: float
    speed: float

@dataclass
class Bay:
    bay_id: int
    entry_x: float
    entry_y: float
    exit_x: float
    exit_y: float
    feed_pos_x: float
    feed_pos_y: float

@dataclass
class MagnetMarker:
    id: int
    pos_x: float
    pos_y: float
    field_strength: float

class Robot:
    def __init__(self, initial_pos: Position):
        self.position = initial_pos
        self.velocity = 0.0  # m/s
        self.steering_angle = 0.0  # degrees
        self.state = RobotState.IDLE
        
        # Navigation history
        self.path_history = [Position(initial_pos.x, initial_pos.y, initial_pos.heading)]
        self.target_position = None
        self.current_step = 0
        
        # Performance metrics
        self.distance_traveled = 0.0
        self.time_elapsed = 0.0
        self.energy_consumed = 0.0
        self.magnet_detections = 0
        self.navigation_errors = 0
    
    def update_position(self, dt: float):
        """Update robot position based on bicycle model"""
        if abs(self.velocity) < 0.01:
            return
            
        # Bicycle model kinematics
        steering_rad = math.radians(self.steering_angle)
        
        # Calculate new position
        dx = self.velocity * math.cos(math.radians(self.position.heading)) * dt
        dy = self.velocity * math.sin(math.radians(self.position.heading)) * dt
        dheading = (self.velocity / WHEEL_BASE) * math.tan(steering_rad) * dt
        
        old_pos = Position(self.position.x, self.position.y, self.position.heading)
        
        self.position.x += dx
        self.position.y += dy
        self.position.heading += math.degrees(dheading)
        
        # Normalize heading
        while self.position.heading < 0:
            self.position.heading += 360
        while self.position.heading >= 360:
            self.position.heading -= 360
        
        # Update metrics
        distance_step = old_pos.distance_to(self.position)
        self.distance_traveled += distance_step
        self.time_elapsed += dt
        self.energy_consumed += abs(self.velocity) * dt + abs(self.steering_angle) * 0.1 * dt
        
        # Store path history
        self.path_history.append(Position(self.position.x, self.position.y, self.position.heading))
    
    def get_metrics(self) -> dict:
        return {
            "distance_traveled": self.distance_traveled,
            "time_elapsed": self.time_elapsed,
            "energy_consumed": self.energy_consumed,
            "magnet_detections": self.magnet_detections,
            "navigation_errors": self.navigation_errors,
            "efficiency": self.distance_traveled / max(self.energy_consumed, 0.001)
        }

class MelkensNavigator:
    def __init__(self, robot: Robot):
        self.robot = robot
        self.route_steps = []
        self.current_step_index = 0
        self.step_start_position = None
        self.step_complete = False
        
        # MELKENS specific parameters
        self.magnet_positions = list(range(-15, 16))  # Magnet1-31, center at 0
        self.magnet_corrections = {i: i * 2.17 for i in self.magnet_positions}  # cm
    
    def set_route(self, steps: List[RouteStep]):
        self.route_steps = steps
        self.current_step_index = 0
        self.step_complete = False
    
    def update(self, dt: float):
        if not self.route_steps or self.current_step_index >= len(self.route_steps):
            self.robot.velocity = 0.0
            self.robot.steering_angle = 0.0
            return
        
        current_step = self.route_steps[self.current_step_index]
        
        # Initialize step
        if self.step_start_position is None:
            self.step_start_position = Position(
                self.robot.position.x, 
                self.robot.position.y, 
                self.robot.position.heading
            )
        
        # Calculate target position for current step
        target_x = self.step_start_position.x + current_step.dx
        target_y = self.step_start_position.y + current_step.dy
        target_pos = Position(target_x, target_y, current_step.angle)
        
        # Distance to target
        distance_to_target = self.robot.position.distance_to(target_pos)
        
        # Check if step is complete
        if distance_to_target < 0.1:  # 10cm tolerance
            self._complete_current_step()
            return
        
        # MELKENS step-based navigation logic
        self._execute_step_navigation(current_step, target_pos, dt)
    
    def _execute_step_navigation(self, step: RouteStep, target: Position, dt: float):
        # Simple step-based navigation
        if step.operation_type == "NORM":
            self.robot.velocity = step.speed
            self.robot.steering_angle = 0.0
            
        elif step.operation_type == "TU_L":
            self.robot.velocity = step.speed * 0.7
            self.robot.steering_angle = -20.0
            
        elif step.operation_type == "TU_R":
            self.robot.velocity = step.speed * 0.7
            self.robot.steering_angle = 20.0
            
        elif step.operation_type == "L_90":
            self.robot.velocity = step.speed * 0.5
            self.robot.steering_angle = -30.0
            
        elif step.operation_type == "R_90":
            self.robot.velocity = step.speed * 0.5
            self.robot.steering_angle = 30.0
        
        # Apply magnet correction
        self._apply_magnet_correction()
    
    def _apply_magnet_correction(self):
        # Simulate magnet detection
        for i, magnet_pos in enumerate(self.magnet_positions):
            magnet_world_x = self.robot.position.x + magnet_pos * 0.0217  # 2.17cm spacing
            magnet_world_y = self.robot.position.y
            
            distance = math.sqrt((self.robot.position.x - magnet_world_x)**2 + 
                               (self.robot.position.y - magnet_world_y)**2)
            
            if distance < 0.1:  # Within detection range
                self.robot.magnet_detections += 1
                
                # Apply predefined correction
                correction = self.magnet_corrections[magnet_pos] / 100.0  # cm to m
                self.robot.steering_angle += correction * 10  # Simple correction
                break
    
    def _complete_current_step(self):
        self.current_step_index += 1
        self.step_start_position = None
        
        if self.current_step_index >= len(self.route_steps):
            print("MELKENS: Route completed")
            self.robot.state = RobotState.IDLE

class WBNavigator:
    def __init__(self, robot: Robot):
        self.robot = robot
        self.target_position = None
        self.track_positions = []
        self.bays = []
        self.magnet_markers = []
        
        # WB specific parameters
        self.cross_track_error = 0.0
        self.heading_error = 0.0
        self.lookahead_distance = 1.0  # meters
        
        # PID controller parameters
        self.kp_cross = 2.0
        self.kp_heading = 1.0
        self.ki_cross = 0.1
        self.kd_cross = 0.05
        
        self.integral_cross = 0.0
        self.previous_cross_error = 0.0
    
    def set_target(self, target: Position):
        self.target_position = target
        self.robot.state = RobotState.NAVIGATING
    
    def set_track_positions(self, tracks: List[TrackPosition]):
        self.track_positions = tracks
    
    def set_bays(self, bays: List[Bay]):
        self.bays = bays
    
    def set_magnet_markers(self, markers: List[MagnetMarker]):
        self.magnet_markers = markers
    
    def update(self, dt: float):
        if self.target_position is None:
            self.robot.velocity = 0.0
            self.robot.steering_angle = 0.0
            return
        
        # Calculate distance to target
        distance_to_target = self.robot.position.distance_to(self.target_position)
        
        # Check if target reached
        if distance_to_target < 0.1:
            print("WB: Target reached")
            self.robot.velocity = 0.0
            self.robot.steering_angle = 0.0
            self.robot.state = RobotState.IDLE
            return
        
        # WB coordinate-based navigation
        self._execute_coordinate_navigation(dt)
        
        # Apply magnetic field corrections
        self._apply_magnetic_field_correction()
    
    def _execute_coordinate_navigation(self, dt: float):
        # Pure pursuit algorithm
        target_heading = self.robot.position.angle_to(self.target_position)
        self.heading_error = self._normalize_angle(target_heading - self.robot.position.heading)
        
        # Calculate cross-track error (simplified)
        self.cross_track_error = math.sin(math.radians(self.heading_error)) * \
                                self.robot.position.distance_to(self.target_position)
        
        # PID controller for steering
        self.integral_cross += self.cross_track_error * dt
        derivative_cross = (self.cross_track_error - self.previous_cross_error) / dt
        
        steering_correction = (self.kp_cross * self.cross_track_error + 
                             self.ki_cross * self.integral_cross + 
                             self.kd_cross * derivative_cross)
        
        # Apply heading correction
        heading_correction = self.kp_heading * self.heading_error
        
        # Combine corrections
        self.robot.steering_angle = steering_correction + heading_correction
        
        # Limit steering angle
        self.robot.steering_angle = max(-MAX_STEERING_ANGLE, 
                                       min(MAX_STEERING_ANGLE, self.robot.steering_angle))
        
        # Speed control based on distance
        distance_to_target = self.robot.position.distance_to(self.target_position)
        if distance_to_target > 2.0:
            self.robot.velocity = MAX_SPEED
        else:
            self.robot.velocity = MAX_SPEED * (distance_to_target / 2.0)
        
        self.robot.velocity = max(0.1, min(MAX_SPEED, self.robot.velocity))
        
        self.previous_cross_error = self.cross_track_error
    
    def _apply_magnetic_field_correction(self):
        # Find nearest magnetic marker
        nearest_marker = None
        min_distance = float('inf')
        
        for marker in self.magnet_markers:
            distance = math.sqrt((self.robot.position.x - marker.pos_x)**2 + 
                               (self.robot.position.y - marker.pos_y)**2)
            if distance < min_distance:
                min_distance = distance
                nearest_marker = marker
        
        # Apply correction if close to marker
        if nearest_marker and min_distance < 0.5:  # Within 50cm
            self.robot.magnet_detections += 1
            
            # Calculate field strength based on distance
            field_strength = max(0, 100 - min_distance * 100)
            
            # Dynamic correction based on field strength
            if field_strength > 50:
                correction_factor = 0.8
            elif field_strength > 20:
                correction_factor = 0.5
            else:
                correction_factor = 0.2
            
            # Apply position correction
            position_error = min_distance * 10  # Convert to correction factor
            self.robot.steering_angle += position_error * correction_factor
    
    def _normalize_angle(self, angle: float) -> float:
        while angle > 180:
            angle -= 360
        while angle < -180:
            angle += 360
        return angle

class NavigationSimulator:
    def __init__(self, width: float = 10.0, height: float = 10.0):
        self.width = width
        self.height = height
        
        # Create robots for comparison
        initial_pos = Position(1.0, 1.0, 0.0)
        self.melkens_robot = Robot(Position(initial_pos.x, initial_pos.y, initial_pos.heading))
        self.wb_robot = Robot(Position(initial_pos.x, initial_pos.y, initial_pos.heading))
        
        # Create navigators
        self.melkens_navigator = MelkensNavigator(self.melkens_robot)
        self.wb_navigator = WBNavigator(self.wb_robot)
        
        # Environment
        self.obstacles = []
        self.tracks = []
        self.bays = []
        self.magnet_markers = []
        
        # Simulation state
        self.running = False
        self.current_time = 0.0
        
        # Setup default environment
        self._setup_default_environment()
    
    def _setup_default_environment(self):
        # Create a simple track layout
        self.tracks = [
            TrackPosition(1, 2.0, 1.0, 0, 0.8),
            TrackPosition(2, 5.0, 1.0, 90, 0.8),
            TrackPosition(3, 5.0, 4.0, 180, 0.8),
            TrackPosition(4, 2.0, 4.0, 270, 0.8),
        ]
        
        # Create feeding bays
        self.bays = [
            Bay(1, 3.0, 2.0, 3.0, 2.5, 3.25, 2.25),
            Bay(2, 6.0, 2.0, 6.0, 2.5, 6.25, 2.25),
            Bay(3, 6.0, 3.0, 6.0, 3.5, 6.25, 3.25),
            Bay(4, 3.0, 3.0, 3.0, 3.5, 3.25, 3.25),
        ]
        
        # Create magnetic markers along the track
        track_points = [(2.0, 1.0), (3.0, 1.0), (4.0, 1.0), (5.0, 1.0),
                       (5.0, 2.0), (5.0, 3.0), (5.0, 4.0),
                       (4.0, 4.0), (3.0, 4.0), (2.0, 4.0)]
        
        for i, (x, y) in enumerate(track_points):
            self.magnet_markers.append(MagnetMarker(i+1, x, y, 75.0))
        
        # Set up WB navigator with environment
        self.wb_navigator.set_track_positions(self.tracks)
        self.wb_navigator.set_bays(self.bays)
        self.wb_navigator.set_magnet_markers(self.magnet_markers)
    
    def create_melkens_route(self) -> List[RouteStep]:
        """Create a MELKENS-style step-based route"""
        return [
            RouteStep("NORM", 1.0, 0.0, 0.8, 0.0, 0.0),      # Forward 1m
            RouteStep("NORM", 2.0, 0.0, 0.8, 0.0, 0.0),      # Forward 2m  
            RouteStep("R_90", 0.0, 0.0, 0.5, 90.0, 0.0),     # Turn right 90°
            RouteStep("NORM", 3.0, 0.0, 0.8, 90.0, 0.0),     # Forward 3m
            RouteStep("R_90", 0.0, 0.0, 0.5, 180.0, 0.0),    # Turn right 90°
            RouteStep("NORM", 3.0, 0.0, 0.8, 180.0, 0.0),    # Forward 3m
            RouteStep("R_90", 0.0, 0.0, 0.5, 270.0, 0.0),    # Turn right 90°
            RouteStep("NORM", 3.0, 0.0, 0.8, 270.0, 0.0),    # Forward 3m
        ]
    
    def start_comparison_simulation(self):
        """Start side-by-side comparison of both algorithms"""
        # Set up MELKENS route
        melkens_route = self.create_melkens_route()
        self.melkens_navigator.set_route(melkens_route)
        
        # Set up WB target (end of track loop)
        self.wb_navigator.set_target(Position(2.0, 4.0, 270.0))
        
        self.running = True
        self.current_time = 0.0
        
        print("Starting navigation comparison simulation...")
        print("MELKENS: Step-based navigation")
        print("WB: Coordinate-based navigation")
    
    def update(self, dt: float):
        if not self.running:
            return
        
        self.current_time += dt
        
        # Update both navigators
        self.melkens_navigator.update(dt)
        self.wb_navigator.update(dt)
        
        # Update robot positions
        self.melkens_robot.update_position(dt)
        self.wb_robot.update_position(dt)
        
        # Check if both robots are idle (simulation complete)
        if (self.melkens_robot.state == RobotState.IDLE and 
            self.wb_robot.state == RobotState.IDLE):
            self.running = False
            self._print_comparison_results()
    
    def _print_comparison_results(self):
        print("\n" + "="*60)
        print("SIMULATION RESULTS")
        print("="*60)
        
        melkens_metrics = self.melkens_robot.get_metrics()
        wb_metrics = self.wb_robot.get_metrics()
        
        print("\nMELKENS (Step-based) Results:")
        for key, value in melkens_metrics.items():
            print(f"  {key}: {value:.3f}")
        
        print("\nWB (Coordinate-based) Results:")
        for key, value in wb_metrics.items():
            print(f"  {key}: {value:.3f}")
        
        print("\nComparison:")
        print(f"  Distance efficiency: WB vs MELKENS = "
              f"{wb_metrics['distance_traveled']/melkens_metrics['distance_traveled']:.3f}")
        print(f"  Time efficiency: WB vs MELKENS = "
              f"{wb_metrics['time_elapsed']/melkens_metrics['time_elapsed']:.3f}")
        print(f"  Energy efficiency: WB vs MELKENS = "
              f"{wb_metrics['efficiency']/melkens_metrics['efficiency']:.3f}")
        
        print("="*60)
    
    def get_visualization_data(self) -> dict:
        """Get data for visualization"""
        return {
            'melkens_robot': {
                'position': self.melkens_robot.position,
                'path': self.melkens_robot.path_history,
                'state': self.melkens_robot.state,
                'metrics': self.melkens_robot.get_metrics()
            },
            'wb_robot': {
                'position': self.wb_robot.position,
                'path': self.wb_robot.path_history,
                'state': self.wb_robot.state,
                'metrics': self.wb_robot.get_metrics()
            },
            'environment': {
                'tracks': self.tracks,
                'bays': self.bays,
                'magnet_markers': self.magnet_markers,
                'width': self.width,
                'height': self.height
            },
            'time': self.current_time,
            'running': self.running
        }

class SimulationVisualizer:
    def __init__(self, simulator: NavigationSimulator):
        self.simulator = simulator
        
        # Create figure and axes
        self.fig, (self.ax_main, self.ax_metrics) = plt.subplots(1, 2, figsize=(16, 8))
        
        # Main simulation plot
        self.ax_main.set_xlim(0, simulator.width)
        self.ax_main.set_ylim(0, simulator.height)
        self.ax_main.set_aspect('equal')
        self.ax_main.grid(True, alpha=0.3)
        self.ax_main.set_title('Robot Navigation Comparison: MELKENS vs WB')
        self.ax_main.set_xlabel('X Position (m)')
        self.ax_main.set_ylabel('Y Position (m)')
        
        # Metrics plot
        self.ax_metrics.set_title('Performance Metrics Comparison')
        
        # Initialize plot elements
        self.melkens_robot_patch = None
        self.wb_robot_patch = None
        self.melkens_path_line = None
        self.wb_path_line = None
        self.environment_patches = []
        
        # Metrics tracking
        self.time_history = []
        self.melkens_metrics_history = []
        self.wb_metrics_history = []
    
    def setup_environment_visualization(self, env_data: dict):
        """Setup static environment elements"""
        self.ax_main.clear()
        self.ax_main.set_xlim(0, env_data['width'])
        self.ax_main.set_ylim(0, env_data['height'])
        self.ax_main.set_aspect('equal')
        self.ax_main.grid(True, alpha=0.3)
        self.ax_main.set_title('Robot Navigation Comparison: MELKENS vs WB')
        self.ax_main.set_xlabel('X Position (m)')
        self.ax_main.set_ylabel('Y Position (m)')
        
        # Draw tracks
        for track in env_data['tracks']:
            circle = Circle((track.pos_x, track.pos_y), 0.1, 
                          color='blue', alpha=0.7, label='Track' if track.track_id == 1 else "")
            self.ax_main.add_patch(circle)
        
        # Draw bays
        for bay in env_data['bays']:
            bay_rect = Rectangle((bay.entry_x - 0.1, bay.entry_y - 0.1), 0.4, 0.4,
                               color='green', alpha=0.5, label='Bay' if bay.bay_id == 1 else "")
            self.ax_main.add_patch(bay_rect)
        
        # Draw magnetic markers
        for marker in env_data['magnet_markers']:
            marker_circle = Circle((marker.pos_x, marker.pos_y), 0.05,
                                 color='red', alpha=0.8, label='Magnet' if marker.id == 1 else "")
            self.ax_main.add_patch(marker_circle)
        
        # Add legend
        self.ax_main.legend(loc='upper right')
        
        # Initialize path lines
        self.melkens_path_line, = self.ax_main.plot([], [], 'b-', linewidth=2, 
                                                   alpha=0.7, label='MELKENS Path')
        self.wb_path_line, = self.ax_main.plot([], [], 'r-', linewidth=2, 
                                              alpha=0.7, label='WB Path')
        
        # Initialize robot representations
        self.melkens_robot_patch = Circle((0, 0), ROBOT_WIDTH/2, color='blue', alpha=0.8)
        self.wb_robot_patch = Circle((0, 0), ROBOT_WIDTH/2, color='red', alpha=0.8)
        self.ax_main.add_patch(self.melkens_robot_patch)
        self.ax_main.add_patch(self.wb_robot_patch)
    
    def update_visualization(self, data: dict):
        """Update visualization with current simulation data"""
        # Update robot positions
        melkens_pos = data['melkens_robot']['position']
        wb_pos = data['wb_robot']['position']
        
        self.melkens_robot_patch.center = (melkens_pos.x, melkens_pos.y)
        self.wb_robot_patch.center = (wb_pos.x, wb_pos.y)
        
        # Update paths
        if len(data['melkens_robot']['path']) > 1:
            melkens_x = [p.x for p in data['melkens_robot']['path']]
            melkens_y = [p.y for p in data['melkens_robot']['path']]
            self.melkens_path_line.set_data(melkens_x, melkens_y)
        
        if len(data['wb_robot']['path']) > 1:
            wb_x = [p.x for p in data['wb_robot']['path']]
            wb_y = [p.y for p in data['wb_robot']['path']]
            self.wb_path_line.set_data(wb_x, wb_y)
        
        # Update metrics plot
        self.time_history.append(data['time'])
        self.melkens_metrics_history.append(data['melkens_robot']['metrics'])
        self.wb_metrics_history.append(data['wb_robot']['metrics'])
        
        self._update_metrics_plot()
        
        # Update title with current time
        self.ax_main.set_title(f'Robot Navigation Comparison (t={data["time"]:.1f}s)')
    
    def _update_metrics_plot(self):
        """Update the metrics comparison plot"""
        if len(self.time_history) < 2:
            return
        
        self.ax_metrics.clear()
        
        # Plot distance traveled
        melkens_distances = [m['distance_traveled'] for m in self.melkens_metrics_history]
        wb_distances = [m['distance_traveled'] for m in self.wb_metrics_history]
        
        self.ax_metrics.plot(self.time_history, melkens_distances, 'b-', 
                           label='MELKENS Distance', linewidth=2)
        self.ax_metrics.plot(self.time_history, wb_distances, 'r-', 
                           label='WB Distance', linewidth=2)
        
        self.ax_metrics.set_xlabel('Time (s)')
        self.ax_metrics.set_ylabel('Distance Traveled (m)')
        self.ax_metrics.set_title('Distance Traveled Comparison')
        self.ax_metrics.legend()
        self.ax_metrics.grid(True, alpha=0.3)
    
    def animate(self, frame):
        """Animation function for matplotlib"""
        self.simulator.update(SIMULATION_TIME_STEP)
        data = self.simulator.get_visualization_data()
        
        if frame == 0:
            self.setup_environment_visualization(data['environment'])
        
        self.update_visualization(data)
        return [self.melkens_robot_patch, self.wb_robot_patch, 
                self.melkens_path_line, self.wb_path_line]
    
    def start_animation(self):
        """Start the animated simulation"""
        self.ani = animation.FuncAnimation(
            self.fig, self.animate, interval=int(SIMULATION_TIME_STEP * 1000),
            blit=False, repeat=False)
        plt.tight_layout()
        plt.show()

def main():
    """Main simulation entry point"""
    print("Robot Navigation Simulator")
    print("MELKENS vs WB Algorithm Comparison")
    print("-" * 40)
    
    # Create simulator
    simulator = NavigationSimulator(width=8.0, height=6.0)
    
    # Create visualizer
    visualizer = SimulationVisualizer(simulator)
    
    # Start simulation
    simulator.start_comparison_simulation()
    
    # Start visualization
    visualizer.start_animation()

if __name__ == "__main__":
    main()