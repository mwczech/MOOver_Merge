import { Router } from 'express';
import { IMUController } from '../controllers/imuController';
import { HardwareBridge } from '../services/HardwareBridge';

export function createIMURoutes(hardwareBridge: HardwareBridge): Router {
  const router = Router();
  const imuController = new IMUController(hardwareBridge);

  // IMU connection management
  router.get('/status', imuController.getStatus);
  router.post('/connect', imuController.connect);
  router.post('/disconnect', imuController.disconnect);
  router.get('/ports/detect', imuController.detectPorts);
  router.get('/test', imuController.testConnection);

  // Hardware mode control
  router.get('/hardware-mode', imuController.getHardwareMode);
  router.post('/hardware-mode/enable', imuController.enableHardwareMode);
  router.post('/hardware-mode/disable', imuController.disableHardwareMode);

  // Sensor data
  router.get('/data', imuController.getSensorData);

  // Fault injection
  router.get('/fault-injection', imuController.getFaultInjection);
  router.post('/fault-injection', imuController.setFaultInjection);
  router.delete('/fault-injection', imuController.clearFaultInjection);
  router.post('/fault-injection/scenario/:scenario', imuController.applyFaultScenario);

  // Logging
  router.get('/log/path', imuController.getLogPath);
  router.get('/log/download', imuController.downloadLog);

  return router;
}