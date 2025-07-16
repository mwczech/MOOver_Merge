import { Request, Response } from 'express';
import { RobotSimulator } from '../services/RobotSimulator';
import { ApiResponse } from '@melkens/shared';

export class RobotController {
  constructor(private robotSimulator: RobotSimulator) {}

  public getState = async (req: Request, res: Response): Promise<void> => {
    try {
      const robotState = this.robotSimulator.getRobotState();
      
      const response: ApiResponse = {
        success: true,
        data: robotState,
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error getting robot state:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to get robot state',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public startRoute = async (req: Request, res: Response): Promise<void> => {
    try {
      const { routeId } = req.body;

      if (typeof routeId !== 'number') {
        const response: ApiResponse = {
          success: false,
          error: 'Invalid routeId - must be a number',
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      const success = this.robotSimulator.startRoute(routeId);
      
      if (!success) {
        const response: ApiResponse = {
          success: false,
          error: 'Failed to start route - robot may be running or route not found',
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      const response: ApiResponse = {
        success: true,
        data: { routeId, started: true },
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error starting route:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to start route',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public stopRobot = async (req: Request, res: Response): Promise<void> => {
    try {
      this.robotSimulator.stopRoute();
      
      const response: ApiResponse = {
        success: true,
        data: { stopped: true },
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error stopping robot:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to stop robot',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public emergencyStop = async (req: Request, res: Response): Promise<void> => {
    try {
      this.robotSimulator.emergencyStop();
      
      const response: ApiResponse = {
        success: true,
        data: { emergencyStop: true },
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error emergency stop:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to emergency stop',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public resetPosition = async (req: Request, res: Response): Promise<void> => {
    try {
      this.robotSimulator.resetPosition();
      
      const response: ApiResponse = {
        success: true,
        data: { resetPosition: true },
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error resetting position:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to reset position',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public getMotorState = async (req: Request, res: Response): Promise<void> => {
    try {
      const robotState = this.robotSimulator.getRobotState();
      
      const response: ApiResponse = {
        success: true,
        data: robotState.motors,
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error getting motor state:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to get motor state',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public getSensorState = async (req: Request, res: Response): Promise<void> => {
    try {
      const robotState = this.robotSimulator.getRobotState();
      
      const response: ApiResponse = {
        success: true,
        data: robotState.sensors,
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error getting sensor state:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to get sensor state',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public getDiagnostics = async (req: Request, res: Response): Promise<void> => {
    try {
      const robotState = this.robotSimulator.getRobotState();
      
      const diagnostics = {
        robotId: robotState.id,
        uptime: Date.now() - (robotState.lastUpdate - 60000), // Mock uptime
        errors: robotState.errors,
        isRunning: robotState.isRunning,
        currentRoute: robotState.currentRoute,
        currentStep: robotState.currentStep,
        motorsRunning: robotState.motors.isRunning,
        sensorsConnected: robotState.sensors.isConnected,
        batteryLevel: robotState.sensors.batteryVoltage,
        temperature: robotState.sensors.temperature,
        lastUpdate: robotState.lastUpdate
      };

      const response: ApiResponse = {
        success: true,
        data: diagnostics,
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error getting diagnostics:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to get diagnostics',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };
}