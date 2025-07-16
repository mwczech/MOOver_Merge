import dotenv from 'dotenv';

dotenv.config();

export const config = {
  server: {
    port: parseInt(process.env.PORT || '3001', 10),
    host: process.env.HOST || 'localhost',
    env: process.env.NODE_ENV || 'development'
  },
  
  websocket: {
    port: parseInt(process.env.WS_PORT || '3002', 10),
    heartbeatInterval: parseInt(process.env.WS_HEARTBEAT_INTERVAL || '5000', 10)
  },
  
  cors: {
    origins: process.env.CORS_ORIGINS?.split(',') || ['http://localhost:3000', 'http://localhost:5173']
  },
  
  simulation: {
    updateRate: parseInt(process.env.SIMULATION_UPDATE_RATE || '100', 10),
    robotSimulationEnabled: process.env.ROBOT_SIMULATION_ENABLED === 'true'
  },
  
  logging: {
    level: process.env.LOG_LEVEL || 'info',
    toFile: process.env.LOG_TO_FILE === 'true'
  }
};

export const isDevelopment = config.server.env === 'development';
export const isProduction = config.server.env === 'production';