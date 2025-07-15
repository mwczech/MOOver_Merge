/*
 * WB_Integration_Example.c
 * 
 * Example usage and integration tests for WB compatibility layer
 * 
 * Author: MELKENS Integration Team
 * Created: 2024
 */

#include "WB_CanOpen.h"
#include "WB_Config.h"
#include "CanHandler.h"
#include "../pmb_MotorManager.h"
#include "../pmb_System.h"
#include <stdio.h>
#include <string.h>

// Example functions demonstrating WB integration

/**
 * Initialize MELKENS with WB compatibility
 */
void WB_Integration_Init(void) {
    printf("=== MELKENS WB Integration Example ===\n");
    
    // Initialize configuration
    WB_Config_Init();
    
    // Initialize CAN handler with WB compatibility
    CANHandler_Init();
    
    // Configure servo controllers with WB parameters
    CANHandler_ConfigureServo(WB_NODE_SERVO_LEFT);
    CANHandler_ConfigureServo(WB_NODE_SERVO_RIGHT);
    CANHandler_ConfigureServo(WB_NODE_SERVO_THUMBLE);
    
    printf("WB Integration initialized successfully\n");
}

/**
 * Example: Manual robot control using WB protocol
 */
void WB_Integration_ManualControl_Example(void) {
    printf("\n=== Manual Control Example ===\n");
    
    // Send manual control commands via WB CANopen
    
    // 1. Set robot to manual mode
    WB_Butler_Control_t butler_cmd = {0};
    butler_cmd.manualRequest = 1;
    WB_CANopen_ProcessButlerCommand(&butler_cmd);
    
    // 2. Send forward movement command
    int8_t speed = 50;      // 50% forward speed
    int8_t steering = 0;    // No steering
    
    // This would normally come from PDO, but we can simulate it
    CANHandler_SendWBMessage(WB_NODE_BUTLER_MAIN, OD_PDO_VARIABLE_MANUAL, 0x01, speed);
    CANHandler_SendWBMessage(WB_NODE_BUTLER_MAIN, OD_PDO_VARIABLE_MANUAL, 0x02, steering);
    
    printf("Manual control: Speed=%d, Steering=%d\n", speed, steering);
    
    // 3. Send motor commands directly to servos
    CANHandler_SendMotorCommand(WB_NODE_SERVO_LEFT, 500, WB_ACCELERATION_DEFAULT);
    CANHandler_SendMotorCommand(WB_NODE_SERVO_RIGHT, 500, WB_ACCELERATION_DEFAULT);
    
    printf("Direct motor commands sent\n");
}

/**
 * Example: Automatic drive sequence
 */
void WB_Integration_AutoDrive_Example(void) {
    printf("\n=== Auto Drive Example ===\n");
    
    // Start automatic drive sequence
    WB_Butler_Control_t butler_cmd = {0};
    butler_cmd.driveRequest = WB_DRIVE_REQ_AUTO;
    butler_cmd.driveLength = 10.0f; // 10 meters
    
    WB_CANopen_ProcessButlerCommand(&butler_cmd);
    
    printf("Auto drive sequence started: 10m forward\n");
    
    // Simulate drive sequence
    for (int i = 0; i < 10; i++) {
        // Update position and send status
        WB_CANopen_UpdateFromMelkens();
        WB_CANopen_SendPDO(1); // Send status PDO
        
        // Simulate 1 second delay
        System_DelayMs(1000);
        
        printf("Auto drive progress: %d/10 meters\n", i+1);
    }
    
    // Stop drive sequence
    butler_cmd.driveRequest = WB_DRIVE_REQ_STOP;
    WB_CANopen_ProcessButlerCommand(&butler_cmd);
    
    printf("Auto drive sequence completed\n");
}

/**
 * Example: Emergency stop handling
 */
void WB_Integration_EmergencyStop_Example(void) {
    printf("\n=== Emergency Stop Example ===\n");
    
    // Simulate emergency condition
    WB_Butler_Control_t butler_cmd = {0};
    butler_cmd.abortRequest = 1;
    
    WB_CANopen_ProcessButlerCommand(&butler_cmd);
    
    // Send emergency message
    WB_CANopen_SendEmergency(WB_ERROR_SYSTEM_FAULT, 0x01, NULL);
    
    printf("Emergency stop executed\n");
}

/**
 * Example: Sensor data monitoring
 */
void WB_Integration_SensorMonitoring_Example(void) {
    printf("\n=== Sensor Monitoring Example ===\n");
    
    // Read sensor values from MELKENS subsystems
    uint16_t battery_voltage = BatteryManager_GetVoltage();
    uint8_t battery_level = BatteryManager_GetLevel();
    int16_t left_current = MotorManager_GetCurrent(Motor_Left);
    int16_t right_current = MotorManager_GetCurrent(Motor_Right);
    float yaw_angle = IMUHandler_GetYaw();
    
    printf("Battery: %dmV (%d%%)\n", battery_voltage, battery_level);
    printf("Motor currents: Left=%dmA, Right=%dmA\n", left_current, right_current);
    printf("Yaw angle: %.2f degrees\n", yaw_angle);
    
    // Send sensor data via CANopen SDO
    CANHandler_SendWBMessage(WB_NODE_BUTLER_MAIN, OD_PDO_VARIABLE_MANUAL, 0x0F, battery_voltage);
    CANHandler_SendWBMessage(WB_NODE_BUTLER_MAIN, OD_PDO_VARIABLE_MANUAL, 0x0E, battery_level);
    CANHandler_SendWBMessage(WB_NODE_BUTLER_MAIN, OD_PDO_VARIABLE_MANUAL, 0x14, left_current);
    CANHandler_SendWBMessage(WB_NODE_BUTLER_MAIN, OD_PDO_VARIABLE_MANUAL, 0x15, right_current);
    
    // Convert angle to WB format (degrees * 100)
    int32_t wb_angle = (int32_t)(yaw_angle * 100);
    CANHandler_SendWBMessage(WB_NODE_BUTLER_MAIN, OD_PDO_VARIABLE_MANUAL, 0x0A, wb_angle);
    
    printf("Sensor data transmitted via CANopen\n");
}

/**
 * Example: Servo parameter configuration
 */
void WB_Integration_ServoConfig_Example(void) {
    printf("\n=== Servo Configuration Example ===\n");
    
    // Configure left servo with custom parameters
    WB_Servo_Profile_t profile;
    profile.feedConstant = 1200;         // Higher precision
    profile.profileAcceleration = 50000; // Faster acceleration
    profile.profileDeceleration = 60000; // Faster deceleration
    profile.maxProfileVelocity = 30000;  // Higher max speed
    profile.currentLimit = 400;          // Higher current limit
    
    WB_CANopen_UpdateServoParameters(WB_NODE_SERVO_LEFT, &profile);
    
    printf("Left servo configured with custom parameters\n");
    printf("- Feed constant: %lu\n", profile.feedConstant);
    printf("- Acceleration: %lu\n", profile.profileAcceleration);
    printf("- Max velocity: %lu\n", profile.maxProfileVelocity);
    printf("- Current limit: %d mA\n", profile.currentLimit);
}

/**
 * Example: CAN bus diagnostics
 */
void WB_Integration_Diagnostics_Example(void) {
    printf("\n=== CAN Diagnostics Example ===\n");
    
    uint32_t rx_count, tx_count;
    CANHandler_GetStatistics(&rx_count, &tx_count);
    
    printf("CAN Statistics:\n");
    printf("- Messages received: %lu\n", rx_count);
    printf("- Messages transmitted: %lu\n", tx_count);
    
    // Check WB compatibility status
    bool wb_enabled = CANHandler_IsWBCompatibilityEnabled();
    printf("- WB compatibility: %s\n", wb_enabled ? "Enabled" : "Disabled");
    
    // Check CANopen state
    CANopen_State_t state = WB_CANopen_GetState();
    const char* state_names[] = {
        "INITIALIZATION", "PRE_OPERATIONAL", "OPERATIONAL", "STOPPED"
    };
    printf("- CANopen state: %s\n", state_names[state]);
    
    // Check error register
    uint8_t error_reg = WB_CANopen_GetErrorRegister();
    printf("- Error register: 0x%02X\n", error_reg);
}

/**
 * Main integration test function
 */
void WB_Integration_RunTests(void) {
    printf("Starting WB Integration Tests...\n\n");
    
    // Initialize system
    WB_Integration_Init();
    
    // Run examples
    WB_Integration_ManualControl_Example();
    System_DelayMs(1000);
    
    WB_Integration_AutoDrive_Example();
    System_DelayMs(1000);
    
    WB_Integration_EmergencyStop_Example();
    System_DelayMs(1000);
    
    WB_Integration_SensorMonitoring_Example();
    System_DelayMs(1000);
    
    WB_Integration_ServoConfig_Example();
    System_DelayMs(1000);
    
    WB_Integration_Diagnostics_Example();
    
    printf("\nWB Integration Tests completed successfully!\n");
}

/**
 * Periodic task example - call from main loop
 */
void WB_Integration_PeriodicTask(void) {
    static uint32_t task_counter = 0;
    task_counter++;
    
    // Update CAN handler tasks
    CANHandler_Task();
    
    // Run periodic tasks at different rates
    CANHandler_PeriodicTasks_1ms();
    
    if (task_counter % 10 == 0) {
        CANHandler_PeriodicTasks_10ms();
    }
    
    if (task_counter % 100 == 0) {
        CANHandler_PeriodicTasks_100ms();
        
        // Every 100ms, update sensor data
        WB_CANopen_UpdateFromMelkens();
    }
    
    if (task_counter % 1000 == 0) {
        // Every second, print status
        printf("WB Integration running... Counter: %lu\n", task_counter);
        task_counter = 0; // Reset to prevent overflow
    }
}

/**
 * Command line interface for testing
 */
void WB_Integration_CLI(char* command) {
    if (strcmp(command, "init") == 0) {
        WB_Integration_Init();
    }
    else if (strcmp(command, "manual") == 0) {
        WB_Integration_ManualControl_Example();
    }
    else if (strcmp(command, "auto") == 0) {
        WB_Integration_AutoDrive_Example();
    }
    else if (strcmp(command, "stop") == 0) {
        WB_Integration_EmergencyStop_Example();
    }
    else if (strcmp(command, "sensors") == 0) {
        WB_Integration_SensorMonitoring_Example();
    }
    else if (strcmp(command, "servo") == 0) {
        WB_Integration_ServoConfig_Example();
    }
    else if (strcmp(command, "diag") == 0) {
        WB_Integration_Diagnostics_Example();
    }
    else if (strcmp(command, "test") == 0) {
        WB_Integration_RunTests();
    }
    else if (strcmp(command, "enable_wb") == 0) {
        CANHandler_SetWBCompatibility(true);
        printf("WB compatibility enabled\n");
    }
    else if (strcmp(command, "disable_wb") == 0) {
        CANHandler_SetWBCompatibility(false);
        printf("WB compatibility disabled\n");
    }
    else {
        printf("Available commands:\n");
        printf("  init      - Initialize WB integration\n");
        printf("  manual    - Manual control example\n");
        printf("  auto      - Auto drive example\n");
        printf("  stop      - Emergency stop example\n");
        printf("  sensors   - Sensor monitoring example\n");
        printf("  servo     - Servo configuration example\n");
        printf("  diag      - Diagnostics example\n");
        printf("  test      - Run all tests\n");
        printf("  enable_wb - Enable WB compatibility\n");
        printf("  disable_wb- Disable WB compatibility\n");
    }
}