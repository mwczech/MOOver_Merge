# MELKENS Hardware-in-the-Loop (HIL) Setup Guide

Complete documentation for connecting MELKENS robot hardware to the simulator for hardware-in-the-loop testing.

## 🔌 Overview

Hardware-in-the-loop (HIL) testing allows you to run simulated scenarios while using real robot hardware components. This provides the most accurate testing environment possible.

## 📋 Prerequisites

### Hardware Requirements
- MELKENS robot with PMB (Power Management Board)
- IMU sensor module
- Magnetic position sensors
- Motor controllers
- USB-to-UART adapters (2-3 units)
- CAN bus interface (optional)
- Development PC/laptop

### Software Requirements
- MELKENS Web Simulator
- Hardware Bridge service
- Serial/CAN drivers
- Python 3.8+ (for configuration tools)

## 🔗 Connection Diagrams

### Primary Connections (UART)

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   Development   │     │  USB-to-UART    │     │  MELKENS PMB    │
│      PC         │────▶│    Adapter      │────▶│   UART Port     │
│                 │ USB │                 │UART │  (115200 bps)   │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

### Pin Mapping - PMB UART Connection

| PMB Pin | UART Adapter | Function |
|---------|--------------|----------|
| TX      | RX           | Data from PMB to PC |
| RX      | TX           | Data from PC to PMB |
| GND     | GND          | Common ground |
| VCC     | VCC (3.3V)   | Power supply |

### IMU Connection (I2C over UART bridge)

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   Development   │     │  I2C-to-UART    │     │   IMU Module    │
│      PC         │────▶│    Bridge       │────▶│   (MPU6050)     │
│                 │ USB │                 │ I2C │                 │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

### CAN Bus Connection (Optional)

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│   Development   │     │   CAN-to-USB    │     │   MELKENS       │
│      PC         │────▶│    Interface    │────▶│   CAN Bus       │
│                 │ USB │  (PEAK, etc.)   │ CAN │   Network       │
└─────────────────┘     └─────────────────┘     └─────────────────┘
```

## ⚙️ Hardware Configuration

### 1. PMB Configuration

Edit `pmb_Settings.h` for HIL mode:

```c
// Enable HIL mode
#define HIL_MODE_ENABLED 1

// UART configuration for HIL
#define HIL_UART_BAUDRATE 115200
#define HIL_UART_BUFFER_SIZE 512

// Protocol settings
#define HIL_PROTOCOL_VERSION "1.0"
#define HIL_HEARTBEAT_INTERVAL 1000  // ms
#define HIL_TIMEOUT_THRESHOLD 5000   // ms
```

### 2. Communication Protocol

#### Message Format (JSON over UART)

**From PC to PMB:**
```json
{
  "type": "command",
  "timestamp": 1642680000000,
  "command": "LOAD_ROUTE",
  "data": {
    "route": [...],
    "settings": {...}
  }
}
```

**From PMB to PC:**
```json
{
  "type": "sensor_data",
  "timestamp": 1642680000001,
  "data": {
    "magneticPosition": {"x": 150, "y": 200, "detected": true},
    "imu": {"ax": 0.1, "ay": 0.2, "az": 9.8, "gx": 0, "gy": 0, "gz": 0},
    "encoders": {"left": 1500, "right": 1520},
    "battery": {"voltage": 12.1, "current": 1.5, "percentage": 85},
    "temperature": [45, 47, 42]
  }
}
```

#### Command Set

| Command | Description | Parameters |
|---------|-------------|------------|
| `PING` | Heartbeat/connectivity check | None |
| `LOAD_ROUTE` | Load route to robot | `route`, `settings` |
| `START_ROUTE` | Start route execution | `routeId` (optional) |
| `STOP_ROUTE` | Stop current route | None |
| `EMERGENCY_STOP` | Immediate stop | None |
| `GET_STATUS` | Request current status | None |
| `SET_PARAMETER` | Change runtime parameter | `parameter`, `value` |

## 🔧 Setup Procedures

### Step 1: Hardware Connection

1. **Power down the robot** completely
2. **Connect UART adapter** to PMB:
   - Red wire (VCC) → PMB VCC (3.3V)
   - Black wire (GND) → PMB GND
   - White wire (TX) → PMB RX
   - Green wire (RX) → PMB TX
3. **Connect USB adapter** to development PC
4. **Verify connection** with multimeter:
   - VCC to GND: 3.3V ±0.1V
   - Continuity on data lines

### Step 2: Software Configuration

1. **Install dependencies:**
```bash
# On Ubuntu/Debian
sudo apt-get install python3-serial python3-can

# On Windows (using pip)
pip install pyserial python-can

# On macOS
brew install python3
pip3 install pyserial python-can
```

2. **Configure hardware bridge:**
```bash
cd web-simulator/backend
npm install serialport @serialport/parser-readline
```

3. **Set environment variables:**
```bash
export HIL_UART_PORT="/dev/ttyUSB0"  # Linux
export HIL_UART_PORT="COM3"          # Windows
export HIL_UART_BAUDRATE="115200"
export HIL_CAN_INTERFACE="can0"      # If using CAN
```

### Step 3: Testing Connection

1. **Start hardware bridge:**
```bash
cd web-simulator/backend
npm run dev
```

2. **Test UART communication:**
```bash
# Using screen (Linux/macOS)
screen /dev/ttyUSB0 115200

# Using PuTTY (Windows)
# Configure: Serial, COM3, 115200, 8N1
```

3. **Send test command:**
```bash
echo '{"type":"command","command":"PING"}' > /dev/ttyUSB0
```

4. **Expected response:**
```json
{"type":"response","command":"PING","status":"OK","timestamp":1642680000000}
```

### Step 4: Simulator Integration

1. **Initialize hardware bridge in simulator:**
```javascript
// POST /api/hardware/initialize
{
  "serialPort": "/dev/ttyUSB0",
  "baudRate": 115200,
  "canInterface": "can0"  // optional
}
```

2. **Enable HIL mode:**
```javascript
// POST /api/hardware/hil-mode
{
  "enabled": true
}
```

3. **Verify connection status:**
```javascript
// GET /api/hardware/status
{
  "connected": true,
  "mode": "hil",
  "lastHeartbeat": 1642680000000
}
```

## 📊 Monitoring and Debugging

### Real-time Monitoring

**Monitor UART traffic:**
```bash
# Linux - Monitor with timestamps
sudo cat /dev/ttyUSB0 | ts '[%Y-%m-%d %H:%M:%S]'

# Log to file
sudo cat /dev/ttyUSB0 | tee hil_communication.log
```

**Monitor system resources:**
```bash
# Check USB devices
lsusb | grep -i uart

# Check serial ports
ls -la /dev/ttyUSB*

# Monitor bandwidth
sudo iftop -i can0  # for CAN interface
```

### Common Issues and Solutions

#### Issue: No communication with PMB

**Symptoms:**
- Timeout errors in simulator
- No response to PING commands
- Connection status shows "disconnected"

**Solutions:**
1. Check physical connections
2. Verify baud rate settings
3. Test with different UART adapter
4. Check PMB power supply
5. Verify HIL mode enabled in firmware

**Debug commands:**
```bash
# Test UART adapter
echo "test" > /dev/ttyUSB0
cat /dev/ttyUSB0

# Check permissions
sudo chmod 666 /dev/ttyUSB0

# Test with minicom
minicom -b 115200 -D /dev/ttyUSB0
```

#### Issue: Data corruption or garbled messages

**Symptoms:**
- JSON parse errors
- Incomplete messages
- Random characters

**Solutions:**
1. Check ground connections
2. Use shorter cables
3. Add ferrite cores to reduce noise
4. Lower baud rate to 57600
5. Enable hardware flow control

#### Issue: CAN bus communication problems

**Symptoms:**
- CAN interface not found
- No CAN frames received
- Bus-off errors

**Solutions:**
1. Check CAN termination resistors
2. Verify CAN bit rate settings
3. Test with `cansend`/`candump` tools
4. Check physical layer integrity

**CAN debugging:**
```bash
# Initialize CAN interface
sudo ip link set can0 type can bitrate 500000
sudo ip link set up can0

# Monitor CAN traffic
candump can0

# Send test frame
cansend can0 123#DEADBEEF
```

### Performance Optimization

**Reduce latency:**
1. Use dedicated USB controllers
2. Set real-time process priority
3. Disable USB power management
4. Use shorter cables
5. Enable hardware timestamps

**Increase throughput:**
1. Use higher baud rates (230400, 460800)
2. Implement message batching
3. Compress repeated data
4. Use binary protocol instead of JSON

## 🧪 Testing Scenarios

### Basic Connectivity Test

```bash
# Start hardware bridge
npm run start:hardware-bridge

# Run connectivity test
fault-injector run connectivity_test

# Expected: All communication tests pass
```

### Sensor Data Validation

```bash
# Compare virtual vs physical sensor data
fault-injector run sensor_validation

# Expected: <5% difference in readings
```

### Motor Control Test

```bash
# Test motor commands
fault-injector run motor_control_test

# Expected: Motors respond within 100ms
```

### Route Execution Test

```bash
# Load and execute simple route
fault-injector run route_execution_test

# Expected: Route completes successfully
```

## 📋 Calibration Procedures

### Magnetic Position Calibration

1. **Place robot at known position (0,0)**
2. **Record magnetic sensor readings**
3. **Move robot to calibration points**
4. **Generate calibration matrix**

```python
# calibration_tool.py
import numpy as np

def calibrate_magnetic_position(readings, actual_positions):
    # Generate transformation matrix
    transform = np.linalg.lstsq(readings, actual_positions, rcond=None)[0]
    return transform

# Usage
readings = [[100, 150], [200, 250], [300, 350]]
positions = [[0, 0], [10, 0], [20, 0]]
transform = calibrate_magnetic_position(readings, positions)
```

### IMU Calibration

1. **Place robot on level surface**
2. **Record accelerometer bias**
3. **Rotate robot through known angles**
4. **Calculate gyroscope scaling factors**

### Motor Encoder Calibration

1. **Mark starting position**
2. **Command known distance/angle**
3. **Measure actual movement**
4. **Calculate scaling factors**

## 🔒 Safety Considerations

### Electrical Safety
- Always disconnect power before making connections
- Use proper grounding techniques
- Check voltage levels with multimeter
- Use isolated USB adapters when possible

### Mechanical Safety
- Secure robot to prevent movement during testing
- Install emergency stop buttons
- Keep clear of moving parts
- Use safety barriers around test area

### Software Safety
- Implement watchdog timers
- Add communication timeouts
- Validate all incoming commands
- Log all safety-critical events

## 📚 Reference Materials

### Pin-out Diagrams
- [PMB Connector Pinout](diagrams/pmb_pinout.pdf)
- [IMU Module Connections](diagrams/imu_connections.pdf)
- [CAN Bus Wiring](diagrams/can_wiring.pdf)

### Protocol Specifications
- [HIL Communication Protocol v1.0](protocols/hil_protocol_v1.0.pdf)
- [MELKENS Command Set](protocols/melkens_commands.pdf)
- [Safety Protocol Requirements](protocols/safety_requirements.pdf)

### Troubleshooting Tools
- [Hardware Diagnostic Script](tools/hardware_diagnostic.py)
- [Communication Test Suite](tools/comm_test_suite.py)
- [Performance Monitoring](tools/performance_monitor.py)

---

## 📞 Support

For technical support or questions about HIL setup:

- **Email:** support@melkens.com
- **Documentation:** https://docs.melkens.com/hil
- **Issue Tracker:** https://github.com/melkens/issues

## 📄 Change Log

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2024-01-15 | Initial HIL documentation |
| 1.1.0 | 2024-01-20 | Added CAN bus support |
| 1.2.0 | 2024-01-25 | Enhanced troubleshooting section |