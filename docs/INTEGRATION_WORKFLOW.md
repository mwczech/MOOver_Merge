# MELKENS Integration Workflow Guide

Complete step-by-step workflow for integrating, testing, and deploying the MELKENS robot system with web simulator and hardware-in-the-loop capabilities.

## 📋 Overview

This document provides a comprehensive workflow for setting up, testing, and validating the complete MELKENS system, from initial simulator setup to production deployment with hardware validation.

## 🎯 Workflow Phases

### Phase 1: Development Setup
### Phase 2: Route Design and Validation
### Phase 3: Simulation Testing
### Phase 4: Hardware Integration
### Phase 5: System Validation
### Phase 6: Production Deployment

---

## 🚀 Phase 1: Development Setup

### 1.1 Environment Preparation

**Prerequisites:**
- Node.js 18+ and npm
- Python 3.8+
- Docker (optional, for containerized deployment)
- Git

**Repository Setup:**
```bash
# Clone the repository
git clone https://github.com/melkens/robot-system.git
cd robot-system

# Install dependencies for all components
npm run install-all

# This runs:
# - npm install (root package scripts)
# - cd web-simulator && npm install
# - cd tools/route-editor && npm install
# - cd tools/fault-injector && npm install
```

### 1.2 Local Simulator Setup

**Start the web simulator:**
```bash
cd web-simulator

# Development mode (with hot reload)
npm run dev

# This starts:
# - Backend API server on http://localhost:3001
# - Frontend React app on http://localhost:3000
# - WebSocket server for real-time communication
```

**Verify simulator is running:**
```bash
# Check backend health
curl http://localhost:3001/api/health

# Expected response:
# {"success": true, "message": "API is healthy", "timestamp": "..."}

# Open frontend
open http://localhost:3000
```

### 1.3 Railway Deployment (Optional)

**Deploy to Railway cloud:**
```bash
# Install Railway CLI
npm install -g @railway/cli

# Login to Railway
railway login

# Deploy from root directory
railway deploy

# Monitor deployment
railway logs
```

**Environment Variables for Railway:**
```env
NODE_ENV=production
PORT=3001
FRONTEND_URL=https://your-app.railway.app
WEBSOCKET_PORT=3001
LOG_LEVEL=info
```

---

## 🎨 Phase 2: Route Design and Validation

### 2.1 Using the Route Editor

**Start standalone route editor:**
```bash
cd tools/route-editor
npm start

# Opens route editor at http://localhost:3002
```

**Design a simple test route:**
1. **Click** on canvas to add waypoints
2. **Select point types:** Normal, Turn Left, Turn Right, Dock, Stop
3. **Configure parameters:** Speed, duration, magnetic correction
4. **Save route** as JSON file

**Example route structure:**
```json
{
  "route": {
    "name": "Test Route 1",
    "description": "Basic rectangular path for testing",
    "steps": [
      {"id": 0, "operation": "NORM", "position": {"x": 0, "y": 0}, "speed": 0.3},
      {"id": 1, "operation": "NORM", "position": {"x": 100, "y": 0}, "speed": 0.3},
      {"id": 2, "operation": "TU_R", "position": {"x": 100, "y": 0}, "speed": 0.2, "duration": 1500},
      {"id": 3, "operation": "NORM", "position": {"x": 100, "y": 100}, "speed": 0.3},
      {"id": 4, "operation": "TU_R", "position": {"x": 100, "y": 100}, "speed": 0.2, "duration": 1500},
      {"id": 5, "operation": "NORM", "position": {"x": 0, "y": 100}, "speed": 0.3},
      {"id": 6, "operation": "TU_R", "position": {"x": 0, "y": 100}, "speed": 0.2, "duration": 1500},
      {"id": 7, "operation": "NORM", "position": {"x": 0, "y": 0}, "speed": 0.3},
      {"id": 8, "operation": "STOP", "position": {"x": 0, "y": 0}, "speed": 0}
    ]
  }
}
```

### 2.2 Route Validation

**Load route into simulator:**
```bash
# Using curl to upload route
curl -X POST http://localhost:3001/api/routes \
  -H "Content-Type: application/json" \
  -d @test_route_1.json

# Response includes route ID for testing
```

**Validate route parameters:**
```bash
# Check route validation
curl http://localhost:3001/api/routes/validate/ROUTE_ID

# Expected response shows validation status and any issues
```

---

## 🧪 Phase 3: Simulation Testing

### 3.1 Basic Simulation Test

**Start route execution in simulator:**
```bash
# Load route
curl -X POST http://localhost:3001/api/robot/load-route \
  -H "Content-Type: application/json" \
  -d '{"routeId": "ROUTE_ID"}'

# Start simulation
curl -X POST http://localhost:3001/api/robot/start

# Monitor progress via WebSocket or polling
curl http://localhost:3001/api/robot/status
```

**Monitor simulation via web interface:**
1. Open http://localhost:3000
2. Navigate to **Dashboard**
3. Select route from dropdown
4. Click **Start Simulation**
5. Monitor real-time robot position and sensor data

### 3.2 Fault Injection Testing

**Start fault injector:**
```bash
cd tools/fault-injector
npm run dev

# Or use CLI directly
npx fault-injector interactive
```

**Run predefined fault scenarios:**
```bash
# List available scenarios
npx fault-injector list

# Run emergency stop test
npx fault-injector run emergency_stop_test

# Run sensor failure cascade
npx fault-injector run sensor_failure_cascade

# Run motor blockage test
npx fault-injector run motor_block_scenario
```

**Custom fault injection:**
```bash
# Inject sensor failure
npx fault-injector custom \
  --type sensor \
  --target magneticPosition \
  --value '{"detected": false}' \
  --duration 5000

# Inject motor blockage
npx fault-injector custom \
  --type motor \
  --target left_motor \
  --value '{"blocked": true}' \
  --duration 3000
```

### 3.3 Performance Testing

**Test route completion:**
```bash
# Run multiple route executions and measure performance
for i in {1..10}; do
  echo "Test run $i"
  curl -X POST http://localhost:3001/api/robot/start
  sleep 30  # Wait for route completion
  curl http://localhost:3001/api/robot/status
done
```

**Collect performance metrics:**
```bash
# Export logs for analysis
curl http://localhost:3001/api/logs/export > simulation_logs.json

# Analyze timing and performance
python3 scripts/analyze_performance.py simulation_logs.json
```

---

## 🔧 Phase 4: Hardware Integration

### 4.1 Hardware Setup

**Physical connections (see tools/hardware-wiring/README.md):**
1. Connect UART adapter to PMB
2. Connect USB adapter to development PC
3. Verify electrical connections
4. Test basic communication

**Configure hardware bridge:**
```bash
# Set environment variables
export HIL_UART_PORT="/dev/ttyUSB0"  # Adjust for your system
export HIL_UART_BAUDRATE="115200"

# Test UART connection
echo '{"type":"command","command":"PING"}' > /dev/ttyUSB0
cat /dev/ttyUSB0  # Should show response
```

### 4.2 Hardware-in-the-Loop Setup

**Initialize hardware bridge in simulator:**
```bash
# Initialize hardware connection
curl -X POST http://localhost:3001/api/hardware/initialize \
  -H "Content-Type: application/json" \
  -d '{
    "serialPort": "/dev/ttyUSB0",
    "baudRate": 115200
  }'

# Enable HIL mode
curl -X POST http://localhost:3001/api/hardware/hil-mode \
  -H "Content-Type: application/json" \
  -d '{"enabled": true}'

# Check connection status
curl http://localhost:3001/api/hardware/status
```

### 4.3 Hardware Validation

**Test hardware communication:**
```bash
# Send test commands to hardware
curl -X POST http://localhost:3001/api/hardware/command \
  -H "Content-Type: application/json" \
  -d '{"command": "GET_STATUS"}'

# Test route upload to hardware
curl -X POST http://localhost:3001/api/hardware/send-route \
  -H "Content-Type: application/json" \
  -d '{"route": [...route_steps...]}'
```

**Monitor hardware data:**
```bash
# View real-time hardware messages
curl http://localhost:3001/api/hardware/messages?limit=50

# Export hardware logs
curl http://localhost:3001/api/hardware/export-logs > hardware_logs.json
```

---

## ✅ Phase 5: System Validation

### 5.1 Route Validation and Certification

**Create golden run:**
```bash
# Execute route and save as golden run
curl -X POST http://localhost:3001/api/validation/save-golden-run \
  -H "Content-Type: application/json" \
  -d '{
    "routeId": "ROUTE_ID",
    "logs": [...execution_logs...],
    "finalState": {...final_robot_state...},
    "checkpoints": [...route_checkpoints...],
    "metadata": {
      "description": "Golden run for Test Route 1",
      "version": "1.0",
      "approved_by": "engineer@melkens.com",
      "certification_level": "development"
    }
  }'
```

**Validate subsequent runs:**
```bash
# Run validation against golden run
curl -X POST http://localhost:3001/api/validation/validate \
  -H "Content-Type: application/json" \
  -d '{
    "routeId": "ROUTE_ID",
    "logs": [...current_execution_logs...],
    "finalState": {...current_final_state...},
    "metadata": {
      "testRun": "validation_001",
      "operator": "test@melkens.com"
    }
  }'

# Generate certification report
curl http://localhost:3001/api/validation/certification/VALIDATION_ID
```

### 5.2 Automated Test Suite

**Run complete validation suite:**
```bash
# Create test script
cat > run_validation_suite.sh << EOF
#!/bin/bash

echo "Starting MELKENS validation suite..."

# Test 1: Basic route execution
echo "Test 1: Basic route execution"
npx fault-injector run route_execution_test

# Test 2: Emergency stop response
echo "Test 2: Emergency stop response"
npx fault-injector run emergency_stop_test

# Test 3: Sensor failure handling
echo "Test 3: Sensor failure handling"
npx fault-injector run sensor_failure_cascade

# Test 4: Hardware communication
echo "Test 4: Hardware communication"
curl -X POST http://localhost:3001/api/hardware/test-connection

# Test 5: Route validation
echo "Test 5: Route validation"
curl -X POST http://localhost:3001/api/validation/auto-validate-all

echo "Validation suite completed!"
EOF

chmod +x run_validation_suite.sh
./run_validation_suite.sh
```

### 5.3 Performance Benchmarks

**Establish performance baselines:**
```bash
# Route execution time
# Expected: <30 seconds for basic rectangular route
# Tolerance: ±10%

# Position accuracy
# Expected: ±5cm deviation from planned path
# Tolerance: ±2cm

# Emergency stop response
# Expected: <100ms response time
# Tolerance: <50ms

# Communication latency
# Expected: <10ms for sensor data updates
# Tolerance: <5ms
```

---

## 🚀 Phase 6: Production Deployment

### 6.1 Environment Configuration

**Production environment setup:**
```bash
# Set production environment variables
export NODE_ENV=production
export LOG_LEVEL=info
export ENABLE_METRICS=true
export HEALTH_CHECK_INTERVAL=30000

# Database configuration (if using persistent storage)
export DATABASE_URL="postgresql://user:pass@host:5432/melkens"

# Security settings
export JWT_SECRET="your-secret-key"
export CORS_ORIGIN="https://your-domain.com"
```

### 6.2 Production Build

**Build for production:**
```bash
# Build all components
npm run build

# This creates:
# - web-simulator/backend/dist/ (compiled backend)
# - web-simulator/frontend/dist/ (optimized frontend)
# - tools/*/dist/ (compiled tools)
```

### 6.3 Deployment Options

#### Option A: Railway Deployment

```bash
# Deploy to Railway with production config
railway deploy --service web-simulator

# Set environment variables in Railway dashboard
railway variables set NODE_ENV=production
railway variables set LOG_LEVEL=info
```

#### Option B: Docker Deployment

```bash
# Build Docker image
docker build -t melkens/web-simulator .

# Run container
docker run -d \
  --name melkens-simulator \
  -p 3000:3000 \
  -p 3001:3001 \
  -e NODE_ENV=production \
  melkens/web-simulator
```

#### Option C: Traditional Server Deployment

```bash
# Copy files to server
rsync -av dist/ user@server:/opt/melkens/

# Start with PM2
pm2 start ecosystem.config.js
pm2 save
pm2 startup
```

### 6.4 Health Monitoring

**Setup monitoring endpoints:**
```bash
# Health check endpoint
curl http://your-domain.com/api/health

# Metrics endpoint
curl http://your-domain.com/api/metrics

# System status
curl http://your-domain.com/api/system/status
```

**Configure alerts:**
```bash
# Setup monitoring with your preferred service
# Examples: DataDog, New Relic, Prometheus + Grafana

# Key metrics to monitor:
# - API response times
# - WebSocket connection count
# - Memory usage
# - CPU usage
# - Hardware connection status
# - Route execution success rate
```

---

## 📊 Testing Scenarios

### Scenario 1: Basic Functionality Test

**Objective:** Verify basic simulator functionality
**Duration:** 15 minutes

```bash
# 1. Start simulator
npm run dev

# 2. Load test route
curl -X POST http://localhost:3001/api/routes -d @test_routes/basic_rectangle.json

# 3. Execute route
curl -X POST http://localhost:3001/api/robot/start

# 4. Verify completion
# Expected: Route completes successfully within 30 seconds
# Expected: Position accuracy within ±5cm
# Expected: No errors in logs
```

### Scenario 2: Fault Tolerance Test

**Objective:** Test system response to failures
**Duration:** 20 minutes

```bash
# 1. Start simulation with route
curl -X POST http://localhost:3001/api/robot/start

# 2. Inject sensor failure
npx fault-injector run sensor_failure_cascade

# 3. Verify recovery
# Expected: System detects failure within 1 second
# Expected: Appropriate safety response activated
# Expected: System recovers when fault cleared
```

### Scenario 3: Hardware Integration Test

**Objective:** Validate hardware-in-the-loop functionality
**Duration:** 30 minutes

```bash
# 1. Connect hardware (see HIL guide)
# 2. Initialize hardware bridge
curl -X POST http://localhost:3001/api/hardware/initialize

# 3. Enable HIL mode
curl -X POST http://localhost:3001/api/hardware/hil-mode -d '{"enabled": true}'

# 4. Execute route on hardware
curl -X POST http://localhost:3001/api/hardware/start-route

# 5. Verify hardware response
# Expected: Hardware executes commands within 100ms
# Expected: Sensor data matches expected values
# Expected: No communication timeouts
```

### Scenario 4: Performance Benchmark

**Objective:** Establish performance baselines
**Duration:** 45 minutes

```bash
# Run automated benchmark suite
python3 scripts/run_benchmarks.py

# Expected results:
# - Route execution: <30s for 10-step route
# - API response time: <100ms for all endpoints
# - WebSocket latency: <50ms
# - Memory usage: <512MB
# - CPU usage: <50% under normal load
```

---

## 🔧 Troubleshooting Guide

### Common Issues

#### Issue: Simulator won't start

**Symptoms:**
- Port already in use errors
- Module not found errors
- Database connection failures

**Solutions:**
```bash
# Check for running processes
lsof -i :3000
lsof -i :3001

# Kill existing processes
pkill -f "node.*simulator"

# Reinstall dependencies
rm -rf node_modules package-lock.json
npm install

# Check environment variables
env | grep NODE_ENV
```

#### Issue: Hardware connection fails

**Symptoms:**
- UART device not found
- Permission denied errors
- No response from hardware

**Solutions:**
```bash
# Check USB devices
lsusb | grep -i uart

# Fix permissions
sudo chmod 666 /dev/ttyUSB0
sudo usermod -a -G dialout $USER

# Test connection
minicom -b 115200 -D /dev/ttyUSB0
```

#### Issue: Route validation fails

**Symptoms:**
- Position accuracy errors
- Timing violations
- Unexpected route termination

**Solutions:**
```bash
# Check route parameters
curl http://localhost:3001/api/routes/validate/ROUTE_ID

# Analyze logs
curl http://localhost:3001/api/logs/export | jq '.[] | select(.level == "error")'

# Adjust route settings
# - Reduce speeds for better accuracy
# - Increase tolerances for timing
# - Add intermediate checkpoints
```

---

## 📋 Deployment Checklist

### Pre-deployment Checklist

- [ ] All tests pass in development environment
- [ ] Route validation completes successfully
- [ ] Hardware integration tested (if applicable)
- [ ] Performance benchmarks meet requirements
- [ ] Security review completed
- [ ] Documentation updated
- [ ] Backup procedures verified

### Deployment Steps

- [ ] Build production artifacts
- [ ] Configure production environment variables
- [ ] Deploy to staging environment
- [ ] Run smoke tests in staging
- [ ] Deploy to production
- [ ] Verify health checks
- [ ] Monitor logs for errors
- [ ] Validate core functionality

### Post-deployment Checklist

- [ ] All health checks passing
- [ ] Monitoring alerts configured
- [ ] Performance metrics within expected ranges
- [ ] User acceptance testing completed
- [ ] Documentation updated with production details
- [ ] Team notified of deployment completion

---

## 📞 Support and Resources

### Documentation
- [Web Simulator README](../web-simulator/README.md)
- [Route Editor Guide](../tools/route-editor/README.md)
- [Fault Injector Manual](../tools/fault-injector/README.md)
- [Hardware Wiring Guide](../tools/hardware-wiring/README.md)

### API Documentation
- REST API endpoints: http://localhost:3001/api/docs
- WebSocket events: [WebSocket API](../docs/WEBSOCKET_API.md)
- Hardware commands: [Hardware API](../docs/HARDWARE_API.md)

### Support Channels
- **Email:** support@melkens.com
- **Issues:** https://github.com/melkens/robot-system/issues
- **Documentation:** https://docs.melkens.com
- **Community:** https://discord.gg/melkens

---

## 📈 Success Metrics

### System Performance
- Route execution success rate: >99%
- Position accuracy: ±5cm
- Emergency stop response: <100ms
- API response time: <100ms
- System uptime: >99.9%

### Development Workflow
- Setup time for new developers: <30 minutes
- Route design to validation time: <1 hour
- Fault scenario creation time: <15 minutes
- Hardware integration time: <2 hours

### User Experience
- Simulator startup time: <30 seconds
- Route loading time: <5 seconds
- Real-time update latency: <50ms
- Documentation completeness: 100% coverage

---

This comprehensive workflow ensures successful integration, testing, and deployment of the MELKENS robot system with confidence in both simulated and hardware environments.