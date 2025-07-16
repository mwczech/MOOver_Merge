/*
 * WB_CanOpen.h
 * 
 * Wasserbauer-compatible CANopen protocol implementation for MELKENS
 * 
 * Author: MELKENS Integration Team
 * Created: 2024
 */

#ifndef WB_CANOPEN_H
#define WB_CANOPEN_H

#include <stdint.h>
#include <stdbool.h>
#include "mcc_generated_files/can1.h"
#include "mcc_generated_files/can_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// CANopen Function Codes
#define CANOPEN_FC_NMT          0x000
#define CANOPEN_FC_SYNC         0x080
#define CANOPEN_FC_EMERGENCY    0x080
#define CANOPEN_FC_PDO1_TX      0x180
#define CANOPEN_FC_PDO1_RX      0x200
#define CANOPEN_FC_PDO2_TX      0x280
#define CANOPEN_FC_PDO2_RX      0x300
#define CANOPEN_FC_PDO3_TX      0x380
#define CANOPEN_FC_PDO3_RX      0x400
#define CANOPEN_FC_PDO4_TX      0x480
#define CANOPEN_FC_PDO4_RX      0x500
#define CANOPEN_FC_SDO_TX       0x580
#define CANOPEN_FC_SDO_RX       0x600
#define CANOPEN_FC_HEARTBEAT    0x700

// WB Node IDs (from DDMap analysis)
#define WB_NODE_BUTLER_ENGINE   0x40
#define WB_NODE_SERVO_LEFT      0x7E
#define WB_NODE_SERVO_RIGHT     0x7F
#define WB_NODE_SERVO_THUMBLE   0x7D
#define WB_NODE_MAGNET_LINEAR   0x10
#define WB_NODE_STEERING_WHEEL  0x20

// CANopen Object Dictionary Indices (from WB .xdd files)
#define OD_DEVICE_TYPE          0x1000
#define OD_ERROR_REGISTER       0x1001
#define OD_MANUFACTURER_STATUS  0x1002
#define OD_ERROR_FIELD          0x1003
#define OD_SYNC_COB_ID          0x1005
#define OD_COMM_CYCLE_PERIOD    0x1006
#define OD_SYNC_WINDOW_LENGTH   0x1007
#define OD_SW_VERSION           0x100A
#define OD_NODE_ID              0x100B

// WB Application Objects (from ButlerEngine.xdd)
#define OD_PDO_VARIABLE_MANUAL  0x2010
#define OD_BUTLER_CTRL          0x4000

// Servo Control Objects (from servo.xdd)
#define OD_FEED_CONSTANT        0x6092
#define OD_PROFILE_ACCELERATION 0x6083
#define OD_PROFILE_DECELERATION 0x6084
#define OD_MAX_PROFILE_VELOCITY 0x607F
#define OD_CURRENT_CONTROL_PARAMS 0x6100
#define OD_VELOCITY_CONTROL_PARAMS 0x6101
#define OD_CURRENT_LIMIT        0x6073

// Data Types
#define DT_BOOLEAN              0x01
#define DT_INTEGER8             0x02
#define DT_INTEGER16            0x03
#define DT_INTEGER32            0x04
#define DT_UNSIGNED8            0x05
#define DT_UNSIGNED16           0x06
#define DT_UNSIGNED32           0x07
#define DT_REAL32               0x08
#define DT_VISIBLE_STRING       0x09

// Access Rights
#define ACCESS_RO               0x01
#define ACCESS_WO               0x02
#define ACCESS_RW               0x03

// SDO Command Specifiers
#define SDO_CMD_DOWNLOAD_INITIATE    0x20
#define SDO_CMD_DOWNLOAD_SEGMENT     0x00
#define SDO_CMD_UPLOAD_INITIATE      0x40
#define SDO_CMD_UPLOAD_SEGMENT       0x60
#define SDO_CMD_ABORT_TRANSFER       0x80

// SDO Response Codes
#define SDO_RESP_DOWNLOAD_INITIATE   0x60
#define SDO_RESP_DOWNLOAD_SEGMENT    0x20
#define SDO_RESP_UPLOAD_INITIATE     0x40
#define SDO_RESP_UPLOAD_SEGMENT      0x00

// Error Codes
#define SDO_ERROR_TOGGLE_BIT         0x05030000
#define SDO_ERROR_SDO_TIMEOUT        0x05040000
#define SDO_ERROR_INVALID_COMMAND    0x05040001
#define SDO_ERROR_INVALID_BLOCK_SIZE 0x05040002
#define SDO_ERROR_INVALID_SEQUENCE   0x05040003
#define SDO_ERROR_CRC_ERROR          0x05040004
#define SDO_ERROR_OUT_OF_MEMORY      0x05040005
#define SDO_ERROR_UNSUPPORTED_ACCESS 0x06010000
#define SDO_ERROR_WRITE_ONLY         0x06010001
#define SDO_ERROR_READ_ONLY          0x06010002
#define SDO_ERROR_OBJECT_NOT_EXIST   0x06020000
#define SDO_ERROR_CANNOT_MAP_PDO     0x06040041
#define SDO_ERROR_PDO_LENGTH_EXCEED  0x06040042
#define SDO_ERROR_GENERAL_PARAM      0x06040043
#define SDO_ERROR_GENERAL_INCOMPAT   0x06040047
#define SDO_ERROR_HARDWARE_FAULT     0x06060000
#define SDO_ERROR_DATA_TYPE_LENGTH   0x06070010
#define SDO_ERROR_DATA_TYPE_HIGH     0x06070012
#define SDO_ERROR_DATA_TYPE_LOW      0x06070013
#define SDO_ERROR_SUBINDEX_NOT_EXIST 0x06090011
#define SDO_ERROR_VALUE_RANGE        0x06090030
#define SDO_ERROR_VALUE_HIGH         0x06090031
#define SDO_ERROR_VALUE_LOW          0x06090032
#define SDO_ERROR_MAX_LESS_MIN       0x06090036
#define SDO_ERROR_GENERAL_ERROR      0x08000000
#define SDO_ERROR_DATA_STORE         0x08000020
#define SDO_ERROR_DATA_STORE_LOCAL   0x08000021
#define SDO_ERROR_DATA_STORE_STATE   0x08000022
#define SDO_ERROR_OBJECT_DICT        0x08000023

// CANopen State Machine
typedef enum {
    CANOPEN_STATE_INITIALIZATION = 0x00,
    CANOPEN_STATE_PRE_OPERATIONAL = 0x7F,
    CANOPEN_STATE_OPERATIONAL = 0x05,
    CANOPEN_STATE_STOPPED = 0x04
} CANopen_State_t;

// NMT Command Specifiers
typedef enum {
    NMT_START_REMOTE_NODE = 0x01,
    NMT_STOP_REMOTE_NODE = 0x02,
    NMT_ENTER_PRE_OPERATIONAL = 0x80,
    NMT_RESET_NODE = 0x81,
    NMT_RESET_COMMUNICATION = 0x82
} NMT_Command_t;

// PDO Manual Variables (Object 2010)
#pragma pack(push, 1)
typedef struct {
    int8_t speed;                   // 0x01 - Robot speed
    int8_t steering;                // 0x02 - Steering angle
    int8_t steeringRx;             // 0x03 - Received steering
    float cruiseCoordX;            // 0x04 - Cruise X coordinate
    float cruiseCoordY;            // 0x05 - Cruise Y coordinate  
    float cruiseYawDeg;            // 0x06 - Cruise yaw in degrees
    float cruiseYawSlipOdoRadFilt; // 0x07 - Filtered yaw slip
    float cruise_omega_realDeg;    // 0x08 - Real omega in degrees
    float l3dg20Rate;              // 0x09 - Gyro rate
    float l3dg20Angle;             // 0x0A - Gyro angle
    int8_t trommelSpeed;           // 0x0B - Drum speed
    uint8_t steeringRxStall;       // 0x0C - Steering stall status
    uint8_t monitorState;          // 0x0D - Monitor state
    uint8_t batteryLevel;          // 0x0E - Battery level
    uint16_t batteryVoltage;       // 0x0F - Battery voltage
    int16_t iShunt;                // 0x10 - Shunt current
    int16_t BLX1Speed;             // 0x11 - Motor 1 speed
    int16_t BLX2Speed;             // 0x12 - Motor 2 speed
    int16_t BLX3Speed;             // 0x13 - Motor 3 speed
    uint16_t BLX1Current;          // 0x14 - Motor 1 current
    uint16_t BLX2Current;          // 0x15 - Motor 2 current
    uint16_t BLX3Current;          // 0x16 - Motor 3 current
    int8_t towerSpeed;             // 0x17 - Tower speed
    float SetCoordX;               // 0x18 - Set X coordinate
    float SetCoordY;               // 0x19 - Set Y coordinate
    float SetYawDeg;               // 0x1A - Set yaw degrees
    uint8_t brake;                 // 0x1B - Brake status
    uint8_t stopFlags;             // 0x1C - Stop flags
    float driven;                  // 0x1D - Distance driven
    uint32_t ConFeedImpulses;      // 0x1E - Feed impulses
} WB_PDO_Manual_t;

// Butler Control (Object 4000)
typedef struct {
    uint16_t driveRequest;         // 0x01 - Drive request
    uint16_t butlerState;          // 0x02 - Butler state
    uint8_t abortRequest;          // 0x03 - Abort request
    uint8_t manualRequest;         // 0x04 - Manual request
    uint8_t pauseRequest;          // 0x05 - Pause request
    uint8_t parkRequest;           // 0x06 - Park request
    uint16_t teachTrackRequest;    // 0x07 - Teach track request
    uint8_t conFeedRequest;        // 0x08 - Continuous feed request
    uint8_t teachMagnetRequest;    // 0x09 - Teach magnet request
    uint8_t tmExistRequest;        // 0x0A - TM exist request
    float driveLength;             // 0x0B - Drive length
    uint8_t calibRequest;          // 0x0C - Calibration request
    uint8_t fillRequest;           // 0x0D - Fill request
    uint8_t stopStateRequest;      // 0x0E - Stop state request
    uint8_t calibProgress;         // 0x0F - Calibration progress
} WB_Butler_Control_t;

// Servo Profile Parameters
typedef struct {
    uint32_t feedConstant;         // Feed constant
    uint32_t profileAcceleration;  // Acceleration
    uint32_t profileDeceleration;  // Deceleration
    uint32_t maxProfileVelocity;   // Max velocity
    uint16_t currentLimit;         // Current limit
} WB_Servo_Profile_t;

// Current Control Parameters
typedef struct {
    uint32_t currentGainP;         // Current P gain
    uint32_t currentGainI;         // Current I gain
    uint32_t currentKw;            // Current Kw
    uint32_t currentKu;            // Current Ku
} WB_Current_Control_t;

// Velocity Control Parameters
typedef struct {
    uint32_t velocityGainP;        // Velocity P gain
    uint32_t velocityGainI;        // Velocity I gain
} WB_Velocity_Control_t;
#pragma pack(pop)

// SDO Message Structure
typedef struct {
    uint8_t command;               // Command specifier
    uint16_t index;                // Object index
    uint8_t subindex;              // Subindex
    uint32_t data;                 // Data (4 bytes max for expedited transfer)
} SDO_Message_t;

// Object Dictionary Entry
typedef struct {
    uint16_t index;                // Object index
    uint8_t subindex;              // Subindex
    uint8_t dataType;              // Data type
    uint8_t access;                // Access rights
    void* data;                    // Pointer to data
    uint16_t dataSize;             // Data size in bytes
} OD_Entry_t;

// CANopen Node Structure
typedef struct {
    uint8_t nodeId;                // Node ID
    CANopen_State_t state;         // Node state
    uint32_t heartbeatTime;        // Heartbeat timing
    bool heartbeatEnabled;         // Heartbeat enable flag
    WB_PDO_Manual_t pdoManual;     // PDO manual data
    WB_Butler_Control_t butlerCtrl; // Butler control data
    WB_Servo_Profile_t servoProfile; // Servo profile
    WB_Current_Control_t currentCtrl; // Current control
    WB_Velocity_Control_t velocityCtrl; // Velocity control
} CANopen_Node_t;

// Function Prototypes

// Initialization
void WB_CANopen_Init(uint8_t nodeId);
void WB_CANopen_SetState(CANopen_State_t newState);
CANopen_State_t WB_CANopen_GetState(void);

// Communication
void WB_CANopen_ProcessMessage(CAN_MSG_OBJ* msg);
bool WB_CANopen_SendSDO(uint8_t targetNode, uint16_t index, uint8_t subindex, uint32_t data);
bool WB_CANopen_SendPDO(uint8_t pdoNumber);
void WB_CANopen_SendHeartbeat(void);
void WB_CANopen_SendEmergency(uint16_t errorCode, uint8_t errorRegister, uint8_t* manufData);

// Object Dictionary
bool WB_CANopen_ReadOD(uint16_t index, uint8_t subindex, void* data, uint16_t* dataSize);
bool WB_CANopen_WriteOD(uint16_t index, uint8_t subindex, void* data, uint16_t dataSize);
void WB_CANopen_InitObjectDictionary(void);

// WB Compatibility Layer
void WB_CANopen_MapToMelkens(void);
void WB_CANopen_UpdateFromMelkens(void);
void WB_CANopen_ProcessButlerCommand(WB_Butler_Control_t* cmd);
void WB_CANopen_UpdateServoParameters(uint8_t servoNode, WB_Servo_Profile_t* profile);

// Periodic Tasks
void WB_CANopen_Task_1ms(void);   // High frequency tasks
void WB_CANopen_Task_10ms(void);  // Medium frequency tasks
void WB_CANopen_Task_100ms(void); // Low frequency tasks

// Utilities
uint32_t WB_CANopen_GetCOBID(uint8_t functionCode, uint8_t nodeId);
bool WB_CANopen_IsValidNodeId(uint8_t nodeId);
void WB_CANopen_ResetCommunication(void);

// Error Handling
void WB_CANopen_HandleError(uint32_t errorCode);
uint8_t WB_CANopen_GetErrorRegister(void);
void WB_CANopen_SetErrorRegister(uint8_t errorBit);
void WB_CANopen_ClearErrorRegister(uint8_t errorBit);

#ifdef __cplusplus
}
#endif

#endif /* WB_CANOPEN_H */