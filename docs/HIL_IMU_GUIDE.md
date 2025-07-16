# MELKENS IMU Hardware-in-the-Loop (HIL) Setup Guide

## Table of Contents
- [Overview](#overview)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Hardware Setup](#hardware-setup)
- [Driver Installation](#driver-installation)
- [Wiring Diagrams](#wiring-diagrams)
- [Software Configuration](#software-configuration)
- [Testing and Validation](#testing-and-validation)
- [Troubleshooting](#troubleshooting)
- [Known Issues](#known-issues)
- [Performance Optimization](#performance-optimization)
- [Safety Considerations](#safety-considerations)

## Overview

The MELKENS IMU Hardware-in-the-Loop (HIL) integration enables real-time connection between the web-based robot simulator and physical IMU hardware. This setup allows for:

- **Real-time IMU data streaming** (50ms intervals)
- **Hardware-based navigation and route execution**
- **Fault injection testing** with physical sensors
- **Complete sensor data logging** for analysis
- **Seamless switching** between simulation and hardware modes

### Supported IMU Hardware
- **Primary**: MELKENS IMU Board (LSM6DSR + LIS3MDL + STM32G4)
- **Sensors**: 
  - LSM6DSR: 3-axis accelerometer + 3-axis gyroscope
  - LIS3MDL: 3-axis magnetometer
  - 32-position magnetic line sensor array
- **Communication**: UART at 115200 bps with CRC validation
- **Protocol**: Custom binary frame format with real-time streaming

## Hardware Requirements

### IMU Hardware
```
┌─────────────────────────────────────────┐
│           MELKENS IMU Board             │
├─────────────────────────────────────────┤
│ • STM32G4 microcontroller               │
│ • LSM6DSR (Accel/Gyro)                  │
│ • LIS3MDL (Magnetometer)                │
│ • 32x Hall effect sensors               │
│ • UART interface                        │
│ • 5V/3.3V power supply                  │
│ • Status LEDs                           │
└─────────────────────────────────────────┘
```

### Host Computer Requirements
- **Operating System**: Windows 10/11, Linux (Ubuntu 18.04+), macOS 10.15+
- **USB Ports**: At least 1 available USB port
- **RAM**: Minimum 4GB, recommended 8GB+
- **CPU**: Intel i5 or equivalent AMD processor
- **Network**: Ethernet or WiFi for web interface access

### Cables and Connectors
- **USB to UART adapter** (FT232RL, CP2102, or CH340G based)
- **Dupont jumper wires** (male-to-female, 20cm minimum)
- **Power supply**: 5V DC, minimum 1A capability
- **Optional**: Logic analyzer for debugging (Saleae, DSLogic, etc.)

## Software Requirements

### Development Environment
```bash
# Node.js and npm
node --version  # v18.0.0 or higher
npm --version   # v8.0.0 or higher

# TypeScript
npm install -g typescript

# Serial port dependencies
npm install serialport @serialport/parser-readline
```

### System Dependencies

#### Linux (Ubuntu/Debian)
```bash
# Install build tools and USB libraries
sudo apt update
sudo apt install -y build-essential libudev-dev

# Add user to dialout group for serial port access
sudo usermod -a -G dialout $USER
# Logout and login again for group changes to take effect

# Install Node.js (if not already installed)
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt-get install -y nodejs
```

#### Windows
```powershell
# Install Node.js from https://nodejs.org/
# Install Windows Build Tools
npm install -g windows-build-tools

# Install USB drivers (automatic with Windows 10+)
# Manual driver installation may be required for some USB-UART adapters
```

#### macOS
```bash
# Install Homebrew (if not already installed)
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install Node.js
brew install node

# Install Xcode command line tools
xcode-select --install
```

## Hardware Setup

### Power Supply Configuration
```
                    ┌─────────────────┐
   5V DC Power ──→  │  IMU Board      │
   GND         ──→  │                 │
                    │  Power LED: ON  │
                    └─────────────────┘
```

**Power Requirements:**
- **Voltage**: 5V DC (±0.25V tolerance)
- **Current**: 200-500mA typical, 800mA maximum
- **Ripple**: <50mV peak-to-peak
- **Power LED**: Should illuminate steady green when properly powered

### IMU Board Status Indicators
| LED Color | Status | Meaning |
|-----------|---------|---------|
| Green (Solid) | Power | Board powered correctly |
| Blue (Blinking) | Communication | UART data transmission active |
| Red (Solid) | Error | Sensor initialization failure |
| Yellow (Blinking) | Calibration | IMU calibration in progress |

## Driver Installation

### Automatic Driver Detection
The system supports automatic detection of common USB-UART adapters:

```typescript
// Supported chip types (auto-detected)
const supportedChips = [
  'FT232R',    // FTDI
  'CP210x',    // Silicon Labs  
  'CH340',     // WinChipHead
  'PL2303',    // Prolific
  'CDC-ACM'    // Generic USB CDC
];
```

### Manual Driver Installation

#### Windows Driver Installation
1. **FTDI Drivers**: Download from [FTDI website](https://ftdichip.com/drivers/)
2. **Silicon Labs CP210x**: Download from [Silicon Labs](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
3. **CH340G**: Often included with Windows 10+, or download from manufacturer

**Installation Steps:**
```powershell
# Check if driver is installed
Get-PnpDevice -FriendlyName "*USB*UART*" | Format-Table

# Install driver manually if needed
# 1. Right-click device in Device Manager
# 2. Select "Update driver"
# 3. Browse to downloaded driver folder
# 4. Complete installation wizard
```

#### Linux Driver Installation
```bash
# Most USB-UART chips are supported by default kernel modules
# Check if device is recognized
lsusb | grep -E "(FT232|CP210|CH340)"

# Check kernel modules
lsmod | grep -E "(ftdi_sio|cp210x|ch341)"

# Manual module loading (if needed)
sudo modprobe ftdi_sio
sudo modprobe cp210x
sudo modprobe ch341-uart

# Set permissions for serial ports
sudo chmod 666 /dev/ttyUSB*
sudo chmod 666 /dev/ttyACM*
```

#### macOS Driver Installation
```bash
# Install FTDI drivers (if needed)
# Download from FTDI website and install .pkg file

# Check if device is recognized
system_profiler SPUSBDataType | grep -A 10 -B 10 "UART\|Serial"

# List serial ports
ls -la /dev/tty.*
ls -la /dev/cu.*
```

## Wiring Diagrams

### Basic Connection Diagram
```
┌─────────────────┐         ┌─────────────────┐         ┌─────────────────┐
│   Host Computer │         │  USB-UART       │         │  MELKENS IMU    │
│                 │         │  Adapter        │         │  Board          │
│                 │         │                 │         │                 │
│  USB Port   ────┼─────────┼──  USB          │         │                 │
│                 │         │                 │         │                 │
│                 │         │  VCC (5V)   ────┼─────────┼──  VIN          │
│                 │         │  GND        ────┼─────────┼──  GND          │
│                 │         │  TXD        ────┼─────────┼──  RX (Pin 2)   │
│                 │         │  RXD        ────┼─────────┼──  TX (Pin 3)   │
│                 │         │                 │         │                 │
└─────────────────┘         └─────────────────┘         └─────────────────┘
```

### Detailed Pin Connection
```
IMU Board Pinout:
┌─────┬─────┬─────┬─────┬─────┐
│ VIN │ GND │ TX  │ RX  │ EN  │
│  1  │  2  │  3  │  4  │  5  │
└─────┴─────┴─────┴─────┴─────┘

USB-UART Adapter Pinout:
┌─────┬─────┬─────┬─────┬─────┐
│ VCC │ GND │ TXD │ RXD │ DTR │
│  1  │  2  │  3  │  4  │  5  │
└─────┴─────┴─────┴─────┴─────┘

Connection Mapping:
IMU Pin 1 (VIN) ← → USB-UART Pin 1 (VCC) [5V]
IMU Pin 2 (GND) ← → USB-UART Pin 2 (GND)
IMU Pin 3 (TX)  ← → USB-UART Pin 4 (RXD)
IMU Pin 4 (RX)  ← → USB-UART Pin 3 (TXD)
IMU Pin 5 (EN)  ← → Pull high to 3.3V (enable)
```

### Advanced Connection with Logic Analyzer
```
                    ┌─────────────────┐
                    │  Logic Analyzer │
                    │                 │
         ┌──────────┼──  CH0 (TX)     │
         │          │  CH1 (RX)   ────┼────┐
         │          │  CH2 (GND)  ────┼────┼──── GND
         │          └─────────────────┘    │
         │                               │
         │          ┌─────────────────┐    │
         │          │  USB-UART       │    │
         │          │  Adapter        │    │
         │          │                 │    │
         └──────────┼──  TXD          │    │
                    │  RXD        ────┼────┤
                    │  GND        ────┼────┘
                    │  VCC (5V)   ────┼────┐
                    └─────────────────┘    │
                                           │
                    ┌─────────────────┐    │
                    │  MELKENS IMU    │    │
                    │  Board          │    │
                    │                 │    │
                    │  RX (Pin 2) ────┼────┤
                    │  TX (Pin 3) ────┼────┘
                    │  GND        ────┼────── GND
                    │  VIN        ────┼────── 5V
                    └─────────────────┘
```

## Software Configuration

### Environment Setup
```bash
# Clone the repository
git clone <repository-url>
cd web-simulator

# Install dependencies
cd backend && npm install
cd ../frontend && npm install

# Build the project
npm run build
```

### Configuration Files

#### Backend Configuration (`backend/src/config/index.ts`)
```typescript
export const imuConfig = {
  // Default serial port configuration
  serialPort: process.env.IMU_SERIAL_PORT || 'auto-detect',
  baudRate: parseInt(process.env.IMU_BAUD_RATE || '115200'),
  
  // Protocol settings
  frameSize: 64,
  timeout: 5000,
  retryAttempts: 3,
  
  // Data streaming
  streamInterval: 50, // ms (20Hz)
  bufferSize: 1000,
  
  // Logging
  logLevel: process.env.IMU_LOG_LEVEL || 'info',
  logPath: './logs/hil_imu_test.csv',
  
  // Platform-specific ports
  defaultPorts: {
    linux: ['/dev/ttyUSB0', '/dev/ttyACM0'],
    windows: ['COM3', 'COM4', 'COM5'],
    darwin: ['/dev/tty.usbserial', '/dev/cu.usbserial']
  }
};
```

#### Environment Variables (`.env`)
```bash
# IMU Configuration
IMU_SERIAL_PORT=auto-detect
IMU_BAUD_RATE=115200
IMU_AUTO_CONNECT=true
IMU_LOG_LEVEL=info

# Hardware Testing
HIL_TIMEOUT=5000
HIL_RETRY_ATTEMPTS=3
HIL_ENABLE_FAULT_INJECTION=true

# Development Settings
NODE_ENV=development
LOG_LEVEL=debug
```

### Starting the HIL System
```bash
# Start backend server
cd backend
npm run dev

# Start frontend (in separate terminal)
cd frontend  
npm start

# Access web interface
# http://localhost:3000
```

## Testing and Validation

### Automated Test Suite
```bash
# Run HIL integration tests
npm run test:hil

# Run specific IMU tests
npm test -- --testNamePattern="IMU"

# Run hardware detection test
npm run test:hardware-detect
```

### Manual Testing Checklist

#### 1. Hardware Detection Test
```bash
# Expected output for successful detection
✅ Serial ports detected: ['/dev/ttyUSB0']
✅ IMU board responds to ping
✅ Protocol version match: v001
✅ Sensor initialization complete
```

#### 2. Data Streaming Test
```bash
# Monitor real-time data stream
curl http://localhost:3001/api/imu/data

# Expected response:
{
  "success": true,
  "data": {
    "accelerometer": {"x": 0.12, "y": -0.05, "z": 9.81},
    "gyroscope": {"x": 0.002, "y": -0.001, "z": 0.003},
    "magnetometer": {"x": 25.4, "y": -12.1, "z": 38.7},
    "ahrs": {"roll": 1.2, "pitch": -0.8, "yaw": 45.3},
    "magnetBar": {"status": 65536, "detectedMagnets": [16]},
    "timestamp": 1703567890123,
    "sequenceNumber": 1847,
    "crcValid": true
  }
}
```

#### 3. Fault Injection Test
```bash
# Apply gyroscope drift scenario
curl -X POST http://localhost:3001/api/imu/fault-injection/scenario/gyro_drift

# Verify fault injection is active
curl http://localhost:3001/api/imu/fault-injection

# Expected response:
{
  "success": true,
  "data": {
    "enabled": true,
    "gyroscopeDrift": {"x": 0.1, "y": 0.05, "z": -0.08},
    "noiseLevel": 0.1
  }
}
```

### Performance Validation

#### Latency Testing
```bash
# Measure end-to-end latency
curl -w "@curl-format.txt" -s http://localhost:3001/api/imu/data

# Expected results:
# Response time: <50ms
# Data freshness: <100ms from IMU timestamp
# Sequence gaps: <1% packet loss
```

#### Throughput Testing
```bash
# Monitor data rate over 60 seconds
for i in {1..60}; do
  curl -s http://localhost:3001/api/imu/data | jq '.data.sequenceNumber'
  sleep 1
done

# Expected results:
# Data rate: 20 Hz (50ms intervals)
# Sequence increment: ~1 per reading
# No sequence number jumps >2
```

## Troubleshooting

### Common Issues and Solutions

#### 1. Serial Port Not Detected
**Symptoms:**
- "No ports detected" message
- Empty port list in UI
- Connection attempts fail immediately

**Diagnostic Steps:**
```bash
# Linux: Check USB device detection
lsusb
dmesg | tail -20

# Windows: Check Device Manager
Get-PnpDevice -Class Ports

# macOS: List serial devices
ls -la /dev/tty* | grep usb
```

**Solutions:**
```bash
# Linux: Fix permissions
sudo usermod -a -G dialout $USER
sudo chmod 666 /dev/ttyUSB0

# Windows: Update drivers
# Download latest drivers from manufacturer website

# macOS: Reset USB subsystem
sudo kextunload -b com.apple.driver.AppleUSBFTDI
sudo kextload -b com.apple.driver.AppleUSBFTDI
```

#### 2. Connection Established But No Data
**Symptoms:**
- Port connects successfully
- Status shows "Connected" but no sensor data
- Timeout errors in logs

**Diagnostic Steps:**
```bash
# Test raw serial communication
screen /dev/ttyUSB0 115200
# or
minicom -D /dev/ttyUSB0 -b 115200

# Check for data transmission
hexdump -C /dev/ttyUSB0
```

**Solutions:**
1. **Verify Wiring:**
   - Check TX/RX cross-connection
   - Verify power supply voltage (5V)
   - Ensure ground connection

2. **Protocol Issues:**
   - Confirm baud rate (115200)
   - Check for hardware flow control
   - Verify frame start sequence (0xAA 0x55)

3. **IMU Board Reset:**
   ```bash
   # Power cycle the IMU board
   # Disconnect and reconnect power
   # Wait for initialization LEDs
   ```

#### 3. CRC Validation Failures
**Symptoms:**
- Data received but marked as invalid
- "CRC Error" messages in logs
- Intermittent data corruption

**Diagnostic Steps:**
```bash
# Monitor raw data stream
cat /dev/ttyUSB0 | hexdump -C

# Check for consistent frame headers
grep -a "CRC Error" logs/hil_imu_test.csv
```

**Solutions:**
1. **Cable Quality:**
   - Use shorter, high-quality USB cable
   - Avoid USB hubs if possible
   - Check for electromagnetic interference

2. **Baud Rate Issues:**
   - Try lower baud rate (57600)
   - Add delay between transmissions
   - Check for timing drift

3. **Buffer Overflow:**
   - Increase system buffer sizes
   - Reduce other system load
   - Close unnecessary applications

#### 4. High Latency or Packet Loss
**Symptoms:**
- Data updates slower than 50ms
- Sequence number gaps
- Delayed response to movement

**Diagnostic Steps:**
```bash
# Monitor system performance
top
iotop -o

# Check USB subsystem
cat /sys/kernel/debug/usb/devices
```

**Solutions:**
1. **System Optimization:**
   ```bash
   # Increase process priority
   sudo nice -n -10 npm run dev
   
   # Optimize system settings
   echo 'vm.swappiness=10' | sudo tee -a /etc/sysctl.conf
   ```

2. **USB Configuration:**
   ```bash
   # Disable USB autosuspend
   echo -1 | sudo tee /sys/module/usbcore/parameters/autosuspend
   ```

#### 5. Fault Injection Not Working
**Symptoms:**
- Fault scenarios applied but no effect on data
- Sensor values remain unchanged
- No warning indicators in UI

**Diagnostic Steps:**
```bash
# Verify fault injection status
curl http://localhost:3001/api/imu/fault-injection

# Check log for fault application
grep "fault" logs/hil_imu_test.csv
```

**Solutions:**
1. **Enable Fault Injection:**
   ```bash
   # Ensure fault injection is enabled
   curl -X POST http://localhost:3001/api/imu/fault-injection \
     -H "Content-Type: application/json" \
     -d '{"enabled": true}'
   ```

2. **Hardware Mode Required:**
   - Fault injection only works in hardware mode
   - Ensure IMU is connected and hardware mode is enabled

### Logging and Diagnostics

#### Enable Debug Logging
```bash
# Set environment variable
export IMU_LOG_LEVEL=debug

# Or modify .env file
echo "IMU_LOG_LEVEL=debug" >> .env
```

#### Log File Locations
```bash
# Main application logs
tail -f logs/app.log

# IMU-specific logs
tail -f logs/hil_imu_test.csv

# System journal (Linux)
journalctl -f -u melkens-simulator
```

#### Common Log Messages
| Message | Severity | Meaning |
|---------|----------|---------|
| `IMU connected` | INFO | Successful connection |
| `CRC Error: expected X, calculated Y` | WARNING | Data corruption |
| `IMU disconnected` | WARNING | Connection lost |
| `Parse error: invalid frame` | ERROR | Protocol violation |
| `Hardware mode enabled` | INFO | Using real sensor data |

## Known Issues

### Platform-Specific Issues

#### Linux
- **Issue**: Permission denied on `/dev/ttyUSB0`
- **Workaround**: Add user to dialout group and reboot
- **Status**: Permanent fix in development

#### Windows  
- **Issue**: Port detection fails with Windows 11
- **Workaround**: Manual port specification in config
- **Status**: Driver compatibility update pending

#### macOS
- **Issue**: Monterey USB power management conflicts
- **Workaround**: Disable USB power management
- **Status**: Working with Apple on resolution

### Hardware Compatibility

#### USB-UART Adapters
| Chip | Status | Notes |
|------|--------|-------|
| FT232RL | ✅ Fully Supported | Recommended |
| CP2102 | ✅ Fully Supported | Good alternative |
| CH340G | ⚠️ Limited Support | May require manual drivers |
| PL2303 | ❌ Not Recommended | Stability issues |

#### IMU Firmware Versions
| Version | Status | Features |
|---------|--------|----------|
| v1.0.0 | ❌ Not Supported | Legacy protocol |
| v1.1.0 | ⚠️ Limited | Missing CRC validation |
| v1.2.0+ | ✅ Fully Supported | All features available |

## Performance Optimization

### System Tuning

#### Linux Optimization
```bash
# Disable CPU frequency scaling
echo performance | sudo tee /sys/devices/system/cpu/cpu*/cpufreq/scaling_governor

# Increase USB buffer sizes
echo 16384 | sudo tee /sys/module/usbcore/parameters/usbfs_memory_mb

# Optimize scheduler
echo 1 | sudo tee /proc/sys/kernel/sched_rt_runtime_us
```

#### Windows Optimization
```powershell
# Set high performance power plan
powercfg -setactive 8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c

# Disable USB selective suspend
# Control Panel > Power Options > Advanced Settings
# USB Settings > USB selective suspend setting > Disabled
```

### Application Tuning

#### Node.js Optimization
```bash
# Increase memory limit
export NODE_OPTIONS="--max-old-space-size=4096"

# Enable garbage collection optimization
export NODE_OPTIONS="$NODE_OPTIONS --optimize-for-size"

# Use faster V8 engine flags
export NODE_OPTIONS="$NODE_OPTIONS --turbo --turbo-fast-api-calls"
```

#### Real-time Settings
```typescript
// Backend configuration for high-performance mode
const highPerformanceConfig = {
  streamInterval: 20,    // 50Hz instead of 20Hz
  bufferSize: 2000,      // Larger buffer
  priority: 'realtime',  // Process priority
  affinity: [0, 1],      // CPU core affinity
};
```

### Network Optimization

#### WebSocket Tuning
```typescript
// Optimize WebSocket for low latency
const wsConfig = {
  perMessageDeflate: false,  // Disable compression
  maxPayload: 1024,          // Small message size
  backlog: 100,              // Connection queue
  clientTracking: true,      // Track connections
};
```

## Safety Considerations

### Electrical Safety
- **Power Supply**: Use isolated 5V supply with current limiting
- **ESD Protection**: Handle IMU board with proper ESD precautions  
- **Short Circuit**: Verify connections before applying power
- **Overcurrent**: Monitor current consumption (<1A typical)

### Data Safety
- **Backup**: Regularly backup log files and configuration
- **Validation**: Always validate sensor data before critical operations
- **Failsafe**: Implement automatic fallback to simulation mode
- **Monitoring**: Set up alerts for connection loss or data corruption

### System Safety
- **Resource Limits**: Set memory and CPU usage limits
- **Graceful Degradation**: Handle hardware failures gracefully
- **Emergency Stop**: Implement emergency stop procedures
- **Documentation**: Keep system documentation updated

---

## Support and Resources

### Documentation
- [API Reference](./API_REFERENCE.md)
- [System Architecture](./SYSTEM_ARCHITECTURE.md)
- [Development Guide](./DEVELOPMENT_GUIDE.md)

### Community
- **Issues**: Report bugs on GitHub Issues
- **Discussions**: Join the community forum
- **Updates**: Subscribe to release notifications

### Professional Support
- **Enterprise Support**: Available for commercial deployments
- **Custom Integration**: Hardware-specific adaptation services
- **Training**: On-site training and workshops available

---

*Last updated: December 2024 | Version: 1.0.0*