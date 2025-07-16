/*
 * WB_Integration_Example.c
 * 
 * Example demonstrating how to integrate WB Compatibility Layer 
 * into MELKENS robot application.
 * 
 * This example shows:
 * - Basic initialization and configuration
 * - Command processing integration
 * - Status reporting integration  
 * - Error handling and diagnostics
 * - Main loop integration
 * 
 * Author: MOOver Integration Team
 * Created: 2024-12-19
 * Phase: 2 - Compatibility Layer
 */

#include "WB_Compatibility.h"
#include "../Melkens_PMB/pmb_System.h"
#include "../Melkens_PMB/pmb_MotorManager.h"
#include "../Melkens_PMB/TimeManager/TimeManager.h"

#include <stdio.h>
#include <stdbool.h>

// ============================================================================
// APPLICATION CONFIGURATION
// ============================================================================

/**
 * WB Compatibility configuration for this application
 */
static const WB_Compatibility_Config_t g_wb_config = {
    .enabled = true,
    .melkens_node_id = 0x01,
    .can_baud_rate = 500000,
    .heartbeat_interval_ms = 1000,
    .timeout_ms = 5000,
    .debug_enabled = true,
    .database_enabled = true,
    .database_path = "/data/butler.db"
};

// Application state
typedef enum {
    APP_STATE_INITIALIZING = 0,
    APP_STATE_READY,
    APP_STATE_RUNNING,
    APP_STATE_ERROR,
    APP_STATE_SHUTDOWN
} AppState_t;

static AppState_t g_app_state = APP_STATE_INITIALIZING;
static bool g_shutdown_requested = false;

// ============================================================================
// EXAMPLE 1: BASIC INITIALIZATION
// ============================================================================

/**
 * @brief Initialize WB compatibility layer in MELKENS application
 * 
 * This function demonstrates how to properly initialize the WB compatibility
 * layer during system startup.
 * 
 * @return true if initialization successful, false otherwise
 */
bool Example_InitializeWB(void) {
    printf("\n=== WB Compatibility Layer Initialization ===\n");
    
    // Initialize WB compatibility layer
    WB_Compatibility_Error_t error = WB_Compatibility_Init(&g_wb_config);
    if (error != WB_COMPAT_ERROR_NONE) {
        printf("ERROR: WB compatibility initialization failed: %d\n", error);
        return false;
    }
    
    // Print system information
    printf("WB Compatibility Version: %s\n", WB_Compatibility_GetVersionString());
    WB_Compatibility_PrintStatus();
    
    printf("WB compatibility layer initialized successfully!\n");
    return true;
}

// ============================================================================
// EXAMPLE 2: COMMAND PROCESSING INTEGRATION
// ============================================================================

/**
 * @brief Example of processing WB Butler commands
 * 
 * This function demonstrates how to receive and process commands from
 * WB Butler Engine within the MELKENS main loop.
 */
void Example_ProcessWBCommands(void) {
    // In a real application, this would receive commands from CAN bus
    // For demonstration, we'll create example commands
    
    static uint32_t example_counter = 0;
    static uint32_t last_command_time = 0;
    
    uint32_t current_time = System_GetTimeMs();
    
    // Send example command every 5 seconds
    if (current_time - last_command_time > 5000) {
        WB_Butler_Command_t example_command = {0};
        
        // Create different types of commands based on counter
        switch (example_counter % 4) {
            case 0: // Manual control example
                example_command.command_id = 0x1001;
                example_command.drive_request = 0x0001; // Manual control
                example_command.manual_request = 1;
                example_command.manual_speed = 50;
                example_command.manual_steering = 0;
                printf("Example: Processing manual control command (speed=50)\n");
                break;
                
            case 1: // Navigation example
                example_command.command_id = 0x1002;
                example_command.drive_request = 0x0002; // Auto navigation
                example_command.target_track_id = 5;
                example_command.target_bay_id = 10;
                example_command.target_x = 25.0f;
                example_command.target_y = 10.0f;
                printf("Example: Processing navigation command (track=5, bay=10)\n");
                break;
                
            case 2: // Bay approach example
                example_command.command_id = 0x1003;
                example_command.drive_request = 0x0004; // Bay approach
                example_command.target_bay_id = 3;
                example_command.feed_amount = 1500; // 15.00 kg
                printf("Example: Processing bay approach command (bay=3, amount=15kg)\n");
                break;
                
            case 3: // Emergency stop example
                example_command.command_id = 0x1004;
                example_command.drive_request = 0x0003; // Emergency stop
                example_command.abort_request = 1;
                printf("Example: Processing emergency stop command\n");
                break;
        }
        
        // Set timestamp and calculate checksum
        example_command.timestamp = current_time;
        example_command.checksum = 0; // Simplified for example
        
        // Process the command
        WB_Compatibility_Error_t error = WB_Protocol_ProcessButlerCommand(&example_command);
        if (error != WB_COMPAT_ERROR_NONE) {
            printf("ERROR: Command processing failed: %d\n", error);
        }
        
        last_command_time = current_time;
        example_counter++;
    }
}

// ============================================================================
// EXAMPLE 3: STATUS REPORTING INTEGRATION
// ============================================================================

/**
 * @brief Example of sending WB status responses
 * 
 * This function demonstrates how to collect MELKENS system status
 * and send it back to WB Butler Engine.
 */
void Example_SendWBStatusUpdates(void) {
    static uint32_t last_status_time = 0;
    uint32_t current_time = System_GetTimeMs();
    
    // Send status updates every 2 seconds
    if (current_time - last_status_time > 2000) {
        WB_Status_Response_t status_response;
        
        // Update status from MELKENS system
        WB_Compatibility_Error_t error = WB_Protocol_UpdateStatusFromMelkens(&status_response);
        if (error == WB_COMPAT_ERROR_NONE) {
            // Send status response to WB Butler
            error = WB_Protocol_SendStatusResponse(&status_response);
            if (error == WB_COMPAT_ERROR_NONE) {
                printf("Status update sent: State=%d, Motors=(%d,%d,%d), Battery=%d%%\n",
                       status_response.operational_state,
                       status_response.motor_left_speed,
                       status_response.motor_right_speed,
                       status_response.motor_thumble_speed,
                       status_response.battery_level);
            } else {
                printf("ERROR: Failed to send status response: %d\n", error);
            }
        } else {
            printf("ERROR: Failed to update status from MELKENS: %d\n", error);
        }
        
        last_status_time = current_time;
    }
}

// ============================================================================
// EXAMPLE 4: DATABASE INTEGRATION
// ============================================================================

/**
 * @brief Example of using WB database functionality
 * 
 * This function demonstrates how to load track and bay configurations
 * from the WB database.
 */
void Example_UseDatabaseFunctions(void) {
    static bool database_example_run = false;
    
    // Run this example only once
    if (database_example_run) return;
    database_example_run = true;
    
    printf("\n=== WB Database Integration Example ===\n");
    
    // Load track configuration
    WB_Track_Record_t track;
    WB_Compatibility_Error_t error = WB_Database_LoadTrack(5, &track);
    if (error == WB_COMPAT_ERROR_NONE) {
        printf("Loaded Track %lu: %s at (%.2f, %.2f), direction=%d°\n",
               track.track_id, track.track_name, track.pos_x, track.pos_y, track.direction);
        printf("  Speeds: Butler=%d, Trommel=%d, Power=%d%%, Active=%s\n",
               track.butler_speed, track.trommel_speed, track.power,
               track.active ? "Yes" : "No");
    } else {
        printf("ERROR: Failed to load track: %d\n", error);
    }
    
    // Load bay configuration
    WB_Bay_Record_t bay;
    error = WB_Database_LoadBay(10, &bay);
    if (error == WB_COMPAT_ERROR_NONE) {
        printf("Loaded Bay %lu: %s\n", bay.bay_id, bay.bay_name);
        printf("  Entry: Near(%.2f,%.2f), Far(%.2f,%.2f)\n",
               bay.entry_near_x, bay.entry_near_y, bay.entry_far_x, bay.entry_far_y);
        printf("  Feed Position: (%.2f, %.2f)\n", bay.feed_pos_x, bay.feed_pos_y);
        printf("  Duration: %dms, Active=%s\n", bay.far_near_duration,
               bay.active ? "Yes" : "No");
    } else {
        printf("ERROR: Failed to load bay: %d\n", error);
    }
    
    // Load configuration parameter
    WB_Config_Record_t config;
    error = WB_Database_LoadConfig("max_speed", &config);
    if (error == WB_COMPAT_ERROR_NONE) {
        printf("Loaded Config %s: %s (%s) - %s\n",
               config.config_name, config.config_value, config.config_type, config.description);
    } else {
        printf("ERROR: Failed to load config: %d\n", error);
    }
}

// ============================================================================
// EXAMPLE 5: TRANSLATION LAYER USAGE
// ============================================================================

/**
 * @brief Example of using coordinate and data translation functions
 * 
 * This function demonstrates how to convert between MELKENS and WB
 * coordinate systems and data formats.
 */
void Example_UseTranslationFunctions(void) {
    static bool translation_example_run = false;
    
    // Run this example only once
    if (translation_example_run) return;
    translation_example_run = true;
    
    printf("\n=== WB Translation Layer Example ===\n");
    
    // Example 1: Position translation
    float melkens_x = 10.5f, melkens_y = 3.2f;
    float wb_x, wb_y;
    
    WB_Compatibility_Error_t error = WB_Translation_MelkensToWB_Position(melkens_x, melkens_y, &wb_x, &wb_y);
    if (error == WB_COMPAT_ERROR_NONE) {
        printf("Position Translation: MELKENS(%.2f, %.2f) -> WB(%.2f, %.2f)\n",
               melkens_x, melkens_y, wb_x, wb_y);
        
        // Convert back to verify
        float melkens_x_back, melkens_y_back;
        error = WB_Translation_WBToMelkens_Position(wb_x, wb_y, &melkens_x_back, &melkens_y_back);
        if (error == WB_COMPAT_ERROR_NONE) {
            printf("Reverse Translation: WB(%.2f, %.2f) -> MELKENS(%.2f, %.2f)\n",
                   wb_x, wb_y, melkens_x_back, melkens_y_back);
        }
    }
    
    // Example 2: Speed translation
    int16_t melkens_speed = 750; // 75% of max speed
    uint16_t wb_velocity;
    
    error = WB_Translation_MelkensToWB_Speed(melkens_speed, &wb_velocity);
    if (error == WB_COMPAT_ERROR_NONE) {
        printf("Speed Translation: MELKENS(%d) -> WB(%u)\n", melkens_speed, wb_velocity);
    }
    
    // Example 3: Magnetic position translation
    uint8_t magnet_number = 20; // Magnet 20 (4 positions right of center)
    float wb_magnetic_position;
    
    error = WB_Translation_MelkensToWB_MagnetPosition(magnet_number, &wb_magnetic_position);
    if (error == WB_COMPAT_ERROR_NONE) {
        printf("Magnetic Translation: MELKENS(Magnet_%u) -> WB(%.2f cm)\n",
               magnet_number, wb_magnetic_position);
    }
}

// ============================================================================
// EXAMPLE 6: ERROR HANDLING AND DIAGNOSTICS
// ============================================================================

/**
 * @brief Example of error handling and diagnostic functions
 * 
 * This function demonstrates how to handle errors and use diagnostic
 * features of the WB compatibility layer.
 */
void Example_ErrorHandlingAndDiagnostics(void) {
    static uint32_t last_diagnostic_time = 0;
    uint32_t current_time = System_GetTimeMs();
    
    // Print diagnostics every 30 seconds
    if (current_time - last_diagnostic_time > 30000) {
        printf("\n=== WB Compatibility Diagnostics ===\n");
        
        // Print current system state
        WB_Compatibility_State_t state = WB_Compatibility_GetState();
        printf("Current WB State: %d\n", state);
        
        // Check for errors
        WB_Compatibility_Error_t last_error = WB_Compatibility_GetLastError();
        if (last_error != WB_COMPAT_ERROR_NONE) {
            printf("WARNING: Last error code: %d\n", last_error);
        }
        
        // Print detailed diagnostics
        WB_Compatibility_PrintDiagnostics();
        
        // Get statistics
        struct {
            uint32_t commands_processed;
            uint32_t responses_sent;
            uint32_t errors_encountered;
            uint32_t database_queries;
            uint32_t translations_performed;
            uint32_t uptime_seconds;
            uint32_t last_heartbeat_time;
        } stats;
        
        WB_Compatibility_Error_t error = WB_Compatibility_GetStatistics(&stats);
        if (error == WB_COMPAT_ERROR_NONE) {
            printf("Performance: %.2f commands/min, %.2f responses/min\n",
                   (float)stats.commands_processed / (stats.uptime_seconds / 60.0f),
                   (float)stats.responses_sent / (stats.uptime_seconds / 60.0f));
        }
        
        last_diagnostic_time = current_time;
    }
}

// ============================================================================
// EXAMPLE 7: MAIN APPLICATION LOOP INTEGRATION
// ============================================================================

/**
 * @brief Main application function demonstrating full WB integration
 * 
 * This function shows how to integrate all WB compatibility functions
 * into a complete MELKENS application main loop.
 */
int Example_MainApplication(void) {
    printf("=== WB-MELKENS Integration Example Application ===\n");
    printf("This example demonstrates complete WB compatibility integration.\n\n");
    
    // Initialize MELKENS system (simplified)
    printf("Initializing MELKENS system...\n");
    // System_Init(); // Assume this exists in real MELKENS
    // MotorManager_Init(); // Assume this exists in real MELKENS
    
    // Initialize WB compatibility layer
    if (!Example_InitializeWB()) {
        printf("FATAL: WB initialization failed\n");
        return -1;
    }
    
    g_app_state = APP_STATE_READY;
    printf("Application ready - entering main loop\n\n");
    
    // Main application loop
    uint32_t loop_counter = 0;
    while (!g_shutdown_requested && loop_counter < 1000) { // Limit for example
        // Update WB compatibility layer
        WB_Compatibility_Error_t error = WB_Compatibility_Update();
        if (error != WB_COMPAT_ERROR_NONE) {
            printf("WARNING: WB update failed: %d\n", error);
            if (error == WB_COMPAT_ERROR_CAN_BUS_FAILURE) {
                g_app_state = APP_STATE_ERROR;
                break;
            }
        }
        
        // Process WB commands
        Example_ProcessWBCommands();
        
        // Send status updates
        Example_SendWBStatusUpdates();
        
        // Run database example (once)
        Example_UseDatabaseFunctions();
        
        // Run translation example (once)
        Example_UseTranslationFunctions();
        
        // Handle diagnostics
        Example_ErrorHandlingAndDiagnostics();
        
        // Regular MELKENS system updates would go here
        // MotorManager_Update();
        // Navigation_Update();
        // BatteryManager_Update();
        
        // Small delay to prevent busy loop
        System_DelayMs(10);
        loop_counter++;
    }
    
    // Cleanup and shutdown
    printf("\nShutting down application...\n");
    g_app_state = APP_STATE_SHUTDOWN;
    
    WB_Compatibility_Error_t error = WB_Compatibility_Shutdown();
    if (error != WB_COMPAT_ERROR_NONE) {
        printf("WARNING: WB shutdown error: %d\n", error);
    }
    
    printf("Application shutdown complete.\n");
    return 0;
}

// ============================================================================
// EXAMPLE 8: SIGNAL HANDLER FOR GRACEFUL SHUTDOWN
// ============================================================================

/**
 * @brief Signal handler for graceful shutdown
 * 
 * This function can be connected to system signals to ensure
 * graceful shutdown of the WB compatibility layer.
 */
void Example_SignalHandler(int signal) {
    printf("\nReceived signal %d - requesting graceful shutdown\n", signal);
    g_shutdown_requested = true;
}

// ============================================================================
// EXAMPLE 9: CUSTOM ERROR RECOVERY
// ============================================================================

/**
 * @brief Example custom error recovery function
 * 
 * This function demonstrates how to implement custom error recovery
 * logic for specific WB compatibility layer errors.
 */
bool Example_HandleWBError(WB_Compatibility_Error_t error) {
    printf("Handling WB error: %d\n", error);
    
    switch (error) {
        case WB_COMPAT_ERROR_CAN_BUS_FAILURE:
            printf("CAN bus failure - attempting recovery...\n");
            // TODO: Implement CAN bus reset/recovery
            System_DelayMs(1000);
            return true;
            
        case WB_COMPAT_ERROR_DATABASE_ERROR:
            printf("Database error - switching to offline mode...\n");
            // TODO: Disable database functionality, use defaults
            return true;
            
        case WB_COMPAT_ERROR_TIMEOUT:
            printf("Communication timeout - retrying...\n");
            // TODO: Retry communication
            return true;
            
        case WB_COMPAT_ERROR_PROTOCOL_VIOLATION:
            printf("Protocol violation - resetting communication...\n");
            // TODO: Reset protocol state
            return true;
            
        default:
            printf("Unhandled error - cannot recover\n");
            return false;
    }
}

// ============================================================================
// EXAMPLE USAGE AND TESTING
// ============================================================================

#ifdef WB_INTEGRATION_EXAMPLE_STANDALONE

/**
 * @brief Standalone example main function
 * 
 * This main function can be used to run the integration example
 * as a standalone application for testing purposes.
 */
int main(void) {
    printf("=== WB-MELKENS Integration Example (Standalone) ===\n");
    
    // Run the main application example
    int result = Example_MainApplication();
    
    printf("\nExample completed with result: %d\n", result);
    return result;
}

#endif /* WB_INTEGRATION_EXAMPLE_STANDALONE */

/*
 * INTEGRATION NOTES:
 * 
 * 1. To use this in a real MELKENS application:
 *    - Include WB_Compatibility.h in your main application file
 *    - Call WB_Compatibility_Init() during system initialization
 *    - Call WB_Compatibility_Update() in your main loop
 *    - Process commands and send status as shown in examples
 *    - Handle errors appropriately for your application
 * 
 * 2. Compile with:
 *    gcc -DWB_INTEGRATION_EXAMPLE_STANDALONE -o wb_example \
 *        WB_Integration_Example.c WB_Compatibility.c \
 *        -lm -lpthread
 * 
 * 3. Customization points:
 *    - Modify g_wb_config for your specific requirements
 *    - Implement actual CAN communication in place of examples
 *    - Add your specific error handling logic
 *    - Integrate with your existing MELKENS modules
 * 
 * 4. Performance considerations:
 *    - WB_Compatibility_Update() should be called frequently (>10Hz)
 *    - Status updates can be sent at lower frequency (1-5Hz)
 *    - Database queries should be cached when possible
 *    - Translation functions are lightweight and can be called frequently
 */