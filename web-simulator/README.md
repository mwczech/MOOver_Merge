# 🤖 MOOver MELKENS Web Simulator

A comprehensive web-based simulator for the MOOver MELKENS autonomous robot with Hardware-in-the-Loop (HIL) integration capabilities.

## 🚀 Features

- **Real-time Robot Control**: WebSocket-based joystick controller for instant robot movement
- **Status Monitoring**: Live monitoring of motor speeds, battery voltage, current consumption, and error counters
- **Route Management**: Autonomous route selection and execution (Routes A-K)
- **Magnetic Line Sensor**: Visual representation of 16-bit magnetic field sensor array
- **HIL Integration**: Connect real hardware for testing and development
- **Firmware Upload**: Over-the-air (OTA) firmware updates for ESP32 and PMB modules
- **Configuration Management**: Upload and manage WiFi/MQTT configurations
- **Responsive Design**: Modern, mobile-friendly interface with beautiful gradients

## 🏗️ Architecture

```
web-simulator/
├── backend/           # Node.js + Express + Socket.IO server
│   ├── server.js     # Main server with robot simulation
│   └── package.json  # Backend dependencies
├── frontend/         # React application
│   ├── src/
│   │   ├── components/   # React components
│   │   ├── App.js       # Main application
│   │   └── index.js     # Entry point
│   └── package.json     # Frontend dependencies
├── Dockerfile           # Multi-stage Docker build
├── railway.toml        # Railway deployment configuration
├── .env.example        # Environment variables template
└── README.md          # This file
```

## 🚀 Deployment on Railway.app

### Prerequisites

1. **GitHub Account**: Your code must be in a GitHub repository
2. **Railway Account**: Sign up at [railway.app](https://railway.app)
3. **Project Structure**: Ensure the `web-simulator/` directory contains all necessary files

### Step-by-Step Deployment

#### 1. Prepare Your Repository

```bash
# Ensure you're in the web-simulator directory
cd web-simulator

# Install all dependencies (optional, for local testing)
npm run install:all
```

#### 2. Connect to Railway

1. Go to [railway.app](https://railway.app)
2. Sign in with your GitHub account
3. Click **"New Project"**
4. Select **"Deploy from GitHub repo"**
5. Choose your repository containing the MOOver_Merge project

#### 3. Configure Root Directory

**IMPORTANT**: Set the Root Directory in Railway:

1. In your Railway project dashboard, go to **Settings**
2. Find **"Root Directory"** setting
3. Set it to: `web-simulator/`
4. Click **Save**

#### 4. Environment Variables (Optional)

Railway will automatically detect the Node.js environment. You can add custom environment variables:

1. Go to **Variables** tab in Railway dashboard
2. Add any required variables from `.env.example`
3. Most variables have sensible defaults

#### 5. Deploy

1. Railway will automatically start building your project
2. The build process will:
   - Build the React frontend
   - Install backend dependencies
   - Create a production Docker image
   - Deploy to Railway's infrastructure

#### 6. Access Your Application

Once deployment is complete, Railway will provide you with a URL like:
```
https://web-production-xxxx.up.railway.app
```

## 🔧 Local Development

### Prerequisites

- Node.js 18.x or higher
- npm or yarn

### Setup

```bash
# Clone and navigate to the web-simulator directory
cd web-simulator

# Install root dependencies
npm install

# Install all dependencies (backend + frontend)
npm run install:all

# Start development servers (backend + frontend)
npm run dev
```

The application will be available at:
- Frontend: http://localhost:3000
- Backend: http://localhost:3001

### Building for Production

```bash
# Build frontend
npm run build:frontend

# Start production server
npm start
```

## 🎮 Using the Simulator

### Motion Controller
- **Joystick**: Drag to control robot movement (differential drive)
- **Auger Speed**: Adjust conveyor/auger motor speed (0-1500)
- **Power**: Toggle main power on/off
- **Charging**: Simulate charging mode

### Route Controller
- **Route Selection**: Choose predefined routes A-K
- **Play/Pause/Stop**: Control autonomous navigation
- **Status Display**: Monitor current route execution status

### Status Panel
- **Real-time Monitoring**: Motor speeds, battery voltage, current consumption
- **Error Counters**: CRC communication errors between modules
- **Connection Status**: PMB, HIL, and power status

### HIL Controller
- **Hardware Connection**: Connect real ESP32/PMB hardware
- **Firmware Upload**: Upload .bin/.hex firmware files via drag-and-drop
- **Configuration**: Upload config.json files
- **Device Selection**: Choose between PMB, IMU, and ESP32 modules

## 🐛 Troubleshooting

### Common Deployment Issues

#### 1. Build Fails - "Module not found"
```bash
# Solution: Ensure all package.json files are present
ls backend/package.json frontend/package.json package.json
```

#### 2. Railway Can't Find Application
**Problem**: Railway shows "Application failed to respond"
**Solution**: 
- Verify Root Directory is set to `web-simulator/`
- Check that PORT environment variable is used correctly
- Ensure health check endpoint `/api/health` is accessible

#### 3. WebSocket Connection Fails
**Problem**: Frontend can't connect to WebSocket
**Solution**:
- Ensure your backend server uses `process.env.PORT`
- Check CORS configuration allows Railway's domain
- Verify Socket.IO is properly configured for production

#### 4. Static Files Not Served
**Problem**: React app shows blank page
**Solution**:
- Verify frontend build is copied to backend in Dockerfile
- Check Express static file serving configuration
- Ensure build files exist in `frontend/build/`

#### 5. Memory/Resource Issues
**Problem**: Application crashes or restarts frequently
**Solution**:
- Monitor Railway resource usage in dashboard
- Optimize Docker image (remove dev dependencies)
- Implement proper error handling and graceful shutdowns

### Debugging Steps

1. **Check Railway Logs**:
   - Go to Railway dashboard → Your project → Deployments
   - Click on latest deployment to view build and runtime logs

2. **Verify Environment**:
   ```bash
   # Test health endpoint
   curl https://your-app.up.railway.app/api/health
   ```

3. **Local Testing**:
   ```bash
   # Test production build locally
   npm run build
   npm start
   ```

## 🔐 Security Considerations

- **CORS**: Configured for development; restrict origins in production
- **File Uploads**: Limited to 10MB, specific file extensions only
- **Rate Limiting**: Built-in rate limiting for API endpoints
- **Input Validation**: All user inputs are validated server-side

## 📈 Performance Optimization

- **Gzip Compression**: Enabled for all responses
- **Static File Caching**: Optimized serving of React build files
- **WebSocket Optimization**: Configurable heartbeat and timeout settings
- **Docker Multi-stage**: Minimal production image size

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes in the `web-simulator/` directory
4. Test locally with `npm run dev`
5. Submit a pull request

## 📄 License

MIT License - see LICENSE file for details

## 🆘 Support

For issues and questions:
- Check the troubleshooting section above
- Review Railway logs in the dashboard
- Open an issue in the GitHub repository

---

**Developed by**: Melkens Sp. z o.o.  
**Project**: MOOver MELKENS Autonomous Robot Integration  
**Version**: 1.0.0