import { EventEmitter } from 'events';
import { SensorState, MotorState } from '@melkens/shared';

export interface InjectedEvent {
  id: string;
  timestamp: number;
  type: 'sensor' | 'motor' | 'system' | 'environment';
  target: string;
  value: any;
  duration?: number; // milliseconds
  description: string;
}

export interface EnvironmentEvent {
  type: 'magnet_shift' | 'obstacle' | 'surface_change' | 'power_fluctuation';
  position?: { x: number; y: number };
  magnitude: number;
  duration?: number;
}

export interface FaultEvent {
  type: 'motor_block' | 'sensor_failure' | 'power_loss' | 'communication_error';
  component: string;
  severity: 'low' | 'medium' | 'high' | 'critical';
  recovery_time?: number;
}

export class EventInjector extends EventEmitter {
  private activeEvents: Map<string, InjectedEvent> = new Map();
  private eventHistory: InjectedEvent[] = [];
  private maxHistorySize = 1000;

  constructor() {
    super();
  }

  // Inject sensor value override
  injectSensorValue(sensorType: keyof SensorState, value: any, duration?: number): string {
    const event: InjectedEvent = {
      id: this.generateEventId(),
      timestamp: Date.now(),
      type: 'sensor',
      target: sensorType,
      value,
      duration,
      description: `Injected ${sensorType} value: ${JSON.stringify(value)}`
    };

    this.addEvent(event);
    this.emit('sensor_injection', { sensorType, value, duration });
    
    if (duration) {
      setTimeout(() => this.removeEvent(event.id), duration);
    }

    return event.id;
  }

  // Inject motor state override
  injectMotorState(motorId: string, state: Partial<MotorState>, duration?: number): string {
    const event: InjectedEvent = {
      id: this.generateEventId(),
      timestamp: Date.now(),
      type: 'motor',
      target: motorId,
      value: state,
      duration,
      description: `Injected motor ${motorId} state: ${JSON.stringify(state)}`
    };

    this.addEvent(event);
    this.emit('motor_injection', { motorId, state, duration });
    
    if (duration) {
      setTimeout(() => this.removeEvent(event.id), duration);
    }

    return event.id;
  }

  // Inject environment change (magnet shift, obstacles, etc.)
  injectEnvironmentEvent(envEvent: EnvironmentEvent): string {
    const event: InjectedEvent = {
      id: this.generateEventId(),
      timestamp: Date.now(),
      type: 'environment',
      target: envEvent.type,
      value: envEvent,
      duration: envEvent.duration,
      description: `Environment event: ${envEvent.type} at ${JSON.stringify(envEvent.position)} with magnitude ${envEvent.magnitude}`
    };

    this.addEvent(event);
    this.emit('environment_injection', envEvent);
    
    if (envEvent.duration) {
      setTimeout(() => this.removeEvent(event.id), envEvent.duration);
    }

    return event.id;
  }

  // Inject system fault
  injectFault(fault: FaultEvent): string {
    const event: InjectedEvent = {
      id: this.generateEventId(),
      timestamp: Date.now(),
      type: 'system',
      target: fault.component,
      value: fault,
      duration: fault.recovery_time,
      description: `Fault injected: ${fault.type} on ${fault.component} (${fault.severity})`
    };

    this.addEvent(event);
    this.emit('fault_injection', fault);
    
    if (fault.recovery_time) {
      setTimeout(() => this.removeEvent(event.id), fault.recovery_time);
    }

    return event.id;
  }

  // Magnet position manipulation (drag & drop simulation)
  injectMagnetShift(fromPosition: { x: number; y: number }, toPosition: { x: number; y: number }): string {
    const envEvent: EnvironmentEvent = {
      type: 'magnet_shift',
      position: toPosition,
      magnitude: Math.sqrt(Math.pow(toPosition.x - fromPosition.x, 2) + Math.pow(toPosition.y - fromPosition.y, 2))
    };

    return this.injectEnvironmentEvent(envEvent);
  }

  // Emergency stop injection
  injectEmergencyStop(): string {
    const fault: FaultEvent = {
      type: 'communication_error',
      component: 'emergency_system',
      severity: 'critical'
    };

    return this.injectFault(fault);
  }

  // Get active injected event for specific target
  getActiveEvent(target: string, type?: string): InjectedEvent | undefined {
    for (const event of this.activeEvents.values()) {
      if (event.target === target && (!type || event.type === type)) {
        return event;
      }
    }
    return undefined;
  }

  // Remove specific event
  removeEvent(eventId: string): boolean {
    const event = this.activeEvents.get(eventId);
    if (event) {
      this.activeEvents.delete(eventId);
      this.emit('event_removed', event);
      return true;
    }
    return false;
  }

  // Clear all events
  clearAllEvents(): void {
    const removedEvents = Array.from(this.activeEvents.values());
    this.activeEvents.clear();
    this.emit('events_cleared', removedEvents);
  }

  // Get all active events
  getActiveEvents(): InjectedEvent[] {
    return Array.from(this.activeEvents.values());
  }

  // Get event history
  getEventHistory(limit?: number): InjectedEvent[] {
    const history = [...this.eventHistory].reverse();
    return limit ? history.slice(0, limit) : history;
  }

  // Export events for analysis
  exportEvents(): { active: InjectedEvent[], history: InjectedEvent[] } {
    return {
      active: this.getActiveEvents(),
      history: this.getEventHistory()
    };
  }

  private addEvent(event: InjectedEvent): void {
    this.activeEvents.set(event.id, event);
    this.eventHistory.push(event);
    
    // Maintain history size limit
    if (this.eventHistory.length > this.maxHistorySize) {
      this.eventHistory = this.eventHistory.slice(-this.maxHistorySize);
    }

    this.emit('event_added', event);
  }

  private generateEventId(): string {
    return `evt_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }
}

export const eventInjector = new EventInjector();