import { EventEmitter } from 'events';
import { SerialPort } from 'serialport';
import { ReadlineParser } from '@serialport/parser-readline';
import { RobotState, SensorState, MotorState, RouteStep } from '@melkens/shared';

export interface HardwareConfig {
  serialPort?: string;
  baudRate: number;
  canInterface?: string;
  tcpPort?: number;
  usbVendorId?: string;
  usbProductId?: string;
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

export class HardwareBridge extends EventEmitter {
  private config: HardwareConfig;
  private serialPort?: SerialPort;
  private parser?: ReadlineParser;
  private isConnected = false;
  private messageBuffer: HardwareMessage[] = [];
  private maxBufferSize = 10000;

  // Hardware abstraction layer flags
  private hilMode = false; // Hardware-in-the-loop mode
  private virtualMode = true; // Virtual simulation mode
  private mixedMode = false; // Mixed virtual/physical mode

  constructor(config: HardwareConfig) {
    super();
    this.config = config;
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
}