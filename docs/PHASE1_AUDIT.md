# PHASE 1: AUDIT AND COMPONENT MAPPING

**Project:** MOOver_Merge Integration  
**Date:** 2024-12-19  
**Status:** 🟡 In Progress  

## 🎯 Executive Summary

This phase provides a comprehensive audit of both MELKENS and WB systems, identifying all components, interfaces, and integration requirements for creating a unified robot platform.

---

## 📁 Component Directory Structure

### MELKENS System Architecture

```
Melkens/
├── Melkens_PMB/                    # Power Management Board (Main Controller)
│   ├── AnalogHandler/              # ADC and analog input processing
│   ├── BatteryManager/             # Battery monitoring and management
│   ├── CANHandler/                 # CAN communication layer
│   │   ├── WB_CanOpen.h/.c         # 🆕 WB CANopen integration (700 lines)
│   │   ├── WB_Config.h             # 🆕 WB configuration mappings (184 lines)
│   │   ├── WB_Navigation_Emulator.h/.c # 🆕 WB navigation emulation (1150 lines)
│   │   └── WB_Integration_Example.c # 🆕 Usage examples (314 lines)
│   ├── DmaController/              # DMA management
│   ├── IMUHandler/                 # IMU communication interface
│   ├── LedHandler/                 # LED status indicators
│   ├── mcc_generated_files/        # Microchip Code Configurator files
│   ├── TimeManager/                # Time and scheduling
│   └── Tools/                      # Utility functions
├── Melkens_IMU/                    # Inertial Measurement Unit (STM32G4)
│   ├── Core/                       # Main STM32 application
│   │   ├── Inc/                    # Header files
│   │   ├── Src/                    # Source implementation
│   │   └── Startup/                # Boot and initialization
│   └── Drivers/                    # STM32 HAL and CMSIS drivers
├── Melkens_Connectivity/           # ESP32 Connectivity Module
│   └── src/
│       ├── BleSerial/              # Bluetooth Low Energy
│       ├── ImuCommunication/       # IMU data exchange
│       ├── MqttNode/               # MQTT messaging
│       ├── WebHandler/             # HTTP server
│       └── WebPage/                # Web interface
└── Melkens_Lib/                    # Shared Libraries
    ├── CRC16/                      # CRC calculation
    └── Types/                      # Common data types
```

### WB (Wasserbauer) System Architecture

```
WB/
├── bin/                           # Executable binaries and runtime
│   ├── ButlerEngine               # Main butler control (20MB)
│   ├── ButlerEvo                  # Evolution version (21MB)
│   ├── CANLogger                  # CAN bus logging (7MB)
│   ├── config.sh                  # System configuration script
│   ├── linux.bin                 # Linux kernel (2.8MB)
│   ├── rootfs.img                 # Root filesystem (59MB)
│   ├── update                     # Update utility (6.3MB)
│   ├── canFlash/                  # CAN firmware flashing tools
│   │   ├── *.hex                  # Firmware files for various devices
│   │   └── *.xdd                  # CANopen device definitions
│   ├── ftp/                       # FTP server files
│   │   └── PCSoftware/            # PC management software
│   └── language/                  # Multi-language support (~15 languages)
├── DDMap.cfg                      # Database mapping configuration
├── Butler.db                      # SQLite database (routes, bays, settings)
└── [MISSING] Source code directories
```

---

## 🔧 Component Functionality Mapping

### 1. **Communication Systems**

| Function | MELKENS Implementation | WB Implementation | Integration Status |
|----------|----------------------|-------------------|-------------------|
| **CAN Bus** | Custom CAN handler | CANopen protocol | 🟡 Partial (WB layer added) |
| **Node IDs** | Fixed addresses | Dynamic CANopen nodes | ✅ Mapped |
| **Message Types** | Proprietary format | SDO/PDO/NMT/Heartbeat | ✅ Implemented |
| **Error Handling** | Simple error codes | CANopen error register | ✅ Implemented |

#### CAN Protocol Details
```c
// MELKENS CAN (Original)
#define CAN_MSG_MOTOR_LEFT     0x101
#define CAN_MSG_MOTOR_RIGHT    0x102
#define CAN_MSG_MOTOR_THUMBLE  0x103

// WB CANopen Protocol (Integrated)
#define WB_NODE_BUTLER_MAIN    0x40   // Main Butler Engine
#define WB_NODE_SERVO_LEFT     0x7E   // Left drive motor
#define WB_NODE_SERVO_RIGHT    0x7F   // Right drive motor
#define WB_NODE_SERVO_THUMBLE  0x7D   // Thumble/Drum motor
```

### 2. **Navigation Systems**

| Component | MELKENS | WB | Integration Status |
|-----------|---------|----|--------------------|
| **Algorithm** | Pure pursuit + step-based | World coordinate navigation | 🟡 Emulated |
| **Position System** | Relative coordinates | Absolute world coordinates | 🟡 Converted |
| **Magnetic Detection** | 31 discrete positions | Dynamic field strength | ✅ Bridged |
| **Route Planning** | RouteStep sequence | Track/Bay coordinate system | 🟡 Converted |

#### Navigation Architecture Comparison
```c
// MELKENS Navigation (Step-based)
typedef struct {
    uint8_t OperationType;    // NORM, TU_L, TU_R, L_90, R_90
    uint16_t dX, dY;         // Relative movement
    uint16_t RightSpeed, LeftSpeed;
    float MagnetCorrection;  // Predefined correction
} RouteStep;

// WB Navigation (Coordinate-based)
typedef struct {
    float posX, posY;        // Absolute world coordinates
    uint16_t direction;      // World heading
    float fieldStrengthThreshold; // Dynamic magnetic threshold
} WB_TrackPos_t;
```

### 3. **Motor Control Systems**

| Motor | MELKENS Pin/Control | WB Node ID | Control Method | Status |
|-------|-------------------|------------|----------------|---------|
| **Left Drive** | Motor_Left | 0x7E | CANopen Servo | ✅ Mapped |
| **Right Drive** | Motor_Right | 0x7F | CANopen Servo | ✅ Mapped |
| **Thumble/Auger** | Motor_Thumble | 0x7D | CANopen Servo | ✅ Mapped |
| **Lift** | Motor_Lift | (N/A) | Future expansion | ⚠️ TODO |

### 4. **Sensor Systems**

| Sensor | MELKENS Interface | WB Interface | Integration Status |
|--------|------------------|--------------|-------------------|
| **IMU** | UART to PMB | CAN sensor node | 🔄 Bridge needed |
| **Encoders** | Direct GPIO | CANopen feedback | 🔄 Bridge needed |
| **Magnetic Array** | 31 digital inputs | Linear encoder node 0x10 | ✅ Emulated |
| **Battery** | ADC monitoring | CAN status messages | 🔄 Bridge needed |

---

## 🔍 Missing Components Analysis

### WB System Gaps (Reverse Engineering Required)

| Component | Status | Priority | Effort | Notes |
|-----------|--------|----------|--------|-------|
| **Source Code** | ❌ Missing | Critical | High | Only binaries available |
| **API Documentation** | ❌ Missing | Critical | Medium | Must infer from behavior |
| **Database Schema** | 🟡 Partial | High | Medium | Some info from DDMap.cfg |
| **CAN Message Specs** | 🟡 Partial | High | Medium | Inferred from .xdd files |
| **Butler Engine Logic** | ❌ Missing | High | High | Black box binary |
| **GUI Source** | ❌ Missing | Medium | Low | Can create new interface |

### MELKENS Compatibility Gaps

| Component | Current State | Required Change | Priority | Effort |
|-----------|---------------|-----------------|----------|--------|
| **CAN Handler** | ✅ Enhanced | Add WB protocol support | Critical | ✅ Done |
| **Navigation** | 🟡 Step-based only | Add coordinate navigation | High | 🟡 Partial |
| **Database** | ❌ No database | Add SQLite for WB compat | Medium | ⚠️ TODO |
| **Web Interface** | 🟡 Basic ESP32 | Full web dashboard | Medium | ⚠️ TODO |
| **Error Handling** | 🟡 Basic | CANopen error system | High | 🟡 Partial |

---

## 📊 Interface Mapping Table

### CAN Message Mapping

| WB Function | CAN ID | Message Type | MELKENS Equivalent | Implementation |
|-------------|--------|--------------|-------------------|----------------|
| Butler Control | 0x40 | PDO/SDO | Direct motor commands | ✅ WB_CANopen_ProcessButlerCommand() |
| Left Motor | 0x7E | Servo Control | MotorManager_SetSpeed(Motor_Left) | ✅ Mapped |
| Right Motor | 0x7F | Servo Control | MotorManager_SetSpeed(Motor_Right) | ✅ Mapped |
| Thumble Motor | 0x7D | Servo Control | MotorManager_SetSpeed(Motor_Thumble) | ✅ Mapped |
| Magnetic Encoder | 0x10 | Position Feedback | MagnetName enum (1-31) | ✅ Emulated |
| Steering Wheel | 0x20 | Input Device | Remote control input | ⚠️ TODO |

### GPIO Pin Mapping

| Function | MELKENS Pin | WB Interface | Bridge Required |
|----------|-------------|--------------|-----------------|
| Magnet Sensors 1-31 | GPIO_A0-A30 | CAN Node 0x10 | ✅ Digital→CAN |
| Motor Left Enable | GPIO_B1 | CAN Servo 0x7E | ✅ GPIO→CAN |
| Motor Right Enable | GPIO_B2 | CAN Servo 0x7F | ✅ GPIO→CAN |
| Emergency Stop | GPIO_C1 | CAN Emergency | ✅ GPIO→CAN |
| Status LEDs | GPIO_D0-D3 | CAN Status | ⚠️ TODO |

### Protocol Translation Requirements

```c
// MELKENS → WB Translation Layer
typedef struct {
    // MELKENS Input
    int16_t melkens_motor_speed;     // -1000 to +1000
    MagnetName detected_magnet;      // Magnet1 to Magnet31
    uint8_t melkens_error_code;      // Custom error codes
    
    // WB Output  
    uint16_t wb_servo_velocity;      // CANopen velocity
    float wb_magnetic_position;      // -15.0 to +15.0 cm
    uint8_t wb_error_register;       // CANopen error register
} WB_Translation_t;
```

---

## 🚨 Critical Integration Challenges

### 1. **Reverse Engineering WB Binaries**
- **Challenge:** No source code for Butler Engine
- **Approach:** Monitor CAN traffic, analyze database, study .xdd files
- **Risk:** Medium - May miss critical functionality
- **Mitigation:** Create comprehensive test suite

### 2. **Real-time Performance**
- **Challenge:** WB emulation overhead in MELKENS
- **Approach:** Optimize critical paths, use interrupt-driven processing
- **Risk:** High - Could affect robot responsiveness
- **Mitigation:** Performance profiling and optimization

### 3. **Database Compatibility**
- **Challenge:** WB uses SQLite database for configuration
- **Approach:** Create minimal database interface for MELKENS
- **Risk:** Low - Can implement subset of functionality
- **Mitigation:** Start with essential tables only

### 4. **Navigation Algorithm Differences**
- **Challenge:** Fundamental differences in navigation approach
- **Approach:** Hybrid system with algorithm selection
- **Risk:** Medium - Complex to maintain both systems
- **Mitigation:** Clear separation of concerns, testing both modes

---

## 📋 Phase 1 Completion Checklist

- [x] Complete MELKENS directory structure audit
- [x] Complete WB directory structure audit  
- [x] Map communication interfaces (CAN, GPIO, protocols)
- [x] Map navigation system differences
- [x] Map motor control systems
- [x] Identify missing WB components
- [x] Identify MELKENS compatibility gaps
- [x] Create comprehensive interface mapping table
- [x] Document critical integration challenges
- [ ] Review and validate mapping with stakeholders
- [ ] Plan Phase 2 compatibility layer design

---

## 🎯 Phase 2 Preparation

### Required for Phase 2: Compatibility Layer
1. **WB Protocol Stubs** - Create minimal WB message handlers
2. **Database Interface** - Minimal SQLite wrapper for MELKENS
3. **Translation Layer** - MELKENS ↔ WB data conversion
4. **Error Handling** - Unified error reporting system
5. **Testing Framework** - Unit tests for all new components

### Estimated Effort: Phase 2
- **Development:** 2-3 weeks
- **Testing:** 1 week  
- **Documentation:** 3-5 days
- **Integration:** 1 week

---

## 📝 Outstanding TODOs

### High Priority
- [ ] Reverse engineer Butler Engine CAN messages
- [ ] Create WB database schema from Butler.db
- [ ] Implement steering wheel interface (Node 0x20)
- [ ] Design unified error handling system

### Medium Priority  
- [ ] Create motor current monitoring bridge
- [ ] Implement advanced WB navigation features
- [ ] Design web interface architecture
- [ ] Plan Railway deployment strategy

### Low Priority
- [ ] Multi-language support integration
- [ ] Advanced diagnostic features
- [ ] Performance optimization
- [ ] Extended sensor support

---

**Phase 1 Status:** 🟡 95% Complete - Ready for Phase 2 transition  
**Next:** Phase 2 - Compatibility Layer Implementation