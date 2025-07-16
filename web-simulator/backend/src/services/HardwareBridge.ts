import { EventEmitter } from 'events';
import { SerialPort } from 'serialport';
import { ReadlineParser } from '@serialport/parser-readline';
import { RobotState, SensorState, MotorState, RouteStep } from '@melkens/shared';
import { IMUHardwareBridge, IMUSensorData, IMUConfig } from './IMUHardwareBridge';

export interface HardwareConfig {
  serialPort?: string;
  baudRate: number;
  canInterface?: string;
  tcpPort?: number;
  usbVendorId?: string;
  usbProductId?: string;
  imu?: {
    enabled: boolean;
    serialPort?: string;
    baudRate?: number;
    autoDetect?: boolean;
  };
}

export interface HardwareMessage {
  timestamp: number;
  source: 'uart' | 'can' | 'usb' | 'tcp';
  type: 'sensor_data' | 'motor_status' | 'system_status' | 'command_response';
  data: any;
  raw?: Buffer;
}

export interface PhysicalSensorData {
  magneticPosition: { x: number; y: number; detected: boolean };
  imuData: { ax: number; ay: number; az: number; gx: number; gy: number; gz: number };
  encoders: { left: number; right: number };
  battery: { voltage: number; current: number; percentage: number };
  temperature: number[];
  ultrasonicDistances: number[];
}

export interface HardwareStatus {
  connected: boolean;
  mode: string;
  config: HardwareConfig;
  imu: {
    connected: boolean;
    hardwareMode: boolean;
    port: string;
    lastDataTimestamp?: number;
    error?: string;
  };
}

export interface SensorReadings {
  magneticPosition?: { x: number; y: number; detected: boolean };
  imu?: IMUSensorData;
  encoders?: { left: number; right: number };
  battery?: { voltage: number; current: number; percentage: number };
  temperature?: number[];
  ultrasonicDistances?: number[];
}

export class HardwareBridge extends EventEmitter {
  private config: HardwareConfig;
  private serialPort?: SerialPort;
  private parser?: ReadlineParser;
  private isConnected = false;
  private messageBuffer: HardwareMessage[] = [];
  private maxBufferSize = 10000;
  private imuBridge?: IMUHardwareBridge;
  private imuUpdateInterval?: NodeJS.Timeout;

  // Hardware abstraction layer flags
  private hilMode = false; // Hardware-in-the-loop mode
  private virtualMode = true; // Virtual simulation mode
  private mixedMode = false; // Mixed virtual/physical mode

  constructor(config: HardwareConfig = {}) {
    super();
    this.config = {
      serialPort: undefined,
      baudRate: 9600,
      canInterface: undefined,
      tcpPort: undefined,
      usbVendorId: undefined,
      usbProductId: undefined,
      imu: {
        enabled: false,
        baudRate: 115200,
        autoDetect: true,
        ...config.imu
      }
    };
  }

  // Initialize hardware connections
  async initialize(): Promise<void> {
    try {
      if (this.config.serialPort) {
        await this.initializeSerial();
      }
      
      if (this.config.canInterface) {
        await this.initializeCAN();
      }
      
      if (this.config.tcpPort) {
        await this.initializeTCP();
      }

      // Initialize IMU bridge if enabled
      if (this.config.imu?.enabled) {
        this.initializeIMU();
      }

      this.emit('hardware_ready');
    } catch (error) {
      this.emit('hardware_error', error);
      throw error;
    }
  }

  // Initialize serial communication (UART)
  private async initializeSerial(): Promise<void> {
    return new Promise((resolve, reject) => {
      this.serialPort = new SerialPort({
        path: this.config.serialPort!,
        baudRate: this.config.baudRate,
        autoOpen: false
      });

      this.parser = this.serialPort.pipe(new ReadlineParser({ delimiter: '\n' }));

      this.serialPort.open((err) => {
        if (err) {
          reject(new Error(`Failed to open serial port: ${err.message}`));
          return;
        }

        this.isConnected = true;
        this.setupSerialHandlers();
        resolve();
      });
    });
  }

  // Setup serial communication handlers
  private setupSerialHandlers(): void {
    if (!this.parser || !this.serialPort) return;

    this.parser.on('data', (data: string) => {
      try {
        const message = this.parseHardwareMessage(data, 'uart');
        this.addMessage(message);
        this.processHardwareMessage(message);
      } catch (error) {
        this.emit('parse_error', { data, error });
      }
    });

    this.serialPort.on('error', (error) => {
      this.emit('hardware_error', error);
      this.isConnected = false;
    });

    this.serialPort.on('close', () => {
      this.isConnected = false;
      this.emit('connection_lost');
    });
  }

  // Initialize CAN bus communication
  private async initializeCAN(): Promise<void> {
    // CAN implementation would go here
    // This is a placeholder for CAN bus integration
    console.log(`Initializing CAN interface: ${this.config.canInterface}`);
    // Implementation depends on specific CAN hardware/library
  }

  // Initialize TCP/IP communication
  private async initializeTCP(): Promise<void> {
    // TCP server implementation would go here
    console.log(`Initializing TCP server on port: ${this.config.tcpPort}`);
    // Implementation for TCP communication with embedded systems
  }

  // Parse incoming hardware message
  private parseHardwareMessage(data: string, source: 'uart' | 'can' | 'usb' | 'tcp'): HardwareMessage {
    // Parse protocol-specific message format
    // This is a simplified parser - real implementation would depend on actual protocol
    const lines = data.trim().split('\n');
    const parsed: any = {};

    for (const line of lines) {
      if (line.startsWith('SENSOR:')) {
        parsed.type = 'sensor_data';
        parsed.data = JSON.parse(line.substring(7));
      } else if (line.startsWith('MOTOR:')) {
        parsed.type = 'motor_status';
        parsed.data = JSON.parse(line.substring(6));
      } else if (line.startsWith('SYSTEM:')) {
        parsed.type = 'system_status';
        parsed.data = JSON.parse(line.substring(7));
      } else if (line.startsWith('ACK:') || line.startsWith('NACK:')) {
        parsed.type = 'command_response';
        parsed.data = { response: line };
      }
    }

    return {
      timestamp: Date.now(),
      source,
      type: parsed.type || 'system_status',
      data: parsed.data || { raw: data }
    };
  }

  // Process incoming hardware messages
  private processHardwareMessage(message: HardwareMessage): void {
    switch (message.type) {
      case 'sensor_data':
        this.handleSensorData(message.data);
        break;
      case 'motor_status':
        this.handleMotorStatus(message.data);
        break;
      case 'system_status':
        this.handleSystemStatus(message.data);
        break;
      case 'command_response':
        this.handleCommandResponse(message.data);
        break;
    }

    this.emit('hardware_message', message);
  }

  // Handle sensor data from hardware
  private handleSensorData(data: PhysicalSensorData): void {
    this.emit('physical_sensor_data', data);
  }

  // Handle motor status from hardware
  private handleMotorStatus(data: any): void {
    this.emit('physical_motor_status', data);
  }

  // Handle system status from hardware
  private handleSystemStatus(data: any): void {
    this.emit('physical_system_status', data);
  }

  // Handle command responses from hardware
  private handleCommandResponse(data: any): void {
    this.emit('hardware_command_response', data);
  }

  // Send command to hardware
  async sendCommand(command: string, data?: any): Promise<void> {
    if (!this.isConnected || !this.serialPort) {
      throw new Error('Hardware not connected');
    }

    const message = data ? `${command}:${JSON.stringify(data)}\n` : `${command}\n`;
    
    return new Promise((resolve, reject) => {
      this.serialPort!.write(message, (error) => {
        if (error) {
          reject(error);
        } else {
          resolve();
        }
      });
    });
  }

  // Send route to physical hardware
  async sendRoute(route: RouteStep[]): Promise<void> {
    await this.sendCommand('LOAD_ROUTE', route);
  }

  // Start route execution on hardware
  async startRoute(): Promise<void> {
    await this.sendCommand('START_ROUTE');
  }

  // Stop route execution on hardware
  async stopRoute(): Promise<void> {
    await this.sendCommand('STOP_ROUTE');
  }

  // Emergency stop
  async emergencyStop(): Promise<void> {
    await this.sendCommand('EMERGENCY_STOP');
  }

  // Set HIL mode (Hardware-in-the-loop)
  setHILMode(enabled: boolean): void {
    this.hilMode = enabled;
    this.virtualMode = !enabled;
    this.mixedMode = false;
    this.emit('mode_changed', { hilMode: this.hilMode, virtualMode: this.virtualMode, mixedMode: this.mixedMode });
  }

  // Set mixed mode (some virtual, some physical)
  setMixedMode(enabled: boolean): void {
    this.mixedMode = enabled;
    this.virtualMode = !enabled;
    this.hilMode = false;
    this.emit('mode_changed', { hilMode: this.hilMode, virtualMode: this.virtualMode, mixedMode: this.mixedMode });
  }

  // Get connection status
  getConnectionStatus(): { connected: boolean; mode: string; config: HardwareConfig } {
    let mode = 'virtual';
    if (this.hilMode) mode = 'hil';
    else if (this.mixedMode) mode = 'mixed';

    return {
      connected: this.isConnected,
      mode,
      config: this.config
    };
  }

  // Get message buffer for debugging
  getMessageBuffer(limit?: number): HardwareMessage[] {
    const buffer = [...this.messageBuffer].reverse();
    return limit ? buffer.slice(0, limit) : buffer;
  }

  // Export hardware logs
  exportHardwareLogs(): { messages: HardwareMessage[]; config: HardwareConfig; stats: any } {
    const stats = {
      totalMessages: this.messageBuffer.length,
      messagesByType: this.getMessageStats(),
      connectionUptime: this.isConnected ? Date.now() - this.getConnectionStartTime() : 0
    };

    return {
      messages: this.messageBuffer,
      config: this.config,
      stats
    };
  }

  // Close hardware connections
  async close(): Promise<void> {
    if (this.serialPort?.isOpen) {
      await new Promise<void>((resolve) => {
        this.serialPort!.close(() => resolve());
      });
    }
    this.isConnected = false;
    this.emit('hardware_disconnected');
    
    if (this.imuBridge) {
      await this.imuBridge.close();
    }
    
    this.stopIMUDataStream();
  }

  // Add message to buffer
  private addMessage(message: HardwareMessage): void {
    this.messageBuffer.push(message);
    
    // Maintain buffer size limit
    if (this.messageBuffer.length > this.maxBufferSize) {
      this.messageBuffer = this.messageBuffer.slice(-this.maxBufferSize);
    }
  }

  // Get message statistics
  private getMessageStats(): Record<string, number> {
    const stats: Record<string, number> = {};
    for (const message of this.messageBuffer) {
      stats[message.type] = (stats[message.type] || 0) + 1;
    }
    return stats;
  }

  // Get connection start time (placeholder)
  private getConnectionStartTime(): number {
    // This would track actual connection start time
    return Date.now() - 10000; // Placeholder
  }

  private async initializeIMU(): Promise<void> {
    try {
      this.imuBridge = new IMUHardwareBridge({
        serialPort: this.config.imu?.serialPort,
        baudRate: this.config.imu?.baudRate || 115200,
        autoDetect: this.config.imu?.autoDetect || true
      });

      this.setupIMUEventHandlers();
      
      if (this.config.imu?.serialPort || this.config.imu?.autoDetect) {
        await this.imuBridge.initialize(this.config.imu?.serialPort);
      }

      this.emit('imu_initialized');
    } catch (error) {
      this.emit('imu_error', error);
      console.error('Failed to initialize IMU hardware:', error);
    }
  }

  private setupIMUEventHandlers(): void {
    if (!this.imuBridge) return;

    this.imuBridge.on('connected', () => {
      this.emit('imu_connected');
      this.startIMUDataStream();
    });

    this.imuBridge.on('disconnected', () => {
      this.emit('imu_disconnected');
      this.stopIMUDataStream();
    });

    this.imuBridge.on('imu_data', (data: IMUSensorData) => {
      this.emit('sensor_data', { imu: data });
      this.emit('imu_data_received', data);
    });

    this.imuBridge.on('error', (error) => {
      this.emit('imu_error', error);
    });

    this.imuBridge.on('hardware_mode_enabled', () => {
      this.emit('imu_hardware_mode_enabled');
    });

    this.imuBridge.on('hardware_mode_disabled', () => {
      this.emit('imu_hardware_mode_disabled');
    });

    this.imuBridge.on('fault_injection_updated', (faults) => {
      this.emit('imu_fault_injection_updated', faults);
    });

    this.imuBridge.on('crc_error', (error) => {
      this.emit('imu_crc_error', error);
    });

    this.imuBridge.on('parse_error', (error) => {
      this.emit('imu_parse_error', error);
    });
  }

  private startIMUDataStream(): void {
    // Stream IMU data every 50ms as requested
    this.imuUpdateInterval = setInterval(() => {
      const data = this.imuBridge?.getCurrentSensorData();
      if (data) {
        this.emit('imu_stream_data', data);
      }
    }, 50);
  }

  private stopIMUDataStream(): void {
    if (this.imuUpdateInterval) {
      clearInterval(this.imuUpdateInterval);
      this.imuUpdateInterval = undefined;
    }
  }

  // IMU Control Methods
  async connectIMU(port?: string): Promise<void> {
    if (!this.imuBridge) {
      await this.initializeIMU();
    }
    
    if (this.imuBridge) {
      await this.imuBridge.initialize(port);
    }
  }

  async disconnectIMU(): Promise<void> {
    if (this.imuBridge) {
      await this.imuBridge.close();
    }
    this.stopIMUDataStream();
  }

  enableIMUHardwareMode(): void {
    if (this.imuBridge) {
      this.imuBridge.enableHardwareMode();
    }
  }

  disableIMUHardwareMode(): void {
    if (this.imuBridge) {
      this.imuBridge.disableHardwareMode();
    }
  }

  isIMUInHardwareMode(): boolean {
    return this.imuBridge?.isInHardwareMode() || false;
  }

  getIMUStatus(): any {
    if (!this.imuBridge) {
      return {
        connected: false,
        hardwareMode: false,
        port: 'Not initialized',
        error: 'IMU bridge not initialized'
      };
    }

    const status = this.imuBridge.getConnectionStatus();
    return {
      connected: status.connected,
      hardwareMode: status.hardwareMode,
      port: status.port,
      lastDataTimestamp: status.lastData?.timestamp,
      error: status.connected ? undefined : 'Not connected'
    };
  }

  getCurrentIMUData(): IMUSensorData | null {
    return this.imuBridge?.getCurrentSensorData() || null;
  }

  // IMU Fault Injection Methods
  setIMUFaultInjection(faults: any): void {
    if (this.imuBridge) {
      this.imuBridge.setFaultInjection(faults);
    }
  }

  getIMUFaultInjection(): any {
    return this.imuBridge?.getFaultInjection() || null;
  }

  clearIMUFaultInjection(): void {
    if (this.imuBridge) {
      this.imuBridge.clearFaultInjection();
    }
  }

  // IMU Testing Methods
  async testIMUConnection(timeoutMs: number = 5000): Promise<boolean> {
    if (!this.imuBridge) {
      return false;
    }
    return await this.imuBridge.testConnection(timeoutMs);
  }

  async detectIMUPorts(): Promise<string[]> {
    if (!this.imuBridge) {
      this.imuBridge = new IMUHardwareBridge();
    }
    return await this.imuBridge.detectAvailablePorts();
  }

  getIMULogFilePath(): string {
    return this.imuBridge?.getLogFilePath() || '';
  }

  // Override existing methods to include IMU data
  getStatus(): HardwareStatus {
    return {
      connected: this.isConnected,
      mode: this.getConnectionStatus().mode,
      config: this.config,
      imu: this.getIMUStatus()
    };
  }

  getCurrentSensorData(): SensorReadings {
    const physicalData = this.getCurrentPhysicalSensorData();
    const imuData = this.getCurrentIMUData();

    return {
      ...physicalData,
      imu: imuData
    };
  }

  private getCurrentPhysicalSensorData(): PhysicalSensorData {
    // This method would fetch actual physical sensor data from hardware
    // For now, it's a placeholder returning dummy data
    return {
      magneticPosition: { x: 0, y: 0, detected: false },
      imuData: { ax: 0, ay: 0, az: 0, gx: 0, gy: 0, gz: 0 },
      encoders: { left: 0, right: 0 },
      battery: { voltage: 12, current: 0, percentage: 100 },
      temperature: [25],
      ultrasonicDistances: [0]
    };
  }
}