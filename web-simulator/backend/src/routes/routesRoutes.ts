import { Router } from 'express';
import { RoutesController } from '../controllers/routesController';
import { RobotSimulator } from '../services/RobotSimulator';

export function createRoutesRoutes(robotSimulator: RobotSimulator): Router {
  const router = Router();
  const routesController = new RoutesController(robotSimulator);

  // CRUD operations for routes
  router.get('/', routesController.getAllRoutes);
  router.get('/:id', routesController.getRoute);
  router.post('/', routesController.createRoute);
  router.put('/:id', routesController.updateRoute);
  router.delete('/:id', routesController.deleteRoute);

  // Execute route
  router.post('/:id/execute', routesController.executeRoute);

  return router;
}