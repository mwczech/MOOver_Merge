import express from 'express';
import cors from 'cors';
import helmet from 'helmet';
import morgan from 'morgan';
import { config, isDevelopment } from './config';
import { RobotSimulator } from './services/RobotSimulator';
import { WebSocketServer } from './services/WebSocketServer';
import { createRobotRoutes } from './routes/robotRoutes';
import { createRoutesRoutes } from './routes/routesRoutes';
import { createLogsRoutes, addLogEntry } from './routes/logsRoutes';
import { errorHandler, notFoundHandler } from './middleware/errorHandler';
import { ApiResponse } from '@melkens/shared';

class SimulatorServer {
  private app: express.Application;
  private robotSimulator: RobotSimulator;
  private webSocketServer: WebSocketServer;

  constructor() {
    this.app = express();
    this.robotSimulator = new RobotSimulator();
    this.webSocketServer = new WebSocketServer(this.robotSimulator);
    
    this.setupMiddleware();
    this.setupRoutes();
    this.setupErrorHandling();
    this.setupRobotSimulatorEvents();
  }

  private setupMiddleware(): void {
    // Security middleware
    this.app.use(helmet());
    
    // CORS configuration
    this.app.use(cors({
      origin: config.cors.origins,
      credentials: true,
      methods: ['GET', 'POST', 'PUT', 'DELETE', 'OPTIONS'],
      allowedHeaders: ['Content-Type', 'Authorization']
    }));

    // Request parsing
    this.app.use(express.json({ limit: '10mb' }));
    this.app.use(express.urlencoded({ extended: true, limit: '10mb' }));

    // Logging
    if (isDevelopment) {
      this.app.use(morgan('dev'));
    } else {
      this.app.use(morgan('combined'));
    }

    // Request ID middleware
    this.app.use((req, res, next) => {
      req.headers['x-request-id'] = req.headers['x-request-id'] || 
        `req_${Date.now()}_${Math.random().toString(36).substr(2, 9)}`;
      next();
    });
  }

  private setupRoutes(): void {
    // Health check endpoint
    this.app.get('/health', (req, res) => {
      const response: ApiResponse = {
        success: true,
        data: {
          status: 'ok',
          timestamp: Date.now(),
          version: '1.0.0',
          environment: config.server.env,
          robotSimulatorActive: this.robotSimulator ? true : false,
          websocketConnections: this.webSocketServer.getConnectedClientsCount()
        },
        timestamp: Date.now()
      };
      res.json(response);
    });

    // API routes
    this.app.use('/api/robot', createRobotRoutes(this.robotSimulator));
    this.app.use('/api/routes', createRoutesRoutes(this.robotSimulator));
    this.app.use('/api/logs', createLogsRoutes());

    // API info endpoint
    this.app.get('/api', (req, res) => {
      const response: ApiResponse = {
        success: true,
        data: {
          name: 'MELKENS WB Robot Simulator API',
          version: '1.0.0',
          description: 'REST API and WebSocket server for robot simulation',
          endpoints: {
            robot: '/api/robot',
            routes: '/api/routes',
            logs: '/api/logs',
            websocket: `/ws (port ${config.websocket.port})`
          },
          documentation: '/docs'
        },
        timestamp: Date.now()
      };
      res.json(response);
    });

    // Serve static documentation if available
    this.app.use('/docs', express.static('docs', { 
      index: 'index.html',
      fallthrough: false
    }));
  }

  private setupErrorHandling(): void {
    // 404 handler
    this.app.use(notFoundHandler);
    
    // Global error handler
    this.app.use(errorHandler);
  }

  private setupRobotSimulatorEvents(): void {
    // Forward robot simulator logs to the logs system
    this.robotSimulator.on('log', (logEntry) => {
      addLogEntry(logEntry);
    });

    // Log robot events
    this.robotSimulator.on('route_started', (data) => {
      console.log(`Route ${data.routeId} started at ${new Date(data.timestamp).toISOString()}`);
    });

    this.robotSimulator.on('route_completed', (data) => {
      console.log(`Route ${data.routeId} completed (success: ${data.success}) at ${new Date(data.timestamp).toISOString()}`);
    });

    this.robotSimulator.on('emergency_stop', (data) => {
      console.log(`Emergency stop activated at ${new Date(data.timestamp).toISOString()}`);
    });
  }

  public start(): void {
    // Start robot simulator
    this.robotSimulator.start();
    console.log('Robot simulator started');

    // Start Express server
    this.app.listen(config.server.port, config.server.host, () => {
      console.log(`
🚀 MELKENS WB Robot Simulator Server Started
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

📡 HTTP Server:     http://${config.server.host}:${config.server.port}
🔌 WebSocket:       ws://${config.server.host}:${config.websocket.port}
🌍 Environment:     ${config.server.env}
📝 API Docs:        http://${config.server.host}:${config.server.port}/docs
🔍 Health Check:    http://${config.server.host}:${config.server.port}/health

API Endpoints:
├── GET    /api                     - API information
├── GET    /api/robot/state         - Robot state
├── POST   /api/robot/control/*     - Robot control
├── GET    /api/routes              - List routes
├── POST   /api/routes              - Create route
├── GET    /api/logs                - Get logs
└── WS     :${config.websocket.port}              - Real-time updates

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
      `);
    });
  }

  public stop(): void {
    console.log('Shutting down server...');
    
    if (this.robotSimulator) {
      this.robotSimulator.stop();
      console.log('Robot simulator stopped');
    }
    
    if (this.webSocketServer) {
      this.webSocketServer.close();
      console.log('WebSocket server stopped');
    }
    
    process.exit(0);
  }
}

// Initialize and start server
const server = new SimulatorServer();

// Graceful shutdown handling
process.on('SIGTERM', () => {
  console.log('SIGTERM received');
  server.stop();
});

process.on('SIGINT', () => {
  console.log('SIGINT received');
  server.stop();
});

process.on('uncaughtException', (error) => {
  console.error('Uncaught Exception:', error);
  server.stop();
});

process.on('unhandledRejection', (reason, promise) => {
  console.error('Unhandled Rejection at:', promise, 'reason:', reason);
  server.stop();
});

// Start the server
server.start();