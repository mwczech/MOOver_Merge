// Constants from pmb_Settings.h
export const DEFAULT_SETTINGS = {
  CORRECTION_ANGLE_THRESHOLD: 0.5, // degrees
  DEG_90_OFFSET: 0,
  DEG_45_OFFSET: 0,
  FEEDING_TABLE_OFFSET: 20,
  ROUTE_REPEAT_COUNT: 5,
  ANGLE_IN_90_TURN: 90,
  ANGLE_IN_45_TURN: 45,
  IMU_JUDGEMENT_FACTOR: 1.0,
  ENCODER_JUDGEMENT_FACTOR: 0.0, // 1.0 - IMU_JUDGEMENT_FACTOR
  ENCODER_STEP_MAX_MULTIPLIER: 1.5,
  DEFAULT_SPEED: 500,
  DEFAULT_SPEED_LIFT: 500,
  DEFAULT_SPEED_BELT: 500
};

// Route feeding table steps (from pmb_Settings.h)
export const ROUTE_FEEDING_STEPS = {
  ROUTE_A_FEEDING_TABLE_1_STEP: 2,
  ROUTE_A_FEEDING_TABLE_2_STEP: 6
};

// CAN communication settings
export const CAN_SETTINGS = {
  BAUD_RATE: 250000,
  TIMEOUT_MS: 1000
};

// WebSocket settings
export const WEBSOCKET_SETTINGS = {
  HEARTBEAT_INTERVAL: 5000, // ms
  RECONNECT_DELAY: 3000, // ms
  MAX_RECONNECT_ATTEMPTS: 5
};

// Simulation settings
export const SIMULATION_SETTINGS = {
  UPDATE_RATE: 100, // ms - how often to update robot state
  GRID_SIZE: 50, // mm - grid size for visualization
  ROBOT_SIZE: {
    WIDTH: 400, // mm
    LENGTH: 600 // mm
  },
  MAP_SIZE: {
    WIDTH: 10000, // mm
    HEIGHT: 8000 // mm
  }
};

// Error codes and messages
export const ERROR_CODES = {
  MOTOR_TIMEOUT: 'E001',
  SENSOR_DISCONNECTED: 'E002',
  MAGNETIC_LOST: 'E003',
  ROUTE_STEP_FAILED: 'E004',
  EMERGENCY_STOP: 'E005',
  BATTERY_LOW: 'E006',
  IMU_CALIBRATION: 'E007',
  CAN_COMMUNICATION: 'E008'
};

export const ERROR_MESSAGES = {
  [ERROR_CODES.MOTOR_TIMEOUT]: 'Motor response timeout',
  [ERROR_CODES.SENSOR_DISCONNECTED]: 'Sensor disconnected',
  [ERROR_CODES.MAGNETIC_LOST]: 'Magnetic line lost',
  [ERROR_CODES.ROUTE_STEP_FAILED]: 'Route step execution failed',
  [ERROR_CODES.EMERGENCY_STOP]: 'Emergency stop activated',
  [ERROR_CODES.BATTERY_LOW]: 'Battery voltage too low',
  [ERROR_CODES.IMU_CALIBRATION]: 'IMU calibration required',
  [ERROR_CODES.CAN_COMMUNICATION]: 'CAN communication error'
};

// Default routes based on typical warehouse operations
export const DEFAULT_ROUTES_CONFIG = [
  {
    id: 0,
    name: "Route A - Picking Line",
    description: "Main picking route with feeding tables",
    steps: [
      { operation: 1, distance: 2000, speed: 500, magnetCorrection: 0, description: "Start from base" },
      { operation: 2, distance: 1000, speed: 300, magnetCorrection: -5 * 2.17, description: "Turn left to shelf 1" },
      { operation: 1, distance: 3000, speed: 500, magnetCorrection: 0, description: "Approach feeding table 1" },
      { operation: 1, distance: 500, speed: 200, magnetCorrection: 0, description: "Precise positioning" },
      { operation: 3, distance: 1000, speed: 300, magnetCorrection: 5 * 2.17, description: "Turn right to shelf 2" },
      { operation: 1, distance: 2000, speed: 500, magnetCorrection: 0, description: "Approach feeding table 2" },
      { operation: 4, distance: 0, speed: 300, magnetCorrection: 0, angle: 90, description: "90° left turn" },
      { operation: 1, distance: 1500, speed: 400, magnetCorrection: 0, description: "Return to base" }
    ],
    repeatCount: 5
  }
];

// API endpoints
export const API_ENDPOINTS = {
  ROUTES: '/api/routes',
  ROBOT_STATE: '/api/robot/state',
  ROBOT_CONTROL: '/api/robot/control',
  LOGS: '/api/logs',
  SETTINGS: '/api/settings',
  WEBSOCKET: '/ws'
};