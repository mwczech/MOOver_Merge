import { Request, Response } from 'express';
import { HardwareBridge } from '../services/HardwareBridge';
import { IMUHardwareBridge } from '../services/IMUHardwareBridge';

export class IMUController {
  private hardwareBridge: HardwareBridge;

  constructor(hardwareBridge: HardwareBridge) {
    this.hardwareBridge = hardwareBridge;
  }

  // Get IMU connection status
  getStatus = async (req: Request, res: Response): Promise<void> => {
    try {
      const status = this.hardwareBridge.getIMUStatus();
      res.json({
        success: true,
        data: status
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to get IMU status'
      });
    }
  };

  // Connect to IMU hardware
  connect = async (req: Request, res: Response): Promise<void> => {
    try {
      const { port } = req.body;
      
      await this.hardwareBridge.connectIMU(port);
      
      res.json({
        success: true,
        message: `IMU connected${port ? ` on ${port}` : ''}`
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to connect to IMU'
      });
    }
  };

  // Disconnect from IMU hardware
  disconnect = async (req: Request, res: Response): Promise<void> => {
    try {
      await this.hardwareBridge.disconnectIMU();
      
      res.json({
        success: true,
        message: 'IMU disconnected'
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to disconnect IMU'
      });
    }
  };

  // Enable hardware-in-the-loop mode
  enableHardwareMode = async (req: Request, res: Response): Promise<void> => {
    try {
      this.hardwareBridge.enableIMUHardwareMode();
      
      res.json({
        success: true,
        message: 'IMU hardware mode enabled'
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to enable hardware mode'
      });
    }
  };

  // Disable hardware-in-the-loop mode
  disableHardwareMode = async (req: Request, res: Response): Promise<void> => {
    try {
      this.hardwareBridge.disableIMUHardwareMode();
      
      res.json({
        success: true,
        message: 'IMU hardware mode disabled'
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to disable hardware mode'
      });
    }
  };

  // Get current IMU sensor data
  getSensorData = async (req: Request, res: Response): Promise<void> => {
    try {
      const data = this.hardwareBridge.getCurrentIMUData();
      
      if (!data) {
        res.status(404).json({
          success: false,
          error: 'No IMU data available'
        });
        return;
      }

      res.json({
        success: true,
        data
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to get sensor data'
      });
    }
  };

  // Detect available serial ports
  detectPorts = async (req: Request, res: Response): Promise<void> => {
    try {
      const ports = await this.hardwareBridge.detectIMUPorts();
      
      res.json({
        success: true,
        data: {
          availablePorts: ports,
          recommended: ports.length > 0 ? ports[0] : null
        }
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to detect ports'
      });
    }
  };

  // Test IMU connection
  testConnection = async (req: Request, res: Response): Promise<void> => {
    try {
      const { timeout = 5000 } = req.query;
      const timeoutMs = parseInt(timeout as string, 10);
      
      const isConnected = await this.hardwareBridge.testIMUConnection(timeoutMs);
      
      res.json({
        success: true,
        data: {
          connected: isConnected,
          message: isConnected 
            ? 'IMU connection test successful' 
            : 'IMU connection test failed - no data received within timeout'
        }
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Connection test failed'
      });
    }
  };

  // Set fault injection parameters
  setFaultInjection = async (req: Request, res: Response): Promise<void> => {
    try {
      const faultConfig = req.body;
      
      // Validate fault injection configuration
      if (typeof faultConfig !== 'object') {
        res.status(400).json({
          success: false,
          error: 'Invalid fault injection configuration'
        });
        return;
      }

      this.hardwareBridge.setIMUFaultInjection(faultConfig);
      
      res.json({
        success: true,
        message: 'Fault injection configuration updated',
        data: this.hardwareBridge.getIMUFaultInjection()
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to set fault injection'
      });
    }
  };

  // Get current fault injection settings
  getFaultInjection = async (req: Request, res: Response): Promise<void> => {
    try {
      const faultConfig = this.hardwareBridge.getIMUFaultInjection();
      
      res.json({
        success: true,
        data: faultConfig
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to get fault injection settings'
      });
    }
  };

  // Clear fault injection
  clearFaultInjection = async (req: Request, res: Response): Promise<void> => {
    try {
      this.hardwareBridge.clearIMUFaultInjection();
      
      res.json({
        success: true,
        message: 'Fault injection cleared'
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to clear fault injection'
      });
    }
  };

  // Get log file path
  getLogPath = async (req: Request, res: Response): Promise<void> => {
    try {
      const logPath = this.hardwareBridge.getIMULogFilePath();
      
      res.json({
        success: true,
        data: {
          logPath,
          exists: logPath ? true : false
        }
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to get log path'
      });
    }
  };

  // Download log file
  downloadLog = async (req: Request, res: Response): Promise<void> => {
    try {
      const logPath = this.hardwareBridge.getIMULogFilePath();
      
      if (!logPath) {
        res.status(404).json({
          success: false,
          error: 'Log file not available'
        });
        return;
      }

      res.download(logPath, 'hil_imu_test.csv', (err) => {
        if (err) {
          console.error('Failed to download log file:', err);
          if (!res.headersSent) {
            res.status(500).json({
              success: false,
              error: 'Failed to download log file'
            });
          }
        }
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to download log file'
      });
    }
  };

  // Predefined fault injection scenarios
  applyFaultScenario = async (req: Request, res: Response): Promise<void> => {
    try {
      const { scenario } = req.params;
      
      let faultConfig;
      
      switch (scenario) {
        case 'gyro_drift':
          faultConfig = {
            enabled: true,
            gyroscopeDrift: { x: 0.1, y: 0.05, z: -0.08 },
            noiseLevel: 0.1
          };
          break;
          
        case 'accel_bias':
          faultConfig = {
            enabled: true,
            accelerometerBias: { x: 0.2, y: -0.15, z: 0.1 },
            noiseLevel: 0.05
          };
          break;
          
        case 'mag_interference':
          faultConfig = {
            enabled: true,
            magnetometerInterference: { x: 50, y: -30, z: 20 },
            noiseLevel: 0.2
          };
          break;
          
        case 'stuck_accel_x':
          faultConfig = {
            enabled: true,
            stuckAxis: { accelerometer: 'x', gyroscope: null, magnetometer: null }
          };
          break;
          
        case 'stuck_gyro_z':
          faultConfig = {
            enabled: true,
            stuckAxis: { accelerometer: null, gyroscope: 'z', magnetometer: null }
          };
          break;
          
        case 'high_noise':
          faultConfig = {
            enabled: true,
            noiseLevel: 0.5
          };
          break;
          
        case 'combined_faults':
          faultConfig = {
            enabled: true,
            accelerometerBias: { x: 0.1, y: 0.1, z: 0.1 },
            gyroscopeDrift: { x: 0.05, y: 0.05, z: 0.05 },
            magnetometerInterference: { x: 20, y: 20, z: 20 },
            noiseLevel: 0.15
          };
          break;
          
        default:
          res.status(400).json({
            success: false,
            error: `Unknown fault scenario: ${scenario}`,
            availableScenarios: [
              'gyro_drift', 'accel_bias', 'mag_interference',
              'stuck_accel_x', 'stuck_gyro_z', 'high_noise', 'combined_faults'
            ]
          });
          return;
      }

      this.hardwareBridge.setIMUFaultInjection(faultConfig);
      
      res.json({
        success: true,
        message: `Fault scenario '${scenario}' applied`,
        data: faultConfig
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to apply fault scenario'
      });
    }
  };

  // Get hardware mode status
  getHardwareMode = async (req: Request, res: Response): Promise<void> => {
    try {
      const isHardwareMode = this.hardwareBridge.isIMUInHardwareMode();
      
      res.json({
        success: true,
        data: {
          hardwareMode: isHardwareMode,
          message: isHardwareMode 
            ? 'IMU is in hardware mode - using real sensor data'
            : 'IMU is in simulation mode - using simulated data'
        }
      });
    } catch (error: any) {
      res.status(500).json({
        success: false,
        error: error.message || 'Failed to get hardware mode status'
      });
    }
  };
}