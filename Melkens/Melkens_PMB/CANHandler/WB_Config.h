/*
 * WB_Config.h
 * 
 * Configuration and mapping settings for WB compatibility layer
 * 
 * Author: MELKENS Integration Team
 * Created: 2024
 */

#ifndef WB_CONFIG_H
#define WB_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// WB System Configuration
#define WB_SYSTEM_ENABLED           true
#define WB_DEBUG_ENABLED            true
#define WB_HEARTBEAT_INTERVAL_MS    1000
#define WB_PDO_TRANSMISSION_RATE_MS 100

// Node ID Mappings (from DDMap.cfg analysis)
typedef enum {
    WB_NODE_BUTLER_MAIN     = 0x40,  // Main Butler Engine
    WB_NODE_SERVO_THUMBLE   = 0x7D,  // Thumble/Drum motor
    WB_NODE_SERVO_LEFT      = 0x7E,  // Left drive motor
    WB_NODE_SERVO_RIGHT     = 0x7F,  // Right drive motor
    WB_NODE_MAGNET_LINEAR   = 0x10,  // Magnetic linear encoder
    WB_NODE_STEERING_WHEEL  = 0x20,  // Steering wheel controller
    WB_NODE_CHARGE_CTRL     = 0x30,  // Charging controller
    WB_NODE_FLAP_SENSOR     = 0x31   // Flap sensor
} WB_NodeID_t;

// Motor Channel Mappings
typedef enum {
    WB_MOTOR_LEFT_DRIVE = 0,    // Maps to MELKENS Motor_Left
    WB_MOTOR_RIGHT_DRIVE = 1,   // Maps to MELKENS Motor_Right
    WB_MOTOR_THUMBLE = 2,       // Maps to MELKENS Motor_Thumble
    WB_MOTOR_LIFT = 3,          // Maps to MELKENS Motor_Lift (if available)
    WB_MOTOR_COUNT
} WB_MotorChannel_t;

// Speed and Control Ranges
#define WB_SPEED_MIN                -100    // Minimum speed value
#define WB_SPEED_MAX                100     // Maximum speed value
#define WB_STEERING_MIN             -100    // Full left steering
#define WB_STEERING_MAX             100     // Full right steering
#define WB_ACCELERATION_DEFAULT     40000   // Default acceleration
#define WB_CURRENT_LIMIT_DEFAULT    350     // Default current limit (mA)

// Butler State Definitions (from Object 4000)
typedef enum {
    WB_BUTLER_STATE_STOPPED     = 0x00,
    WB_BUTLER_STATE_RUNNING     = 0x01,
    WB_BUTLER_STATE_MANUAL      = 0x02,
    WB_BUTLER_STATE_PAUSED      = 0x03,
    WB_BUTLER_STATE_ERROR       = 0x04,
    WB_BUTLER_STATE_TEACHING    = 0x05,
    WB_BUTLER_STATE_PARKING     = 0x06
} WB_ButlerState_t;

// Drive Request Types
typedef enum {
    WB_DRIVE_REQ_STOP           = 0x00,
    WB_DRIVE_REQ_START          = 0x01,
    WB_DRIVE_REQ_MANUAL         = 0x02,
    WB_DRIVE_REQ_AUTO           = 0x03,
    WB_DRIVE_REQ_TEACH_TRACK    = 0x04,
    WB_DRIVE_REQ_CALIBRATE      = 0x05
} WB_DriveRequest_t;

// Conversion factors between WB and MELKENS units
#define WB_TO_MELKENS_SPEED_FACTOR      10      // WB: -100..100, MELKENS: -1000..1000
#define WB_TO_MELKENS_CURRENT_FACTOR    1       // Same units (mA)
#define WB_TO_MELKENS_VOLTAGE_FACTOR    1       // Same units (mV)
#define WB_TO_MELKENS_ANGLE_FACTOR      0.01f   // WB: degrees*100, MELKENS: degrees

// Error Code Mappings
typedef enum {
    WB_ERROR_NONE               = 0x0000,
    WB_ERROR_COMMUNICATION      = 0x8100,
    WB_ERROR_MOTOR_OVERCURRENT  = 0x2310,
    WB_ERROR_MOTOR_OVERHEAT     = 0x4210,
    WB_ERROR_BATTERY_LOW        = 0x5100,
    WB_ERROR_SENSOR_FAULT       = 0x6100,
    WB_ERROR_SYSTEM_FAULT       = 0xFF00
} WB_ErrorCode_t;

// Configuration Structure
typedef struct {
    bool enableWBCompatibility;         // Enable WB protocol
    bool enableHeartbeat;               // Enable heartbeat messages
    bool enablePDOTransmission;         // Enable PDO transmission
    bool enableServoControl;            // Enable servo control via CANopen
    bool enableDiagnostics;             // Enable diagnostic messages
    uint16_t heartbeatInterval;         // Heartbeat interval in ms
    uint16_t pdoTransmissionRate;       // PDO transmission rate in ms
    uint8_t nodeId;                     // This node's ID
    uint16_t speedScalingFactor;        // Speed scaling factor
    uint16_t currentLimitDefault;       // Default current limit
} WB_Config_t;

// Default configuration
extern const WB_Config_t WB_DefaultConfig;

// Function prototypes

/**
 * Initialize WB configuration with default values
 */
void WB_Config_Init(void);

/**
 * Load configuration from persistent storage
 */
bool WB_Config_Load(WB_Config_t* config);

/**
 * Save configuration to persistent storage
 */
bool WB_Config_Save(const WB_Config_t* config);

/**
 * Get current configuration
 */
const WB_Config_t* WB_Config_Get(void);

/**
 * Update configuration
 */
void WB_Config_Set(const WB_Config_t* config);

/**
 * Convert WB speed value to MELKENS speed value
 */
int16_t WB_Config_ConvertSpeedToMelkens(int8_t wbSpeed);

/**
 * Convert MELKENS speed value to WB speed value
 */
int8_t WB_Config_ConvertSpeedToWB(int16_t melkensSpeed);

/**
 * Convert WB steering value to differential motor speeds
 */
void WB_Config_ConvertSteering(int8_t steering, int8_t baseSpeed, 
                               int16_t* leftSpeed, int16_t* rightSpeed);

/**
 * Map WB motor node ID to MELKENS motor channel
 */
uint8_t WB_Config_MapMotorNode(uint8_t wbNodeId);

/**
 * Map MELKENS motor channel to WB motor node ID
 */
uint8_t WB_Config_MapMelkensMotor(uint8_t melkensMotor);

/**
 * Get default servo profile for node
 */
void WB_Config_GetDefaultServoProfile(uint8_t nodeId, uint32_t* feedConstant,
                                      uint32_t* acceleration, uint32_t* deceleration,
                                      uint32_t* maxVelocity, uint16_t* currentLimit);

/**
 * Validate WB configuration
 */
bool WB_Config_Validate(const WB_Config_t* config);

/**
 * Print current configuration (debug)
 */
void WB_Config_Print(void);

#ifdef __cplusplus
}
#endif

#endif /* WB_CONFIG_H */