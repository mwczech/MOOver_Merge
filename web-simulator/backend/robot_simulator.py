#!/usr/bin/env python3
"""
MELKENS Robot Simulator
Simulates robot movement and navigation based on IMU data
"""

import math
import time
from typing import Dict, List, Optional, Tuple
from dataclasses import dataclass
from enum import Enum
import structlog

logger = structlog.get_logger()

class RouteState(Enum):
    IDLE = "idle"
    RUNNING = "running"
    PAUSED = "paused"
    COMPLETED = "completed"
    ERROR = "error"

@dataclass
class RouteStep:
    """Individual step in a route"""
    dx: float           # X displacement (meters)
    dy: float           # Y displacement (meters)
    speed: int          # Movement speed (0-1500)
    auger_speed: int    # Auger/tumbler speed (0-1500)
    magnet_correction: float  # Magnetic correction offset

@dataclass
class Route:
    """Route definition with steps"""
    name: str
    steps: List[RouteStep]
    description: str

class RobotSimulator:
    """Simulates MELKENS robot behavior"""
    
    def __init__(self):
        # Robot state
        self.position = [0.0, 0.0]  # x, y in meters
        self.angle = 0.0            # heading angle in radians
        self.velocity = [0.0, 0.0]  # velocity in m/s
        
        # Motor state
        self.motor_left_speed = 0   # -1500 to 1500
        self.motor_right_speed = 0  # -1500 to 1500
        self.auger_speed = 0        # 0 to 1500
        
        # Route state
        self.current_route: Optional[str] = None
        self.route_state = RouteState.IDLE
        self.current_step = 0
        self.step_progress = 0.0
        
        # Simulation parameters
        self.wheel_base = 0.5       # meters between wheels
        self.max_speed = 2.0        # max linear speed m/s
        self.last_update_time = time.time()
        
        # Define predefined routes based on MELKENS route system
        self.routes = self._initialize_routes()
        
        # Navigation state
        self.target_position = [0.0, 0.0]
        self.navigation_enabled = False
        self.magnet_detection = False
        self.magnet_offset = 0.0
        
        logger.info("Robot simulator initialized")
    
    def _initialize_routes(self) -> Dict[str, Route]:
        """Initialize predefined routes A, B, C, D"""
        routes = {}
        
        # Route A - Simple rectangular pattern
        routes['A'] = Route(
            name="Route A",
            description="Simple rectangular pattern for testing",
            steps=[
                RouteStep(dx=2.0, dy=0.0, speed=500, auger_speed=0, magnet_correction=0.0),
                RouteStep(dx=0.0, dy=1.5, speed=500, auger_speed=800, magnet_correction=0.0),
                RouteStep(dx=-2.0, dy=0.0, speed=500, auger_speed=800, magnet_correction=0.0),
                RouteStep(dx=0.0, dy=-1.5, speed=500, auger_speed=0, magnet_correction=0.0),
            ]
        )
        
        # Route B - L-shaped pattern
        routes['B'] = Route(
            name="Route B", 
            description="L-shaped pattern with magnetic corrections",
            steps=[
                RouteStep(dx=3.0, dy=0.0, speed=600, auger_speed=1000, magnet_correction=-2.17),
                RouteStep(dx=0.0, dy=2.0, speed=600, auger_speed=1000, magnet_correction=0.0),
                RouteStep(dx=1.0, dy=0.0, speed=600, auger_speed=1000, magnet_correction=2.17),
            ]
        )
        
        # Route C - Complex pattern with turns
        routes['C'] = Route(
            name="Route C",
            description="Complex pattern with multiple direction changes",
            steps=[
                RouteStep(dx=1.5, dy=0.0, speed=400, auger_speed=1200, magnet_correction=0.0),
                RouteStep(dx=1.0, dy=1.0, speed=400, auger_speed=1200, magnet_correction=-4.34),
                RouteStep(dx=0.0, dy=1.5, speed=400, auger_speed=1200, magnet_correction=0.0),
                RouteStep(dx=-1.0, dy=1.0, speed=400, auger_speed=1200, magnet_correction=4.34),
                RouteStep(dx=-1.5, dy=0.0, speed=400, auger_speed=1200, magnet_correction=0.0),
                RouteStep(dx=0.0, dy=-3.5, speed=400, auger_speed=0, magnet_correction=0.0),
            ]
        )
        
        # Route D - Circular pattern
        routes['D'] = Route(
            name="Route D",
            description="Circular pattern for continuous operation",
            steps=[
                RouteStep(dx=1.0, dy=0.0, speed=700, auger_speed=1500, magnet_correction=0.0),
                RouteStep(dx=0.707, dy=0.707, speed=700, auger_speed=1500, magnet_correction=-10.85),
                RouteStep(dx=0.0, dy=1.0, speed=700, auger_speed=1500, magnet_correction=0.0),
                RouteStep(dx=-0.707, dy=0.707, speed=700, auger_speed=1500, magnet_correction=10.85),
                RouteStep(dx=-1.0, dy=0.0, speed=700, auger_speed=1500, magnet_correction=0.0),
                RouteStep(dx=-0.707, dy=-0.707, speed=700, auger_speed=1500, magnet_correction=-10.85),
                RouteStep(dx=0.0, dy=-1.0, speed=700, auger_speed=1500, magnet_correction=0.0),
                RouteStep(dx=0.707, dy=-0.707, speed=700, auger_speed=1500, magnet_correction=10.85),
            ]
        )
        
        return routes
    
    def set_route(self, route_id: str) -> bool:
        """Set the current route"""
        if route_id not in self.routes:
            logger.error("Invalid route ID", route_id=route_id)
            return False
        
        self.current_route = route_id
        self.route_state = RouteState.IDLE
        self.current_step = 0
        self.step_progress = 0.0
        
        logger.info("Route set", route_id=route_id)
        return True
    
    def start_route(self):
        """Start executing the current route"""
        if not self.current_route:
            logger.error("No route selected")
            return False
        
        self.route_state = RouteState.RUNNING
        self.current_step = 0
        self.step_progress = 0.0
        
        logger.info("Route started", route_id=self.current_route)
        return True
    
    def pause_route(self):
        """Pause route execution"""
        if self.route_state == RouteState.RUNNING:
            self.route_state = RouteState.PAUSED
            self.motor_left_speed = 0
            self.motor_right_speed = 0
            self.auger_speed = 0
            logger.info("Route paused")
    
    def resume_route(self):
        """Resume route execution"""
        if self.route_state == RouteState.PAUSED:
            self.route_state = RouteState.RUNNING
            logger.info("Route resumed")
    
    def stop_route(self):
        """Stop route execution"""
        self.route_state = RouteState.IDLE
        self.current_step = 0
        self.step_progress = 0.0
        self.motor_left_speed = 0
        self.motor_right_speed = 0
        self.auger_speed = 0
        logger.info("Route stopped")
    
    def update_with_imu_data(self, imu_data: Dict):
        """Update robot state based on IMU data"""
        if not imu_data:
            return
        
        current_time = time.time()
        dt = current_time - self.last_update_time
        self.last_update_time = current_time
        
        # Extract IMU data
        euler_angles = imu_data.get('euler_angles', {})
        position = imu_data.get('position', {})
        
        # Update robot heading from IMU yaw
        if 'yaw' in euler_angles:
            self.angle = euler_angles['yaw']
        
        # Update position if available (in HIL mode)
        if 'x' in position and 'y' in position:
            self.position[0] = position['x']
            self.position[1] = position['y']
        
        # Update navigation if route is running
        if self.route_state == RouteState.RUNNING:
            self._update_route_execution(dt)
        
        # Simulate differential drive kinematics
        self._update_kinematics(dt)
    
    def _update_route_execution(self, dt: float):
        """Update route execution logic"""
        if not self.current_route or self.current_route not in self.routes:
            return
        
        route = self.routes[self.current_route]
        
        if self.current_step >= len(route.steps):
            # Route completed
            self.route_state = RouteState.COMPLETED
            self.motor_left_speed = 0
            self.motor_right_speed = 0
            self.auger_speed = 0
            logger.info("Route completed", route_id=self.current_route)
            return
        
        current_step_data = route.steps[self.current_step]
        
        # Calculate target position for current step
        step_target = [
            self.position[0] + current_step_data.dx,
            self.position[1] + current_step_data.dy
        ]
        
        # Calculate distance to target
        distance_to_target = math.sqrt(
            (step_target[0] - self.position[0])**2 + 
            (step_target[1] - self.position[1])**2
        )
        
        # If close to target, advance to next step
        if distance_to_target < 0.1:  # 10cm tolerance
            self.current_step += 1
            self.step_progress = 0.0
            logger.info("Step completed", step=self.current_step-1)
            return
        
        # Calculate desired heading to target
        desired_angle = math.atan2(
            step_target[1] - self.position[1],
            step_target[0] - self.position[0]
        )
        
        # Apply magnetic correction
        desired_angle += math.radians(current_step_data.magnet_correction)
        
        # Calculate angular error
        angle_error = self._normalize_angle(desired_angle - self.angle)
        
        # Simple P controller for steering
        turn_rate = angle_error * 2.0  # Proportional gain
        
        # Convert speed to motor commands
        base_speed = current_step_data.speed
        left_speed = base_speed - turn_rate * 100
        right_speed = base_speed + turn_rate * 100
        
        # Clamp speeds
        self.motor_left_speed = max(-1500, min(1500, int(left_speed)))
        self.motor_right_speed = max(-1500, min(1500, int(right_speed)))
        self.auger_speed = current_step_data.auger_speed
        
        # Update step progress
        total_distance = math.sqrt(current_step_data.dx**2 + current_step_data.dy**2)
        if total_distance > 0:
            self.step_progress = 1.0 - (distance_to_target / total_distance)
    
    def _update_kinematics(self, dt: float):
        """Update robot kinematics based on motor speeds"""
        # Convert motor speeds to wheel velocities
        # Assuming linear relationship: speed 1500 = max velocity
        max_wheel_speed = self.max_speed  # m/s
        
        left_vel = (self.motor_left_speed / 1500.0) * max_wheel_speed
        right_vel = (self.motor_right_speed / 1500.0) * max_wheel_speed
        
        # Differential drive kinematics
        linear_velocity = (left_vel + right_vel) / 2.0
        angular_velocity = (right_vel - left_vel) / self.wheel_base
        
        # Update position and angle (only in simulation mode)
        # In HIL mode, position comes from IMU integration
        if dt > 0 and dt < 0.1:  # Sanity check
            # Update angle
            self.angle += angular_velocity * dt
            self.angle = self._normalize_angle(self.angle)
            
            # Update position (only in simulation mode)
            # In real HIL mode, this would come from IMU integration
            self.position[0] += linear_velocity * math.cos(self.angle) * dt
            self.position[1] += linear_velocity * math.sin(self.angle) * dt
            
            # Update velocity
            self.velocity[0] = linear_velocity * math.cos(self.angle)
            self.velocity[1] = linear_velocity * math.sin(self.angle)
    
    def _normalize_angle(self, angle: float) -> float:
        """Normalize angle to [-pi, pi]"""
        while angle > math.pi:
            angle -= 2 * math.pi
        while angle < -math.pi:
            angle += 2 * math.pi
        return angle
    
    def get_position(self) -> Dict:
        """Get current robot position and state"""
        return {
            'x': self.position[0],
            'y': self.position[1],
            'angle': self.angle,
            'angle_degrees': math.degrees(self.angle),
            'velocity_x': self.velocity[0],
            'velocity_y': self.velocity[1],
            'motor_left_speed': self.motor_left_speed,
            'motor_right_speed': self.motor_right_speed,
            'auger_speed': self.auger_speed,
            'current_route': self.current_route,
            'route_state': self.route_state.value,
            'current_step': self.current_step,
            'step_progress': self.step_progress,
            'total_steps': len(self.routes[self.current_route].steps) if self.current_route else 0
        }
    
    def get_route_info(self, route_id: Optional[str] = None) -> Dict:
        """Get information about a specific route or all routes"""
        if route_id:
            if route_id not in self.routes:
                return {'error': f'Route {route_id} not found'}
            
            route = self.routes[route_id]
            return {
                'name': route.name,
                'description': route.description,
                'steps': [
                    {
                        'dx': step.dx,
                        'dy': step.dy,
                        'speed': step.speed,
                        'auger_speed': step.auger_speed,
                        'magnet_correction': step.magnet_correction
                    } for step in route.steps
                ]
            }
        else:
            # Return all routes
            return {
                route_id: {
                    'name': route.name,
                    'description': route.description,
                    'step_count': len(route.steps)
                } for route_id, route in self.routes.items()
            }
    
    def set_manual_control(self, left_speed: int, right_speed: int, auger_speed: int = 0):
        """Set manual motor control (overrides route execution)"""
        if self.route_state == RouteState.RUNNING:
            self.pause_route()
        
        self.motor_left_speed = max(-1500, min(1500, left_speed))
        self.motor_right_speed = max(-1500, min(1500, right_speed))
        self.auger_speed = max(0, min(1500, auger_speed))
        
        logger.info("Manual control set", 
                   left=self.motor_left_speed, 
                   right=self.motor_right_speed, 
                   auger=self.auger_speed)
    
    def simulate_magnet_detection(self, magnet_position: float):
        """Simulate magnetic line detection"""
        self.magnet_detection = True
        self.magnet_offset = magnet_position  # Position relative to center (-10 to +10)
        
        # This would trigger navigation corrections in real system
        logger.debug("Magnet detected", position=magnet_position)
    
    def get_debug_info(self) -> Dict:
        """Get detailed debug information"""
        return {
            'kinematics': {
                'wheel_base': self.wheel_base,
                'max_speed': self.max_speed,
                'last_update_time': self.last_update_time
            },
            'navigation': {
                'target_position': self.target_position,
                'navigation_enabled': self.navigation_enabled,
                'magnet_detection': self.magnet_detection,
                'magnet_offset': self.magnet_offset
            },
            'available_routes': list(self.routes.keys()),
            'current_route_details': self.get_route_info(self.current_route) if self.current_route else None
        }