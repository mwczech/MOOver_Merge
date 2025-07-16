const express = require('express');
const http = require('http');
const socketIO = require('socket.io');
const cors = require('cors');
const helmet = require('helmet');
const compression = require('compression');
const path = require('path');
const multer = require('multer');
require('dotenv').config();

const app = express();
const server = http.createServer(app);
const io = socketIO(server, {
    cors: {
        origin: "*",
        methods: ["GET", "POST"]
    }
});

const PORT = process.env.PORT || 3001;

// Middleware
app.use(helmet({
    contentSecurityPolicy: false,
    crossOriginEmbedderPolicy: false
}));
app.use(compression());
app.use(cors());
app.use(express.json());
app.use(express.urlencoded({ extended: true }));

// Serve static files from React build
app.use(express.static(path.join(__dirname, '../frontend/build')));

// Configure multer for file uploads
const upload = multer({
    storage: multer.memoryStorage(),
    limits: {
        fileSize: 10 * 1024 * 1024 // 10MB limit
    }
});

// MOOver MELKENS Robot Simulator State
class RobotSimulator {
    constructor() {
        this.state = {
            // Motor states
            motorRightSpeed: 0,
            motorLeftSpeed: 0,
            augerSpeed: 50,
            
            // Power and battery
            batteryVoltage: 12400, // mV
            adcCurrent: 250, // mA
            thumbleCurrent: 180, // mA
            powerOn: false,
            charging: false,
            
            // Connection states
            pmbConnection: "Connected",
            
            // Error counters
            crcImu2PmbErrorCount: 0,
            crcPmb2ImuErrorCount: 0,
            crcEsp2ImuErrorCount: 0,
            
            // Position and sensors
            magnetBarStatus: 0x0000, // 16-bit sensor reading
            currentRoute: null,
            routeStatus: "stopped", // "playing", "paused", "stopped"
            
            // Joystick
            joystickX: 0,
            joystickY: 0,
            
            // Configuration
            config: {
                wifi: { ssid: "MELKENS-AP", password: "" },
                mqtt: { broker: "192.168.1.100" },
                network: { 
                    espIp: "192.168.1.201", 
                    gatewayIp: "192.168.1.1",
                    subnetIp: "255.255.255.0"
                }
            }
        };
        
        this.routes = ['A', 'B', 'C', 'D', 'F', 'G', 'H', 'I', 'J', 'K'];
        this.isSimulating = false;
        this.simulationInterval = null;
        
        this.startSimulation();
    }
    
    startSimulation() {
        if (this.isSimulating) return;
        
        this.isSimulating = true;
        this.simulationInterval = setInterval(() => {
            this.updateSimulation();
        }, 100); // Update every 100ms
    }
    
    stopSimulation() {
        this.isSimulating = false;
        if (this.simulationInterval) {
            clearInterval(this.simulationInterval);
            this.simulationInterval = null;
        }
    }
    
    updateSimulation() {
        // Simulate battery drain
        if (this.state.powerOn && !this.state.charging) {
            this.state.batteryVoltage = Math.max(10000, this.state.batteryVoltage - 0.1);
        } else if (this.state.charging) {
            this.state.batteryVoltage = Math.min(13800, this.state.batteryVoltage + 0.5);
        }
        
        // Simulate current consumption based on motor speeds
        const totalMotorSpeed = Math.abs(this.state.motorRightSpeed) + Math.abs(this.state.motorLeftSpeed);
        this.state.adcCurrent = 200 + (totalMotorSpeed * 0.8);
        this.state.thumbleCurrent = 150 + (totalMotorSpeed * 0.5);
        
        // Simulate random sensor data (magnetic line following)
        if (this.state.routeStatus === "playing") {
            // Simulate moving along magnetic line
            this.state.magnetBarStatus = this.generateMagnetReading();
        }
        
        // Randomly generate some errors for realism
        if (Math.random() < 0.001) { // 0.1% chance per update
            this.state.crcImu2PmbErrorCount++;
        }
        if (Math.random() < 0.0005) {
            this.state.crcPmb2ImuErrorCount++;
        }
    }
    
    generateMagnetReading() {
        // Simulate 16-bit magnetic sensor reading
        // Bits represent magnetic field detection across sensor array
        const centerBits = [6, 7, 8, 9]; // Center sensors
        let reading = 0;
        
        // Simulate line following - usually 2-4 center bits active
        const numActiveBits = 2 + Math.floor(Math.random() * 3);
        const startBit = centerBits[0] + Math.floor(Math.random() * (centerBits.length - numActiveBits + 1));
        
        for (let i = 0; i < numActiveBits; i++) {
            reading |= (1 << (startBit + i));
        }
        
        return reading;
    }
    
    setJoystick(x, y) {
        this.state.joystickX = Math.max(-100, Math.min(100, x));
        this.state.joystickY = Math.max(-100, Math.min(100, y));
        
        // Convert joystick to motor speeds (differential drive)
        const forward = this.state.joystickY;
        const turn = this.state.joystickX;
        
        this.state.motorRightSpeed = Math.round(forward + turn);
        this.state.motorLeftSpeed = Math.round(forward - turn);
        
        // Limit motor speeds
        this.state.motorRightSpeed = Math.max(-255, Math.min(255, this.state.motorRightSpeed));
        this.state.motorLeftSpeed = Math.max(-255, Math.min(255, this.state.motorLeftSpeed));
    }
    
    setRoute(routeIndex) {
        if (routeIndex >= 0 && routeIndex < this.routes.length) {
            this.state.currentRoute = routeIndex;
            console.log(`Route ${this.routes[routeIndex]} selected`);
        }
    }
    
    setRouteStatus(status) {
        this.state.routeStatus = status;
        console.log(`Route status: ${status}`);
        
        if (status === "stopped") {
            this.state.motorRightSpeed = 0;
            this.state.motorLeftSpeed = 0;
            this.state.magnetBarStatus = 0;
        }
    }
    
    setPower(powerOn) {
        this.state.powerOn = powerOn;
        if (!powerOn) {
            this.state.motorRightSpeed = 0;
            this.state.motorLeftSpeed = 0;
            this.state.routeStatus = "stopped";
        }
        console.log(`Power ${powerOn ? 'ON' : 'OFF'}`);
    }
    
    setCharging(charging) {
        this.state.charging = charging;
        console.log(`Charging ${charging ? 'ON' : 'OFF'}`);
    }
    
    setAugerSpeed(speed) {
        this.state.augerSpeed = Math.max(0, Math.min(1500, speed));
        console.log(`Auger speed: ${this.state.augerSpeed}`);
    }
    
    getState() {
        return { ...this.state };
    }
}

// Initialize robot simulator
const robot = new RobotSimulator();

// API Routes
app.get('/api/health', (req, res) => {
    res.json({ 
        status: 'OK', 
        timestamp: new Date().toISOString(),
        version: '1.0.0'
    });
});

app.get('/api/status', (req, res) => {
    res.json(robot.getState());
});

app.post('/api/config', upload.single('config'), (req, res) => {
    try {
        if (req.file) {
            const config = JSON.parse(req.file.buffer.toString());
            robot.state.config = { ...robot.state.config, ...config };
            console.log('Configuration updated from file');
        } else if (req.body) {
            robot.state.config = { ...robot.state.config, ...req.body };
            console.log('Configuration updated from JSON');
        }
        
        res.json({ 
            success: true, 
            message: 'Configuration updated successfully',
            config: robot.state.config
        });
    } catch (error) {
        console.error('Config update error:', error);
        res.status(400).json({ 
            success: false, 
            message: 'Invalid configuration data' 
        });
    }
});

app.post('/api/firmware/esp', upload.single('firmware'), (req, res) => {
    console.log('ESP firmware upload simulated');
    res.json({ 
        success: true, 
        message: 'ESP firmware upload simulated successfully',
        fileSize: req.file ? req.file.size : 0
    });
});

app.post('/api/firmware/pmb', upload.single('firmware'), (req, res) => {
    console.log('PMB firmware upload simulated');
    res.json({ 
        success: true, 
        message: 'PMB firmware upload simulated successfully',
        fileSize: req.file ? req.file.size : 0
    });
});

// HIL (Hardware-in-the-Loop) simulation endpoints
app.post('/api/hil/connect', (req, res) => {
    const { deviceType, connectionString } = req.body;
    console.log(`HIL connection request: ${deviceType} at ${connectionString}`);
    
    // Simulate connection attempt
    const success = Math.random() > 0.1; // 90% success rate
    
    if (success) {
        robot.state.pmbConnection = "HIL Connected";
        res.json({ 
            success: true, 
            message: `Connected to ${deviceType} via HIL`,
            deviceType,
            connectionString
        });
    } else {
        res.status(500).json({ 
            success: false, 
            message: `Failed to connect to ${deviceType}` 
        });
    }
});

app.post('/api/hil/disconnect', (req, res) => {
    robot.state.pmbConnection = "Disconnected";
    res.json({ 
        success: true, 
        message: 'HIL disconnected' 
    });
});

// WebSocket handling
io.on('connection', (socket) => {
    console.log('Client connected:', socket.id);
    
    // Send initial state
    socket.emit('robotState', robot.getState());
    
    // Set up periodic state updates
    const stateInterval = setInterval(() => {
        socket.emit('robotState', robot.getState());
    }, 200); // Send updates every 200ms
    
    // Joystick control
    socket.on('joystick', (data) => {
        robot.setJoystick(data.x, data.y);
    });
    
    // Route control
    socket.on('routeSelect', (routeIndex) => {
        robot.setRoute(routeIndex);
    });
    
    socket.on('routeControl', (action) => {
        robot.setRouteStatus(action); // "playing", "paused", "stopped"
    });
    
    // Power control
    socket.on('powerControl', (powerOn) => {
        robot.setPower(powerOn);
    });
    
    socket.on('chargingControl', (charging) => {
        robot.setCharging(charging);
    });
    
    // Auger speed control
    socket.on('augerSpeed', (speed) => {
        robot.setAugerSpeed(speed);
    });
    
    // Motor direct control
    socket.on('motorControl', (data) => {
        const { command, value } = data;
        
        switch (command) {
            case 'V': // Forward speed
                robot.state.motorRightSpeed = robot.state.motorLeftSpeed = value;
                break;
            case 'X': // Turn
                robot.state.motorRightSpeed = value;
                robot.state.motorLeftSpeed = -value;
                break;
            case 'WH': // Power high
                robot.setPower(true);
                break;
            case 'WL': // Power low
                robot.setPower(false);
                break;
            default:
                console.log('Unknown motor command:', command, value);
        }
    });
    
    socket.on('disconnect', () => {
        console.log('Client disconnected:', socket.id);
        clearInterval(stateInterval);
    });
});

// Serve React app for all other routes
app.get('*', (req, res) => {
    res.sendFile(path.join(__dirname, '../frontend/build/index.html'));
});

// Graceful shutdown
process.on('SIGTERM', () => {
    console.log('SIGTERM received, shutting down gracefully');
    robot.stopSimulation();
    server.close(() => {
        process.exit(0);
    });
});

process.on('SIGINT', () => {
    console.log('SIGINT received, shutting down gracefully');
    robot.stopSimulation();
    server.close(() => {
        process.exit(0);
    });
});

server.listen(PORT, '0.0.0.0', () => {
    console.log(`🚀 MOOver MELKENS Web Simulator running on port ${PORT}`);
    console.log(`📊 Environment: ${process.env.NODE_ENV || 'development'}`);
    console.log(`🔗 WebSocket server ready`);
    console.log(`🤖 Robot simulation started`);
});

module.exports = app;