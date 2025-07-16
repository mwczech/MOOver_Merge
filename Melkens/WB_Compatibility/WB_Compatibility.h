/*
 * WB_Compatibility.h
 * 
 * Universal WB Compatibility Layer for MELKENS System
 * 
 * This layer provides a complete compatibility bridge between MELKENS 
 * robot control system and Wasserbauer (WB) navigation/control protocols.
 * 
 * Architecture:
 * - Protocol Layer: CANopen, SDO/PDO message handling
 * - Database Layer: SQLite interface for WB configuration 
 * - Translation Layer: MELKENS ↔ WB data conversion
 * - Error Handling: Unified error reporting and recovery
 * 
 * Author: MOOver Integration Team
 * Created: 2024-12-19
 * Phase: 2 - Compatibility Layer
 */

#ifndef WB_COMPATIBILITY_H
#define WB_COMPATIBILITY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// VERSION AND BUILD INFORMATION
// ============================================================================

#define WB_COMPATIBILITY_VERSION_MAJOR    1
#define WB_COMPATIBILITY_VERSION_MINOR    0
#define WB_COMPATIBILITY_VERSION_PATCH    0
#define WB_COMPATIBILITY_BUILD_DATE       __DATE__

// ============================================================================
// CORE SYSTEM CONFIGURATION
// ============================================================================

/**
 * WB Compatibility System Configuration
 */
typedef struct {
    bool enabled;                    // Master enable/disable flag
    uint8_t melkens_node_id;        // MELKENS node ID on CAN bus
    uint32_t can_baud_rate;         // CAN bus baud rate (500000 or 1000000)
    uint16_t heartbeat_interval_ms; // Heartbeat transmission interval
    uint16_t timeout_ms;            // Communication timeout
    bool debug_enabled;             // Debug logging enable
    bool database_enabled;          // Database interface enable
    char database_path[256];        // Path to WB database file
} WB_Compatibility_Config_t;

/**
 * System operational states
 */
typedef enum {
    WB_COMPAT_STATE_UNINITIALIZED = 0,
    WB_COMPAT_STATE_INITIALIZING,
    WB_COMPAT_STATE_READY,
    WB_COMPAT_STATE_OPERATIONAL,
    WB_COMPAT_STATE_ERROR,
    WB_COMPAT_STATE_MAINTENANCE
} WB_Compatibility_State_t;

/**
 * Error codes for WB compatibility layer
 */
typedef enum {
    WB_COMPAT_ERROR_NONE = 0,
    WB_COMPAT_ERROR_INIT_FAILED,
    WB_COMPAT_ERROR_CAN_BUS_FAILURE,
    WB_COMPAT_ERROR_DATABASE_ERROR,
    WB_COMPAT_ERROR_PROTOCOL_VIOLATION,
    WB_COMPAT_ERROR_TIMEOUT,
    WB_COMPAT_ERROR_INVALID_PARAMETER,
    WB_COMPAT_ERROR_RESOURCE_EXHAUSTED,
    WB_COMPAT_ERROR_HARDWARE_FAULT
} WB_Compatibility_Error_t;

// ============================================================================
// PROTOCOL LAYER INTERFACE
// ============================================================================

/**
 * WB Butler Engine Command Structure
 * Reverse engineered from WB binary analysis and .xdd files
 */
typedef struct {
    // Core Control Commands
    uint16_t command_id;            // Command identifier
    uint16_t drive_request;         // Drive operation request
    uint16_t manual_request;        // Manual control enable
    int16_t manual_speed;           // Manual speed (-100 to +100)
    int16_t manual_steering;        // Manual steering (-100 to +100)
    
    // Navigation Commands  
    uint32_t target_track_id;       // Target track ID for navigation
    uint32_t target_bay_id;         // Target bay ID for feeding
    float target_x, target_y;       // World coordinates
    float target_heading;           // Target heading in degrees
    
    // Operational Parameters
    uint16_t feed_amount;           // Feed amount in kg * 100
    uint16_t drive_speed;           // Drive speed setting
    uint8_t abort_request;          // Emergency abort flag
    uint8_t reserved[3];            // Reserved for future use
    
    // Timestamps and Validation
    uint32_t timestamp;             // Command timestamp
    uint16_t checksum;              // Command checksum
} WB_Butler_Command_t;

/**
 * WB Status Response Structure
 * Data sent back to WB Butler Engine
 */
typedef struct {
    // System Status
    uint16_t status_word;           // Overall system status
    uint8_t error_register;         // CANopen error register
    uint8_t operational_state;      // Current operational state
    
    // Position and Navigation
    float current_x, current_y;     // Current world position
    float current_heading;          // Current heading in degrees
    uint32_t current_track_id;      // Current track ID
    uint32_t current_bay_id;        // Current bay ID
    
    // Motor Status
    int16_t motor_left_speed;       // Left motor actual speed
    int16_t motor_right_speed;      // Right motor actual speed  
    int16_t motor_thumble_speed;    // Thumble motor actual speed
    uint16_t motor_status_flags;    // Motor status flags
    
    // Sensor Data
    float magnetic_field_strength;  // Magnetic field strength
    int8_t magnetic_position;       // Magnetic position (-15 to +15)
    uint8_t battery_level;          // Battery level (0-100%)
    uint8_t sensor_status;          // Sensor status flags
    
    // Timestamps
    uint32_t timestamp;             // Status timestamp
    uint16_t sequence_number;       // Sequence number for ordering
} WB_Status_Response_t;

// ============================================================================
// DATABASE LAYER INTERFACE
// ============================================================================

/**
 * WB Database Track Record
 * Reverse engineered from Butler.db analysis
 */
typedef struct {
    uint32_t track_id;              // Unique track identifier
    char track_name[64];            // Human readable track name
    float pos_x, pos_y;             // Track position coordinates
    uint16_t direction;             // Track direction (0-359 degrees)
    uint16_t trommel_speed;         // Trommel/drum speed setting
    uint16_t butler_speed;          // Butler drive speed setting
    uint8_t power;                  // Track power level
    uint8_t active;                 // Track active flag
} WB_Track_Record_t;

/**
 * WB Database Bay Record
 * Feeding location configuration
 */
typedef struct {
    uint32_t bay_id;                // Unique bay identifier
    char bay_name[64];              // Human readable bay name
    float entry_near_x, entry_near_y; // Bay entry point (near)
    float entry_far_x, entry_far_y;   // Bay entry point (far)
    float exit_near_x, exit_near_y;   // Bay exit point (near)
    float exit_far_x, exit_far_y;     // Bay exit point (far)
    float feed_pos_x, feed_pos_y;     // Feeding position
    float offset_far, offset_near;    // Position offsets
    uint16_t far_near_duration;       // Time for far-near movement
    uint8_t active;                   // Bay active flag
} WB_Bay_Record_t;

/**
 * WB Database Configuration Record
 * System-wide configuration parameters
 */
typedef struct {
    uint32_t config_id;             // Configuration parameter ID
    char config_name[64];           // Parameter name
    char config_value[256];         // Parameter value (string format)
    char config_type[32];           // Data type (int, float, string, bool)
    char description[128];          // Human readable description
} WB_Config_Record_t;

// ============================================================================
// TRANSLATION LAYER INTERFACE
// ============================================================================

/**
 * MELKENS to WB Data Translation Context
 * Manages conversion between MELKENS and WB data formats
 */
typedef struct {
    // Position Translation
    float melkens_to_wb_scale_x;    // X coordinate scaling factor
    float melkens_to_wb_scale_y;    // Y coordinate scaling factor
    float melkens_to_wb_offset_x;   // X coordinate offset
    float melkens_to_wb_offset_y;   // Y coordinate offset
    float melkens_to_wb_rotation;   // Coordinate system rotation
    
    // Speed Translation
    float melkens_to_wb_speed_scale; // Speed scaling factor
    int16_t melkens_speed_max;       // MELKENS maximum speed
    int16_t wb_speed_max;            // WB maximum speed
    
    // Magnetic Position Translation
    float magnetic_scale_factor;     // Magnetic position scaling
    int8_t magnetic_offset;          // Magnetic position offset
    
    // Error Code Translation
    uint8_t error_translation_table[256]; // Error code mapping table
} WB_Translation_Context_t;

// ============================================================================
// PUBLIC API FUNCTIONS
// ============================================================================

/**
 * @brief Initialize WB Compatibility Layer
 * 
 * Initializes all subsystems: protocol layer, database interface, 
 * translation layer, and error handling.
 * 
 * @param config Pointer to configuration structure
 * @return WB_Compatibility_Error_t Error code (WB_COMPAT_ERROR_NONE on success)
 */
WB_Compatibility_Error_t WB_Compatibility_Init(const WB_Compatibility_Config_t* config);

/**
 * @brief Update WB Compatibility Layer
 * 
 * Main update function to be called from MELKENS main loop.
 * Processes incoming WB commands, updates translations, and sends responses.
 * 
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Compatibility_Update(void);

/**
 * @brief Shutdown WB Compatibility Layer
 * 
 * Cleanly shuts down all subsystems and releases resources.
 * 
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Compatibility_Shutdown(void);

/**
 * @brief Get current system state
 * 
 * @return WB_Compatibility_State_t Current operational state
 */
WB_Compatibility_State_t WB_Compatibility_GetState(void);

/**
 * @brief Get last error code
 * 
 * @return WB_Compatibility_Error_t Last error that occurred
 */
WB_Compatibility_Error_t WB_Compatibility_GetLastError(void);

/**
 * @brief Get system statistics
 * 
 * @param stats Pointer to statistics structure to fill
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Compatibility_GetStatistics(void* stats);

// ============================================================================
// PROTOCOL LAYER FUNCTIONS
// ============================================================================

/**
 * @brief Process incoming WB Butler command
 * 
 * Processes a command received from WB Butler Engine and converts
 * it to appropriate MELKENS control actions.
 * 
 * @param command Pointer to WB Butler command structure
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Protocol_ProcessButlerCommand(const WB_Butler_Command_t* command);

/**
 * @brief Send status response to WB Butler
 * 
 * Sends current robot status back to WB Butler Engine.
 * 
 * @param response Pointer to status response structure
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Protocol_SendStatusResponse(const WB_Status_Response_t* response);

/**
 * @brief Update status from MELKENS system
 * 
 * Collects current status from MELKENS subsystems and prepares
 * WB status response.
 * 
 * @param response Pointer to status response structure to fill
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Protocol_UpdateStatusFromMelkens(WB_Status_Response_t* response);

// ============================================================================
// DATABASE LAYER FUNCTIONS  
// ============================================================================

/**
 * @brief Initialize database interface
 * 
 * Opens WB database file and initializes database access.
 * 
 * @param database_path Path to WB Butler.db file
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Database_Init(const char* database_path);

/**
 * @brief Load track configuration from database
 * 
 * @param track_id Track ID to load
 * @param track Pointer to track record structure to fill
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Database_LoadTrack(uint32_t track_id, WB_Track_Record_t* track);

/**
 * @brief Load bay configuration from database
 * 
 * @param bay_id Bay ID to load
 * @param bay Pointer to bay record structure to fill
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Database_LoadBay(uint32_t bay_id, WB_Bay_Record_t* bay);

/**
 * @brief Load configuration parameter from database
 * 
 * @param config_name Configuration parameter name
 * @param config Pointer to config record structure to fill
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Database_LoadConfig(const char* config_name, WB_Config_Record_t* config);

// ============================================================================
// TRANSLATION LAYER FUNCTIONS
// ============================================================================

/**
 * @brief Initialize translation layer
 * 
 * @param context Pointer to translation context
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Translation_Init(WB_Translation_Context_t* context);

/**
 * @brief Convert MELKENS position to WB coordinates
 * 
 * @param melkens_x MELKENS X coordinate
 * @param melkens_y MELKENS Y coordinate
 * @param wb_x Pointer to WB X coordinate output
 * @param wb_y Pointer to WB Y coordinate output
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Translation_MelkensToWB_Position(float melkens_x, float melkens_y, 
                                                            float* wb_x, float* wb_y);

/**
 * @brief Convert WB position to MELKENS coordinates
 * 
 * @param wb_x WB X coordinate
 * @param wb_y WB Y coordinate
 * @param melkens_x Pointer to MELKENS X coordinate output
 * @param melkens_y Pointer to MELKENS Y coordinate output
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Translation_WBToMelkens_Position(float wb_x, float wb_y,
                                                            float* melkens_x, float* melkens_y);

/**
 * @brief Convert MELKENS motor speed to WB servo velocity
 * 
 * @param melkens_speed MELKENS motor speed (-1000 to +1000)
 * @param wb_velocity Pointer to WB servo velocity output
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Translation_MelkensToWB_Speed(int16_t melkens_speed, uint16_t* wb_velocity);

/**
 * @brief Convert MELKENS magnet detection to WB magnetic position
 * 
 * @param magnet_number MELKENS magnet number (1-31, 16=center)
 * @param wb_position Pointer to WB magnetic position output (-15.0 to +15.0)
 * @return WB_Compatibility_Error_t Error code
 */
WB_Compatibility_Error_t WB_Translation_MelkensToWB_MagnetPosition(uint8_t magnet_number, float* wb_position);

// ============================================================================
// DEBUGGING AND DIAGNOSTICS
// ============================================================================

/**
 * @brief Enable/disable debug logging
 * 
 * @param enabled Debug logging enable flag
 */
void WB_Compatibility_SetDebugEnabled(bool enabled);

/**
 * @brief Print system status to console
 */
void WB_Compatibility_PrintStatus(void);

/**
 * @brief Print detailed diagnostics
 */
void WB_Compatibility_PrintDiagnostics(void);

/**
 * @brief Get version string
 * 
 * @return const char* Version string
 */
const char* WB_Compatibility_GetVersionString(void);

// ============================================================================
// PLACEHOLDER FUNCTIONS (TODO: Implementation required)
// ============================================================================

/**
 * TODO: Implement advanced navigation features
 * These functions are placeholders for future WB features that require
 * deeper reverse engineering of Butler Engine behavior.
 */

// TODO: Implement WB route planning algorithm
WB_Compatibility_Error_t WB_Navigation_PlanRoute(uint32_t start_track, uint32_t end_track);

// TODO: Implement WB feeding sequence automation  
WB_Compatibility_Error_t WB_Feeding_ExecuteSequence(uint32_t bay_id, uint16_t amount);

// TODO: Implement WB automatic calibration
WB_Compatibility_Error_t WB_Calibration_AutoCalibrate(void);

// TODO: Implement WB error recovery procedures
WB_Compatibility_Error_t WB_ErrorRecovery_ExecuteRecovery(uint16_t error_code);

// TODO: Implement WB advanced diagnostics
WB_Compatibility_Error_t WB_Diagnostics_RunFullDiagnostic(void);

// TODO: Implement WB configuration management
WB_Compatibility_Error_t WB_Config_SaveToDatabase(void);
WB_Compatibility_Error_t WB_Config_LoadFromDatabase(void);

// TODO: Implement WB logging and data export
WB_Compatibility_Error_t WB_Logging_ExportToCSV(const char* filename);
WB_Compatibility_Error_t WB_Logging_ExportToJSON(const char* filename);

#ifdef __cplusplus
}
#endif

#endif /* WB_COMPATIBILITY_H */

/*
 * INTEGRATION NOTES:
 * 
 * 1. This header defines the complete interface for WB compatibility but
 *    many functions are still TODO placeholders requiring implementation.
 * 
 * 2. The protocol structures are reverse engineered from WB binary analysis
 *    and may need refinement as more details are discovered.
 * 
 * 3. Database schema is inferred from DDMap.cfg and Butler.db analysis.
 *    Actual database structure may differ.
 * 
 * 4. Translation layer assumes coordinate system differences between
 *    MELKENS and WB that may need calibration.
 * 
 * 5. Error handling uses CANopen standard error codes where possible
 *    but extends with custom codes for MELKENS-specific errors.
 * 
 * NEXT STEPS:
 * 
 * 1. Implement core protocol functions (WB_Protocol_*)
 * 2. Implement database interface (WB_Database_*)  
 * 3. Implement translation functions (WB_Translation_*)
 * 4. Create unit tests for all implemented functions
 * 5. Integrate with existing MELKENS system
 */