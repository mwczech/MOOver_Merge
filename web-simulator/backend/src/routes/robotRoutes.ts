import { Router } from 'express';
import { RobotController } from '../controllers/robotController';
import { RobotSimulator } from '../services/RobotSimulator';

export function createRobotRoutes(robotSimulator: RobotSimulator): Router {
  const router = Router();
  const robotController = new RobotController(robotSimulator);

  // Get robot state
  router.get('/state', robotController.getState);

  // Robot control endpoints
  router.post('/control/start', robotController.startRoute);
  router.post('/control/stop', robotController.stopRobot);
  router.post('/control/emergency-stop', robotController.emergencyStop);
  router.post('/control/reset-position', robotController.resetPosition);

  // Get specific subsystem states
  router.get('/motors', robotController.getMotorState);
  router.get('/sensors', robotController.getSensorState);

  // Get diagnostics
  router.get('/diagnostics', robotController.getDiagnostics);

  return router;
}