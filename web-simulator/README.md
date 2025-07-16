# MELKENS HIL Simulator - Railway Deployment Ready

A comprehensive Hardware-in-the-Loop simulator for MELKENS robot platform with advanced fault injection capabilities.

## 🚀 Quick Deploy to Railway

### Option 1: One-Click Deploy (Recommended)

1. **Fork this repository** to your GitHub account
2. **Go to [Railway.app](https://railway.app)**
3. **Click "Start a New Project"**
4. **Select "Deploy from GitHub repo"**
5. **Choose your forked repository**
6. **Set root directory to `/web-simulator`**
7. **Deploy automatically starts!**

### Option 2: Railway CLI Deploy

```bash
# Install Railway CLI
npm install -g @railway/cli

# Login to Railway
railway login

# Navigate to web-simulator directory
cd web-simulator

# Deploy to Railway
railway up

# (Optional) Set custom domain
railway domain
```

### Environment Variables

Railway automatically detects these from `railway.toml`, but you can override:

| Variable | Default | Description |
|----------|---------|-------------|
| `PORT` | `3000` | Application port (Railway sets automatically) |
| `PYTHONUNBUFFERED` | `1` | Python output buffering |
| `PYTHONPATH` | `/app/backend` | Python module path |

## 🐳 Local Docker Development

```bash
# Build the Docker image
docker build -t melkens-hil .

# Run the container
docker run -p 3000:3000 melkens-hil

# Access the application
open http://localhost:3000
```

## 📁 Project Structure

```
web-simulator/
├── Dockerfile              # Multi-stage Docker build
├── railway.toml            # Railway deployment config
├── package.json            # Project metadata
├── start.sh                # Application startup script
├── .env.example           # Environment variables template
├── .dockerignore          # Docker build exclusions
├── backend/               # Python FastAPI backend
│   ├── main.py           # Main application server
│   ├── scenario_runner.py # Fault injection framework
│   ├── imu_manager.py    # IMU communication
│   ├── robot_simulator.py # Physics simulation
│   ├── data_logger.py    # Data logging system
│   ├── requirements.txt  # Python dependencies
│   └── scenarios/        # Example fault scenarios
└── frontend/             # Static web interface
    ├── index.html        # Main dashboard
    ├── style.css         # Responsive dark theme
    └── main.js          # Real-time visualization
```

## 🎯 Features

- **Real-time IMU simulation** with physics-based robot model
- **Advanced fault injection** with 8 different fault types
- **Web-based dashboard** with live data visualization
- **Scenario management** via JSON/CSV file upload
- **Comprehensive reporting** with performance analysis
- **WebSocket streaming** at 50Hz for real-time monitoring
- **RESTful API** for external integration
- **Docker deployment** ready for any platform

## 🔧 API Endpoints

- `GET /` - Main dashboard
- `GET /api/health` - Health check for monitoring
- `GET /api/status` - System status
- `POST /api/scenarios/upload` - Upload test scenarios
- `GET /api/scenarios` - List all scenarios
- `POST /api/scenarios/{id}/run` - Run fault injection test
- `GET /api/scenarios/results` - Get test results
- `WebSocket /ws` - Real-time data stream

## 🧪 Advanced Testing

The simulator includes comprehensive fault injection capabilities:

```bash
# Run built-in test scenarios
curl -X POST https://your-app.railway.app/api/scenarios/basic_acc_stuck/run

# Upload custom scenarios
curl -X POST https://your-app.railway.app/api/scenarios/upload \
     -F "file=@your_scenarios.json"

# Generate test reports
curl https://your-app.railway.app/api/scenarios/report
```

## 🚀 Production Features

- **Auto-scaling** on Railway platform
- **Health monitoring** with `/api/health` endpoint
- **Error recovery** with automatic restarts
- **CORS enabled** for frontend integration
- **Secure deployment** with environment variables
- **Performance optimized** Python FastAPI backend

## 📖 Documentation

- **Advanced Validation Guide**: See `/docs/ADVANCED_VALIDATION.md`
- **API Documentation**: Available at `/docs` when running
- **Fault Injection Manual**: Comprehensive testing scenarios included

## 🐛 Troubleshooting

### Common Issues

1. **Port conflicts**: Railway automatically assigns ports
2. **Memory limits**: Adjust Railway plan if needed
3. **Build failures**: Check Docker logs in Railway dashboard
4. **CORS issues**: Enable CORS in production if needed

### Health Check

```bash
curl https://your-app.railway.app/api/health
```

Should return:
```json
{
  "status": "healthy",
  "timestamp": "2024-01-15T10:30:00",
  "service": "MELKENS HIL Simulator",
  "version": "1.0"
}
```

## 📄 License

MIT License - See LICENSE file for details.

---

**🚀 Ready for immediate deployment on Railway!**