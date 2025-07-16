# MELKENS Robot HIL IMU Integration Guide

## Overview

This guide provides complete instructions for setting up Hardware-in-the-Loop (HIL) integration with the MELKENS IMU board. The system supports both simulated and real hardware modes, allowing seamless development and testing.

## Table of Contents

1. [Hardware Requirements](#hardware-requirements)
2. [Software Dependencies](#software-dependencies)  
3. [IMU Board Setup](#imu-board-setup)
4. [Wiring and Connections](#wiring-and-connections)
5. [Driver Installation](#driver-installation)
6. [Web Simulator Setup](#web-simulator-setup)
7. [Configuration](#configuration)
8. [Operation Modes](#operation-modes)
9. [Troubleshooting](#troubleshooting)
10. [Known Issues](#known-issues)
11. [Advanced Features](#advanced-features)

## Hardware Requirements

### MELKENS IMU Board
- **Sensors**: LSM6DSR (accelerometer + gyroscope) + LIS3MDL (magnetometer)
- **MCU**: STM32G473 series
- **Communication**: UART/USB interface
- **Power**: 3.3V/5V compatible
- **Update Rate**: Up to 100 Hz

### Host Computer
- **Operating System**: Linux (Ubuntu 20.04+), Windows 10+, macOS 10.15+
- **USB Port**: USB 2.0 or higher
- **RAM**: Minimum 4GB, recommended 8GB+
- **Python**: Version 3.8 or higher

### Cables and Connections
- USB cable (Type-A to USB-C/Micro-USB depending on board)
- Optional: UART cable for direct serial connection
- Optional: External power supply (5V, 500mA)

## Software Dependencies

### Python Packages
```bash
pip install -r requirements.txt
```

Required packages:
- `fastapi==0.104.1` - Web framework
- `uvicorn==0.24.0` - ASGI server
- `pyserial==3.5` - Serial communication
- `websockets==12.0` - Real-time communication
- `pydantic==2.5.0` - Data validation
- `numpy==1.24.3` - Numerical computing
- `pandas==2.0.3` - Data analysis
- `structlog==23.2.0` - Structured logging
- `crcmod==1.7` - CRC calculations

### System Dependencies

#### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install python3-dev python3-pip udev
```

#### Windows
- Install Python 3.8+ from python.org
- Install Microsoft Visual C++ Build Tools

#### macOS
```bash
brew install python@3.9
```

## IMU Board Setup

### Firmware Configuration

The MELKENS IMU board comes pre-programmed with compatible firmware. Key features:

1. **Communication Protocol**: Binary frames with CRC16 validation
2. **Data Rate**: Configurable from 10-100 Hz (default: 50 Hz)
3. **Sensor Fusion**: Built-in Madgwick AHRS filter
4. **Auto-detection**: Responds to handshake requests

### Frame Format

The IMU transmits data in the following binary format:

```c
typedef struct {
    uint16_t motorRightSpeed;
    uint16_t motorLeftSpeed;
    uint16_t Xpos1;
    uint16_t Ypos1;
    uint16_t Xpos2;
    uint16_t Ypos2;
    uint16_t angle;
    uint16_t motorBelt2Speed;
    uint16_t crc;
} Imu2PCFrame_t;
```

**Raw sensor data frame (48 bytes):**
```
[timestamp:4][acc_x:4][acc_y:4][acc_z:4][gyro_x:4][gyro_y:4][gyro_z:4]
[mag_x:4][mag_y:4][mag_z:4][temp:4][reserved:4][crc:2]
```

## Wiring and Connections

### USB Connection (Recommended)
```
IMU Board          Host Computer
---------          -------------
USB-C    --------> USB-A Port
GND      --------> GND (if external power)
VCC      --------> 5V (if external power)
```

### UART Connection (Alternative)
```
IMU Board          USB-to-Serial Adapter
---------          --------------------
TX       --------> RX
RX       --------> TX  
GND      --------> GND
VCC      --------> 3.3V or 5V
```

### Pin Configuration
| Pin | Function | Description |
|-----|----------|-------------|
| PA2 | USART2_TX | Debug output |
| PA3 | USART2_RX | Debug input |
| PB6 | USART1_TX | IMU data output |
| PB7 | USART1_RX | Command input |
| PA9 | USB_D+ | USB data positive |
| PA10| USB_D- | USB data negative |

## Driver Installation

### Linux

1. **Check USB device detection:**
```bash
lsusb
dmesg | grep tty
```

2. **Add user to dialout group:**
```bash
sudo usermod -a -G dialout $USER
```

3. **Create udev rule (optional):**
```bash
sudo nano /etc/udev/rules.d/99-melkens-imu.rules
```

Add:
```
SUBSYSTEM=="tty", ATTRS{idVendor}=="0483", ATTRS{idProduct}=="5740", SYMLINK+="melkens_imu"
```

4. **Reload udev rules:**
```bash
sudo udevadm control --reload-rules
sudo udevadm trigger
```

### Windows

1. **Install ST-Link drivers** from STMicroelectronics website
2. **Check Device Manager** for "STMicroelectronics Virtual COM Port"
3. **Note COM port number** (e.g., COM3, COM4)

### macOS

1. **Install drivers (if needed):**
```bash
brew install libusb
```

2. **Check device:**
```bash
ls /dev/cu.*
```

## Web Simulator Setup

### Installation

1. **Clone/extract the project:**
```bash
cd web-simulator
```

2. **Install dependencies:**
```bash
cd backend
pip install -r requirements.txt
```

3. **Create logs directory:**
```bash
mkdir -p ../../logs
```

### Starting the Server

1. **Navigate to backend directory:**
```bash
cd web-simulator/backend
```

2. **Start the server:**
```bash
python main.py
```

3. **Access the web interface:**
Open browser to `http://localhost:8000`

### Auto-start on Boot (Linux)

Create systemd service:
```bash
sudo nano /etc/systemd/system/melkens-hil.service
```

Add:
```ini
[Unit]
Description=MELKENS HIL Simulator
After=network.target

[Service]
Type=simple
User=your_username
WorkingDirectory=/path/to/web-simulator/backend
ExecStart=/usr/bin/python3 main.py
Restart=always

[Install]
WantedBy=multi-user.target
```

Enable:
```bash
sudo systemctl enable melkens-hil
sudo systemctl start melkens-hil
```

## Configuration

### System Configuration

Access the web interface and configure:

1. **IMU Mode**: 
   - `Simulated` - Uses software-generated sensor data
   - `Hardware` - Connects to real IMU board

2. **Serial Port**:
   - Linux: `/dev/ttyUSB0`, `/dev/ttyACM0`
   - Windows: `COM3`, `COM4`, etc.
   - macOS: `/dev/cu.usbmodem*`

3. **Baud Rate**: 
   - Default: `115200`
   - Supported: `9600`, `57600`, `115200`, `230400`

4. **Update Rate**: 
   - Range: 10-1000ms
   - Default: 50ms (20 Hz)

### Configuration File

Settings are stored in JSON format:
```json
{
  "system": {
    "imu_mode": "hardware",
    "serial_port": "/dev/ttyUSB0",
    "baud_rate": 115200,
    "update_rate_ms": 50
  },
  "fault_injection": {
    "enabled": false,
    "fault_type": "none",
    "axis": "x",
    "severity": 1.0
  }
}
```

## Operation Modes

### Simulated Mode

Features:
- Software-generated sensor data
- Realistic physics simulation
- Circular motion pattern with noise
- No hardware dependencies
- Perfect for development and testing

Data characteristics:
- Accelerometer: ±2g range with 0.01g noise
- Gyroscope: ±250°/s range with 0.1°/s noise  
- Magnetometer: Earth's field simulation with disturbances

### Hardware Mode

Features:
- Real-time data from physical IMU
- Hardware validation and self-test
- CRC error detection and correction
- Automatic reconnection on failures
- Performance monitoring

Requirements:
- Connected MELKENS IMU board
- Proper driver installation
- Configured serial port

### Hybrid Mode

Features:
- Seamless switching between modes
- Hardware data with software fallback
- Fault injection capabilities
- Comparative analysis

## Troubleshooting

### Connection Issues

**Problem**: "Serial port not found"
```
Solutions:
1. Check physical USB connection
2. Verify correct port name in configuration  
3. Ensure user has permission to access serial ports
4. Try different USB ports/cables
5. Check Device Manager (Windows) or lsusb (Linux)
```

**Problem**: "Permission denied accessing /dev/ttyUSB0"
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and back in
```

**Problem**: "Device not responding"
```
Solutions:
1. Power cycle the IMU board
2. Try lower baud rate (57600)
3. Check for conflicting applications using the port
4. Verify IMU firmware is compatible
```

### Data Quality Issues

**Problem**: "High error rate or corrupted packets"
```
Solutions:
1. Check cable quality and connections
2. Reduce baud rate
3. Add ferrite cores to reduce EMI
4. Check power supply stability
5. Verify grounding
```

**Problem**: "IMU data appears frozen"
```
Solutions:
1. Reset IMU board
2. Check for software hangs
3. Verify update rate configuration
4. Monitor CPU usage
```

### Performance Issues

**Problem**: "Low data rate or lag"
```
Solutions:
1. Close unnecessary applications
2. Check system resources (CPU, memory)
3. Reduce plot data buffer size
4. Increase update rate interval
5. Use wired Ethernet instead of WiFi
```

**Problem**: "WebSocket disconnections"
```
Solutions:
1. Check browser console for errors
2. Verify network connectivity
3. Disable browser ad blockers
4. Try different browser
5. Check firewall settings
```

### Self-Test Failures

**Problem**: "Hardware connection test failed"
```
Diagnostic steps:
1. Run self-test from web interface
2. Check "Available Ports" list
3. Verify target port matches connected device
4. Test with terminal application (minicom, PuTTY)
```

**Problem**: "Data validation test failed"
```
Diagnostic steps:
1. Check sensor magnitude values
2. Verify accelerometer detects gravity (~9.8 m/s²)
3. Ensure magnetometer detects Earth's field (>1000 µT)
4. Check for excessive noise or stuck values
```

## Known Issues

### Hardware Limitations

1. **USB Hub Compatibility**: Some USB hubs may cause communication issues
   - **Workaround**: Connect directly to computer USB port

2. **Power Management**: USB power saving may interrupt communication
   - **Solution**: Disable USB selective suspend in power settings

3. **EMI Sensitivity**: Electromagnetic interference can corrupt data
   - **Mitigation**: Use shielded cables, proper grounding

### Software Limitations

1. **Windows COM Port Changes**: Port numbers may change after reboot
   - **Solution**: Use Device Manager to assign fixed COM port

2. **Browser Compatibility**: Older browsers may not support WebSockets
   - **Requirement**: Chrome 16+, Firefox 11+, Safari 7+, Edge 12+

3. **Large Log Files**: Extended operation creates large CSV files
   - **Management**: Implement log rotation or periodic cleanup

### Performance Considerations

1. **High CPU Usage**: Real-time plotting can be CPU intensive
   - **Optimization**: Reduce plot update rate, limit data points

2. **Memory Usage**: Long runs accumulate data in browser memory
   - **Solution**: Refresh browser periodically

## Advanced Features

### Fault Injection

The system supports various fault injection modes for testing robustness:

#### Sensor Drift
```json
{
  "fault_type": "drift",
  "axis": "x",
  "severity": 2.0
}
```
Simulates gradual sensor offset over time.

#### Stuck Axis
```json
{
  "fault_type": "stuck_axis", 
  "axis": "z",
  "severity": 1.0
}
```
Freezes sensor axis at current value.

#### Excessive Noise
```json
{
  "fault_type": "noise",
  "axis": "y", 
  "severity": 5.0
}
```
Adds high-amplitude random noise.

#### Missing Packets
```json
{
  "fault_type": "missing_packets",
  "severity": 0.3
}
```
Randomly drops data packets (30% loss).

### Data Logging and Analysis

#### CSV Export Format
```csv
timestamp,iso_time,data_source,imu_connected,fault_injection_enabled,fault_type,
acc_x,acc_y,acc_z,gyro_x,gyro_y,gyro_z,mag_x,mag_y,mag_z,temperature,
quat_w,quat_x,quat_y,quat_z,roll,pitch,yaw,pos_x,pos_y,pos_z,
vel_x,vel_y,vel_z,robot_route,robot_pos_x,robot_pos_y,robot_angle,
motor_left_speed,motor_right_speed,packet_count,error_count,data_rate
```

#### Analysis Tools
Use Python pandas for post-processing:
```python
import pandas as pd
import matplotlib.pyplot as plt

# Load log data
df = pd.read_csv('logs/hil_imu_test.csv')

# Plot accelerometer data
plt.figure(figsize=(12, 8))
plt.plot(df['timestamp'], df['acc_x'], label='X')
plt.plot(df['timestamp'], df['acc_y'], label='Y') 
plt.plot(df['timestamp'], df['acc_z'], label='Z')
plt.xlabel('Time (s)')
plt.ylabel('Acceleration (m/s²)')
plt.legend()
plt.show()

# Calculate statistics
print(f"Data rate: {len(df) / (df['timestamp'].max() - df['timestamp'].min()):.1f} Hz")
print(f"Error rate: {df['error_count'].max() / len(df) * 100:.2f}%")
```

### Route Programming

The system supports predefined routes with magnetic line following:

#### Route Definition
```python
RouteStep(
    dx=2.0,              # X displacement (meters)
    dy=0.0,              # Y displacement (meters)  
    speed=500,           # Motor speed (0-1500)
    auger_speed=800,     # Auger speed (0-1500)
    magnet_correction=-2.17  # Magnetic offset (degrees)
)
```

#### Magnetic Corrections
Based on MELKENS magnetic constants:
- `dMAGNET_L10 = -10*2.17` (left 10cm)
- `dMAGNET_L5 = -5*2.17` (left 5cm)
- `dMAGNET_MID = 0` (center)
- `dMAGNET_R5 = 5*2.17` (right 5cm)
- `dMAGNET_R10 = 10*2.17` (right 10cm)

### API Integration

REST API endpoints for external integration:

```bash
# Get system status
curl http://localhost:8000/api/imu/status

# Set route
curl -X POST http://localhost:8000/api/robot/route/A

# Get configuration  
curl http://localhost:8000/api/config

# Run self-test
curl -X POST http://localhost:8000/api/imu/self-test
```

WebSocket for real-time data:
```javascript
const ws = new WebSocket('ws://localhost:8000/ws');
ws.onmessage = (event) => {
    const data = JSON.parse(event.data);
    if (data.type === 'imu_data') {
        console.log('IMU data:', data.data);
    }
};
```

## Support and Resources

### Documentation
- [MELKENS Hardware Manual](hardware_manual.pdf)
- [STM32G4 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0440-stm32g4-series-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [LSM6DSR Datasheet](https://www.st.com/resource/en/datasheet/lsm6dsr.pdf)
- [LIS3MDL Datasheet](https://www.st.com/resource/en/datasheet/lis3mdl.pdf)

### Contact Information
- **Technical Support**: support@melkens.com
- **Documentation**: docs@melkens.com
- **Hardware Issues**: hardware@melkens.com

### Version History
- **v1.0.0** (2024-01-15): Initial HIL implementation
- **v1.1.0** (2024-02-01): Added fault injection
- **v1.2.0** (2024-03-01): Improved self-test capabilities
- **v1.3.0** (2024-04-01): Enhanced route management

---

**Document Version**: 1.3.0  
**Last Updated**: March 2024  
**Compatible Hardware**: MELKENS IMU Board v2.1+  
**Compatible Firmware**: v3.0+