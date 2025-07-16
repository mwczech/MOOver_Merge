import { EventEmitter } from 'events';
import { SerialPort } from 'serialport';
import { ReadlineParser } from '@serialport/parser-readline';
import * as fs from 'fs';
import * as path from 'path';

export interface IMUConfig {
  serialPort: string;
  baudRate: number;
  autoDetect: boolean;
  platform: 'windows' | 'linux' | 'darwin';
}

export interface IMUSensorData {
  accelerometer: {
    x: number;  // ax in g
    y: number;  // ay in g  
    z: number;  // az in g
  };
  gyroscope: {
    x: number;  // gx in rad/s
    y: number;  // gy in rad/s
    z: number;  // gz in rad/s
  };
  magnetometer: {
    x: number;  // mx in gauss
    y: number;  // my in gauss
    z: number;  // mz in gauss
  };
  ahrs: {
    roll: number;   // in degrees
    pitch: number;  // in degrees  
    yaw: number;    // in degrees
  };
  magnetBar: {
    status: number;        // 32-bit magnet status
    detectedMagnets: number[];  // array of detected magnet indices
  };
  timestamp: number;
  sequenceNumber: number;
  crcValid: boolean;
}

export interface IMUFaultInjection {
  enabled: boolean;
  accelerometerBias: { x: number; y: number; z: number };
  gyroscopeDrift: { x: number; y: number; z: number };
  magnetometerInterference: { x: number; y: number; z: number };
  stuckAxis: {
    accelerometer: string | null;  // 'x', 'y', 'z', or null
    gyroscope: string | null;
    magnetometer: string | null;
  };
  noiseLevel: number; // 0.0 to 1.0
}

export interface IMUProtocolFrame {
  startByte: number;
  messageType: number;
  dataLength: number;
  magnetBarStatus: number;
  accelerometer: { x: number; y: number; z: number };
  gyroscope: { x: number; y: number; z: number };
  magnetometer: { x: number; y: number; z: number };
  ahrs: { roll: number; pitch: number; yaw: number };
  sequenceNumber: number;
  crc: number;
}

export class IMUHardwareBridge extends EventEmitter {
  private config: IMUConfig;
  private serialPort?: SerialPort;
  private parser?: ReadlineParser;
  private isConnected = false;
  private isHardwareMode = false;
  private dataBuffer: Buffer = Buffer.alloc(0);
  private expectedFrameSize = 64; // Based on IMU protocol
  private sequenceNumber = 0;
  private lastValidData?: IMUSensorData;
  private faultInjection: IMUFaultInjection;
  private csvLogger?: fs.WriteStream;
  private logFilePath: string;

  // Default ports by platform
  private static DEFAULT_PORTS = {
    linux: '/dev/ttyUSB0',
    windows: 'COM3',
    darwin: '/dev/tty.usbserial'
  };

  constructor(config: Partial<IMUConfig> = {}) {
    super();
    
    this.config = {
      serialPort: this.getDefaultPort(),
      baudRate: 115200,
      autoDetect: true,
      platform: this.detectPlatform(),
      ...config
    };

    this.faultInjection = {
      enabled: false,
      accelerometerBias: { x: 0, y: 0, z: 0 },
      gyroscopeDrift: { x: 0, y: 0, z: 0 },
      magnetometerInterference: { x: 0, y: 0, z: 0 },
      stuckAxis: {
        accelerometer: null,
        gyroscope: null,
        magnetometer: null
      },
      noiseLevel: 0.0
    };

    this.logFilePath = path.join(process.cwd(), 'logs', 'hil_imu_test.csv');
    this.initializeLogging();
  }

  private detectPlatform(): 'windows' | 'linux' | 'darwin' {
    const platform = process.platform;
    if (platform === 'win32') return 'windows';
    if (platform === 'darwin') return 'darwin';
    return 'linux';
  }

  private getDefaultPort(): string {
    const platform = this.detectPlatform();
    return IMUHardwareBridge.DEFAULT_PORTS[platform];
  }

  private initializeLogging(): void {
    try {
      const logDir = path.dirname(this.logFilePath);
      if (!fs.existsSync(logDir)) {
        fs.mkdirSync(logDir, { recursive: true });
      }

      this.csvLogger = fs.createWriteStream(this.logFilePath, { flags: 'w' });
      
      // Write CSV header
      const header = [
        'timestamp',
        'sequence',
        'ax', 'ay', 'az',
        'gx', 'gy', 'gz', 
        'mx', 'my', 'mz',
        'roll', 'pitch', 'yaw',
        'magnetBarStatus',
        'detectedMagnets',
        'crcValid',
        'faultInjected'
      ].join(',') + '\n';
      
      this.csvLogger.write(header);
    } catch (error) {
      console.error('Failed to initialize CSV logging:', error);
    }
  }

  // Auto-detect available serial ports
  async detectAvailablePorts(): Promise<string[]> {
    try {
      const ports = await SerialPort.list();
      const availablePorts = ports
        .filter(port => port.path && (
          port.path.includes('USB') || 
          port.path.includes('ACM') ||
          port.path.includes('COM') ||
          port.vendorId || 
          port.productId
        ))
        .map(port => port.path);

      this.emit('ports_detected', availablePorts);
      return availablePorts;
    } catch (error) {
      this.emit('detection_error', error);
      return [];
    }
  }

  // Initialize IMU hardware connection
  async initialize(customPort?: string): Promise<void> {
    try {
      if (customPort) {
        this.config.serialPort = customPort;
      } else if (this.config.autoDetect) {
        const availablePorts = await this.detectAvailablePorts();
        if (availablePorts.length > 0) {
          this.config.serialPort = availablePorts[0];
        }
      }

      await this.connect();
      this.emit('initialized');
    } catch (error) {
      this.emit('initialization_error', error);
      throw error;
    }
  }

  private async connect(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.serialPort = new SerialPort({
        path: this.config.serialPort,
        baudRate: this.config.baudRate,
        autoOpen: false
      });

      this.setupEventHandlers();

      this.serialPort.open((err) => {
        if (err) {
          reject(new Error(`Failed to open IMU serial port ${this.config.serialPort}: ${err.message}`));
          return;
        }

        this.isConnected = true;
        this.emit('connected');
        resolve();
      });
    });
  }

  private setupEventHandlers(): void {
    if (!this.serialPort) return;

    this.serialPort.on('data', (data: Buffer) => {
      this.handleIncomingData(data);
    });

    this.serialPort.on('error', (error) => {
      this.isConnected = false;
      this.emit('error', error);
    });

    this.serialPort.on('close', () => {
      this.isConnected = false;
      this.emit('disconnected');
    });
  }

  private handleIncomingData(data: Buffer): void {
    this.dataBuffer = Buffer.concat([this.dataBuffer, data]);
    
    // Process complete frames
    while (this.dataBuffer.length >= this.expectedFrameSize) {
      try {
        const frame = this.parseIMUFrame(this.dataBuffer.slice(0, this.expectedFrameSize));
        if (frame) {
          let sensorData = this.convertFrameToSensorData(frame);
          
          // Apply fault injection if enabled
          if (this.faultInjection.enabled) {
            sensorData = this.applyFaultInjection(sensorData);
          }

          this.lastValidData = sensorData;
          this.logSensorData(sensorData);
          this.emit('imu_data', sensorData);
        }
        
        // Remove processed frame from buffer
        this.dataBuffer = this.dataBuffer.slice(this.expectedFrameSize);
      } catch (error) {
        // Skip to next potential frame start
        const nextStart = this.findNextFrameStart(this.dataBuffer);
        if (nextStart > 0) {
          this.dataBuffer = this.dataBuffer.slice(nextStart);
        } else {
          this.dataBuffer = this.dataBuffer.slice(1);
        }
        this.emit('parse_error', error);
      }
    }
  }

  private findNextFrameStart(buffer: Buffer): number {
    // Look for frame start pattern (0xAA 0x55 or similar)
    for (let i = 1; i < buffer.length - 1; i++) {
      if (buffer[i] === 0xAA && buffer[i + 1] === 0x55) {
        return i;
      }
    }
    return -1;
  }

  private parseIMUFrame(data: Buffer): IMUProtocolFrame | null {
    try {
      // Parse frame according to MELKENS IMU protocol
      // This is based on the MessageTypes.h structure
      
      let offset = 0;
      
      const frame: IMUProtocolFrame = {
        startByte: data.readUInt8(offset), // 0xAA
        messageType: data.readUInt8(offset + 1), // 0x55
        dataLength: data.readUInt8(offset + 2),
        magnetBarStatus: data.readUInt32LE(offset + 3),
        accelerometer: {
          x: data.readInt16LE(offset + 7) / 16000.0,   // Convert to g
          y: data.readInt16LE(offset + 9) / 16000.0,
          z: data.readInt16LE(offset + 11) / 16000.0
        },
        gyroscope: {
          x: data.readInt16LE(offset + 13) * 0.000285, // Convert to rad/s
          y: data.readInt16LE(offset + 15) * 0.000285,
          z: data.readInt16LE(offset + 17) * 0.000285
        },
        magnetometer: {
          x: data.readInt16LE(offset + 19) / 1000.0,   // Convert to gauss
          y: data.readInt16LE(offset + 21) / 1000.0,
          z: data.readInt16LE(offset + 23) / 1000.0
        },
        ahrs: {
          roll: data.readInt16LE(offset + 25) / 100.0,  // Convert to degrees
          pitch: data.readInt16LE(offset + 27) / 100.0,
          yaw: data.readInt16LE(offset + 29) / 100.0
        },
        sequenceNumber: data.readUInt16LE(offset + 31),
        crc: data.readUInt16LE(offset + 33)
      };

      // Validate frame
      if (frame.startByte !== 0xAA || frame.messageType !== 0x55) {
        return null;
      }

      // Validate CRC
      const calculatedCRC = this.calculateCRC16(data.slice(0, -2));
      if (frame.crc !== calculatedCRC) {
        this.emit('crc_error', { expected: frame.crc, calculated: calculatedCRC });
        return null;
      }

      return frame;
    } catch (error) {
      this.emit('frame_parse_error', error);
      return null;
    }
  }

  private calculateCRC16(data: Buffer): number {
    // CRC-16 calculation (same as used in MELKENS firmware)
    let crc = 0xFFFF;
    
    for (let i = 0; i < data.length; i++) {
      crc ^= data[i];
      for (let j = 0; j < 8; j++) {
        if (crc & 0x0001) {
          crc = (crc >> 1) ^ 0xA001;
        } else {
          crc = crc >> 1;
        }
      }
    }
    
    return crc;
  }

  private convertFrameToSensorData(frame: IMUProtocolFrame): IMUSensorData {
    // Extract detected magnets from magnet bar status
    const detectedMagnets: number[] = [];
    for (let i = 0; i < 32; i++) {
      if (frame.magnetBarStatus & (1 << i)) {
        detectedMagnets.push(i);
      }
    }

    return {
      accelerometer: frame.accelerometer,
      gyroscope: frame.gyroscope,
      magnetometer: frame.magnetometer,
      ahrs: frame.ahrs,
      magnetBar: {
        status: frame.magnetBarStatus,
        detectedMagnets
      },
      timestamp: Date.now(),
      sequenceNumber: frame.sequenceNumber,
      crcValid: true
    };
  }

  private applyFaultInjection(data: IMUSensorData): IMUSensorData {
    const faulted = { ...data };

    // Apply accelerometer bias
    faulted.accelerometer = {
      x: this.applyStuckAxis('accelerometer', 'x', data.accelerometer.x + this.faultInjection.accelerometerBias.x),
      y: this.applyStuckAxis('accelerometer', 'y', data.accelerometer.y + this.faultInjection.accelerometerBias.y),
      z: this.applyStuckAxis('accelerometer', 'z', data.accelerometer.z + this.faultInjection.accelerometerBias.z)
    };

    // Apply gyroscope drift
    faulted.gyroscope = {
      x: this.applyStuckAxis('gyroscope', 'x', data.gyroscope.x + this.faultInjection.gyroscopeDrift.x),
      y: this.applyStuckAxis('gyroscope', 'y', data.gyroscope.y + this.faultInjection.gyroscopeDrift.y),
      z: this.applyStuckAxis('gyroscope', 'z', data.gyroscope.z + this.faultInjection.gyroscopeDrift.z)
    };

    // Apply magnetometer interference
    faulted.magnetometer = {
      x: this.applyStuckAxis('magnetometer', 'x', data.magnetometer.x + this.faultInjection.magnetometerInterference.x),
      y: this.applyStuckAxis('magnetometer', 'y', data.magnetometer.y + this.faultInjection.magnetometerInterference.y),
      z: this.applyStuckAxis('magnetometer', 'z', data.magnetometer.z + this.faultInjection.magnetometerInterference.z)
    };

    // Apply noise
    if (this.faultInjection.noiseLevel > 0) {
      const noise = this.faultInjection.noiseLevel;
      
      faulted.accelerometer.x += (Math.random() - 0.5) * noise;
      faulted.accelerometer.y += (Math.random() - 0.5) * noise;
      faulted.accelerometer.z += (Math.random() - 0.5) * noise;
      
      faulted.gyroscope.x += (Math.random() - 0.5) * noise * 0.1;
      faulted.gyroscope.y += (Math.random() - 0.5) * noise * 0.1;
      faulted.gyroscope.z += (Math.random() - 0.5) * noise * 0.1;
      
      faulted.magnetometer.x += (Math.random() - 0.5) * noise * 10;
      faulted.magnetometer.y += (Math.random() - 0.5) * noise * 10;
      faulted.magnetometer.z += (Math.random() - 0.5) * noise * 10;
    }

    return faulted;
  }

  private applyStuckAxis(sensor: keyof IMUFaultInjection['stuckAxis'], axis: string, value: number): number {
    const stuckAxis = this.faultInjection.stuckAxis[sensor];
    if (stuckAxis === axis && this.lastValidData) {
      // Return last known value for stuck axis
      switch (sensor) {
        case 'accelerometer':
          return this.lastValidData.accelerometer[axis as keyof typeof this.lastValidData.accelerometer];
        case 'gyroscope':
          return this.lastValidData.gyroscope[axis as keyof typeof this.lastValidData.gyroscope];
        case 'magnetometer':
          return this.lastValidData.magnetometer[axis as keyof typeof this.lastValidData.magnetometer];
      }
    }
    return value;
  }

  private logSensorData(data: IMUSensorData): void {
    if (!this.csvLogger) return;

    try {
      const row = [
        data.timestamp,
        data.sequenceNumber,
        data.accelerometer.x.toFixed(6),
        data.accelerometer.y.toFixed(6),
        data.accelerometer.z.toFixed(6),
        data.gyroscope.x.toFixed(6),
        data.gyroscope.y.toFixed(6),
        data.gyroscope.z.toFixed(6),
        data.magnetometer.x.toFixed(6),
        data.magnetometer.y.toFixed(6),
        data.magnetometer.z.toFixed(6),
        data.ahrs.roll.toFixed(2),
        data.ahrs.pitch.toFixed(2),
        data.ahrs.yaw.toFixed(2),
        data.magnetBar.status,
        `"[${data.magnetBar.detectedMagnets.join(',')}]"`,
        data.crcValid,
        this.faultInjection.enabled
      ].join(',') + '\n';

      this.csvLogger.write(row);
    } catch (error) {
      console.error('Failed to log sensor data:', error);
    }
  }

  // Control methods
  enableHardwareMode(): void {
    this.isHardwareMode = true;
    this.emit('hardware_mode_enabled');
  }

  disableHardwareMode(): void {
    this.isHardwareMode = false;
    this.emit('hardware_mode_disabled');
  }

  isInHardwareMode(): boolean {
    return this.isHardwareMode && this.isConnected;
  }

  getConnectionStatus(): { connected: boolean; port: string; hardwareMode: boolean; lastData?: IMUSensorData } {
    return {
      connected: this.isConnected,
      port: this.config.serialPort,
      hardwareMode: this.isHardwareMode,
      lastData: this.lastValidData
    };
  }

  // Fault injection methods
  setFaultInjection(faults: Partial<IMUFaultInjection>): void {
    this.faultInjection = { ...this.faultInjection, ...faults };
    this.emit('fault_injection_updated', this.faultInjection);
  }

  getFaultInjection(): IMUFaultInjection {
    return { ...this.faultInjection };
  }

  clearFaultInjection(): void {
    this.faultInjection = {
      enabled: false,
      accelerometerBias: { x: 0, y: 0, z: 0 },
      gyroscopeDrift: { x: 0, y: 0, z: 0 },
      magnetometerInterference: { x: 0, y: 0, z: 0 },
      stuckAxis: {
        accelerometer: null,
        gyroscope: null,
        magnetometer: null
      },
      noiseLevel: 0.0
    };
    this.emit('fault_injection_cleared');
  }

  // Testing methods
  async testConnection(timeoutMs: number = 5000): Promise<boolean> {
    return new Promise((resolve) => {
      if (!this.isConnected) {
        resolve(false);
        return;
      }

      let dataReceived = false;
      
      const onData = () => {
        dataReceived = true;
        this.removeListener('imu_data', onData);
        resolve(true);
      };

      this.on('imu_data', onData);

      setTimeout(() => {
        this.removeListener('imu_data', onData);
        resolve(dataReceived);
      }, timeoutMs);
    });
  }

  // Cleanup
  async close(): Promise<void> {
    if (this.csvLogger) {
      this.csvLogger.end();
    }

    if (this.serialPort?.isOpen) {
      await new Promise<void>((resolve) => {
        this.serialPort!.close(() => resolve());
      });
    }

    this.isConnected = false;
    this.isHardwareMode = false;
    this.emit('closed');
  }

  // Get current sensor data (last received)
  getCurrentSensorData(): IMUSensorData | null {
    return this.lastValidData || null;
  }

  // Export log file path
  getLogFilePath(): string {
    return this.logFilePath;
  }
}