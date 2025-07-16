// Types based on MELKENS PMB DataTypes.h and RoutesDataTypes.h

export enum OperationType {
  NORM = 1,
  TU_L = 2,
  TU_R = 3,
  L_90 = 4,
  R_90 = 5,
  DIFF = 6,
  NORM_NOMAGNET = 7,
  NO_OPERATION = 8
}

export enum RouteId {
  RouteA = 0,
  RouteB = 1,
  RouteC = 2,
  RouteD = 3
}

export enum MotorDirection {
  LEFT_FORWARD = 1,
  LEFT_REVERSE = 2,
  RIGHT_FORWARD = 2,
  RIGHT_REVERSE = 1
}

export enum ThresholdState {
  ON = 1,
  OFF = 0
}

// Magnetic sensor positions (from RoutesDataTypes.h)
export const MagnetPositions = {
  NO_CORRECTION: 255.0,
  R5: 5 * 2.17,
  R10: 10 * 2.17,
  L5: -5 * 2.17,
  L10: -10 * 2.17,
  MID: 0,
  L1: -1 * 2.17,
  L2: -2 * 2.17,
  L3: -3 * 2.17,
  L4: -4 * 2.17,
  L6: -6 * 2.17,
  L7: -7 * 2.17
} as const;

export interface RouteStep {
  id: number;
  operation: OperationType;
  distance: number; // in mm
  speed: number; // motor speed
  magnetCorrection: number;
  angle?: number; // for turns in degrees
  description: string;
}

export interface Route {
  id: RouteId;
  name: string;
  steps: RouteStep[];
  repeatCount: number;
  isActive: boolean;
}

export interface MotorState {
  leftSpeed: number;
  rightSpeed: number;
  leftDirection: MotorDirection;
  rightDirection: MotorDirection;
  liftSpeed: number;
  beltSpeed: number;
  isRunning: boolean;
}

export interface SensorState {
  magneticPosition: number;
  imuAngle: number;
  encoderLeft: number;
  encoderRight: number;
  batteryVoltage: number;
  temperature: number;
  isConnected: boolean;
}

export interface RobotPosition {
  x: number;
  y: number;
  angle: number; // in degrees
  timestamp: number;
}

export interface RobotState {
  id: string;
  position: RobotPosition;
  motors: MotorState;
  sensors: SensorState;
  currentRoute?: RouteId;
  currentStep?: number;
  isRunning: boolean;
  errors: string[];
  lastUpdate: number;
}

export interface LogEntry {
  timestamp: number;
  level: 'info' | 'warning' | 'error' | 'debug';
  source: string;
  message: string;
  data?: any;
}

export interface SimulatorSettings {
  correctionAngleThreshold: number; // dCORRECTION_ANGLE_THRESHOLD
  deg90Offset: number; // d90DEG_OFFSET
  deg45Offset: number; // d45DEG_OFFSET
  imuJudgementFactor: number; // dIMU_JUDGEMENT_FACTOR
  encoderJudgementFactor: number; // dECODER_JUDGEMENT_FACTOR
  encoderStepMaxMultiplier: number; // dENCODER_STEP_MAX_MULTIPLIER
  defaultSpeed: number; // DEFAULT_SPEED
  defaultSpeedLift: number; // DEFAULT_SPEED_LIFT
  defaultSpeedBelt: number; // DEFAULT_SPEED_BELT
}

// WebSocket message types
export interface WebSocketMessage {
  type: string;
  timestamp: number;
  data: any;
}

export interface RobotStateUpdate extends WebSocketMessage {
  type: 'robot_state_update';
  data: RobotState;
}

export interface LogMessage extends WebSocketMessage {
  type: 'log_message';
  data: LogEntry;
}

export interface RouteStarted extends WebSocketMessage {
  type: 'route_started';
  data: {
    routeId: RouteId;
    timestamp: number;
  };
}

export interface RouteCompleted extends WebSocketMessage {
  type: 'route_completed';
  data: {
    routeId: RouteId;
    success: boolean;
    timestamp: number;
  };
}

// API Response types
export interface ApiResponse<T = any> {
  success: boolean;
  data?: T;
  error?: string;
  timestamp: number;
}

export interface RouteListResponse extends ApiResponse {
  data: Route[];
}

export interface RobotStateResponse extends ApiResponse {
  data: RobotState;
}

export interface CreateRouteRequest {
  name: string;
  steps: Omit<RouteStep, 'id'>[];
  repeatCount?: number;
}

export interface UpdateRouteRequest extends Partial<CreateRouteRequest> {
  id: RouteId;
}

export interface StartRouteRequest {
  routeId: RouteId;
}

export interface SimulatorCommand {
  type: 'start_route' | 'stop_robot' | 'emergency_stop' | 'reset_position' | 'update_settings';
  data?: any;
}