/*
 * WB_CanOpen.c
 * 
 * Wasserbauer-compatible CANopen protocol implementation for MELKENS
 * 
 * Author: MELKENS Integration Team
 * Created: 2024
 */

#include "WB_CanOpen.h"
#include "../pmb_MotorManager.h"
#include "../pmb_Functions.h"
#include "../IMUHandler/IMUHandler.h"
#include "../DiagnosticsHandler.h"
#include "../BatteryManager/BatteryManager.h"
#include "../pmb_System.h"
#include <string.h>
#include <stdio.h>

// Global Variables
static CANopen_Node_t canopen_node;
static OD_Entry_t object_dictionary[256];  // Object dictionary entries
static uint16_t od_entry_count = 0;
static uint32_t heartbeat_counter = 0;
static uint32_t sync_counter = 0;

// Software version string
static const char SW_VERSION[] = "MELKENS_WB_v1.0.0";

// Device type for Butler-compatible robot
static const uint32_t DEVICE_TYPE = 0x00000033; // Generic I/O device

// Error register bits
static uint8_t error_register = 0x00;

// Private function prototypes
static void WB_CANopen_ProcessSDO(CAN_MSG_OBJ* msg);
static void WB_CANopen_ProcessPDO(CAN_MSG_OBJ* msg);
static void WB_CANopen_ProcessNMT(CAN_MSG_OBJ* msg);
static void WB_CANopen_ProcessHeartbeat(CAN_MSG_OBJ* msg);
static bool WB_CANopen_SendMessage(uint32_t cobId, uint8_t* data, uint8_t length);
static void WB_CANopen_AddODEntry(uint16_t index, uint8_t subindex, uint8_t dataType, 
                                  uint8_t access, void* data, uint16_t dataSize);

/**
 * Initialize CANopen layer with specified node ID
 */
void WB_CANopen_Init(uint8_t nodeId) {
    // Initialize node structure
    memset(&canopen_node, 0, sizeof(CANopen_Node_t));
    canopen_node.nodeId = nodeId;
    canopen_node.state = CANOPEN_STATE_INITIALIZATION;
    canopen_node.heartbeatEnabled = true;
    canopen_node.heartbeatTime = 1000; // 1 second heartbeat
    
    // Initialize Object Dictionary
    WB_CANopen_InitObjectDictionary();
    
    // Set initial state to Pre-operational
    WB_CANopen_SetState(CANOPEN_STATE_PRE_OPERATIONAL);
    
    printf("WB CANopen initialized with Node ID: 0x%02X\n", nodeId);
}

/**
 * Set CANopen state
 */
void WB_CANopen_SetState(CANopen_State_t newState) {
    canopen_node.state = newState;
    
    switch(newState) {
        case CANOPEN_STATE_INITIALIZATION:
            printf("CANopen: Entering INITIALIZATION state\n");
            break;
        case CANOPEN_STATE_PRE_OPERATIONAL:
            printf("CANopen: Entering PRE-OPERATIONAL state\n");
            break;
        case CANOPEN_STATE_OPERATIONAL:
            printf("CANopen: Entering OPERATIONAL state\n");
            break;
        case CANOPEN_STATE_STOPPED:
            printf("CANopen: Entering STOPPED state\n");
            break;
    }
}

/**
 * Get current CANopen state
 */
CANopen_State_t WB_CANopen_GetState(void) {
    return canopen_node.state;
}

/**
 * Process incoming CAN message
 */
void WB_CANopen_ProcessMessage(CAN_MSG_OBJ* msg) {
    if (msg == NULL) return;
    
    uint16_t function_code = (msg->msgId >> 7) & 0x0F;
    uint8_t node_id = msg->msgId & 0x7F;
    
    // Check if message is for us or broadcast
    if (node_id != 0 && node_id != canopen_node.nodeId) {
        return; // Not for us
    }
    
    switch (function_code) {
        case (CANOPEN_FC_NMT >> 7):
            WB_CANopen_ProcessNMT(msg);
            break;
            
        case (CANOPEN_FC_SDO_RX >> 7):
            WB_CANopen_ProcessSDO(msg);
            break;
            
        case (CANOPEN_FC_PDO1_RX >> 7):
        case (CANOPEN_FC_PDO2_RX >> 7):
        case (CANOPEN_FC_PDO3_RX >> 7):
        case (CANOPEN_FC_PDO4_RX >> 7):
            WB_CANopen_ProcessPDO(msg);
            break;
            
        case (CANOPEN_FC_HEARTBEAT >> 7):
            WB_CANopen_ProcessHeartbeat(msg);
            break;
            
        default:
            // Unknown function code
            break;
    }
}

/**
 * Process SDO (Service Data Object) message
 */
static void WB_CANopen_ProcessSDO(CAN_MSG_OBJ* msg) {
    if (msg->field.dlc < 8) return; // SDO must be 8 bytes
    
    uint8_t command = msg->data[0];
    uint16_t index = (msg->data[2] << 8) | msg->data[1];
    uint8_t subindex = msg->data[3];
    uint32_t data = (msg->data[7] << 24) | (msg->data[6] << 16) | 
                   (msg->data[5] << 8) | msg->data[4];
    
    uint8_t response[8] = {0};
    bool send_response = true;
    
    switch (command & 0xE0) {
        case SDO_CMD_DOWNLOAD_INITIATE: {
            // Write data to object dictionary
            uint16_t dataSize = 4 - ((command >> 2) & 0x03); // Expedited transfer
            if (WB_CANopen_WriteOD(index, subindex, &data, dataSize)) {
                response[0] = SDO_RESP_DOWNLOAD_INITIATE;
            } else {
                response[0] = SDO_CMD_ABORT_TRANSFER;
                uint32_t error = SDO_ERROR_OBJECT_NOT_EXIST;
                memcpy(&response[4], &error, 4);
            }
            break;
        }
        
        case SDO_CMD_UPLOAD_INITIATE: {
            // Read data from object dictionary
            uint32_t read_data = 0;
            uint16_t dataSize = sizeof(read_data);
            
            if (WB_CANopen_ReadOD(index, subindex, &read_data, &dataSize)) {
                response[0] = SDO_RESP_UPLOAD_INITIATE | ((4 - dataSize) << 2) | 0x01; // Expedited
                memcpy(&response[4], &read_data, dataSize);
            } else {
                response[0] = SDO_CMD_ABORT_TRANSFER;
                uint32_t error = SDO_ERROR_OBJECT_NOT_EXIST;
                memcpy(&response[4], &error, 4);
            }
            break;
        }
        
        default:
            // Unsupported command
            response[0] = SDO_CMD_ABORT_TRANSFER;
            uint32_t error = SDO_ERROR_INVALID_COMMAND;
            memcpy(&response[4], &error, 4);
            break;
    }
    
    if (send_response) {
        response[1] = msg->data[1]; // Index low
        response[2] = msg->data[2]; // Index high
        response[3] = msg->data[3]; // Subindex
        
        uint32_t response_cobid = WB_CANopen_GetCOBID(CANOPEN_FC_SDO_TX, canopen_node.nodeId);
        WB_CANopen_SendMessage(response_cobid, response, 8);
    }
}

/**
 * Process PDO (Process Data Object) message
 */
static void WB_CANopen_ProcessPDO(CAN_MSG_OBJ* msg) {
    // For now, we'll handle PDO1 which should contain manual control data
    uint16_t function_code = (msg->msgId >> 7) & 0x0F;
    
    if (function_code == (CANOPEN_FC_PDO1_RX >> 7)) {
        // This is manual control data - map to MELKENS motor commands
        if (msg->field.dlc >= 2) {
            int8_t speed = (int8_t)msg->data[0];
            int8_t steering = (int8_t)msg->data[1];
            
            // Map WB commands to MELKENS motor manager
            // Convert speed (-100 to +100) to MELKENS format
            if (speed != 0) {
                MotorManager_SetSpeed(Motor_Left, speed);
                MotorManager_SetSpeed(Motor_Right, speed);
            }
            
            // Convert steering (-100 to +100) to differential drive
            if (steering != 0) {
                int16_t left_speed = speed + steering/2;
                int16_t right_speed = speed - steering/2;
                
                MotorManager_SetSpeed(Motor_Left, left_speed);
                MotorManager_SetSpeed(Motor_Right, right_speed);
            }
        }
    }
}

/**
 * Process NMT (Network Management) message
 */
static void WB_CANopen_ProcessNMT(CAN_MSG_OBJ* msg) {
    if (msg->field.dlc < 2) return;
    
    uint8_t command = msg->data[0];
    uint8_t target_node = msg->data[1];
    
    // Check if command is for us or broadcast
    if (target_node != 0 && target_node != canopen_node.nodeId) {
        return;
    }
    
    switch (command) {
        case NMT_START_REMOTE_NODE:
            WB_CANopen_SetState(CANOPEN_STATE_OPERATIONAL);
            break;
            
        case NMT_STOP_REMOTE_NODE:
            WB_CANopen_SetState(CANOPEN_STATE_STOPPED);
            break;
            
        case NMT_ENTER_PRE_OPERATIONAL:
            WB_CANopen_SetState(CANOPEN_STATE_PRE_OPERATIONAL);
            break;
            
        case NMT_RESET_NODE:
            WB_CANopen_SetState(CANOPEN_STATE_INITIALIZATION);
            // Perform node reset
            WB_CANopen_ResetCommunication();
            break;
            
        case NMT_RESET_COMMUNICATION:
            WB_CANopen_ResetCommunication();
            break;
    }
}

/**
 * Process Heartbeat message from other nodes
 */
static void WB_CANopen_ProcessHeartbeat(CAN_MSG_OBJ* msg) {
    if (msg->field.dlc < 1) return;
    
    uint8_t remote_node = msg->msgId & 0x7F;
    uint8_t remote_state = msg->data[0];
    
    // Update diagnostics based on received heartbeats
    switch (remote_node) {
        case WB_NODE_SERVO_LEFT:
            Diagnostics_SetEvent(dDebug_LeftInverterConnected);
            break;
            
        case WB_NODE_SERVO_RIGHT:
            Diagnostics_SetEvent(dDebug_RightInverterConnected);
            break;
            
        default:
            // Other nodes
            break;
    }
}

/**
 * Send SDO message
 */
bool WB_CANopen_SendSDO(uint8_t targetNode, uint16_t index, uint8_t subindex, uint32_t data) {
    uint8_t sdo_data[8] = {0};
    
    sdo_data[0] = SDO_CMD_DOWNLOAD_INITIATE | 0x03; // Expedited, 4 bytes
    sdo_data[1] = index & 0xFF;         // Index low
    sdo_data[2] = (index >> 8) & 0xFF;  // Index high
    sdo_data[3] = subindex;             // Subindex
    memcpy(&sdo_data[4], &data, 4);     // Data
    
    uint32_t cobid = WB_CANopen_GetCOBID(CANOPEN_FC_SDO_RX, targetNode);
    return WB_CANopen_SendMessage(cobid, sdo_data, 8);
}

/**
 * Send PDO message
 */
bool WB_CANopen_SendPDO(uint8_t pdoNumber) {
    uint8_t pdo_data[8] = {0};
    uint8_t length = 8;
    
    switch (pdoNumber) {
        case 1: {
            // Send manual control data
            pdo_data[0] = (uint8_t)canopen_node.pdoManual.speed;
            pdo_data[1] = (uint8_t)canopen_node.pdoManual.steering;
            pdo_data[2] = canopen_node.pdoManual.batteryLevel;
            pdo_data[3] = canopen_node.pdoManual.monitorState;
            memcpy(&pdo_data[4], &canopen_node.pdoManual.batteryVoltage, 2);
            memcpy(&pdo_data[6], &canopen_node.pdoManual.iShunt, 2);
            break;
        }
        
        default:
            return false;
    }
    
    uint32_t cobid = WB_CANopen_GetCOBID(CANOPEN_FC_PDO1_TX + ((pdoNumber-1) << 7), canopen_node.nodeId);
    return WB_CANopen_SendMessage(cobid, pdo_data, length);
}

/**
 * Send Heartbeat message
 */
void WB_CANopen_SendHeartbeat(void) {
    uint8_t heartbeat_data[1];
    heartbeat_data[0] = (uint8_t)canopen_node.state;
    
    uint32_t cobid = WB_CANopen_GetCOBID(CANOPEN_FC_HEARTBEAT, canopen_node.nodeId);
    WB_CANopen_SendMessage(cobid, heartbeat_data, 1);
}

/**
 * Send Emergency message
 */
void WB_CANopen_SendEmergency(uint16_t errorCode, uint8_t errorRegister, uint8_t* manufData) {
    uint8_t emergency_data[8] = {0};
    
    emergency_data[0] = errorCode & 0xFF;
    emergency_data[1] = (errorCode >> 8) & 0xFF;
    emergency_data[2] = errorRegister;
    
    if (manufData != NULL) {
        memcpy(&emergency_data[3], manufData, 5);
    }
    
    uint32_t cobid = WB_CANopen_GetCOBID(CANOPEN_FC_EMERGENCY, canopen_node.nodeId);
    WB_CANopen_SendMessage(cobid, emergency_data, 8);
}

/**
 * Read from Object Dictionary
 */
bool WB_CANopen_ReadOD(uint16_t index, uint8_t subindex, void* data, uint16_t* dataSize) {
    // Find entry in object dictionary
    for (uint16_t i = 0; i < od_entry_count; i++) {
        if (object_dictionary[i].index == index && 
            object_dictionary[i].subindex == subindex) {
            
            if (object_dictionary[i].access & ACCESS_RO) {
                uint16_t copySize = (*dataSize < object_dictionary[i].dataSize) ? 
                                  *dataSize : object_dictionary[i].dataSize;
                memcpy(data, object_dictionary[i].data, copySize);
                *dataSize = copySize;
                return true;
            } else {
                return false; // Write-only
            }
        }
    }
    
    return false; // Not found
}

/**
 * Write to Object Dictionary
 */
bool WB_CANopen_WriteOD(uint16_t index, uint8_t subindex, void* data, uint16_t dataSize) {
    // Find entry in object dictionary
    for (uint16_t i = 0; i < od_entry_count; i++) {
        if (object_dictionary[i].index == index && 
            object_dictionary[i].subindex == subindex) {
            
            if (object_dictionary[i].access & ACCESS_WO) {
                uint16_t copySize = (dataSize < object_dictionary[i].dataSize) ? 
                                  dataSize : object_dictionary[i].dataSize;
                memcpy(object_dictionary[i].data, data, copySize);
                
                // Handle special cases for motor control
                if (index == OD_PDO_VARIABLE_MANUAL) {
                    WB_CANopen_MapToMelkens();
                }
                
                return true;
            } else {
                return false; // Read-only
            }
        }
    }
    
    return false; // Not found
}

/**
 * Initialize Object Dictionary with WB-compatible objects
 */
void WB_CANopen_InitObjectDictionary(void) {
    od_entry_count = 0;
    
    // Standard CANopen objects
    WB_CANopen_AddODEntry(OD_DEVICE_TYPE, 0x00, DT_UNSIGNED32, ACCESS_RO, 
                         (void*)&DEVICE_TYPE, sizeof(uint32_t));
    
    WB_CANopen_AddODEntry(OD_ERROR_REGISTER, 0x00, DT_UNSIGNED8, ACCESS_RO, 
                         &error_register, sizeof(uint8_t));
    
    WB_CANopen_AddODEntry(OD_SW_VERSION, 0x00, DT_VISIBLE_STRING, ACCESS_RO, 
                         (void*)SW_VERSION, strlen(SW_VERSION));
    
    WB_CANopen_AddODEntry(OD_NODE_ID, 0x00, DT_UNSIGNED8, ACCESS_RW, 
                         &canopen_node.nodeId, sizeof(uint8_t));
    
    // WB Application objects
    WB_CANopen_AddODEntry(OD_PDO_VARIABLE_MANUAL, 0x01, DT_INTEGER8, ACCESS_RW, 
                         &canopen_node.pdoManual.speed, sizeof(int8_t));
    
    WB_CANopen_AddODEntry(OD_PDO_VARIABLE_MANUAL, 0x02, DT_INTEGER8, ACCESS_RW, 
                         &canopen_node.pdoManual.steering, sizeof(int8_t));
    
    WB_CANopen_AddODEntry(OD_PDO_VARIABLE_MANUAL, 0x0E, DT_UNSIGNED8, ACCESS_RO, 
                         &canopen_node.pdoManual.batteryLevel, sizeof(uint8_t));
    
    WB_CANopen_AddODEntry(OD_PDO_VARIABLE_MANUAL, 0x0F, DT_UNSIGNED16, ACCESS_RO, 
                         &canopen_node.pdoManual.batteryVoltage, sizeof(uint16_t));
    
    // Butler Control objects
    WB_CANopen_AddODEntry(OD_BUTLER_CTRL, 0x01, DT_UNSIGNED16, ACCESS_RW, 
                         &canopen_node.butlerCtrl.driveRequest, sizeof(uint16_t));
    
    WB_CANopen_AddODEntry(OD_BUTLER_CTRL, 0x02, DT_UNSIGNED16, ACCESS_RO, 
                         &canopen_node.butlerCtrl.butlerState, sizeof(uint16_t));
    
    printf("Object Dictionary initialized with %d entries\n", od_entry_count);
}

/**
 * Add entry to Object Dictionary
 */
static void WB_CANopen_AddODEntry(uint16_t index, uint8_t subindex, uint8_t dataType, 
                                  uint8_t access, void* data, uint16_t dataSize) {
    if (od_entry_count < 256) {
        object_dictionary[od_entry_count].index = index;
        object_dictionary[od_entry_count].subindex = subindex;
        object_dictionary[od_entry_count].dataType = dataType;
        object_dictionary[od_entry_count].access = access;
        object_dictionary[od_entry_count].data = data;
        object_dictionary[od_entry_count].dataSize = dataSize;
        od_entry_count++;
    }
}

/**
 * Map WB data to MELKENS subsystems
 */
void WB_CANopen_MapToMelkens(void) {
    // Map speed and steering to motor manager
    if (canopen_node.pdoManual.speed != 0 || canopen_node.pdoManual.steering != 0) {
        int16_t left_speed = canopen_node.pdoManual.speed + canopen_node.pdoManual.steering/2;
        int16_t right_speed = canopen_node.pdoManual.speed - canopen_node.pdoManual.steering/2;
        
        MotorManager_SetSpeed(Motor_Left, left_speed);
        MotorManager_SetSpeed(Motor_Right, right_speed);
    }
    
    // Map drum speed
    if (canopen_node.pdoManual.trommelSpeed != 0) {
        MotorManager_SetSpeed(Motor_Thumble, canopen_node.pdoManual.trommelSpeed);
    }
}

/**
 * Update WB data from MELKENS subsystems
 */
void WB_CANopen_UpdateFromMelkens(void) {
    // Update battery data
    canopen_node.pdoManual.batteryVoltage = BatteryManager_GetVoltage();
    canopen_node.pdoManual.batteryLevel = BatteryManager_GetLevel();
    
    // Update motor currents
    canopen_node.pdoManual.BLX1Current = MotorManager_GetCurrent(Motor_Left);
    canopen_node.pdoManual.BLX2Current = MotorManager_GetCurrent(Motor_Right);
    canopen_node.pdoManual.BLX3Current = MotorManager_GetCurrent(Motor_Thumble);
    
    // Update motor speeds
    canopen_node.pdoManual.BLX1Speed = MotorManager_GetSpeed(Motor_Left);
    canopen_node.pdoManual.BLX2Speed = MotorManager_GetSpeed(Motor_Right);
    canopen_node.pdoManual.BLX3Speed = MotorManager_GetSpeed(Motor_Thumble);
    
    // Update IMU data if available
    float yaw = IMUHandler_GetYaw();
    canopen_node.pdoManual.cruiseYawDeg = yaw;
    canopen_node.pdoManual.l3dg20Angle = yaw;
}

/**
 * Process Butler commands
 */
void WB_CANopen_ProcessButlerCommand(WB_Butler_Control_t* cmd) {
    if (cmd == NULL) return;
    
    // Handle drive requests
    if (cmd->driveRequest) {
        // Start automatic drive mode
        canopen_node.butlerCtrl.butlerState = 0x01; // Running
    }
    
    // Handle abort request
    if (cmd->abortRequest) {
        // Stop all motors
        MotorManager_Stop(Motor_Left);
        MotorManager_Stop(Motor_Right);
        MotorManager_Stop(Motor_Thumble);
        canopen_node.butlerCtrl.butlerState = 0x00; // Stopped
    }
    
    // Handle manual mode
    if (cmd->manualRequest) {
        canopen_node.butlerCtrl.butlerState = 0x02; // Manual mode
    }
}

/**
 * Update servo parameters
 */
void WB_CANopen_UpdateServoParameters(uint8_t servoNode, WB_Servo_Profile_t* profile) {
    if (profile == NULL) return;
    
    // Send servo configuration via SDO
    WB_CANopen_SendSDO(servoNode, OD_FEED_CONSTANT, 0x01, profile->feedConstant);
    WB_CANopen_SendSDO(servoNode, OD_PROFILE_ACCELERATION, 0x00, profile->profileAcceleration);
    WB_CANopen_SendSDO(servoNode, OD_PROFILE_DECELERATION, 0x00, profile->profileDeceleration);
    WB_CANopen_SendSDO(servoNode, OD_MAX_PROFILE_VELOCITY, 0x00, profile->maxProfileVelocity);
    WB_CANopen_SendSDO(servoNode, OD_CURRENT_LIMIT, 0x00, profile->currentLimit);
}

/**
 * Get COB-ID from function code and node ID
 */
uint32_t WB_CANopen_GetCOBID(uint8_t functionCode, uint8_t nodeId) {
    return ((uint32_t)functionCode << 7) | nodeId;
}

/**
 * Check if node ID is valid
 */
bool WB_CANopen_IsValidNodeId(uint8_t nodeId) {
    return (nodeId > 0 && nodeId <= 127);
}

/**
 * Reset communication
 */
void WB_CANopen_ResetCommunication(void) {
    // Reset all communication parameters
    sync_counter = 0;
    heartbeat_counter = 0;
    error_register = 0x00;
    
    // Reinitialize Object Dictionary
    WB_CANopen_InitObjectDictionary();
    
    printf("CANopen communication reset\n");
}

/**
 * Send CAN message
 */
static bool WB_CANopen_SendMessage(uint32_t cobId, uint8_t* data, uint8_t length) {
    CAN_MSG_OBJ msg;
    
    msg.msgId = cobId;
    msg.field.formatType = CAN_FRAME_EXT; // Use extended frame
    msg.field.brs = CAN_NON_BRS_MODE;
    msg.field.dlc = length;
    msg.field.idType = CAN_FRAME_STD;    // Actually use standard frame for CANopen
    msg.field.frameType = CAN_FRAME_DATA;
    
    memcpy(msg.data, data, length);
    
    return CAN1_Transmit(CAN1_FIFO_CH2, &msg);
}

/**
 * Handle errors
 */
void WB_CANopen_HandleError(uint32_t errorCode) {
    // Set appropriate error bits
    if (errorCode & 0xFF000000) {
        error_register |= 0x01; // Generic error
    }
    
    // Send emergency message
    uint8_t manufData[5] = {0};
    WB_CANopen_SendEmergency((uint16_t)(errorCode & 0xFFFF), error_register, manufData);
    
    printf("CANopen Error: 0x%08lX\n", errorCode);
}

/**
 * Get error register
 */
uint8_t WB_CANopen_GetErrorRegister(void) {
    return error_register;
}

/**
 * Set error register bit
 */
void WB_CANopen_SetErrorRegister(uint8_t errorBit) {
    error_register |= errorBit;
}

/**
 * Clear error register bit
 */
void WB_CANopen_ClearErrorRegister(uint8_t errorBit) {
    error_register &= ~errorBit;
}

/**
 * Periodic tasks - 1ms
 */
void WB_CANopen_Task_1ms(void) {
    // High frequency tasks - none for now
}

/**
 * Periodic tasks - 10ms
 */
void WB_CANopen_Task_10ms(void) {
    // Update data from MELKENS subsystems
    WB_CANopen_UpdateFromMelkens();
    
    // Send PDO if in operational state
    if (canopen_node.state == CANOPEN_STATE_OPERATIONAL) {
        static uint8_t pdo_counter = 0;
        if (++pdo_counter >= 10) { // Every 100ms
            WB_CANopen_SendPDO(1);
            pdo_counter = 0;
        }
    }
}

/**
 * Periodic tasks - 100ms
 */
void WB_CANopen_Task_100ms(void) {
    // Send heartbeat
    if (canopen_node.heartbeatEnabled) {
        static uint16_t heartbeat_timer = 0;
        if (++heartbeat_timer >= (canopen_node.heartbeatTime / 100)) {
            WB_CANopen_SendHeartbeat();
            heartbeat_timer = 0;
        }
    }
}