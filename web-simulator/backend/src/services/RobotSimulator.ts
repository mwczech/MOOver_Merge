import { EventEmitter } from 'events';
import { 
  RobotState, 
  MotorState, 
  SensorState, 
  RobotPosition, 
  Route, 
  RouteStep, 
  OperationType,
  MotorDirection,
  LogEntry,
  DEFAULT_SETTINGS,
  SIMULATION_SETTINGS,
  ERROR_CODES,
  ERROR_MESSAGES
} from '@melkens/shared';
import { v4 as uuidv4 } from 'uuid';

export class RobotSimulator extends EventEmitter {
  private robotState: RobotState;
  private simulationInterval?: NodeJS.Timeout;
  private routes: Map<number, Route> = new Map();
  private currentRoute?: Route;
  private currentStepIndex: number = 0;
  private stepStartTime: number = 0;
  private stepProgress: number = 0;
  private isExecutingStep: boolean = false;

  constructor() {
    super();
    this.robotState = this.initializeRobotState();
    this.initializeDefaultRoutes();
  }

  private initializeRobotState(): RobotState {
    return {
      id: uuidv4(),
      position: {
        x: SIMULATION_SETTINGS.MAP_SIZE.WIDTH / 2,
        y: SIMULATION_SETTINGS.MAP_SIZE.HEIGHT / 2,
        angle: 0,
        timestamp: Date.now()
      },
      motors: {
        leftSpeed: 0,
        rightSpeed: 0,
        leftDirection: MotorDirection.LEFT_FORWARD,
        rightDirection: MotorDirection.RIGHT_FORWARD,
        liftSpeed: 0,
        beltSpeed: 0,
        isRunning: false
      },
      sensors: {
        magneticPosition: 0,
        imuAngle: 0,
        encoderLeft: 0,
        encoderRight: 0,
        batteryVoltage: 24.5,
        temperature: 25.0,
        isConnected: true
      },
      isRunning: false,
      errors: [],
      lastUpdate: Date.now()
    };
  }

  private initializeDefaultRoutes(): void {
    const defaultRoute: Route = {
      id: 0,
      name: "Route A - Picking Line",
      steps: [
        { id: 1, operation: OperationType.NORM, distance: 2000, speed: 500, magnetCorrection: 0, description: "Start from base" },
        { id: 2, operation: OperationType.TU_L, distance: 1000, speed: 300, magnetCorrection: -5 * 2.17, description: "Turn left to shelf 1" },
        { id: 3, operation: OperationType.NORM, distance: 3000, speed: 500, magnetCorrection: 0, description: "Approach feeding table 1" },
        { id: 4, operation: OperationType.NORM, distance: 500, speed: 200, magnetCorrection: 0, description: "Precise positioning" },
        { id: 5, operation: OperationType.TU_R, distance: 1000, speed: 300, magnetCorrection: 5 * 2.17, description: "Turn right to shelf 2" },
        { id: 6, operation: OperationType.NORM, distance: 2000, speed: 500, magnetCorrection: 0, description: "Approach feeding table 2" },
        { id: 7, operation: OperationType.L_90, distance: 0, speed: 300, magnetCorrection: 0, angle: 90, description: "90° left turn" },
        { id: 8, operation: OperationType.NORM, distance: 1500, speed: 400, magnetCorrection: 0, description: "Return to base" }
      ],
      repeatCount: 5,
      isActive: false
    };
    this.routes.set(0, defaultRoute);
  }

  public start(): void {
    if (this.simulationInterval) {
      return;
    }

    this.simulationInterval = setInterval(() => {
      this.updateSimulation();
    }, SIMULATION_SETTINGS.UPDATE_RATE);

    this.log('info', 'RobotSimulator', 'Simulation started');
  }

  public stop(): void {
    if (this.simulationInterval) {
      clearInterval(this.simulationInterval);
      this.simulationInterval = undefined;
    }

    this.robotState.isRunning = false;
    this.robotState.motors.isRunning = false;
    this.currentRoute = undefined;
    this.currentStepIndex = 0;
    this.isExecutingStep = false;

    this.log('info', 'RobotSimulator', 'Simulation stopped');
  }

  public startRoute(routeId: number): boolean {
    const route = this.routes.get(routeId);
    if (!route) {
      this.log('error', 'RobotSimulator', `Route ${routeId} not found`);
      return false;
    }

    if (this.robotState.isRunning) {
      this.log('warning', 'RobotSimulator', 'Robot is already running');
      return false;
    }

    this.currentRoute = route;
    this.currentStepIndex = 0;
    this.robotState.isRunning = true;
    this.robotState.currentRoute = routeId;
    this.robotState.currentStep = 0;
    this.isExecutingStep = false;

    route.isActive = true;

    this.log('info', 'RobotSimulator', `Started route ${route.name}`);
    this.emit('route_started', { routeId, timestamp: Date.now() });

    return true;
  }

  public stopRoute(): void {
    if (this.currentRoute) {
      this.currentRoute.isActive = false;
      this.log('info', 'RobotSimulator', `Stopped route ${this.currentRoute.name}`);
    }

    this.robotState.isRunning = false;
    this.robotState.motors.isRunning = false;
    this.robotState.currentRoute = undefined;
    this.robotState.currentStep = undefined;
    this.currentRoute = undefined;
    this.currentStepIndex = 0;
    this.isExecutingStep = false;
  }

  public emergencyStop(): void {
    this.stopRoute();
    this.robotState.errors.push(ERROR_CODES.EMERGENCY_STOP);
    this.log('error', 'RobotSimulator', 'Emergency stop activated');
    this.emit('emergency_stop', { timestamp: Date.now() });
  }

  public getRobotState(): RobotState {
    return { ...this.robotState };
  }

  public getRoutes(): Route[] {
    return Array.from(this.routes.values());
  }

  public addRoute(route: Route): void {
    this.routes.set(route.id, route);
    this.log('info', 'RobotSimulator', `Added route ${route.name}`);
  }

  public updateRoute(route: Route): boolean {
    if (!this.routes.has(route.id)) {
      return false;
    }
    this.routes.set(route.id, route);
    this.log('info', 'RobotSimulator', `Updated route ${route.name}`);
    return true;
  }

  public deleteRoute(routeId: number): boolean {
    if (!this.routes.has(routeId)) {
      return false;
    }
    this.routes.delete(routeId);
    this.log('info', 'RobotSimulator', `Deleted route ${routeId}`);
    return true;
  }

  private updateSimulation(): void {
    this.updateSensors();
    
    if (this.robotState.isRunning && this.currentRoute) {
      this.executeCurrentStep();
    }

    this.robotState.lastUpdate = Date.now();
    this.emit('state_update', this.robotState);
  }

  private updateSensors(): void {
    const now = Date.now();
    
    // Simulate sensor noise and realistic values
    this.robotState.sensors.imuAngle = this.robotState.position.angle + (Math.random() - 0.5) * 0.1;
    this.robotState.sensors.magneticPosition = this.calculateMagneticPosition();
    this.robotState.sensors.temperature = 25 + (Math.random() - 0.5) * 5;
    this.robotState.sensors.batteryVoltage = Math.max(20, 24.5 - (Math.random() * 0.1));

    // Simulate encoder ticks based on movement
    if (this.robotState.motors.isRunning) {
      const deltaTime = (now - this.robotState.position.timestamp) / 1000;
      const distance = (this.robotState.motors.leftSpeed + this.robotState.motors.rightSpeed) / 2 * deltaTime * 0.01;
      this.robotState.sensors.encoderLeft += distance * 100; // ticks per mm
      this.robotState.sensors.encoderRight += distance * 100;
    }
  }

  private calculateMagneticPosition(): number {
    // Simulate magnetic line following with some noise
    const basePosition = this.robotState.position.x % 1000 - 500;
    return basePosition + (Math.random() - 0.5) * 20;
  }

  private executeCurrentStep(): void {
    if (!this.currentRoute || this.currentStepIndex >= this.currentRoute.steps.length) {
      this.completeRoute();
      return;
    }

    const currentStep = this.currentRoute.steps[this.currentStepIndex];
    
    if (!this.isExecutingStep) {
      this.startStep(currentStep);
    }

    const elapsed = Date.now() - this.stepStartTime;
    const stepDuration = this.calculateStepDuration(currentStep);
    
    this.stepProgress = Math.min(elapsed / stepDuration, 1.0);
    
    this.updateRobotMotion(currentStep, this.stepProgress);

    if (this.stepProgress >= 1.0) {
      this.completeStep(currentStep);
    }
  }

  private startStep(step: RouteStep): void {
    this.isExecutingStep = true;
    this.stepStartTime = Date.now();
    this.stepProgress = 0;
    this.robotState.currentStep = this.currentStepIndex;

    // Set motor speeds based on step
    this.robotState.motors.leftSpeed = step.speed;
    this.robotState.motors.rightSpeed = step.speed;
    this.robotState.motors.isRunning = true;

    this.log('info', 'RobotSimulator', `Starting step ${step.id}: ${step.description}`);
  }

  private calculateStepDuration(step: RouteStep): number {
    // Calculate duration based on operation type
    switch (step.operation) {
      case OperationType.L_90:
      case OperationType.R_90:
        return 3000; // 3 seconds for 90-degree turn
      case OperationType.TU_L:
      case OperationType.TU_R:
        return Math.max(2000, step.distance / step.speed * 1000);
      default:
        return step.distance / step.speed * 1000; // distance/speed * 1000ms
    }
  }

  private updateRobotMotion(step: RouteStep, progress: number): void {
    const deltaTime = SIMULATION_SETTINGS.UPDATE_RATE / 1000; // seconds
    const speed = step.speed * 0.01; // mm/s conversion factor

    switch (step.operation) {
      case OperationType.NORM:
      case OperationType.NORM_NOMAGNET:
        this.updateLinearMotion(speed, deltaTime);
        break;
      case OperationType.L_90:
        this.updateTurnMotion(-90, progress);
        break;
      case OperationType.R_90:
        this.updateTurnMotion(90, progress);
        break;
      case OperationType.TU_L:
        this.updateLinearMotion(speed, deltaTime);
        this.updateTurnMotion(-30, progress * 0.5);
        break;
      case OperationType.TU_R:
        this.updateLinearMotion(speed, deltaTime);
        this.updateTurnMotion(30, progress * 0.5);
        break;
    }

    this.robotState.position.timestamp = Date.now();
  }

  private updateLinearMotion(speed: number, deltaTime: number): void {
    const distance = speed * deltaTime;
    const angle = this.robotState.position.angle * Math.PI / 180;
    
    this.robotState.position.x += distance * Math.cos(angle);
    this.robotState.position.y += distance * Math.sin(angle);
  }

  private updateTurnMotion(targetAngle: number, progress: number): void {
    const currentAngle = this.robotState.position.angle;
    this.robotState.position.angle = currentAngle + (targetAngle * progress);
  }

  private completeStep(step: RouteStep): void {
    this.isExecutingStep = false;
    this.currentStepIndex++;
    
    this.log('info', 'RobotSimulator', `Completed step ${step.id}: ${step.description}`);

    if (this.currentStepIndex >= this.currentRoute!.steps.length) {
      this.completeRoute();
    }
  }

  private completeRoute(): void {
    if (!this.currentRoute) return;

    const route = this.currentRoute;
    route.isActive = false;
    
    this.robotState.isRunning = false;
    this.robotState.motors.isRunning = false;
    this.robotState.currentRoute = undefined;
    this.robotState.currentStep = undefined;

    this.log('info', 'RobotSimulator', `Completed route ${route.name}`);
    this.emit('route_completed', { 
      routeId: route.id, 
      success: true, 
      timestamp: Date.now() 
    });

    // Check if should repeat
    if (route.repeatCount > 1) {
      route.repeatCount--;
      setTimeout(() => {
        this.startRoute(route.id);
      }, 2000); // 2 second pause between repeats
    } else {
      this.currentRoute = undefined;
      this.currentStepIndex = 0;
    }
  }

  private log(level: LogEntry['level'], source: string, message: string, data?: any): void {
    const logEntry: LogEntry = {
      timestamp: Date.now(),
      level,
      source,
      message,
      data
    };
    
    this.emit('log', logEntry);
  }

  public resetPosition(): void {
    this.robotState.position = {
      x: SIMULATION_SETTINGS.MAP_SIZE.WIDTH / 2,
      y: SIMULATION_SETTINGS.MAP_SIZE.HEIGHT / 2,
      angle: 0,
      timestamp: Date.now()
    };
    
    this.robotState.sensors.encoderLeft = 0;
    this.robotState.sensors.encoderRight = 0;
    
    this.log('info', 'RobotSimulator', 'Robot position reset');
  }
}