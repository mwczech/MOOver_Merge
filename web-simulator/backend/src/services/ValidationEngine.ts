import { EventEmitter } from 'events';
import crypto from 'crypto';
import { RouteStep, RobotState, LogEntry } from '@melkens/shared';

export interface ValidationResult {
  id: string;
  timestamp: number;
  routeId: string;
  status: 'PASS' | 'FAIL' | 'WARNING';
  score: number; // 0-100
  checks: ValidationCheck[];
  duration: number;
  goldenRunComparison?: GoldenRunComparison;
  checksum: string;
  metadata: {
    robotId?: string;
    firmwareVersion?: string;
    testEnvironment: string;
    operator?: string;
  };
}

export interface ValidationCheck {
  id: string;
  name: string;
  category: 'safety' | 'performance' | 'accuracy' | 'compliance';
  status: 'pass' | 'fail' | 'warning' | 'skipped';
  message: string;
  expected?: any;
  actual?: any;
  tolerance?: number;
  weight: number; // Impact on overall score
}

export interface GoldenRun {
  id: string;
  routeId: string;
  timestamp: number;
  duration: number;
  logs: LogEntry[];
  finalState: RobotState;
  checkpoints: RouteCheckpoint[];
  metadata: {
    description: string;
    version: string;
    approved_by: string;
    certification_level: 'development' | 'testing' | 'production';
  };
}

export interface RouteCheckpoint {
  stepIndex: number;
  timestamp: number;
  position: { x: number; y: number };
  orientation: number;
  speed: number;
  sensors: any;
  tolerance: {
    position: number;
    orientation: number;
    timing: number;
  };
}

export interface GoldenRunComparison {
  similarity: number; // 0-100
  deviations: Deviation[];
  checkpointResults: CheckpointResult[];
  timingAnalysis: TimingAnalysis;
}

export interface Deviation {
  type: 'position' | 'timing' | 'sensor' | 'performance';
  severity: 'minor' | 'major' | 'critical';
  location: string;
  message: string;
  expectedValue: any;
  actualValue: any;
  deviation: number;
}

export interface CheckpointResult {
  checkpointId: string;
  passed: boolean;
  positionError: number;
  orientationError: number;
  timingError: number;
  message: string;
}

export interface TimingAnalysis {
  totalTime: number;
  expectedTime: number;
  variance: number;
  stepTimings: { stepIndex: number; actual: number; expected: number; variance: number }[];
}

export class ValidationEngine extends EventEmitter {
  private goldenRuns: Map<string, GoldenRun> = new Map();
  private validationResults: Map<string, ValidationResult> = new Map();
  private validationRules: ValidationCheck[] = [];

  constructor() {
    super();
    this.initializeDefaultRules();
  }

  // Initialize default validation rules
  private initializeDefaultRules(): void {
    this.validationRules = [
      {
        id: 'safety_emergency_stop',
        name: 'Emergency Stop Response',
        category: 'safety',
        status: 'skipped',
        message: 'Emergency stop must respond within 100ms',
        weight: 30
      },
      {
        id: 'safety_collision_avoidance',
        name: 'Collision Avoidance',
        category: 'safety',
        status: 'skipped',
        message: 'Must stop when obstacle detected within safety distance',
        weight: 25
      },
      {
        id: 'performance_position_accuracy',
        name: 'Position Accuracy',
        category: 'performance',
        status: 'skipped',
        message: 'Position error must be within ±5cm',
        tolerance: 0.05,
        weight: 20
      },
      {
        id: 'performance_speed_consistency',
        name: 'Speed Consistency',
        category: 'performance',
        status: 'skipped',
        message: 'Speed variation must be within ±10%',
        tolerance: 0.1,
        weight: 15
      },
      {
        id: 'accuracy_route_completion',
        name: 'Route Completion',
        category: 'accuracy',
        status: 'skipped',
        message: 'Route must be completed fully',
        weight: 10
      }
    ];
  }

  // Validate route execution
  async validateRoute(
    routeId: string,
    logs: LogEntry[],
    finalState: RobotState,
    metadata: any = {}
  ): Promise<ValidationResult> {
    const validationId = this.generateValidationId();
    const startTime = Date.now();
    
    try {
      // Run validation checks
      const checks = await this.runValidationChecks(logs, finalState);
      
      // Calculate overall score
      const score = this.calculateScore(checks);
      
      // Determine status
      const status = this.determineStatus(checks, score);
      
      // Compare with golden run if available
      let goldenRunComparison: GoldenRunComparison | undefined;
      const goldenRun = this.goldenRuns.get(routeId);
      if (goldenRun) {
        goldenRunComparison = await this.compareWithGoldenRun(goldenRun, logs, finalState);
      }
      
      const duration = Date.now() - startTime;
      
      const result: ValidationResult = {
        id: validationId,
        timestamp: Date.now(),
        routeId,
        status,
        score,
        checks,
        duration,
        goldenRunComparison,
        checksum: this.generateChecksum(logs, finalState),
        metadata: {
          testEnvironment: 'simulator',
          ...metadata
        }
      };
      
      this.validationResults.set(validationId, result);
      this.emit('validation_completed', result);
      
      return result;
    } catch (error) {
      this.emit('validation_error', { validationId, error });
      throw error;
    }
  }

  // Run individual validation checks
  private async runValidationChecks(logs: LogEntry[], finalState: RobotState): Promise<ValidationCheck[]> {
    const checks: ValidationCheck[] = [];
    
    for (const rule of this.validationRules) {
      const check = { ...rule };
      
      try {
        switch (rule.id) {
          case 'safety_emergency_stop':
            check.status = await this.checkEmergencyStopResponse(logs);
            break;
          case 'safety_collision_avoidance':
            check.status = await this.checkCollisionAvoidance(logs);
            break;
          case 'performance_position_accuracy':
            check.status = await this.checkPositionAccuracy(logs, rule.tolerance!);
            break;
          case 'performance_speed_consistency':
            check.status = await this.checkSpeedConsistency(logs, rule.tolerance!);
            break;
          case 'accuracy_route_completion':
            check.status = await this.checkRouteCompletion(logs, finalState);
            break;
          default:
            check.status = 'skipped';
        }
        
        if (check.status === 'pass') {
          check.message = `✓ ${check.message}`;
        } else if (check.status === 'fail') {
          check.message = `✗ ${check.message}`;
        } else if (check.status === 'warning') {
          check.message = `⚠ ${check.message}`;
        }
        
      } catch (error) {
        check.status = 'fail';
        check.message = `Error during check: ${error}`;
      }
      
      checks.push(check);
    }
    
    return checks;
  }

  // Check emergency stop response time
  private async checkEmergencyStopResponse(logs: LogEntry[]): Promise<'pass' | 'fail' | 'warning' | 'skipped'> {
    const emergencyLogs = logs.filter(log => 
      log.level === 'error' && log.message.includes('emergency')
    );
    
    if (emergencyLogs.length === 0) {
      return 'skipped'; // No emergency stops to test
    }
    
    // Check if stop occurred within 100ms
    for (const emergencyLog of emergencyLogs) {
      const stopLogs = logs.filter(log => 
        log.timestamp > emergencyLog.timestamp && 
        log.timestamp < emergencyLog.timestamp + 100 &&
        log.message.includes('stopped')
      );
      
      if (stopLogs.length === 0) {
        return 'fail';
      }
    }
    
    return 'pass';
  }

  // Check collision avoidance
  private async checkCollisionAvoidance(logs: LogEntry[]): Promise<'pass' | 'fail' | 'warning' | 'skipped'> {
    const obstacleDetections = logs.filter(log => 
      log.message.includes('obstacle') || log.message.includes('collision')
    );
    
    if (obstacleDetections.length === 0) {
      return 'skipped';
    }
    
    // Check if robot stopped appropriately
    for (const detection of obstacleDetections) {
      const stopResponse = logs.find(log => 
        log.timestamp > detection.timestamp && 
        log.timestamp < detection.timestamp + 500 &&
        log.message.includes('stopped')
      );
      
      if (!stopResponse) {
        return 'fail';
      }
    }
    
    return 'pass';
  }

  // Check position accuracy
  private async checkPositionAccuracy(logs: LogEntry[], tolerance: number): Promise<'pass' | 'fail' | 'warning' | 'skipped'> {
    const positionLogs = logs.filter(log => log.data?.position);
    
    if (positionLogs.length === 0) {
      return 'skipped';
    }
    
    let maxError = 0;
    for (const log of positionLogs) {
      if (log.data?.expectedPosition && log.data?.position) {
        const error = Math.sqrt(
          Math.pow(log.data.position.x - log.data.expectedPosition.x, 2) +
          Math.pow(log.data.position.y - log.data.expectedPosition.y, 2)
        );
        maxError = Math.max(maxError, error);
      }
    }
    
    if (maxError > tolerance) {
      return 'fail';
    } else if (maxError > tolerance * 0.8) {
      return 'warning';
    }
    
    return 'pass';
  }

  // Check speed consistency
  private async checkSpeedConsistency(logs: LogEntry[], tolerance: number): Promise<'pass' | 'fail' | 'warning' | 'skipped'> {
    const speedLogs = logs.filter(log => log.data?.speed);
    
    if (speedLogs.length === 0) {
      return 'skipped';
    }
    
    const speeds = speedLogs.map(log => log.data.speed);
    const avgSpeed = speeds.reduce((a, b) => a + b, 0) / speeds.length;
    const maxVariation = Math.max(...speeds.map(speed => Math.abs(speed - avgSpeed) / avgSpeed));
    
    if (maxVariation > tolerance) {
      return 'fail';
    } else if (maxVariation > tolerance * 0.8) {
      return 'warning';
    }
    
    return 'pass';
  }

  // Check route completion
  private async checkRouteCompletion(logs: LogEntry[], finalState: RobotState): Promise<'pass' | 'fail' | 'warning' | 'skipped'> {
    const completionLog = logs.find(log => log.message.includes('route completed'));
    return completionLog ? 'pass' : 'fail';
  }

  // Calculate overall validation score
  private calculateScore(checks: ValidationCheck[]): number {
    let weightedScore = 0;
    let totalWeight = 0;
    
    for (const check of checks) {
      if (check.status !== 'skipped') {
        let checkScore = 0;
        switch (check.status) {
          case 'pass': checkScore = 100; break;
          case 'warning': checkScore = 70; break;
          case 'fail': checkScore = 0; break;
        }
        
        weightedScore += checkScore * check.weight;
        totalWeight += check.weight;
      }
    }
    
    return totalWeight > 0 ? Math.round(weightedScore / totalWeight) : 0;
  }

  // Determine overall validation status
  private determineStatus(checks: ValidationCheck[], score: number): 'PASS' | 'FAIL' | 'WARNING' {
    const failedChecks = checks.filter(check => check.status === 'fail');
    const warningChecks = checks.filter(check => check.status === 'warning');
    
    if (failedChecks.length > 0 || score < 60) {
      return 'FAIL';
    } else if (warningChecks.length > 0 || score < 90) {
      return 'WARNING';
    }
    
    return 'PASS';
  }

  // Save golden run
  async saveGoldenRun(
    routeId: string,
    logs: LogEntry[],
    finalState: RobotState,
    checkpoints: RouteCheckpoint[],
    metadata: any
  ): Promise<string> {
    const goldenRunId = this.generateGoldenRunId();
    
    const goldenRun: GoldenRun = {
      id: goldenRunId,
      routeId,
      timestamp: Date.now(),
      duration: this.calculateDuration(logs),
      logs,
      finalState,
      checkpoints,
      metadata: {
        description: metadata.description || 'Golden run',
        version: metadata.version || '1.0',
        approved_by: metadata.approved_by || 'system',
        certification_level: metadata.certification_level || 'development'
      }
    };
    
    this.goldenRuns.set(routeId, goldenRun);
    this.emit('golden_run_saved', goldenRun);
    
    return goldenRunId;
  }

  // Compare with golden run
  private async compareWithGoldenRun(
    goldenRun: GoldenRun,
    logs: LogEntry[],
    finalState: RobotState
  ): Promise<GoldenRunComparison> {
    const deviations: Deviation[] = [];
    const checkpointResults: CheckpointResult[] = [];
    
    // Compare duration
    const actualDuration = this.calculateDuration(logs);
    const durationVariance = Math.abs(actualDuration - goldenRun.duration) / goldenRun.duration;
    
    if (durationVariance > 0.1) {
      deviations.push({
        type: 'timing',
        severity: durationVariance > 0.3 ? 'major' : 'minor',
        location: 'overall',
        message: `Duration variance: ${Math.round(durationVariance * 100)}%`,
        expectedValue: goldenRun.duration,
        actualValue: actualDuration,
        deviation: durationVariance
      });
    }
    
    // Compare checkpoints
    for (const checkpoint of goldenRun.checkpoints) {
      const result = this.compareCheckpoint(checkpoint, logs);
      checkpointResults.push(result);
      
      if (!result.passed) {
        deviations.push({
          type: 'position',
          severity: result.positionError > 0.1 ? 'major' : 'minor',
          location: `step_${checkpoint.stepIndex}`,
          message: `Checkpoint failed: ${result.message}`,
          expectedValue: checkpoint.position,
          actualValue: { error: result.positionError },
          deviation: result.positionError
        });
      }
    }
    
    // Calculate similarity
    const similarity = this.calculateSimilarity(deviations);
    
    const timingAnalysis: TimingAnalysis = {
      totalTime: actualDuration,
      expectedTime: goldenRun.duration,
      variance: durationVariance,
      stepTimings: [] // Would be populated with detailed step timing analysis
    };
    
    return {
      similarity,
      deviations,
      checkpointResults,
      timingAnalysis
    };
  }

  // Compare individual checkpoint
  private compareCheckpoint(checkpoint: RouteCheckpoint, logs: LogEntry[]): CheckpointResult {
    // Find corresponding log entry
    const correspondingLog = logs.find(log => 
      Math.abs(log.timestamp - checkpoint.timestamp) < checkpoint.tolerance.timing
    );
    
    if (!correspondingLog || !correspondingLog.data?.position) {
      return {
        checkpointId: `checkpoint_${checkpoint.stepIndex}`,
        passed: false,
        positionError: Infinity,
        orientationError: Infinity,
        timingError: Infinity,
        message: 'No corresponding log entry found'
      };
    }
    
    const positionError = Math.sqrt(
      Math.pow(correspondingLog.data.position.x - checkpoint.position.x, 2) +
      Math.pow(correspondingLog.data.position.y - checkpoint.position.y, 2)
    );
    
    const orientationError = Math.abs(correspondingLog.data.orientation - checkpoint.orientation);
    const timingError = Math.abs(correspondingLog.timestamp - checkpoint.timestamp);
    
    const passed = 
      positionError <= checkpoint.tolerance.position &&
      orientationError <= checkpoint.tolerance.orientation &&
      timingError <= checkpoint.tolerance.timing;
    
    return {
      checkpointId: `checkpoint_${checkpoint.stepIndex}`,
      passed,
      positionError,
      orientationError,
      timingError,
      message: passed ? 'Checkpoint passed' : 'Checkpoint failed'
    };
  }

  // Calculate similarity percentage
  private calculateSimilarity(deviations: Deviation[]): number {
    if (deviations.length === 0) return 100;
    
    const totalPenalty = deviations.reduce((sum, dev) => {
      const penalty = dev.severity === 'critical' ? 30 : 
                     dev.severity === 'major' ? 20 : 
                     dev.severity === 'minor' ? 10 : 0;
      return sum + penalty;
    }, 0);
    
    return Math.max(0, 100 - totalPenalty);
  }

  // Calculate duration from logs
  private calculateDuration(logs: LogEntry[]): number {
    if (logs.length === 0) return 0;
    return logs[logs.length - 1].timestamp - logs[0].timestamp;
  }

  // Generate validation ID
  private generateValidationId(): string {
    return `val_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }

  // Generate golden run ID
  private generateGoldenRunId(): string {
    return `golden_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }

  // Generate checksum
  private generateChecksum(logs: LogEntry[], finalState: RobotState): string {
    const data = JSON.stringify({ logs, finalState });
    return crypto.createHash('sha256').update(data).digest('hex');
  }

  // Get validation result
  getValidationResult(validationId: string): ValidationResult | undefined {
    return this.validationResults.get(validationId);
  }

  // Get golden run
  getGoldenRun(routeId: string): GoldenRun | undefined {
    return this.goldenRuns.get(routeId);
  }

  // Export validation results
  exportValidationResults(): { results: ValidationResult[]; goldenRuns: GoldenRun[] } {
    return {
      results: Array.from(this.validationResults.values()),
      goldenRuns: Array.from(this.goldenRuns.values())
    };
  }
}

export const validationEngine = new ValidationEngine();