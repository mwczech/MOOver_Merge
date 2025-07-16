# MELKENS WB Robot Simulator

![Version](https://img.shields.io/badge/version-1.0.0-blue)
![License](https://img.shields.io/badge/license-MIT-green)
![Node](https://img.shields.io/badge/node-%3E%3D18.0.0-brightgreen)
![TypeScript](https://img.shields.io/badge/typescript-5.3+-blue)

A comprehensive web-based simulator for MELKENS robot with WB (Wasserbauer) navigation algorithm integration. This application provides real-time visualization, route management, and complete system monitoring for robot operations.

## 🚀 Features

### 🎯 Core Functionality
- **Real-time Robot Visualization** - Interactive 2D map with robot position, orientation, and path tracking
- **Route Management** - Complete CRUD operations for robot routes with visual editor
- **Live Monitoring** - Real-time sensor data, motor states, and system diagnostics  
- **WebSocket Integration** - Real-time updates with automatic reconnection
- **Log Management** - Comprehensive logging with filtering, search, and export capabilities
- **Settings Configuration** - Runtime parameter adjustment based on embedded C code

### 🛠 Technical Features
- **TypeScript** - Full type safety across frontend and backend
- **React + Vite** - Modern frontend with hot reload and optimized builds
- **Node.js + Express** - Robust backend API with WebSocket support
- **Tailwind CSS** - Beautiful, responsive UI design
- **Docker Ready** - Multi-stage builds for production deployment
- **Railway Deployment** - One-click cloud deployment configuration

## 📁 Project Structure

```
web-simulator/
├── shared/              # Shared TypeScript types and constants
│   ├── src/
│   │   ├── types.ts     # Robot state, routes, sensors interfaces
│   │   ├── constants.ts # Default settings from PMB C code
│   │   └── index.ts     # Export definitions
├── backend/             # Node.js/Express API server
│   ├── src/
│   │   ├── controllers/ # REST API controllers
│   │   ├── routes/      # API routing
│   │   ├── services/    # Robot simulator and WebSocket server
│   │   ├── middleware/  # Error handling and validation
│   │   └── config/      # Environment configuration
├── frontend/            # React/Vite web application  
│   ├── src/
│   │   ├── components/  # Reusable UI components
│   │   ├── pages/       # Main application pages
│   │   ├── hooks/       # API and WebSocket hooks
│   │   └── main.tsx     # Application entry point
├── scripts/             # Deployment and build scripts
├── Dockerfile           # Multi-stage production build
├── railway.toml         # Railway deployment configuration
└── README.md           # This file
```

## 🎯 Robot Integration

This simulator is designed to replicate the behavior of the MELKENS PMB (Power Management Board) system with WB navigation algorithm:

### Supported Operations
- **NORM** - Normal forward movement with magnetic line following
- **TU_L/TU_R** - Gradual turn left/right while moving
- **L_90/R_90** - Precise 90-degree turns in place
- **DIFF** - Differential drive operations
- **NORM_NOMAGNET** - Movement without magnetic guidance

### Hardware Simulation
- **Motor Control** - Left/right drive motors, lift, and belt systems
- **Sensor Emulation** - Magnetic position, IMU, encoders, battery, temperature
- **Navigation Logic** - Magnetic line following with configurable parameters
- **Error Handling** - Comprehensive error states and recovery

## 🚀 Quick Start

### Prerequisites
- Node.js 18+ 
- npm or yarn
- Docker (for deployment)

### Development Setup

1. **Clone and Install Dependencies**
```bash
cd web-simulator
npm run install:all
```

2. **Start Development Servers**
```bash
# Start both frontend and backend in development mode
npm run dev

# Or start individually:
npm run dev:backend  # Backend on :3001, WebSocket on :3002
npm run dev:frontend # Frontend on :3000
```

3. **Access the Application**
- Frontend: http://localhost:3000
- Backend API: http://localhost:3001/api
- Health Check: http://localhost:3001/health

### Production Build

```bash
# Build all components
npm run build

# Start production server
npm start
```

## 🐳 Docker Deployment

### Build Production Image
```bash
docker build -t melkens-wb-simulator .
```

### Run Container
```bash
docker run -p 3001:3001 -p 3002:3002 melkens-wb-simulator
```

## ☁️ Railway Deployment

This project is configured for one-click deployment to Railway:

1. **Fork this repository**
2. **Connect to Railway** 
3. **Deploy** - Railway will automatically detect the configuration
4. **Access** - Your simulator will be available at the provided Railway URL

### Environment Variables
Railway automatically configures:
- `RAILWAY_STATIC_URL` - Used for CORS configuration
- `PORT` - Assigned by Railway (defaults to 3001)
- `WS_PORT` - WebSocket port (defaults to 3002)

## 📊 API Documentation

### REST Endpoints

#### Robot Control
- `GET /api/robot/state` - Get current robot state
- `POST /api/robot/control/start` - Start route execution
- `POST /api/robot/control/stop` - Stop robot
- `POST /api/robot/control/emergency-stop` - Emergency stop
- `POST /api/robot/control/reset-position` - Reset robot position

#### Route Management  
- `GET /api/routes` - List all routes
- `POST /api/routes` - Create new route
- `GET /api/routes/:id` - Get route details
- `PUT /api/routes/:id` - Update route
- `DELETE /api/routes/:id` - Delete route
- `POST /api/routes/:id/execute` - Execute route

#### Logging
- `GET /api/logs` - Get filtered logs
- `GET /api/logs/export` - Export logs (JSON/CSV)
- `DELETE /api/logs` - Clear all logs

### WebSocket Events

#### Incoming (Client → Server)
```typescript
{
  type: 'start_route' | 'stop_robot' | 'emergency_stop' | 'reset_position',
  data?: any
}
```

#### Outgoing (Server → Client)
```typescript
{
  type: 'robot_state_update' | 'log_message' | 'route_started' | 'route_completed',
  timestamp: number,
  data: any
}
```

## 🎨 UI Components

### Dashboard
- **Robot Visualization** - Interactive canvas with zoom, pan, and robot tracking
- **Control Panel** - Route selection and robot control buttons
- **Metrics Display** - Real-time sensor readings and system status
- **Activity Feed** - Recent logs and system events

### Route Editor
- **Visual Editor** - Drag-and-drop route building with step validation
- **Parameter Configuration** - Speed, distance, magnetic correction settings
- **Preview Mode** - Route visualization and estimated execution time

### System Monitoring
- **Real-time Logs** - Filterable log viewer with export capabilities
- **System Settings** - Runtime parameter adjustment
- **Health Dashboard** - System status and connection monitoring

## ⚙️ Configuration

### Default Settings (from PMB C code)
```typescript
{
  CORRECTION_ANGLE_THRESHOLD: 0.5,    // degrees
  DEFAULT_SPEED: 500,                 // motor speed
  IMU_JUDGEMENT_FACTOR: 1.0,         // IMU vs encoder weight
  ENCODER_STEP_MAX_MULTIPLIER: 1.5   // step completion threshold
}
```

### Simulation Parameters
```typescript
{
  UPDATE_RATE: 100,                   // ms - simulation frequency
  GRID_SIZE: 50,                      // mm - visualization grid
  ROBOT_SIZE: { WIDTH: 400, LENGTH: 600 },  // mm
  MAP_SIZE: { WIDTH: 10000, HEIGHT: 8000 }  // mm
}
```

## 🧪 Testing

### Unit Tests
```bash
npm run test              # Run all tests
npm run test:backend      # Backend tests only
npm run test:frontend     # Frontend tests only
npm run test:watch        # Watch mode
```

### E2E Tests
```bash
npm run test:e2e          # End-to-end tests
```

## 🔧 Development

### Adding New Features

1. **Shared Types** - Add interfaces to `shared/src/types.ts`
2. **Backend Logic** - Implement in appropriate controller/service
3. **Frontend Components** - Create React components with hooks
4. **WebSocket Events** - Add real-time communication if needed

### Code Style
- **TypeScript** - Strict mode enabled
- **ESLint** - Configured for React and Node.js
- **Prettier** - Automatic code formatting
- **Conventional Commits** - For better change tracking

## 📈 Performance

### Optimizations
- **WebSocket** - Efficient real-time communication
- **React Query** - Smart data caching and synchronization
- **Canvas Rendering** - Optimized robot visualization
- **Chunked Builds** - Optimized loading with Vite

### Monitoring
- **Health Checks** - Automated system health monitoring
- **Error Boundaries** - Graceful error handling
- **Memory Management** - Limited log retention and cleanup

## 🛡️ Security

### Production Considerations
- **CORS** - Configured for allowed origins
- **Input Validation** - Joi schemas for API validation
- **Error Handling** - Sanitized error responses
- **Rate Limiting** - Configurable request limits

## 🤝 Contributing

1. **Fork** the repository
2. **Create** a feature branch
3. **Commit** changes with conventional commits
4. **Test** thoroughly
5. **Submit** a pull request

## 📄 License

MIT License - see LICENSE file for details.

## 🆘 Support

### Common Issues

**WebSocket Connection Failed**
- Check if backend is running on port 3002
- Verify firewall settings allow WebSocket connections

**Robot Not Moving in Simulation**
- Ensure a route is selected and started
- Check console for error messages
- Verify robot state via API endpoint

**Build Failures**
- Ensure Node.js 18+ is installed
- Run `npm run install:all` to install all dependencies
- Check TypeScript compilation errors

### Getting Help
- Check the GitHub Issues for known problems
- Review API documentation for correct usage
- Verify environment configuration

---

**Built with ❤️ by MELKENS Team**

*This simulator enables comprehensive testing and development of robot navigation systems without requiring physical hardware, accelerating development cycles and reducing costs.*