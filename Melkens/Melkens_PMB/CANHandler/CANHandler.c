/*
 * CANHandler.c
 * 
 * Enhanced CAN Handler with WB compatibility layer
 * 
 * Author: MELKENS Integration Team
 * Updated: 2024
 */

#include "CanHandler.h"
#include "WB_CanOpen.h"
#include "../pmb_CAN.h"
#include "../mcc_generated_files/can1.h"
#include <stdio.h>

// Global variables
static bool wb_compatibility_enabled = true;
static uint32_t can_rx_count = 0;
static uint32_t can_tx_count = 0;

/**
 * Initialize CAN Handler with WB compatibility
 */
void CANHandler_Init(void) {
    printf("Initializing CAN Handler with WB compatibility...\n");
    
    // Initialize WB CANopen layer with node ID 0x40 (Butler Engine compatible)
    if (wb_compatibility_enabled) {
        WB_CANopen_Init(WB_NODE_BUTLER_ENGINE);
        printf("WB CANopen compatibility layer enabled\n");
    }
    
    // Initialize legacy CAN polling
    CAN_Internal_Init();
    
    printf("CAN Handler initialization complete\n");
}

/**
 * Main CAN message processing task
 */
void CANHandler_Task(void) {
    CAN_MSG_OBJ received_msg;
    
    // Check for received messages
    if (CAN1_ReceivedMessageCountGet() > 0) {
        if (CAN1_Receive(&received_msg)) {
            can_rx_count++;
            
            // Try WB CANopen processing first
            if (wb_compatibility_enabled) {
                if (CANHandler_IsWBMessage(&received_msg)) {
                    WB_CANopen_ProcessMessage(&received_msg);
                    return; // Message handled by WB layer
                }
            }
            
            // Fall back to legacy MELKENS processing
            CANHandler_ProcessLegacyMessage(&received_msg);
        }
    }
}

/**
 * Check if message is WB CANopen format
 */
bool CANHandler_IsWBMessage(CAN_MSG_OBJ* msg) {
    if (msg == NULL) return false;
    
    uint32_t msg_id = msg->msgId;
    uint16_t function_code = (msg_id >> 7) & 0x0F;
    uint8_t node_id = msg_id & 0x7F;
    
    // Check for CANopen function codes
    switch (function_code) {
        case (CANOPEN_FC_NMT >> 7):
        case (CANOPEN_FC_SDO_TX >> 7):
        case (CANOPEN_FC_SDO_RX >> 7):
        case (CANOPEN_FC_PDO1_TX >> 7):
        case (CANOPEN_FC_PDO1_RX >> 7):
        case (CANOPEN_FC_PDO2_TX >> 7):
        case (CANOPEN_FC_PDO2_RX >> 7):
        case (CANOPEN_FC_PDO3_TX >> 7):
        case (CANOPEN_FC_PDO3_RX >> 7):
        case (CANOPEN_FC_PDO4_TX >> 7):
        case (CANOPEN_FC_PDO4_RX >> 7):
        case (CANOPEN_FC_HEARTBEAT >> 7):
        case (CANOPEN_FC_EMERGENCY >> 7):
            return true;
            
        default:
            break;
    }
    
    // Check for known WB node IDs
    if (node_id == WB_NODE_BUTLER_ENGINE ||
        node_id == WB_NODE_SERVO_LEFT ||
        node_id == WB_NODE_SERVO_RIGHT ||
        node_id == WB_NODE_SERVO_THUMBLE ||
        node_id == WB_NODE_MAGNET_LINEAR ||
        node_id == WB_NODE_STEERING_WHEEL) {
        return true;
    }
    
    return false;
}

/**
 * Process legacy MELKENS CAN messages
 */
void CANHandler_ProcessLegacyMessage(CAN_MSG_OBJ* msg) {
    // Use existing CAN_Polling function for backward compatibility
    // This maintains compatibility with existing MELKENS code
    
    // Note: We need to set the global CAN_Rx variable for legacy code
    extern CAN_MSG_OBJ CAN_Rx;
    CAN_Rx = *msg;
    
    // Process using existing logic
    CAN_Polling();
}

/**
 * Send WB-compatible message
 */
bool CANHandler_SendWBMessage(uint8_t targetNode, uint16_t index, uint8_t subindex, uint32_t data) {
    if (!wb_compatibility_enabled) {
        return false;
    }
    
    bool result = WB_CANopen_SendSDO(targetNode, index, subindex, data);
    if (result) {
        can_tx_count++;
    }
    
    return result;
}

/**
 * Send motor command in WB format
 */
bool CANHandler_SendMotorCommand(uint8_t motorNode, int16_t speed, uint16_t acceleration) {
    if (!wb_compatibility_enabled) {
        return false;
    }
    
    // Send speed command
    bool result1 = WB_CANopen_SendSDO(motorNode, OD_MAX_PROFILE_VELOCITY, 0x00, abs(speed));
    
    // Send acceleration
    bool result2 = WB_CANopen_SendSDO(motorNode, OD_PROFILE_ACCELERATION, 0x00, acceleration);
    
    // Send direction/enable command
    uint32_t control_word = (speed >= 0) ? 0x000F : 0x020F; // Forward/reverse + enable
    bool result3 = WB_CANopen_SendSDO(motorNode, 0x6040, 0x00, control_word); // Control word
    
    if (result1 && result2 && result3) {
        can_tx_count += 3;
        return true;
    }
    
    return false;
}

/**
 * Configure servo parameters for WB compatibility
 */
bool CANHandler_ConfigureServo(uint8_t servoNode) {
    if (!wb_compatibility_enabled) {
        return false;
    }
    
    // Set up default WB servo profile
    WB_Servo_Profile_t profile;
    profile.feedConstant = 1000;           // From servo.xdd
    profile.profileAcceleration = 40000;   // From servo.xdd
    profile.profileDeceleration = 40000;   // From servo.xdd
    profile.maxProfileVelocity = 25000;    // From servo.xdd
    profile.currentLimit = 350;            // From servo.xdd
    
    WB_CANopen_UpdateServoParameters(servoNode, &profile);
    
    printf("Servo 0x%02X configured with WB parameters\n", servoNode);
    return true;
}

/**
 * Enable/disable WB compatibility mode
 */
void CANHandler_SetWBCompatibility(bool enable) {
    wb_compatibility_enabled = enable;
    
    if (enable) {
        printf("WB compatibility mode enabled\n");
        WB_CANopen_SetState(CANOPEN_STATE_OPERATIONAL);
    } else {
        printf("WB compatibility mode disabled\n");
        WB_CANopen_SetState(CANOPEN_STATE_STOPPED);
    }
}

/**
 * Get WB compatibility status
 */
bool CANHandler_IsWBCompatibilityEnabled(void) {
    return wb_compatibility_enabled;
}

/**
 * Get CAN statistics
 */
void CANHandler_GetStatistics(uint32_t* rxCount, uint32_t* txCount) {
    if (rxCount) *rxCount = can_rx_count;
    if (txCount) *txCount = can_tx_count;
}

/**
 * Periodic tasks - call from main loop
 */
void CANHandler_PeriodicTasks_1ms(void) {
    if (wb_compatibility_enabled) {
        WB_CANopen_Task_1ms();
    }
}

void CANHandler_PeriodicTasks_10ms(void) {
    if (wb_compatibility_enabled) {
        WB_CANopen_Task_10ms();
    }
}

void CANHandler_PeriodicTasks_100ms(void) {
    if (wb_compatibility_enabled) {
        WB_CANopen_Task_100ms();
    }
}
