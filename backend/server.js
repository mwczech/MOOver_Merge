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
            motorRightSpeed: 0,
            motorLeftSpeed: 0,
            augerSpeed: 50,
            batteryVoltage: 12400,
            adcCurrent: 250,
            thumbleCurrent: 180,
            powerOn: false,
            charging: false,
            pmbConnection: "Connected",
            crcImu2PmbErrorCount: 0,
            crcPmb2ImuErrorCount: 0,
            crcEsp2ImuErrorCount: 0,
            magnetBarStatus: 0x0000,
            currentRoute: null,
            routeStatus: "stopped",
            joystickX: 0,
            joystickY: 0
        };
        this.startSimulation();
    }
    
    startSimulation() {
        setInterval(() => {
            this.updateSimulation();
        }, 100);
    }
    
    updateSimulation() {
        if (this.state.powerOn && !this.state.charging) {
            this.state.batteryVoltage = Math.max(10000, this.state.batteryVoltage - 0.1);
        } else if (this.state.charging) {
            this.state.batteryVoltage = Math.min(13800, this.state.batteryVoltage + 0.5);
        }
        
        const totalMotorSpeed = Math.abs(this.state.motorRightSpeed) + Math.abs(this.state.motorLeftSpeed);
        this.state.adcCurrent = 200 + (totalMotorSpeed * 0.8);
        this.state.thumbleCurrent = 150 + (totalMotorSpeed * 0.5);
        
        if (this.state.routeStatus === "playing") {
            this.state.magnetBarStatus = this.generateMagnetReading();
        }
        
        if (Math.random() < 0.001) {
            this.state.crcImu2PmbErrorCount++;
        }
    }
    
    generateMagnetReading() {
        const centerBits = [6, 7, 8, 9];
        let reading = 0;
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
        
        const forward = this.state.joystickY;
        const turn = this.state.joystickX;
        
        this.state.motorRightSpeed = Math.max(-255, Math.min(255, Math.round(forward + turn)));
        this.state.motorLeftSpeed = Math.max(-255, Math.min(255, Math.round(forward - turn)));
    }
    
    getState() {
        return { ...this.state };
    }
}

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
    console.log('Configuration updated');
    res.json({ 
        success: true, 
        message: 'Configuration updated successfully'
    });
});

app.post('/api/firmware/esp', upload.single('firmware'), (req, res) => {
    console.log('ESP firmware upload simulated');
    res.json({ 
        success: true, 
        message: 'ESP firmware upload simulated successfully'
    });
});

app.post('/api/firmware/pmb', upload.single('firmware'), (req, res) => {
    console.log('PMB firmware upload simulated');
    res.json({ 
        success: true, 
        message: 'PMB firmware upload simulated successfully'
    });
});

// Local Simulator Synchronization API
app.get('/api/routes', (req, res) => {
    // Return available routes for local simulator
    const routes = {
        "A": {
            "name": "Route A - Square Pattern",
            "waypoints": [
                {"x": 0, "y": 0, "speed": 50},
                {"x": 100, "y": 0, "speed": 50},
                {"x": 100, "y": 100, "speed": 50},
                {"x": 0, "y": 100, "speed": 50},
                {"x": 0, "y": 0, "speed": 30}
            ],
            "description": "Basic square navigation pattern"
        },
        "B": {
            "name": "Route B - Figure Eight", 
            "waypoints": [
                {"x": 0, "y": 50, "speed": 40},
                {"x": 50, "y": 0, "speed": 40},
                {"x": 100, "y": 50, "speed": 40},
                {"x": 50, "y": 100, "speed": 40},
                {"x": 0, "y": 50, "speed": 30}
            ],
            "description": "Figure-eight pattern for advanced testing"
        },
        "C": {
            "name": "Route C - Linear Sweep",
            "waypoints": [
                {"x": 0, "y": 0, "speed": 60},
                {"x": 200, "y": 0, "speed": 60},
                {"x": 200, "y": 20, "speed": 30},
                {"x": 0, "y": 20, "speed": 60},
                {"x": 0, "y": 40, "speed": 30}
            ],
            "description": "Linear sweeping pattern for area coverage"
        }
    };
    
    res.json(routes);
});

app.post('/api/upload_route', (req, res) => {
    const { name, data } = req.body;
    
    if (!name || !data) {
        return res.status(400).json({
            success: false,
            message: 'Route name and data are required'
        });
    }
    
    console.log(`Route uploaded from local simulator: ${name}`);
    
    // In a real implementation, you would save this to a database
    // For now, we just acknowledge the upload
    res.json({
        success: true,
        message: `Route ${name} uploaded successfully`,
        route: { name, data }
    });
});

app.get('/api/local_sync_status', (req, res) => {
    res.json({
        server_status: 'online',
        last_sync: new Date().toISOString(),
        routes_count: 3,
        version: '1.0.0',
        supports_local_sync: true
    });
});

app.post('/api/hil/connect', (req, res) => {
    robot.state.pmbConnection = "HIL Connected";
    res.json({ 
        success: true, 
        message: 'Connected via HIL'
    });
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
    
    socket.emit('robotState', robot.getState());
    
    const stateInterval = setInterval(() => {
        socket.emit('robotState', robot.getState());
    }, 200);
    
    socket.on('joystick', (data) => {
        robot.setJoystick(data.x, data.y);
    });
    
    socket.on('routeSelect', (routeIndex) => {
        robot.state.currentRoute = routeIndex;
    });
    
    socket.on('routeControl', (action) => {
        robot.state.routeStatus = action;
    });
    
    socket.on('powerControl', (powerOn) => {
        robot.state.powerOn = powerOn;
    });
    
    socket.on('chargingControl', (charging) => {
        robot.state.charging = charging;
    });
    
    socket.on('augerSpeed', (speed) => {
        robot.state.augerSpeed = Math.max(0, Math.min(1500, speed));
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

server.listen(PORT, '0.0.0.0', () => {
    console.log(`🚀 MOOver MELKENS Web Simulator running on port ${PORT}`);
    console.log(`🔗 WebSocket server ready`);
    console.log(`🤖 Robot simulation started`);
});

module.exports = app;
