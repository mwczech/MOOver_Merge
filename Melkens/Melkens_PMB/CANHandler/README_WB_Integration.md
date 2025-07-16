# MELKENS WB Compatibility Layer

## Opis

Warstwa kompatybilności WB (Wasserbauer) dla systemu MELKENS umożliwia pełną integrację z protokołem CANopen używanym przez roboty WB. Implementacja zapewnia dwukierunkową kompatybilność - system MELKENS może komunikować się z urządzeniami WB oraz być kontrolowany przez oprogramowanie WB.

## Architektura

```
┌─────────────────────────────────────────────────────────────┐
│                   MELKENS PMB System                        │
├─────────────────────────────────────────────────────────────┤
│  pmb_MotorManager │ pmb_System │ IMUHandler │ BatteryManager │
├─────────────────────────────────────────────────────────────┤
│                WB Compatibility Layer                       │
│  ┌─────────────────┬─────────────────┬─────────────────────┐ │
│  │   CANHandler    │   WB_CANopen    │     WB_Config       │ │
│  │   (Enhanced)    │  (Protocol)     │   (Mapping)         │ │
│  └─────────────────┴─────────────────┴─────────────────────┘ │
├─────────────────────────────────────────────────────────────┤
│                  Physical CAN Bus                           │
├─────────────────────────────────────────────────────────────┤
│     WB Devices (Servo Controllers, Sensors, etc.)          │
└─────────────────────────────────────────────────────────────┘
```

## Komponenty

### 1. WB_CanOpen.h/c
**Główna implementacja protokołu CANopen zgodnego z WB**

**Funkcjonalności:**
- Implementacja protokołu CANopen (ISO 11898-1)
- Object Dictionary zgodny z WB
- Obsługa SDO (Service Data Objects)
- Obsługa PDO (Process Data Objects)
- Network Management (NMT)
- Emergency handling
- Heartbeat monitoring

**Kluczowe obiekty:**
```c
// Object Dictionary Indices
#define OD_DEVICE_TYPE          0x1000  // Device identification
#define OD_ERROR_REGISTER       0x1001  // Error status
#define OD_PDO_VARIABLE_MANUAL  0x2010  // Manual control data
#define OD_BUTLER_CTRL          0x4000  // Butler control commands
#define OD_SERVO_CONTROL        0x6xxx  // Servo parameters
```

### 2. CANHandler.h/c (Enhanced)
**Rozszerzony handler CAN z integracją WB**

**Funkcjonalności:**
- Automatyczne rozpoznawanie komunikatów WB/MELKENS
- Routing komunikatów do odpowiednich warstw
- Backward compatibility z istniejącym kodem MELKENS
- Statystyki komunikacji
- Konfiguracja servo controllers

### 3. WB_Config.h
**Konfiguracja i mapowanie parametrów**

**Mapowania:**
```c
// Node ID mappings
#define WB_NODE_BUTLER_ENGINE   0x40
#define WB_NODE_SERVO_LEFT      0x7E
#define WB_NODE_SERVO_RIGHT     0x7F
#define WB_NODE_SERVO_THUMBLE   0x7D

// Unit conversions
#define WB_TO_MELKENS_SPEED_FACTOR  10  // WB: ±100, MELKENS: ±1000
```

### 4. WB_Integration_Example.c
**Przykłady użycia i testy integracyjne**

## Protokół komunikacji

### CANopen Message Format
```
COB-ID = Function Code (7 bits) + Node ID (7 bits)
```

**Function Codes:**
- `0x000`: NMT (Network Management)
- `0x080`: SYNC/Emergency
- `0x180-0x500`: PDO (Process Data Objects)
- `0x580-0x600`: SDO (Service Data Objects)
- `0x700`: Heartbeat

### WB Message Types

#### 1. Manual Control (PDO 0x2010)
```c
typedef struct {
    int8_t speed;           // Robot speed (-100 to +100)
    int8_t steering;        // Steering (-100 to +100)
    uint8_t batteryLevel;   // Battery level (0-100%)
    uint16_t batteryVoltage; // Battery voltage (mV)
    int16_t motorCurrent;   // Motor current (mA)
    float yawAngle;         // Yaw angle (degrees)
} WB_PDO_Manual_t;
```

#### 2. Butler Control (Object 0x4000)
```c
typedef struct {
    uint16_t driveRequest;     // Drive command
    uint16_t butlerState;      // Current state
    uint8_t abortRequest;      // Emergency stop
    uint8_t manualRequest;     // Manual mode
    float driveLength;         // Distance to drive
} WB_Butler_Control_t;
```

#### 3. Servo Control (Objects 0x6xxx)
```c
typedef struct {
    uint32_t profileAcceleration;  // Acceleration
    uint32_t profileDeceleration;  // Deceleration
    uint32_t maxProfileVelocity;   // Max velocity
    uint16_t currentLimit;         // Current limit
} WB_Servo_Profile_t;
```

## API Usage

### Inicjalizacja

```c
#include "CANHandler/CanHandler.h"
#include "CANHandler/WB_Config.h"

void system_init(void) {
    // Initialize WB compatibility layer
    CANHandler_Init();
    
    // Configure servos with WB parameters
    CANHandler_ConfigureServo(WB_NODE_SERVO_LEFT);
    CANHandler_ConfigureServo(WB_NODE_SERVO_RIGHT);
    CANHandler_ConfigureServo(WB_NODE_SERVO_THUMBLE);
}
```

### Sterowanie robotem

```c
// Manual control
bool manual_drive(int8_t speed, int8_t steering) {
    // Send speed command
    CANHandler_SendWBMessage(WB_NODE_BUTLER_ENGINE, 
                            OD_PDO_VARIABLE_MANUAL, 0x01, speed);
    
    // Send steering command
    return CANHandler_SendWBMessage(WB_NODE_BUTLER_ENGINE, 
                                   OD_PDO_VARIABLE_MANUAL, 0x02, steering);
}

// Direct motor control
bool motor_command(uint8_t motor_node, int16_t speed, uint16_t acceleration) {
    return CANHandler_SendMotorCommand(motor_node, speed, acceleration);
}
```

### Butler commands

```c
// Start automatic drive
WB_Butler_Control_t cmd = {0};
cmd.driveRequest = WB_DRIVE_REQ_AUTO;
cmd.driveLength = 10.0f; // 10 meters
WB_CANopen_ProcessButlerCommand(&cmd);

// Emergency stop
cmd.abortRequest = 1;
WB_CANopen_ProcessButlerCommand(&cmd);
```

### Periodic tasks

```c
void main_loop(void) {
    while (1) {
        // Process CAN messages
        CANHandler_Task();
        
        // 1ms tasks
        CANHandler_PeriodicTasks_1ms();
        
        // 10ms tasks (every 10 cycles)
        if (cycle_counter % 10 == 0) {
            CANHandler_PeriodicTasks_10ms();
        }
        
        // 100ms tasks (every 100 cycles)
        if (cycle_counter % 100 == 0) {
            CANHandler_PeriodicTasks_100ms();
        }
        
        cycle_counter++;
        delay_ms(1);
    }
}
```

## Mapowanie danych

### Speed/Steering Conversion
```c
// WB to MELKENS speed conversion
int16_t melkens_speed = wb_speed * WB_TO_MELKENS_SPEED_FACTOR;

// Differential steering
int16_t left_speed = base_speed + (steering / 2);
int16_t right_speed = base_speed - (steering / 2);
```

### Sensor Data Mapping
```c
// Battery data
canopen_node.pdoManual.batteryVoltage = BatteryManager_GetVoltage();
canopen_node.pdoManual.batteryLevel = BatteryManager_GetLevel();

// Motor currents
canopen_node.pdoManual.BLX1Current = MotorManager_GetCurrent(Motor_Left);
canopen_node.pdoManual.BLX2Current = MotorManager_GetCurrent(Motor_Right);

// IMU data
float yaw = IMUHandler_GetYaw();
canopen_node.pdoManual.cruiseYawDeg = yaw;
```

## Diagnostics & Monitoring

### CAN Statistics
```c
uint32_t rx_count, tx_count;
CANHandler_GetStatistics(&rx_count, &tx_count);
printf("RX: %lu, TX: %lu\n", rx_count, tx_count);
```

### Error Handling
```c
// Check error register
uint8_t error_reg = WB_CANopen_GetErrorRegister();

// Send emergency message
WB_CANopen_SendEmergency(WB_ERROR_MOTOR_OVERCURRENT, error_reg, NULL);
```

### State Monitoring
```c
CANopen_State_t state = WB_CANopen_GetState();
bool wb_enabled = CANHandler_IsWBCompatibilityEnabled();
```

## Konfiguracja Build

### Dodanie do Makefile
```makefile
# WB Compatibility Layer
SOURCES += CANHandler/WB_CanOpen.c
SOURCES += CANHandler/CANHandler.c
SOURCES += CANHandler/WB_Integration_Example.c

INCLUDES += -ICANHandler/
```

### Dependencies
```c
// Required MELKENS modules
#include "pmb_MotorManager.h"
#include "pmb_System.h"
#include "IMUHandler/IMUHandler.h"
#include "BatteryManager/BatteryManager.h"
#include "DiagnosticsHandler.h"

// Required MCC generated files
#include "mcc_generated_files/can1.h"
#include "mcc_generated_files/can_types.h"
```

## Testing

### Unit Tests
```c
// Run all integration tests
WB_Integration_RunTests();

// Individual tests
WB_Integration_ManualControl_Example();
WB_Integration_AutoDrive_Example();
WB_Integration_SensorMonitoring_Example();
```

### CLI Commands
```c
// Command line interface
WB_Integration_CLI("test");      // Run all tests
WB_Integration_CLI("manual");    // Manual control test
WB_Integration_CLI("enable_wb"); // Enable WB compatibility
WB_Integration_CLI("diag");      // Show diagnostics
```

## Troubleshooting

### Common Issues

1. **No WB messages received**
   - Check CAN bus termination
   - Verify Node IDs configuration
   - Ensure WB compatibility is enabled

2. **Motor commands not working**
   - Verify servo configuration
   - Check current limits
   - Ensure proper acceleration/deceleration values

3. **Heartbeat timeouts**
   - Check heartbeat interval settings
   - Verify CAN bus integrity
   - Check for bus-off conditions

### Debug Output
```c
// Enable debug mode
#define WB_DEBUG_ENABLED  true

// Debug functions
WB_Config_Print();                    // Print configuration
WB_Integration_Diagnostics_Example(); // Show detailed status
```

## Compatibility Matrix

| Function | WB Original | MELKENS WB Layer | Status |
|----------|-------------|------------------|---------|
| Manual Drive | ✅ | ✅ | Full |
| Auto Navigation | ✅ | ⚠️ | Partial* |
| Emergency Stop | ✅ | ✅ | Full |
| Servo Control | ✅ | ✅ | Full |
| Heartbeat | ✅ | ✅ | Full |
| PDO Transmission | ✅ | ✅ | Full |
| SDO Communication | ✅ | ✅ | Full |
| Error Handling | ✅ | ✅ | Full |

*Auto Navigation requires route data and navigation algorithms

## Future Enhancements

1. **Route Management**: Implementacja pełnego systemu tras WB
2. **Advanced Navigation**: Integracja z algorytmami nawigacji WB
3. **Configuration GUI**: Graficzny interfejs konfiguracji
4. **Data Logging**: Zaawansowane logowanie komunikacji
5. **Performance Optimization**: Optymalizacja wydajności komunikacji

## Contact

**MELKENS Integration Team**  
Email: integration@melkens.com  
Documentation: [Internal Wiki]  
Support: [Ticket System]