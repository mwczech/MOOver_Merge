import { Request, Response } from 'express';
import { RobotSimulator } from '../services/RobotSimulator';
import { 
  ApiResponse, 
  Route, 
  CreateRouteRequest, 
  UpdateRouteRequest,
  RouteId,
  OperationType 
} from '@melkens/shared';
import Joi from 'joi';

export class RoutesController {
  constructor(private robotSimulator: RobotSimulator) {}

  private routeStepSchema = Joi.object({
    operation: Joi.number().valid(...Object.values(OperationType)).required(),
    distance: Joi.number().min(0).required(),
    speed: Joi.number().min(0).max(1000).required(),
    magnetCorrection: Joi.number().required(),
    angle: Joi.number().min(-180).max(180).optional(),
    description: Joi.string().max(200).required()
  });

  private createRouteSchema = Joi.object({
    name: Joi.string().min(1).max(100).required(),
    steps: Joi.array().items(this.routeStepSchema).min(1).required(),
    repeatCount: Joi.number().min(1).max(100).default(1)
  });

  private updateRouteSchema = Joi.object({
    id: Joi.number().required(),
    name: Joi.string().min(1).max(100).optional(),
    steps: Joi.array().items(this.routeStepSchema).min(1).optional(),
    repeatCount: Joi.number().min(1).max(100).optional()
  });

  public getAllRoutes = async (req: Request, res: Response): Promise<void> => {
    try {
      const routes = this.robotSimulator.getRoutes();
      
      const response: ApiResponse = {
        success: true,
        data: routes,
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error getting routes:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to get routes',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public getRoute = async (req: Request, res: Response): Promise<void> => {
    try {
      const routeId = parseInt(req.params.id);
      
      if (isNaN(routeId)) {
        const response: ApiResponse = {
          success: false,
          error: 'Invalid route ID',
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      const routes = this.robotSimulator.getRoutes();
      const route = routes.find(r => r.id === routeId);

      if (!route) {
        const response: ApiResponse = {
          success: false,
          error: 'Route not found',
          timestamp: Date.now()
        };
        res.status(404).json(response);
        return;
      }

      const response: ApiResponse = {
        success: true,
        data: route,
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error getting route:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to get route',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public createRoute = async (req: Request, res: Response): Promise<void> => {
    try {
      const { error, value } = this.createRouteSchema.validate(req.body);
      
      if (error) {
        const response: ApiResponse = {
          success: false,
          error: `Validation error: ${error.details[0].message}`,
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      const createRequest: CreateRouteRequest = value;
      
      // Generate new route ID
      const existingRoutes = this.robotSimulator.getRoutes();
      const newId = existingRoutes.length > 0 
        ? Math.max(...existingRoutes.map(r => r.id)) + 1 
        : 1;

      // Create route steps with IDs
      const steps = createRequest.steps.map((step, index) => ({
        id: index + 1,
        ...step
      }));

      const newRoute: Route = {
        id: newId,
        name: createRequest.name,
        steps: steps,
        repeatCount: createRequest.repeatCount || 1,
        isActive: false
      };

      this.robotSimulator.addRoute(newRoute);

      const response: ApiResponse = {
        success: true,
        data: newRoute,
        timestamp: Date.now()
      };

      res.status(201).json(response);
    } catch (error) {
      console.error('Error creating route:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to create route',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public updateRoute = async (req: Request, res: Response): Promise<void> => {
    try {
      const routeId = parseInt(req.params.id);
      
      if (isNaN(routeId)) {
        const response: ApiResponse = {
          success: false,
          error: 'Invalid route ID',
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      const { error, value } = this.updateRouteSchema.validate({
        id: routeId,
        ...req.body
      });
      
      if (error) {
        const response: ApiResponse = {
          success: false,
          error: `Validation error: ${error.details[0].message}`,
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      const updateRequest: UpdateRouteRequest = value;
      
      // Get existing route
      const existingRoutes = this.robotSimulator.getRoutes();
      const existingRoute = existingRoutes.find(r => r.id === routeId);

      if (!existingRoute) {
        const response: ApiResponse = {
          success: false,
          error: 'Route not found',
          timestamp: Date.now()
        };
        res.status(404).json(response);
        return;
      }

      // Check if route is currently active
      if (existingRoute.isActive) {
        const response: ApiResponse = {
          success: false,
          error: 'Cannot update active route',
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      // Update route
      const updatedRoute: Route = {
        ...existingRoute,
        ...(updateRequest.name && { name: updateRequest.name }),
        ...(updateRequest.repeatCount && { repeatCount: updateRequest.repeatCount }),
        ...(updateRequest.steps && { 
          steps: updateRequest.steps.map((step, index) => ({
            id: index + 1,
            ...step
          }))
        })
      };

      const success = this.robotSimulator.updateRoute(updatedRoute);

      if (!success) {
        const response: ApiResponse = {
          success: false,
          error: 'Failed to update route',
          timestamp: Date.now()
        };
        res.status(500).json(response);
        return;
      }

      const response: ApiResponse = {
        success: true,
        data: updatedRoute,
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error updating route:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to update route',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public deleteRoute = async (req: Request, res: Response): Promise<void> => {
    try {
      const routeId = parseInt(req.params.id);
      
      if (isNaN(routeId)) {
        const response: ApiResponse = {
          success: false,
          error: 'Invalid route ID',
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      // Check if route exists and is not active
      const existingRoutes = this.robotSimulator.getRoutes();
      const existingRoute = existingRoutes.find(r => r.id === routeId);

      if (!existingRoute) {
        const response: ApiResponse = {
          success: false,
          error: 'Route not found',
          timestamp: Date.now()
        };
        res.status(404).json(response);
        return;
      }

      if (existingRoute.isActive) {
        const response: ApiResponse = {
          success: false,
          error: 'Cannot delete active route',
          timestamp: Date.now()
        };
        res.status(400).json(response);
        return;
      }

      const success = this.robotSimulator.deleteRoute(routeId);

      if (!success) {
        const response: ApiResponse = {
          success: false,
          error: 'Failed to delete route',
          timestamp: Date.now()
        };
        res.status(500).json(response);
        return;
      }

      const response: ApiResponse = {
        success: true,
        data: { deleted: true, routeId },
        timestamp: Date.now()
      };

      res.json(response);
    } catch (error) {
      console.error('Error deleting route:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to delete route',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };

  public executeRoute = async (req: Request, res: Response): Promise<void> => {
    try {
      const routeId = parseInt(req.params.id);
      
      if (isNaN(routeId)) {
        const response: ApiResponse = {
          success: false,
          error: 'Invalid route ID',
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
      console.error('Error executing route:', error);
      const response: ApiResponse = {
        success: false,
        error: 'Failed to execute route',
        timestamp: Date.now()
      };
      res.status(500).json(response);
    }
  };
}