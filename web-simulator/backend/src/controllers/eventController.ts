import { Request, Response } from 'express';
import { eventInjector, InjectedEvent, EnvironmentEvent, FaultEvent } from '../services/EventInjector';
import { ApiResponse } from '@melkens/shared';

// Inject sensor value
export const injectSensorValue = async (req: Request, res: Response): Promise<void> => {
  try {
    const { sensorType, value, duration } = req.body;
    
    if (!sensorType || value === undefined) {
      res.status(400).json({
        success: false,
        error: 'sensorType and value are required'
      } as ApiResponse<null>);
      return;
    }
    
    const eventId = eventInjector.injectSensorValue(sensorType, value, duration);
    
    res.json({
      success: true,
      data: { eventId },
      message: `Sensor injection created: ${eventId}`
    } as ApiResponse<{ eventId: string }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Inject motor state
export const injectMotorState = async (req: Request, res: Response): Promise<void> => {
  try {
    const { motorId, state, duration } = req.body;
    
    if (!motorId || !state) {
      res.status(400).json({
        success: false,
        error: 'motorId and state are required'
      } as ApiResponse<null>);
      return;
    }
    
    const eventId = eventInjector.injectMotorState(motorId, state, duration);
    
    res.json({
      success: true,
      data: { eventId },
      message: `Motor injection created: ${eventId}`
    } as ApiResponse<{ eventId: string }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Inject environment event
export const injectEnvironmentEvent = async (req: Request, res: Response): Promise<void> => {
  try {
    const envEvent: EnvironmentEvent = req.body;
    
    if (!envEvent.type || envEvent.magnitude === undefined) {
      res.status(400).json({
        success: false,
        error: 'type and magnitude are required'
      } as ApiResponse<null>);
      return;
    }
    
    const eventId = eventInjector.injectEnvironmentEvent(envEvent);
    
    res.json({
      success: true,
      data: { eventId },
      message: `Environment event injected: ${eventId}`
    } as ApiResponse<{ eventId: string }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Inject fault
export const injectFault = async (req: Request, res: Response): Promise<void> => {
  try {
    const fault: FaultEvent = req.body;
    
    if (!fault.type || !fault.component || !fault.severity) {
      res.status(400).json({
        success: false,
        error: 'type, component, and severity are required'
      } as ApiResponse<null>);
      return;
    }
    
    const eventId = eventInjector.injectFault(fault);
    
    res.json({
      success: true,
      data: { eventId },
      message: `Fault injected: ${eventId}`
    } as ApiResponse<{ eventId: string }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Inject magnet shift (drag & drop simulation)
export const injectMagnetShift = async (req: Request, res: Response): Promise<void> => {
  try {
    const { fromPosition, toPosition } = req.body;
    
    if (!fromPosition || !toPosition) {
      res.status(400).json({
        success: false,
        error: 'fromPosition and toPosition are required'
      } as ApiResponse<null>);
      return;
    }
    
    const eventId = eventInjector.injectMagnetShift(fromPosition, toPosition);
    
    res.json({
      success: true,
      data: { eventId },
      message: `Magnet shift injected: ${eventId}`
    } as ApiResponse<{ eventId: string }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Emergency stop injection
export const injectEmergencyStop = async (req: Request, res: Response): Promise<void> => {
  try {
    const eventId = eventInjector.injectEmergencyStop();
    
    res.json({
      success: true,
      data: { eventId },
      message: `Emergency stop injected: ${eventId}`
    } as ApiResponse<{ eventId: string }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Get active events
export const getActiveEvents = async (req: Request, res: Response): Promise<void> => {
  try {
    const events = eventInjector.getActiveEvents();
    
    res.json({
      success: true,
      data: events,
      message: `Retrieved ${events.length} active events`
    } as ApiResponse<InjectedEvent[]>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Get event history
export const getEventHistory = async (req: Request, res: Response): Promise<void> => {
  try {
    const limit = req.query.limit ? parseInt(req.query.limit as string) : undefined;
    const events = eventInjector.getEventHistory(limit);
    
    res.json({
      success: true,
      data: events,
      message: `Retrieved ${events.length} historical events`
    } as ApiResponse<InjectedEvent[]>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Remove specific event
export const removeEvent = async (req: Request, res: Response): Promise<void> => {
  try {
    const { eventId } = req.params;
    
    const removed = eventInjector.removeEvent(eventId);
    
    if (removed) {
      res.json({
        success: true,
        data: { removed: true },
        message: `Event ${eventId} removed`
      } as ApiResponse<{ removed: boolean }>);
    } else {
      res.status(404).json({
        success: false,
        error: `Event ${eventId} not found`
      } as ApiResponse<null>);
    }
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Clear all events
export const clearAllEvents = async (req: Request, res: Response): Promise<void> => {
  try {
    eventInjector.clearAllEvents();
    
    res.json({
      success: true,
      data: { cleared: true },
      message: 'All events cleared'
    } as ApiResponse<{ cleared: boolean }>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};

// Export events for analysis
export const exportEvents = async (req: Request, res: Response): Promise<void> => {
  try {
    const exportData = eventInjector.exportEvents();
    
    res.json({
      success: true,
      data: exportData,
      message: 'Events exported successfully'
    } as ApiResponse<typeof exportData>);
  } catch (error) {
    res.status(500).json({
      success: false,
      error: error instanceof Error ? error.message : 'Unknown error'
    } as ApiResponse<null>);
  }
};