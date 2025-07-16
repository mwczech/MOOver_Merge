# WB-MELKENS Integration Analysis & Test Plan

## 📊 Recent PR Changes Analysis

### Commit Overview (Last 4 Commits)
```
1141611 - Add WB navigation emulator and simulator for MELKENS integration
b7d4b17 - Add WB CANopen compatibility layer for MELKENS system  
49d53cc - Add completeness analysis document
04360a8 - Add comprehensive project structure overview
```

### 📁 Files Modified/Added in Integration

#### Core Integration Files
| File | Purpose | Lines | Critical Level |
|------|---------|-------|----------------|
| `WB_CanOpen.h` | CANopen protocol definitions | 300 | 🔴 Critical |
| `WB_CanOpen.c` | CANopen implementation | 700 | 🔴 Critical |
| `WB_Config.h` | System configuration & mappings | 184 | 🟡 High |
| `CANHandler.c` | Enhanced CAN message routing | 200 | 🔴 Critical |
| `WB_Navigation_Emulator.h` | WB navigation emulation API | 350 | 🟡 High |
| `WB_Navigation_Emulator.c` | WB navigation implementation | 800 | 🟡 High |
| `WB_Integration_Example.c` | Usage examples & tests | 314 | 🟢 Medium |

#### Analysis & Documentation Files
| File | Purpose | Impact |
|------|---------|---------|
| `Navigation_Differences_Analysis.md` | MELKENS vs WB comparison | Strategic |
| `Robot_Navigation_Simulator.py` | Visual algorithm comparison | Validation |
| `README_WB_MELKENS_Integration.md` | Integration documentation | Operational |
| `WB_CAN_Compatibility_Summary.md` | Implementation summary | Technical |

---

## 🔗 Key Integration Points Analysis

### 1. **CANopen Protocol Layer**

#### Critical Integration Functions
```c
// Protocol State Management
void WB_CANopen_Init(uint8_t nodeId);                    // CRITICAL
void WB_CANopen_SetState(CANopen_State_t newState);      // CRITICAL
CANopen_State_t WB_CANopen_GetState(void);               // HIGH

// Message Processing
void WB_CANopen_ProcessMessage(CAN_MSG_OBJ* msg);        // CRITICAL
bool WB_CANopen_SendSDO(uint8_t targetNode, ...);        // CRITICAL
bool WB_CANopen_SendPDO(uint8_t pdoNumber);              // HIGH
void WB_CANopen_SendHeartbeat(void);                     // HIGH

// Object Dictionary
bool WB_CANopen_ReadOD(uint16_t index, ...);             // CRITICAL
bool WB_CANopen_WriteOD(uint16_t index, ...);            // CRITICAL
void WB_CANopen_InitObjectDictionary(void);              // CRITICAL

// MELKENS Integration Bridge
void WB_CANopen_MapToMelkens(void);                      // CRITICAL
void WB_CANopen_UpdateFromMelkens(void);                 // CRITICAL
void WB_CANopen_ProcessButlerCommand(...);               // HIGH
```

### 2. **Node ID Mappings** (WB_Config.h)
```c
WB_NODE_BUTLER_MAIN     = 0x40   // Main Butler Engine
WB_NODE_SERVO_LEFT      = 0x7E   // Left drive motor  
WB_NODE_SERVO_RIGHT     = 0x7F   // Right drive motor
WB_NODE_SERVO_THUMBLE   = 0x7D   // Thumble/Drum motor
WB_NODE_MAGNET_LINEAR   = 0x10   // Magnetic linear encoder
WB_NODE_STEERING_WHEEL  = 0x20   // Steering wheel controller
```

### 3. **Motor Control Bridge**
```c
// MELKENS Motor Manager → WB Servo Control
Motor_Left   → WB_NODE_SERVO_LEFT   (0x7E)
Motor_Right  → WB_NODE_SERVO_RIGHT  (0x7F)  
Motor_Thumble → WB_NODE_SERVO_THUMBLE (0x7D)
```

### 4. **Navigation System Bridge**
```c
// MELKENS Route Steps → WB World Coordinates
RouteStep.dX/dY → WB_WorldPosition_t
MagnetName (1-31) → WB magnetic field strength
MELKENS pure pursuit → WB coordinate navigation
```

---

## 🧪 Comprehensive Integration Test Plan

### Phase 1: Unit Tests (Critical Functions)

#### A. CANopen Protocol Tests
```c
void test_canopen_initialization(void) {
    // Test: WB_CANopen_Init()
    WB_CANopen_Init(WB_NODE_BUTLER_MAIN);
    assert(WB_CANopen_GetState() == CANOPEN_STATE_PRE_OPERATIONAL);
    assert(canopen_node.nodeId == WB_NODE_BUTLER_MAIN);
}

void test_canopen_state_transitions(void) {
    // Test: State machine transitions
    WB_CANopen_SetState(CANOPEN_STATE_OPERATIONAL);
    assert(WB_CANopen_GetState() == CANOPEN_STATE_OPERATIONAL);
    
    WB_CANopen_SetState(CANOPEN_STATE_STOPPED);
    assert(WB_CANopen_GetState() == CANOPEN_STATE_STOPPED);
}

void test_sdo_communication(void) {
    // Test: SDO write/read operations
    uint32_t test_data = 0x12345678;
    bool result = WB_CANopen_SendSDO(WB_NODE_SERVO_LEFT, 0x6040, 0x00, test_data);
    assert(result == true);
    
    // Verify Object Dictionary access
    uint32_t read_data;
    uint16_t data_size = sizeof(uint32_t);
    result = WB_CANopen_ReadOD(0x6040, 0x00, &read_data, &data_size);
    assert(result == true);
    assert(read_data == test_data);
}

void test_pdo_transmission(void) {
    // Test: PDO data transmission
    WB_CANopen_UpdateFromMelkens();  // Populate with MELKENS data
    bool result = WB_CANopen_SendPDO(1);
    assert(result == true);
    
    // Verify PDO content matches MELKENS state
    verify_pdo_data_integrity();
}

void test_heartbeat_mechanism(void) {
    // Test: Heartbeat transmission
    uint32_t start_time = System_GetTimeMs();
    WB_CANopen_SendHeartbeat();
    uint32_t end_time = System_GetTimeMs();
    
    assert((end_time - start_time) < 5); // Should complete quickly
    verify_heartbeat_message_format();
}
```

#### B. Motor Control Integration Tests
```c
void test_motor_mapping(void) {
    // Test: MELKENS → WB motor mapping
    int16_t melkens_speed = 800;
    MotorManager_SetSpeed(Motor_Left, melkens_speed);
    
    WB_CANopen_UpdateFromMelkens();
    
    // Verify WB servo receives correct command
    verify_servo_command(WB_NODE_SERVO_LEFT, melkens_speed);
}

void test_servo_parameter_update(void) {
    // Test: WB servo configuration
    WB_Servo_Profile_t profile = {
        .maxVelocity = 1000,
        .acceleration = 500,
        .deceleration = 500,
        .motionProfileType = 1
    };
    
    WB_CANopen_UpdateServoParameters(WB_NODE_SERVO_LEFT, &profile);
    verify_servo_configuration(WB_NODE_SERVO_LEFT, &profile);
}

void test_emergency_stop(void) {
    // Test: Emergency stop propagation
    MotorManager_EmergencyStop();
    WB_CANopen_UpdateFromMelkens();
    
    // Verify all WB servos receive stop command
    verify_all_servos_stopped();
    verify_emergency_message_sent();
}
```

#### C. Navigation Integration Tests
```c
void test_route_conversion(void) {
    // Test: MELKENS RouteStep → WB navigation
    RouteStep melkens_step = {
        .OperationType = NORM,
        .dX = 500,  // 5 meters
        .dY = 0,
        .RightSpeed = 800,
        .LeftSpeed = 800
    };
    
    bool result = WB_NavEmulator_ConvertMelkensRoute(&melkens_step);
    assert(result == true);
    
    WB_WorldPosition_t target = WB_NavEmulator_GetPosition();
    assert(abs(target.x - 5.0) < 0.01);  // 5 meters converted correctly
}

void test_magnet_detection_emulation(void) {
    // Test: MELKENS magnet → WB magnetic field
    WB_NavEmulator_SimulateMagnetDetection(Magnet16);  // Center magnet
    
    const WB_NavigationContext_t* ctx = WB_NavEmulator_GetContext();
    assert(ctx->magneticPositionValid == true);
    assert(abs(ctx->magneticFieldStrength - 100.0) < 1.0);  // Strong signal at center
}
```

### Phase 2: Integration Tests (System Level)

#### A. Full Communication Stack Test
```c
void test_full_can_communication_stack(void) {
    // Initialize complete system
    CANHandler_Init();
    WB_Integration_Init();
    
    // Send manual control command
    WB_Butler_Control_t cmd = {
        .manualRequest = 1,
        .manualSpeed = 50,
        .manualSteering = 10,
        .driveRequest = WB_DRIVE_REQ_MANUAL
    };
    
    // Process through entire stack
    WB_CANopen_ProcessButlerCommand(&cmd);
    WB_CANopen_UpdateFromMelkens();
    
    // Verify motors respond correctly
    assert(MotorManager_GetSpeed(Motor_Left) != 0);
    assert(MotorManager_GetSpeed(Motor_Right) != 0);
}

void test_navigation_sequence_integration(void) {
    // Test complete navigation sequence
    WB_NavEmulator_Init();
    
    // Set navigation target
    WB_NavEmulator_SetTarget(5.0f, 3.0f, 90.0f);
    
    // Simulate navigation loop
    for (int i = 0; i < 100; i++) {
        WB_NavEmulator_Update();
        
        // Simulate magnet detection every 10 cycles
        if (i % 10 == 0) {
            WB_NavEmulator_SimulateMagnetDetection(Magnet16);
        }
        
        // Update CAN communication
        WB_CANopen_Task_10ms();
    }
    
    // Verify navigation completed
    WB_NavigationState_t state = WB_NavEmulator_GetState();
    assert(state == WB_NAV_STATE_IDLE);
}
```

#### B. Error Handling Tests
```c
void test_can_error_recovery(void) {
    // Simulate CAN bus error
    simulate_can_bus_error();
    
    // Verify error handling
    WB_CANopen_HandleError(0x8001);
    assert(WB_CANopen_GetErrorRegister() & 0x01);
    
    // Test recovery
    WB_CANopen_ResetCommunication();
    assert(WB_CANopen_GetState() == CANOPEN_STATE_PRE_OPERATIONAL);
}

void test_node_timeout_handling(void) {
    // Simulate node timeout
    uint32_t start_time = System_GetTimeMs();
    
    // Wait for heartbeat timeout
    while (System_GetTimeMs() - start_time < 2000) {
        WB_CANopen_Task_100ms();
    }
    
    // Verify timeout detection
    verify_node_timeout_detected(WB_NODE_SERVO_LEFT);
}
```

### Phase 3: Performance Tests

#### A. Timing Requirements
```c
void test_canopen_task_timing(void) {
    // Test 1ms task performance
    uint32_t start = System_GetMicros();
    WB_CANopen_Task_1ms();
    uint32_t elapsed = System_GetMicros() - start;
    assert(elapsed < 500);  // Must complete in <500µs
    
    // Test 10ms task performance  
    start = System_GetMicros();
    WB_CANopen_Task_10ms();
    elapsed = System_GetMicros() - start;
    assert(elapsed < 5000);  // Must complete in <5ms
}

void test_message_throughput(void) {
    // Test CAN message processing rate
    uint32_t start_time = System_GetTimeMs();
    uint16_t messages_processed = 0;
    
    while (System_GetTimeMs() - start_time < 1000) {  // 1 second test
        simulate_can_message();
        messages_processed++;
    }
    
    assert(messages_processed > 100);  // Minimum 100 msg/sec
}
```

---

## 📋 Step-by-Step Migration Checklist

### Pre-Migration Phase ✅

#### 1. Environment Preparation
```bash
□ Backup existing MELKENS configuration
□ Verify CAN hardware compatibility (check CAN transceivers)
□ Update development environment (MPLAB X, compilers)
□ Set up WB device simulators/test environment
□ Prepare rollback procedures
```

#### 2. Code Integration
```c
□ Add WB compatibility files to MELKENS project:
   ├── WB_CanOpen.h/.c
   ├── WB_Config.h  
   ├── WB_Navigation_Emulator.h/.c
   └── Modified CANHandler.c

□ Update project includes and linker settings
□ Configure CAN baud rate for WB compatibility (500kbps/1Mbps)
□ Verify no symbol conflicts with existing MELKENS code
```

#### 3. Configuration Setup
```c
□ Configure Node IDs in WB_Config.h:
   WB_NODE_BUTLER_MAIN     = 0x40
   WB_NODE_SERVO_LEFT      = 0x7E  
   WB_NODE_SERVO_RIGHT     = 0x7F
   WB_NODE_SERVO_THUMBLE   = 0x7D

□ Set up Object Dictionary entries
□ Configure PDO mappings
□ Set heartbeat intervals (1000ms default)
```

### Migration Phase 1: Parallel Operation 🔄

#### 4. Initialize WB Layer (Non-Intrusive)
```c
// In main() initialization
void migration_phase1_init(void) {
    // Initialize WB layer in monitoring mode
    WB_CANopen_Init(WB_NODE_BUTLER_MAIN);
    WB_CANopen_SetState(CANOPEN_STATE_PRE_OPERATIONAL);
    
    // Keep MELKENS navigation active
    RouteManager_Init();  // Keep original system
    
    // Start WB emulator in background
    WB_NavEmulator_Init();
    WB_NavEmulator_SetEmulationMode(false);  // Monitor only
}

□ Deploy and test basic CAN communication
□ Verify WB layer initializes without affecting MELKENS
□ Monitor CAN bus traffic for conflicts
□ Test heartbeat transmission
□ Validate Object Dictionary access
```

#### 5. Enable WB Motor Monitoring
```c
// In main loop
void migration_phase1_loop(void) {
    // Original MELKENS operation
    RouteManager_Update();
    MotorManager_Update();
    
    // WB monitoring (read-only)
    WB_CANopen_UpdateFromMelkens();  // Mirror MELKENS state
    WB_CANopen_Task_10ms();
    
    // Optional: Log comparison data
    log_melkens_vs_wb_state();
}

□ Verify WB layer correctly mirrors MELKENS motor states
□ Test all motor operations (forward, reverse, stop)
□ Verify thumble motor operation
□ Check emergency stop propagation
□ Monitor for timing impacts on MELKENS performance
```

### Migration Phase 2: WB Motor Control 🚗

#### 6. Switch Motor Control to WB
```c
// Modify motor control calls
void migration_phase2_motor_control(void) {
    // OLD: Direct MELKENS motor control
    // MotorManager_SetSpeed(Motor_Left, speed);
    
    // NEW: WB motor control
    WB_Butler_Control_t cmd = {
        .manualRequest = 1,
        .manualSpeed = speed,
        .manualSteering = steering
    };
    WB_CANopen_ProcessButlerCommand(&cmd);
}

□ Replace direct MotorManager calls with WB commands
□ Test manual control through WB protocol
□ Verify speed/direction mapping correctness
□ Test differential steering calculations
□ Validate emergency stop through WB
□ Monitor motor response times
```

#### 7. Enable WB Servo Parameters
```c
□ Configure servo acceleration/deceleration profiles
□ Set maximum velocity limits via WB protocol
□ Test servo fault detection and reporting
□ Validate motor current monitoring
□ Check servo status feedback
```

### Migration Phase 3: WB Navigation 🧭

#### 8. Enable WB Navigation Emulation
```c
void migration_phase3_navigation(void) {
    // Enable WB navigation layer
    WB_NavEmulator_SetEmulationMode(true);
    
    // Convert existing routes to WB format
    RouteStep* current_route = RouteManager_GetCurrentRoute();
    for (int i = 0; i < route_length; i++) {
        WB_NavEmulator_ConvertMelkensRoute(&current_route[i]);
    }
}

□ Test route conversion from MELKENS steps to WB coordinates
□ Verify magnet detection integration
□ Test bay approach and feeding operations
□ Validate world coordinate navigation
□ Compare navigation accuracy vs original MELKENS
```

#### 9. Magnetic Positioning Integration
```c
// In magnet detection handler
void migration_phase3_magnets(void) {
    if (magnet_detected) {
        MagnetName detected = get_detected_magnet();
        
        // Parallel processing
        apply_melkens_correction(detected);        // Keep original
        WB_NavEmulator_SimulateMagnetDetection(detected);  // Add WB
        
        // Compare results
        compare_navigation_corrections();
    }
}

□ Test WB magnetic field simulation
□ Verify dynamic correction calculations
□ Compare correction accuracy: MELKENS vs WB
□ Test reference position updates
□ Validate magnetic field strength thresholds
```

### Migration Phase 4: Full WB Operation 🎯

#### 10. Switch to Pure WB Navigation
```c
void migration_phase4_full_wb(void) {
    // Disable MELKENS route manager
    RouteManager_SetEnabled(false);
    
    // Enable full WB navigation
    WB_NavEmulator_SetEmulationMode(true);
    WB_CANopen_SetState(CANOPEN_STATE_OPERATIONAL);
    
    // Use WB for all navigation decisions
    // (Keep MELKENS as fallback)
}

□ Disable MELKENS navigation completely
□ Run robot purely on WB navigation
□ Test complete feeding cycles
□ Verify track following accuracy
□ Test complex route scenarios
□ Performance comparison vs original MELKENS
```

#### 11. Production Validation
```c
□ 24-hour continuous operation test
□ Test all feeding scenarios (morning, evening cycles)
□ Verify operation in different environmental conditions
□ Test error recovery scenarios
□ Performance metrics collection and analysis
□ User acceptance testing
```

### Post-Migration Phase ✅

#### 12. Cleanup and Optimization
```c
□ Remove unused MELKENS navigation code (optional)
□ Optimize WB task scheduling
□ Fine-tune PID parameters for optimal performance
□ Update documentation and user manuals
□ Train operators on WB system
□ Establish monitoring and maintenance procedures
```

#### 13. Rollback Procedures (If Needed)
```c
void emergency_rollback_to_melkens(void) {
    // Disable WB layer
    WB_CANopen_SetState(CANOPEN_STATE_STOPPED);
    WB_NavEmulator_SetEmulationMode(false);
    
    // Re-enable MELKENS
    RouteManager_SetEnabled(true);
    MotorManager_ResetToDirectControl();
    
    log_rollback_reason();
}

□ Document rollback triggers and procedures
□ Test rollback scenarios during migration
□ Maintain MELKENS capability as fallback
□ Create troubleshooting guides
```

---

## 📁 CANopen Compatibility Implementation Files

### Core CANopen Implementation

#### 1. **WB_CanOpen.h** (300 lines) - Protocol Definitions
```c
// Key Structures and Enums
typedef enum {
    CANOPEN_STATE_INITIALIZATION = 0x00,
    CANOPEN_STATE_PRE_OPERATIONAL = 0x7F,
    CANOPEN_STATE_OPERATIONAL = 0x05,
    CANOPEN_STATE_STOPPED = 0x04
} CANopen_State_t;

typedef struct {
    uint16_t index;
    uint8_t subindex;
    uint8_t dataType;
    void* data;
    uint16_t dataSize;
    bool writable;
} WB_ObjectDictionary_Entry_t;

// WB Butler Control (Compatible with Butler Engine commands)
typedef struct {
    uint16_t driveRequest;      // WB_DRIVE_REQ_*
    uint16_t manualRequest;     // Manual control enable
    int16_t manualSpeed;        // Manual speed (-100 to +100)
    int16_t manualSteering;     // Manual steering (-100 to +100)
    uint16_t routeRequest;      // Route execution request
    float driveLength;          // Distance to drive
    uint8_t abortRequest;       // Emergency abort
} WB_Butler_Control_t;
```

#### 2. **WB_CanOpen.c** (700 lines) - Core Implementation
```c
// Critical Functions Implemented:

// Protocol Management
void WB_CANopen_Init(uint8_t nodeId)
void WB_CANopen_SetState(CANopen_State_t newState)
void WB_CANopen_ProcessMessage(CAN_MSG_OBJ* msg)

// Communication Services
bool WB_CANopen_SendSDO(uint8_t targetNode, uint16_t index, uint8_t subindex, uint32_t data)
bool WB_CANopen_SendPDO(uint8_t pdoNumber)
void WB_CANopen_SendHeartbeat(void)
void WB_CANopen_SendEmergency(uint16_t errorCode, uint8_t errorRegister, uint8_t* manufData)

// Object Dictionary
bool WB_CANopen_ReadOD(uint16_t index, uint8_t subindex, void* data, uint16_t* dataSize)
bool WB_CANopen_WriteOD(uint16_t index, uint8_t subindex, void* data, uint16_t dataSize)

// MELKENS Integration Bridge
void WB_CANopen_MapToMelkens(void)      // WB → MELKENS data flow
void WB_CANopen_UpdateFromMelkens(void) // MELKENS → WB data flow
```

#### 3. **WB_Config.h** (184 lines) - System Configuration
```c
// Node ID Mappings (Critical for CAN addressing)
WB_NODE_BUTLER_MAIN     = 0x40  // Main Butler Engine
WB_NODE_SERVO_LEFT      = 0x7E  // Left drive motor
WB_NODE_SERVO_RIGHT     = 0x7F  // Right drive motor
WB_NODE_SERVO_THUMBLE   = 0x7D  // Thumble/Drum motor
WB_NODE_MAGNET_LINEAR   = 0x10  // Magnetic linear encoder
WB_NODE_STEERING_WHEEL  = 0x20  // Steering wheel controller

// Object Dictionary Indices (CANopen standard)
#define OD_DEVICE_TYPE           0x1000
#define OD_ERROR_REGISTER        0x1001  
#define OD_HEARTBEAT_TIME        0x1017
#define OD_IDENTITY_OBJECT       0x1018
#define OD_PDO_VARIABLE_MANUAL   0x2010  // WB-specific manual control
#define OD_BUTLER_CTRL           0x4000  // WB-specific butler commands
#define OD_CONTROLWORD           0x6040  // Servo control word
#define OD_STATUSWORD            0x6041  // Servo status word
#define OD_MAX_PROFILE_VELOCITY  0x607F  // Maximum velocity
#define OD_PROFILE_ACCELERATION  0x6083  // Profile acceleration
```

### Enhanced CAN Message Routing

#### 4. **CANHandler.c** (Enhanced with WB Support)
```c
// Key Integration Points:

void CANHandler_Init(void) {
    // Initialize standard MELKENS CAN
    CAN_Initialize();
    
    // Initialize WB CANopen layer
    WB_CANopen_Init(WB_NODE_BUTLER_ENGINE);
    
    // Set up intelligent message routing
    setup_wb_message_filters();
}

void CANHandler_ProcessReceivedMessage(CAN_MSG_OBJ* msg) {
    // Intelligent message routing
    if (is_wb_canopen_message(msg)) {
        WB_CANopen_ProcessMessage(msg);  // Route to WB layer
    } else {
        process_melkens_message(msg);    // Route to MELKENS layer
    }
}

// WB Motor Control Integration
void CANHandler_SetMotorSpeed_WB(uint8_t motor, int16_t speed) {
    uint8_t motorNode = get_wb_motor_node(motor);
    uint32_t acceleration = 1000;  // Default acceleration
    
    // Send velocity command via CANopen SDO
    WB_CANopen_SendSDO(motorNode, OD_MAX_PROFILE_VELOCITY, 0x00, abs(speed));
    WB_CANopen_SendSDO(motorNode, OD_PROFILE_ACCELERATION, 0x00, acceleration);
    
    // Send control word to start motion
    uint16_t control_word = 0x003F;  // Enable operation + new set-point
    WB_CANopen_SendSDO(motorNode, 0x6040, 0x00, control_word);
}
```

### Navigation Emulation Layer

#### 5. **WB_Navigation_Emulator.h/.c** (1150 lines total)
```c
// WB-style navigation implementation compatible with MELKENS

// Core navigation context
typedef struct {
    WB_WorldPosition_t currentPos;    // Current world position
    WB_WorldPosition_t targetPos;     // Target world position
    WB_NavigationState_t state;       // Navigation state
    
    float crossTrackError;            // Cross-track error
    float headingError;               // Heading error
    float magneticFieldStrength;      // Current magnetic field
    bool magneticPositionValid;       // Position fix valid
} WB_NavigationContext_t;

// Key Functions:
void WB_NavEmulator_SimulateMagnetDetection(MagnetName detectedMagnet)
bool WB_NavEmulator_NavigateToTrack(uint32_t trackId)
bool WB_NavEmulator_ApproachBay(uint32_t bayId)
bool WB_NavEmulator_ConvertMelkensRoute(const RouteStep* melkensStep)
```

### Integration Examples and Testing

#### 6. **WB_Integration_Example.c** (314 lines)
```c
// Practical usage examples for developers

void WB_Integration_ManualControl_Example(void) {
    WB_Butler_Control_t butler_cmd = {
        .manualRequest = 1,
        .manualSpeed = 50,        // 50% forward speed
        .manualSteering = -20,    // 20% left steering
        .driveRequest = WB_DRIVE_REQ_MANUAL
    };
    WB_CANopen_ProcessButlerCommand(&butler_cmd);
}

void WB_Integration_AutoFeeding_Example(void) {
    // Approach bay 1 for feeding
    WB_NavEmulator_ApproachBay(1);
    
    // Execute feeding operation
    WB_NavEmulator_ExecuteFeeding(1, 2.5f);  // 2.5kg feed
}
```

---

## 🔍 CAN Communication Diagnostics Framework

### Real-Time Diagnostics Implementation

#### 1. **CAN Message Monitor**
```c
// WB_CANDiagnostics.h
typedef struct {
    uint32_t totalMessages;
    uint32_t sdoMessages;
    uint32_t pdoMessages;
    uint32_t heartbeatMessages;
    uint32_t emergencyMessages;
    uint32_t errorCount;
    uint32_t lastMessageTime;
    uint8_t busLoadPercent;
} WB_CANStatistics_t;

typedef struct {
    uint8_t nodeId;
    uint32_t lastHeartbeat;
    bool online;
    uint8_t errorRegister;
    CANopen_State_t nodeState;
} WB_NodeStatus_t;

// Implementation in WB_CANDiagnostics.c
void WB_CANDiag_Init(void) {
    memset(&can_stats, 0, sizeof(WB_CANStatistics_t));
    
    // Initialize node monitoring
    for (int i = 0; i < MAX_NODES; i++) {
        node_status[i].nodeId = 0;
        node_status[i].online = false;
    }
    
    printf("CAN Diagnostics initialized\n");
}

void WB_CANDiag_ProcessMessage(CAN_MSG_OBJ* msg) {
    can_stats.totalMessages++;
    can_stats.lastMessageTime = System_GetTimeMs();
    
    // Decode message type
    uint32_t cobId = msg->msgobj.id;
    uint8_t functionCode = (cobId >> 7) & 0x0F;
    uint8_t nodeId = cobId & 0x7F;
    
    switch (functionCode) {
        case CANOPEN_FC_HEARTBEAT:
            can_stats.heartbeatMessages++;
            WB_CANDiag_UpdateNodeStatus(nodeId, msg->msgobj.data[0]);
            break;
            
        case CANOPEN_FC_SDO_TX:
        case CANOPEN_FC_SDO_RX:
            can_stats.sdoMessages++;
            WB_CANDiag_AnalyzeSDO(msg);
            break;
            
        case CANOPEN_FC_PDO1_TX:
        case CANOPEN_FC_PDO2_TX:
        case CANOPEN_FC_PDO3_TX:
        case CANOPEN_FC_PDO4_TX:
            can_stats.pdoMessages++;
            WB_CANDiag_AnalyzePDO(msg);
            break;
            
        case CANOPEN_FC_EMERGENCY:
            can_stats.emergencyMessages++;
            WB_CANDiag_ProcessEmergency(msg);
            break;
    }
    
    // Calculate bus load
    WB_CANDiag_CalculateBusLoad();
}
```

#### 2. **Node Health Monitoring**
```c
void WB_CANDiag_UpdateNodeStatus(uint8_t nodeId, uint8_t state) {
    WB_NodeStatus_t* node = WB_CANDiag_FindNode(nodeId);
    if (node == NULL) {
        node = WB_CANDiag_AddNode(nodeId);
    }
    
    node->lastHeartbeat = System_GetTimeMs();
    node->online = true;
    node->nodeState = (CANopen_State_t)state;
    
    printf("Node 0x%02X: State=0x%02X, Online=true\n", nodeId, state);
}

void WB_CANDiag_CheckNodeTimeouts(void) {
    uint32_t current_time = System_GetTimeMs();
    
    for (int i = 0; i < MAX_NODES; i++) {
        if (node_status[i].nodeId == 0) continue;
        
        uint32_t timeout = current_time - node_status[i].lastHeartbeat;
        
        if (timeout > WB_HEARTBEAT_TIMEOUT_MS) {
            if (node_status[i].online) {
                printf("WARNING: Node 0x%02X timeout (%lu ms)\n", 
                       node_status[i].nodeId, timeout);
                node_status[i].online = false;
                can_stats.errorCount++;
                
                // Trigger error handling
                WB_CANDiag_HandleNodeTimeout(node_status[i].nodeId);
            }
        }
    }
}
```

#### 3. **SDO Communication Verification**
```c
typedef struct {
    uint8_t targetNode;
    uint16_t index;
    uint8_t subindex;
    uint32_t data;
    uint32_t timestamp;
    bool responseReceived;
    uint32_t responseTime;
} WB_SDOTransaction_t;

void WB_CANDiag_TrackSDORequest(uint8_t targetNode, uint16_t index, 
                                uint8_t subindex, uint32_t data) {
    // Find free transaction slot
    WB_SDOTransaction_t* txn = WB_CANDiag_GetFreeTransaction();
    if (txn == NULL) return;
    
    txn->targetNode = targetNode;
    txn->index = index;
    txn->subindex = subindex;
    txn->data = data;
    txn->timestamp = System_GetTimeMs();
    txn->responseReceived = false;
    
    printf("SDO Request: Node=0x%02X, Index=0x%04X, Subindex=0x%02X\n",
           targetNode, index, subindex);
}

void WB_CANDiag_AnalyzeSDO(CAN_MSG_OBJ* msg) {
    uint8_t* data = msg->msgobj.data;
    uint8_t cmd = data[0];
    uint16_t index = (data[2] << 8) | data[1];
    uint8_t subindex = data[3];
    
    if (cmd & 0x80) {  // SDO response
        WB_SDOTransaction_t* txn = WB_CANDiag_FindPendingTransaction(index, subindex);
        if (txn) {
            txn->responseReceived = true;
            txn->responseTime = System_GetTimeMs() - txn->timestamp;
            
            printf("SDO Response: Index=0x%04X, Time=%lu ms\n", 
                   index, txn->responseTime);
                   
            if (txn->responseTime > WB_SDO_WARNING_TIME_MS) {
                printf("WARNING: Slow SDO response (%lu ms)\n", txn->responseTime);
            }
        }
    }
}
```

#### 4. **Motor Performance Diagnostics**
```c
typedef struct {
    uint8_t nodeId;
    int16_t commandedSpeed;
    int16_t actualSpeed;
    uint32_t timestamp;
    uint16_t current;
    uint8_t temperature;
    uint16_t statusWord;
} WB_MotorDiagnostics_t;

void WB_CANDiag_MonitorMotorPerformance(void) {
    for (int motor = 0; motor < WB_MOTOR_COUNT; motor++) {
        uint8_t nodeId = get_motor_node_id(motor);
        
        // Request actual velocity
        WB_CANopen_SendSDO(nodeId, 0x606C, 0x00, 0);  // Read actual velocity
        
        // Request motor current
        WB_CANopen_SendSDO(nodeId, 0x6078, 0x00, 0);  // Read current actual value
        
        // Request status word
        WB_CANopen_SendSDO(nodeId, 0x6041, 0x00, 0);  // Read status word
        
        // Analyze performance
        WB_MotorDiagnostics_t* diag = &motor_diagnostics[motor];
        
        if (diag->timestamp > 0) {
            int16_t speed_error = abs(diag->commandedSpeed - diag->actualSpeed);
            
            if (speed_error > WB_MOTOR_SPEED_TOLERANCE) {
                printf("WARNING: Motor %d speed error: cmd=%d, actual=%d\n",
                       motor, diag->commandedSpeed, diag->actualSpeed);
            }
            
            if (diag->current > WB_MOTOR_CURRENT_WARNING) {
                printf("WARNING: Motor %d high current: %d mA\n", 
                       motor, diag->current);
            }
            
            // Check for servo faults
            if (diag->statusWord & 0x0008) {  // Fault bit
                printf("ERROR: Motor %d servo fault detected: 0x%04X\n",
                       motor, diag->statusWord);
                WB_CANDiag_HandleMotorFault(motor, diag->statusWord);
            }
        }
    }
}
```

#### 5. **Diagnostic Commands Interface**
```c
void WB_CANDiag_ExecuteCommand(const char* cmd) {
    if (strcmp(cmd, "status") == 0) {
        WB_CANDiag_PrintStatus();
    }
    else if (strcmp(cmd, "nodes") == 0) {
        WB_CANDiag_PrintNodeStatus();
    }
    else if (strcmp(cmd, "motors") == 0) {
        WB_CANDiag_PrintMotorStatus();
    }
    else if (strcmp(cmd, "sdo") == 0) {
        WB_CANDiag_PrintSDOStatistics();
    }
    else if (strncmp(cmd, "test_motor ", 11) == 0) {
        int motor = atoi(cmd + 11);
        WB_CANDiag_TestMotor(motor);
    }
    else if (strncmp(cmd, "ping ", 5) == 0) {
        int nodeId = strtol(cmd + 5, NULL, 16);
        WB_CANDiag_PingNode(nodeId);
    }
    else if (strcmp(cmd, "reset") == 0) {
        WB_CANDiag_ResetStatistics();
    }
    else {
        WB_CANDiag_PrintHelp();
    }
}

void WB_CANDiag_PrintStatus(void) {
    printf("\n=== WB CAN Diagnostics Status ===\n");
    printf("Total Messages: %lu\n", can_stats.totalMessages);
    printf("SDO Messages: %lu\n", can_stats.sdoMessages);
    printf("PDO Messages: %lu\n", can_stats.pdoMessages);
    printf("Heartbeat Messages: %lu\n", can_stats.heartbeatMessages);
    printf("Emergency Messages: %lu\n", can_stats.emergencyMessages);
    printf("Error Count: %lu\n", can_stats.errorCount);
    printf("Bus Load: %d%%\n", can_stats.busLoadPercent);
    printf("Last Message: %lu ms ago\n", 
           System_GetTimeMs() - can_stats.lastMessageTime);
    printf("==================================\n");
}

void WB_CANDiag_TestMotor(uint8_t motor) {
    printf("Testing motor %d...\n", motor);
    
    uint8_t nodeId = get_motor_node_id(motor);
    
    // Test sequence
    printf("1. Ping node...\n");
    WB_CANDiag_PingNode(nodeId);
    
    printf("2. Read device info...\n");
    WB_CANopen_SendSDO(nodeId, 0x1000, 0x00, 0);  // Device type
    WB_CANopen_SendSDO(nodeId, 0x1018, 0x01, 0);  // Vendor ID
    
    printf("3. Test motion...\n");
    WB_CANopen_SendSDO(nodeId, 0x607F, 0x00, 100);  // Set max velocity
    WB_CANopen_SendSDO(nodeId, 0x6040, 0x00, 0x003F);  // Enable operation
    
    delay_ms(1000);
    
    WB_CANopen_SendSDO(nodeId, 0x6040, 0x00, 0x0006);  // Stop
    
    printf("Motor test completed\n");
}
```

#### 6. **Integration with MELKENS Diagnostics**
```c
void WB_CANDiag_IntegrateWithMelkens(void) {
    // Hook into MELKENS diagnostic system
    DiagnosticsHandler_RegisterCallback(WB_CANDiag_MelkensCallback);
    
    // Add WB diagnostics to system status
    SystemStatus_RegisterProvider("WB_CAN", WB_CANDiag_GetSystemStatus);
    
    // Integration with MELKENS error reporting
    ErrorManager_RegisterErrorSource("WB_CANopen", WB_CANDiag_GetErrorStatus);
}

SystemStatus_t WB_CANDiag_GetSystemStatus(void) {
    SystemStatus_t status = {0};
    
    status.component_id = COMPONENT_WB_CAN;
    status.health = HEALTH_OK;
    
    // Check critical conditions
    if (can_stats.errorCount > WB_ERROR_THRESHOLD) {
        status.health = HEALTH_WARNING;
        strcpy(status.message, "High CAN error count");
    }
    
    // Check node availability
    int offline_nodes = WB_CANDiag_CountOfflineNodes();
    if (offline_nodes > 0) {
        status.health = HEALTH_ERROR;
        snprintf(status.message, sizeof(status.message), 
                "%d nodes offline", offline_nodes);
    }
    
    // Check bus load
    if (can_stats.busLoadPercent > WB_BUS_LOAD_WARNING) {
        status.health = HEALTH_WARNING;
        strcpy(status.message, "High CAN bus load");
    }
    
    return status;
}
```

---

## 📊 Critical Test Matrix Summary

| Component | Critical Functions | Test Priority | Migration Phase |
|-----------|-------------------|---------------|-----------------|
| **CANopen Protocol** | `WB_CANopen_Init()`, `WB_CANopen_ProcessMessage()` | 🔴 Critical | Phase 1 |
| **Motor Control** | `WB_CANopen_ProcessButlerCommand()` | 🔴 Critical | Phase 2 |
| **Navigation** | `WB_NavEmulator_ConvertMelkensRoute()` | 🟡 High | Phase 3 |
| **Diagnostics** | `WB_CANDiag_MonitorMotorPerformance()` | 🟡 High | All Phases |
| **Error Handling** | `WB_CANopen_HandleError()` | 🟡 High | Phase 1 |

### Success Criteria for Migration
- ✅ All critical functions pass unit tests
- ✅ CAN bus communication stable for 24+ hours
- ✅ Motor control accuracy within ±2% of commanded speed
- ✅ Navigation accuracy within ±10cm of target positions
- ✅ Emergency stop response time <100ms
- ✅ Zero data loss during WB/MELKENS operation
- ✅ System performance impact <5% vs original MELKENS

This comprehensive integration analysis provides the roadmap for successfully migrating MELKENS to WB navigation while maintaining system reliability and performance.