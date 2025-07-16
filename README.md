
# MOOver_Merge – MELKENS + WB Integration with HIL Simulator

## Overview

This repository provides a complete Hardware-in-the-Loop (HIL) integration system for the MELKENS robot platform. It combines the MELKENS hardware with Wasserbauer (WB) navigation software, featuring a modern web-based simulator for development and testing.

## 🚀 New: HIL IMU Integration

The system now includes full Hardware-in-the-Loop support with:
- **Real-time IMU data acquisition** from MELKENS IMU board (LSM6DSR + LIS3MDL)
- **Web-based visualization** with interactive dashboard
- **Seamless switching** between simulated and hardware modes
- **Fault injection capabilities** for robustness testing
- **Comprehensive data logging** for analysis
- **Automated self-test** and diagnostics

## Repository Structure

```
MOOver_Merge/
├── Melkens/                    # MELKENS hardware code
│   ├── Melkens_IMU/           # IMU board firmware (STM32G473)
│   ├── Melkens_PMB/           # Power Management Board
│   ├── Melkens_Connectivity/  # ESP32 connectivity module
│   └── Melkens_Lib/          # Shared libraries and types
├── WB/                        # Wasserbauer navigation system
│   ├── bin/                   # Compiled binaries
│   ├── config.sh             # System configuration
│   └── firmware files
├── web-simulator/             # 🆕 HIL Web Simulator
│   ├── backend/              # FastAPI backend server
│   │   ├── main.py           # Main application
│   │   ├── imu_manager.py    # IMU communication handler
│   │   ├── robot_simulator.py # Robot physics simulation
│   │   ├── data_logger.py    # Data logging system
│   │   ├── requirements.txt  # Python dependencies
│   │   ├── start.sh         # Linux/macOS startup script
│   │   └── start.bat        # Windows startup script
│   └── frontend/             # Modern web interface
│       ├── index.html        # Main dashboard
│       ├── style.css         # Responsive dark theme
│       └── main.js          # Real-time visualization
├── docs/                     # Documentation
│   └── HIL_IMU_GUIDE.md     # 🆕 Complete HIL setup guide
├── logs/                     # 🆕 Data logging directory
└── README.md                 # This file
```

## Quick Start

### 1. HIL Simulator Setup (Recommended)

The fastest way to get started is with the new web-based HIL simulator:

#### Linux/macOS:
```bash
cd web-simulator/backend
chmod +x start.sh
./start.sh
```

#### Windows:
```cmd
cd web-simulator\backend
start.bat
```

#### Access the Interface:
Open your browser to `http://localhost:8000`

### 2. Hardware Requirements

**For HIL Mode:**
- MELKENS IMU Board with STM32G473
- USB connection to host computer
- Python 3.8+ with pip

**For Simulation Mode:**
- Any computer with Python 3.8+
- Modern web browser (Chrome, Firefox, Safari, Edge)

### 3. Configuration

The web interface provides easy configuration for:
- **IMU Mode**: Switch between simulated and hardware
- **Serial Port**: Auto-detection with manual override
- **Update Rate**: Configurable from 10Hz to 100Hz
- **Fault Injection**: Test system robustness

## Features

### 🎛️ Web Dashboard
- **Real-time visualization** of IMU data and robot position
- **Interactive plots** for accelerometer, gyroscope, and magnetometer
- **Robot path tracking** with live position updates
- **System status monitoring** with error reporting

### 🔧 Hardware Integration
- **MELKENS Protocol Support** with CRC16 validation
- **Auto-detection** of connected IMU boards
- **Seamless fallback** to simulation mode
- **Performance monitoring** with packet statistics

### 🧪 Testing & Validation
- **Fault injection** (drift, stuck axis, noise, packet loss)
- **Automated self-test** with detailed reporting
- **Data logging** to CSV for post-analysis
- **Route execution** with predefined patterns

### 📊 Advanced Features
- **Multiple route support** (A, B, C, D) with magnetic line following
- **Real-time WebSocket** communication
- **RESTful API** for external integration
- **Comprehensive error handling** and recovery

## System Architecture

### Hardware Layer
```
MELKENS IMU Board (STM32G473)
├── LSM6DSR (Accelerometer + Gyroscope)
├── LIS3MDL (Magnetometer)
└── USB/UART Communication
```

### Software Layer
```
Web Browser ↔ WebSocket ↔ FastAPI Backend ↔ Serial Port ↔ IMU Board
     ↑              ↑           ↑               ↑
Real-time UI   Live Data   IMU Manager    Hardware
```

### Data Flow
1. **IMU Board** → Sensor readings via UART
2. **IMU Manager** → Protocol parsing & validation
3. **Robot Simulator** → Physics & navigation logic
4. **Data Logger** → CSV export for analysis
5. **WebSocket** → Real-time dashboard updates

## Route System

The system supports predefined routes with magnetic line following:

### Route A - Rectangle
```python
# Simple rectangular pattern
steps = [
    RouteStep(dx=2.0, dy=0.0, speed=500),      # Move right 2m
    RouteStep(dx=0.0, dy=1.5, speed=500),      # Move up 1.5m  
    RouteStep(dx=-2.0, dy=0.0, speed=500),     # Move left 2m
    RouteStep(dx=0.0, dy=-1.5, speed=500)      # Return to start
]
```

### Route B - L-Shape
With magnetic corrections for line following precision.

### Route C - Complex
Multiple direction changes with variable speeds.

### Route D - Circular
Continuous operation pattern.

## Development Workflow

### 1. Simulation Development
Start with simulated IMU data for rapid prototyping:
```bash
# Access web interface, set mode to "Simulated"
# Develop and test navigation algorithms
# Verify route execution and fault handling
```

### 2. Hardware Integration
Connect real IMU board for validation:
```bash
# Connect MELKENS IMU via USB
# Switch mode to "Hardware" in web interface
# Run self-test to verify connection
# Compare simulated vs hardware data
```

### 3. Testing & Validation
Use fault injection for robustness testing:
```bash
# Enable fault injection in web interface
# Test sensor drift, stuck axis, noise scenarios
# Validate error handling and recovery
# Analyze logged data for performance metrics
```

## Troubleshooting

### Common Issues

**"Serial port not found"**
- Check USB connection
- Verify user permissions (Linux: add to `dialout` group)
- Try different USB ports/cables

**"IMU data appears frozen"**
- Power cycle the IMU board
- Check baud rate configuration
- Verify firmware compatibility

**"High error rate"**
- Check cable quality
- Reduce baud rate
- Verify power supply stability

**"WebSocket disconnections"**
- Check browser console for errors
- Verify network connectivity
- Disable ad blockers

See `docs/HIL_IMU_GUIDE.md` for comprehensive troubleshooting.

## API Reference

### REST Endpoints
```bash
GET  /api/imu/status          # Get IMU system status
GET  /api/imu/data            # Get latest IMU data
POST /api/imu/self-test       # Run comprehensive self-test
GET  /api/config              # Get system configuration
POST /api/config/system       # Update system settings
POST /api/config/fault-injection  # Configure fault injection
POST /api/robot/route/{id}    # Set robot route (A, B, C, D)
GET  /api/logs                # Get recent log entries
```

### WebSocket Events
```javascript
// Real-time data streaming
{
  "type": "imu_data",
  "data": {
    "accelerometer": {"x": 0.12, "y": -0.05, "z": 9.81},
    "gyroscope": {"x": 0.001, "y": 0.002, "z": -0.001},
    "magnetometer": {"x": 25000, "y": -2000, "z": 40000},
    "euler_angles": {"roll": 0.01, "pitch": -0.005, "yaw": 1.57},
    "position": {"x": 1.23, "y": 0.45, "z": 0.0},
    "hardware_connected": true
  }
}
```

## Contributing

### Development Setup
```bash
# Clone repository
git clone <repository-url>
cd MOOver_Merge

# Set up Python environment
cd web-simulator/backend
python3 -m venv venv
source venv/bin/activate  # Linux/macOS
# or: venv\Scripts\activate.bat  # Windows

# Install dependencies
pip install -r requirements.txt

# Start development server
python main.py
```

### Code Style
- **Python**: Follow PEP 8, use type hints
- **JavaScript**: ES6+, async/await preferred
- **HTML/CSS**: Semantic markup, responsive design
- **Documentation**: Clear docstrings and comments

## License

This project is proprietary to Melkens Sp. z o.o. Unauthorized distribution is prohibited.

## Support

- **Technical Support**: support@melkens.com
- **Documentation**: docs@melkens.com  
- **Hardware Issues**: hardware@melkens.com

## Version History

- **v2.0.0** (Current) - Full HIL integration with web simulator
- **v1.5.0** - Enhanced MELKENS IMU firmware
- **v1.0.0** - Initial MELKENS + WB integration

---

**Powered by MELKENS** | **Compatible with WB Navigation System**
