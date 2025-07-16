import WebSocket from 'ws';
import { IncomingMessage } from 'http';
import { 
  WebSocketMessage, 
  RobotStateUpdate, 
  LogMessage, 
  RouteStarted, 
  RouteCompleted,
  SimulatorCommand 
} from '@melkens/shared';
import { RobotSimulator } from './RobotSimulator';
import { config } from '../config';

interface ExtendedWebSocket extends WebSocket {
  id: string;
  isAlive: boolean;
}

export class WebSocketServer {
  private wss: WebSocket.Server;
  private clients: Map<string, ExtendedWebSocket> = new Map();
  private heartbeatInterval?: NodeJS.Timeout;
  private robotSimulator: RobotSimulator;

  constructor(robotSimulator: RobotSimulator) {
    this.robotSimulator = robotSimulator;
    this.wss = new WebSocket.Server({ 
      port: config.websocket.port,
      perMessageDeflate: false
    });

    this.setupEventHandlers();
    this.startHeartbeat();
    
    console.log(`WebSocket server started on port ${config.websocket.port}`);
  }

  private setupEventHandlers(): void {
    this.wss.on('connection', this.handleConnection.bind(this));
    
    // Robot simulator events
    this.robotSimulator.on('state_update', this.broadcastRobotState.bind(this));
    this.robotSimulator.on('log', this.broadcastLog.bind(this));
    this.robotSimulator.on('route_started', this.broadcastRouteStarted.bind(this));
    this.robotSimulator.on('route_completed', this.broadcastRouteCompleted.bind(this));
    this.robotSimulator.on('emergency_stop', this.broadcastEmergencyStop.bind(this));
  }

  private handleConnection(ws: WebSocket, request: IncomingMessage): void {
    const extWs = ws as ExtendedWebSocket;
    extWs.id = this.generateClientId();
    extWs.isAlive = true;

    this.clients.set(extWs.id, extWs);

    console.log(`Client ${extWs.id} connected. Total clients: ${this.clients.size}`);

    // Send initial robot state
    this.sendToClient(extWs, {
      type: 'robot_state_update',
      timestamp: Date.now(),
      data: this.robotSimulator.getRobotState()
    });

    // Send available routes
    this.sendToClient(extWs, {
      type: 'routes_update',
      timestamp: Date.now(),
      data: this.robotSimulator.getRoutes()
    });

    ws.on('message', (data: WebSocket.Data) => {
      this.handleMessage(extWs, data);
    });

    ws.on('pong', () => {
      extWs.isAlive = true;
    });

    ws.on('close', () => {
      this.handleDisconnection(extWs);
    });

    ws.on('error', (error) => {
      console.error(`WebSocket error for client ${extWs.id}:`, error);
      this.handleDisconnection(extWs);
    });
  }

  private handleMessage(ws: ExtendedWebSocket, data: WebSocket.Data): void {
    try {
      const message = JSON.parse(data.toString()) as SimulatorCommand;
      this.processCommand(ws, message);
    } catch (error) {
      console.error(`Invalid message from client ${ws.id}:`, error);
      this.sendError(ws, 'Invalid JSON message');
    }
  }

  private processCommand(ws: ExtendedWebSocket, command: SimulatorCommand): void {
    try {
      switch (command.type) {
        case 'start_route':
          const routeId = command.data?.routeId;
          if (typeof routeId === 'number') {
            const success = this.robotSimulator.startRoute(routeId);
            this.sendToClient(ws, {
              type: 'command_response',
              timestamp: Date.now(),
              data: { command: 'start_route', success, routeId }
            });
          } else {
            this.sendError(ws, 'Invalid routeId');
          }
          break;

        case 'stop_robot':
          this.robotSimulator.stopRoute();
          this.sendToClient(ws, {
            type: 'command_response',
            timestamp: Date.now(),
            data: { command: 'stop_robot', success: true }
          });
          break;

        case 'emergency_stop':
          this.robotSimulator.emergencyStop();
          this.sendToClient(ws, {
            type: 'command_response',
            timestamp: Date.now(),
            data: { command: 'emergency_stop', success: true }
          });
          break;

        case 'reset_position':
          this.robotSimulator.resetPosition();
          this.sendToClient(ws, {
            type: 'command_response',
            timestamp: Date.now(),
            data: { command: 'reset_position', success: true }
          });
          break;

        default:
          this.sendError(ws, `Unknown command: ${command.type}`);
      }
    } catch (error) {
      console.error(`Error processing command from client ${ws.id}:`, error);
      this.sendError(ws, 'Command processing failed');
    }
  }

  private handleDisconnection(ws: ExtendedWebSocket): void {
    this.clients.delete(ws.id);
    console.log(`Client ${ws.id} disconnected. Total clients: ${this.clients.size}`);
  }

  private broadcastRobotState(robotState: any): void {
    const message: RobotStateUpdate = {
      type: 'robot_state_update',
      timestamp: Date.now(),
      data: robotState
    };
    this.broadcast(message);
  }

  private broadcastLog(logEntry: any): void {
    const message: LogMessage = {
      type: 'log_message',
      timestamp: Date.now(),
      data: logEntry
    };
    this.broadcast(message);
  }

  private broadcastRouteStarted(data: any): void {
    const message: RouteStarted = {
      type: 'route_started',
      timestamp: Date.now(),
      data
    };
    this.broadcast(message);
  }

  private broadcastRouteCompleted(data: any): void {
    const message: RouteCompleted = {
      type: 'route_completed',
      timestamp: Date.now(),
      data
    };
    this.broadcast(message);
  }

  private broadcastEmergencyStop(data: any): void {
    const message: WebSocketMessage = {
      type: 'emergency_stop',
      timestamp: Date.now(),
      data
    };
    this.broadcast(message);
  }

  private broadcast(message: WebSocketMessage): void {
    const messageStr = JSON.stringify(message);
    
    this.clients.forEach((client) => {
      if (client.readyState === WebSocket.OPEN) {
        client.send(messageStr);
      }
    });
  }

  private sendToClient(client: ExtendedWebSocket, message: WebSocketMessage): void {
    if (client.readyState === WebSocket.OPEN) {
      client.send(JSON.stringify(message));
    }
  }

  private sendError(client: ExtendedWebSocket, error: string): void {
    this.sendToClient(client, {
      type: 'error',
      timestamp: Date.now(),
      data: { error }
    });
  }

  private generateClientId(): string {
    return `client_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
  }

  private startHeartbeat(): void {
    this.heartbeatInterval = setInterval(() => {
      this.clients.forEach((client, id) => {
        if (!client.isAlive) {
          console.log(`Terminating inactive client ${id}`);
          client.terminate();
          this.clients.delete(id);
          return;
        }

        client.isAlive = false;
        client.ping();
      });
    }, config.websocket.heartbeatInterval);
  }

  public close(): void {
    if (this.heartbeatInterval) {
      clearInterval(this.heartbeatInterval);
    }

    this.clients.forEach((client) => {
      client.close();
    });

    this.wss.close(() => {
      console.log('WebSocket server closed');
    });
  }

  public getConnectedClientsCount(): number {
    return this.clients.size;
  }

  public getClientIds(): string[] {
    return Array.from(this.clients.keys());
  }
}