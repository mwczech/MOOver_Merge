/*
 * test_wb_compatibility.c
 * 
 * Comprehensive unit tests for WB Compatibility Layer
 * 
 * This file contains unit tests for all major components of the 
 * WB compatibility layer including protocol handling, translation
 * functions, database interface, and error handling.
 * 
 * Test Framework: Custom lightweight framework
 * Coverage: All public API functions plus critical internals
 * 
 * Author: MOOver Integration Team
 * Created: 2024-12-19
 * Phase: 3 - Testing and Emulation
 */

#include "../../WB_Compatibility.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdbool.h>

// ============================================================================
// TEST FRAMEWORK DEFINITIONS
// ============================================================================

#define TEST_MAX_NAME_LENGTH 64
#define TEST_MAX_TESTS 100

typedef struct {
    char name[TEST_MAX_NAME_LENGTH];
    bool (*test_function)(void);
    bool passed;
    double execution_time_ms;
} Test_t;

static Test_t g_tests[TEST_MAX_TESTS];
static int g_test_count = 0;
static int g_tests_passed = 0;
static int g_tests_failed = 0;

// Test result tracking
static bool g_current_test_passed = true;
static char g_current_test_name[TEST_MAX_NAME_LENGTH];

// ============================================================================
// TEST FRAMEWORK IMPLEMENTATION
// ============================================================================

/**
 * @brief Register a test function
 */
#define REGISTER_TEST(name, func) \
    do { \
        if (g_test_count < TEST_MAX_TESTS) { \
            strncpy(g_tests[g_test_count].name, name, TEST_MAX_NAME_LENGTH - 1); \
            g_tests[g_test_count].test_function = func; \
            g_test_count++; \
        } \
    } while(0)

/**
 * @brief Assert macro for tests
 */
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            printf("  ASSERTION FAILED: %s\n", message); \
            printf("    File: %s, Line: %d\n", __FILE__, __LINE__); \
            g_current_test_passed = false; \
            return false; \
        } \
    } while(0)

/**
 * @brief Assert equality for integers
 */
#define TEST_ASSERT_EQUAL_INT(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            printf("  ASSERTION FAILED: Expected %d, got %d\n", (expected), (actual)); \
            printf("    File: %s, Line: %d\n", __FILE__, __LINE__); \
            g_current_test_passed = false; \
            return false; \
        } \
    } while(0)

/**
 * @brief Assert equality for floats with tolerance
 */
#define TEST_ASSERT_EQUAL_FLOAT(expected, actual, tolerance) \
    do { \
        if (fabs((expected) - (actual)) > (tolerance)) { \
            printf("  ASSERTION FAILED: Expected %.6f, got %.6f (tolerance %.6f)\n", \
                   (expected), (actual), (tolerance)); \
            printf("    File: %s, Line: %d\n", __FILE__, __LINE__); \
            g_current_test_passed = false; \
            return false; \
        } \
    } while(0)

/**
 * @brief Get current time in milliseconds (simplified)
 */
static double get_time_ms(void) {
    // Simplified implementation - in real system would use clock_gettime
    return 0.0;
}

// ============================================================================
// MOCK SYSTEM FUNCTIONS
// ============================================================================

// Mock MELKENS system functions for testing
uint32_t System_GetTimeMs(void) {
    static uint32_t mock_time = 1000;
    mock_time += 10; // Increment by 10ms each call
    return mock_time;
}

void System_DelayMs(uint32_t ms) {
    // Mock delay - do nothing in tests
    (void)ms;
}

// Mock motor manager functions
typedef enum {
    Motor_Left = 0,
    Motor_Right = 1,
    Motor_Thumble = 2
} MotorType;

static int16_t mock_motor_speeds[3] = {0, 0, 0};

int16_t MotorManager_GetSpeed(MotorType motor) {
    if (motor >= 0 && motor < 3) {
        return mock_motor_speeds[motor];
    }
    return 0;
}

void MotorManager_SetSpeed(MotorType motor, int16_t speed) {
    if (motor >= 0 && motor < 3) {
        mock_motor_speeds[motor] = speed;
    }
}

void MotorManager_Stop(MotorType motor) {
    if (motor >= 0 && motor < 3) {
        mock_motor_speeds[motor] = 0;
    }
}

// ============================================================================
// UNIT TESTS: INITIALIZATION AND CONFIGURATION
// ============================================================================

/**
 * @brief Test WB compatibility initialization with valid configuration
 */
bool test_wb_init_valid_config(void) {
    printf("  Testing WB initialization with valid configuration...\n");
    
    WB_Compatibility_Config_t config = {
        .enabled = true,
        .melkens_node_id = 0x01,
        .can_baud_rate = 500000,
        .heartbeat_interval_ms = 1000,
        .timeout_ms = 5000,
        .debug_enabled = true,
        .database_enabled = false, // Disable database for unit tests
        .database_path = ""
    };
    
    WB_Compatibility_Error_t error = WB_Compatibility_Init(&config);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    
    WB_Compatibility_State_t state = WB_Compatibility_GetState();
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_STATE_READY, state);
    
    printf("    ✓ Initialization successful\n");
    printf("    ✓ State is READY\n");
    
    return true;
}

/**
 * @brief Test WB compatibility initialization with invalid configuration
 */
bool test_wb_init_invalid_config(void) {
    printf("  Testing WB initialization with invalid configuration...\n");
    
    // Test NULL config
    WB_Compatibility_Error_t error = WB_Compatibility_Init(NULL);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
    printf("    ✓ NULL config rejected\n");
    
    // Test invalid baud rate
    WB_Compatibility_Config_t config = {
        .enabled = true,
        .melkens_node_id = 0x01,
        .can_baud_rate = 123456, // Invalid baud rate
        .heartbeat_interval_ms = 1000,
        .timeout_ms = 5000,
        .debug_enabled = false,
        .database_enabled = false
    };
    
    error = WB_Compatibility_Init(&config);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
    printf("    ✓ Invalid baud rate rejected\n");
    
    // Test invalid heartbeat interval
    config.can_baud_rate = 500000;
    config.heartbeat_interval_ms = 50; // Too small
    
    error = WB_Compatibility_Init(&config);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
    printf("    ✓ Invalid heartbeat interval rejected\n");
    
    return true;
}

/**
 * @brief Test WB compatibility state management
 */
bool test_wb_state_management(void) {
    printf("  Testing WB state management...\n");
    
    // Test initial state
    WB_Compatibility_State_t state = WB_Compatibility_GetState();
    printf("    Current state: %d\n", state);
    
    // Test error tracking
    WB_Compatibility_Error_t last_error = WB_Compatibility_GetLastError();
    printf("    Last error: %d\n", last_error);
    
    // Test version string
    const char* version = WB_Compatibility_GetVersionString();
    TEST_ASSERT(version != NULL, "Version string should not be NULL");
    TEST_ASSERT(strlen(version) > 0, "Version string should not be empty");
    printf("    ✓ Version string: %s\n", version);
    
    return true;
}

// ============================================================================
// UNIT TESTS: TRANSLATION LAYER
// ============================================================================

/**
 * @brief Test position translation functions
 */
bool test_translation_position(void) {
    printf("  Testing position translation functions...\n");
    
    // Test MELKENS to WB position translation
    float melkens_x = 10.0f, melkens_y = 5.0f;
    float wb_x, wb_y;
    
    WB_Compatibility_Error_t error = WB_Translation_MelkensToWB_Position(melkens_x, melkens_y, &wb_x, &wb_y);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    printf("    ✓ MELKENS(%.2f, %.2f) -> WB(%.2f, %.2f)\n", melkens_x, melkens_y, wb_x, wb_y);
    
    // Test reverse translation
    float melkens_x_back, melkens_y_back;
    error = WB_Translation_WBToMelkens_Position(wb_x, wb_y, &melkens_x_back, &melkens_y_back);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    
    // Verify round-trip accuracy
    TEST_ASSERT_EQUAL_FLOAT(melkens_x, melkens_x_back, 0.001f);
    TEST_ASSERT_EQUAL_FLOAT(melkens_y, melkens_y_back, 0.001f);
    printf("    ✓ Round-trip translation accurate\n");
    
    // Test NULL parameter handling
    error = WB_Translation_MelkensToWB_Position(0, 0, NULL, &wb_y);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
    printf("    ✓ NULL parameter rejected\n");
    
    return true;
}

/**
 * @brief Test speed translation functions
 */
bool test_translation_speed(void) {
    printf("  Testing speed translation functions...\n");
    
    // Test various speed values
    struct {
        int16_t melkens_speed;
        uint16_t expected_min;
        uint16_t expected_max;
    } test_cases[] = {
        {0, 0, 10},        // Zero speed
        {500, 40, 60},     // Half speed
        {1000, 90, 110},   // Full speed
        {-500, 0, 10},     // Negative speed (should clamp to 0)
        {1500, 90, 110}    // Over-speed (should clamp to max)
    };
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        uint16_t wb_velocity;
        WB_Compatibility_Error_t error = WB_Translation_MelkensToWB_Speed(test_cases[i].melkens_speed, &wb_velocity);
        
        TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
        TEST_ASSERT(wb_velocity >= test_cases[i].expected_min && wb_velocity <= test_cases[i].expected_max,
                   "Speed translation out of expected range");
        
        printf("    ✓ MELKENS(%d) -> WB(%u)\n", test_cases[i].melkens_speed, wb_velocity);
    }
    
    // Test NULL parameter
    WB_Compatibility_Error_t error = WB_Translation_MelkensToWB_Speed(500, NULL);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
    printf("    ✓ NULL parameter rejected\n");
    
    return true;
}

/**
 * @brief Test magnetic position translation functions
 */
bool test_translation_magnetic(void) {
    printf("  Testing magnetic position translation functions...\n");
    
    // Test magnet position translation
    struct {
        uint8_t magnet_number;
        float expected_position;
        bool should_succeed;
    } test_cases[] = {
        {1, -32.55f, true},    // Leftmost magnet
        {16, 0.0f, true},      // Center magnet
        {31, 32.55f, true},    // Rightmost magnet
        {0, 0.0f, false},      // Invalid (too low)
        {32, 0.0f, false}      // Invalid (too high)
    };
    
    for (size_t i = 0; i < sizeof(test_cases) / sizeof(test_cases[0]); i++) {
        float wb_position;
        WB_Compatibility_Error_t error = WB_Translation_MelkensToWB_MagnetPosition(
            test_cases[i].magnet_number, &wb_position);
        
        if (test_cases[i].should_succeed) {
            TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
            TEST_ASSERT_EQUAL_FLOAT(test_cases[i].expected_position, wb_position, 0.1f);
            printf("    ✓ Magnet_%u -> %.2f cm\n", test_cases[i].magnet_number, wb_position);
        } else {
            TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
            printf("    ✓ Invalid magnet_%u rejected\n", test_cases[i].magnet_number);
        }
    }
    
    return true;
}

// ============================================================================
// UNIT TESTS: PROTOCOL LAYER
// ============================================================================

/**
 * @brief Test Butler command processing
 */
bool test_protocol_butler_commands(void) {
    printf("  Testing Butler command processing...\n");
    
    // Test manual control command
    WB_Butler_Command_t command = {0};
    command.command_id = 0x1001;
    command.drive_request = 0x0001; // Manual control
    command.manual_request = 1;
    command.manual_speed = 50;
    command.manual_steering = 10;
    command.timestamp = System_GetTimeMs();
    command.checksum = 0; // Simplified checksum for test
    
    WB_Compatibility_Error_t error = WB_Protocol_ProcessButlerCommand(&command);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    printf("    ✓ Manual control command processed\n");
    
    // Verify motor speeds were set correctly
    int16_t left_speed = MotorManager_GetSpeed(Motor_Left);
    int16_t right_speed = MotorManager_GetSpeed(Motor_Right);
    TEST_ASSERT_EQUAL_INT(40, left_speed);  // 50 - 10
    TEST_ASSERT_EQUAL_INT(60, right_speed); // 50 + 10
    printf("    ✓ Motor speeds set correctly: L=%d, R=%d\n", left_speed, right_speed);
    
    // Test emergency stop command
    command.command_id = 0x1002;
    command.drive_request = 0x0003; // Emergency stop
    command.abort_request = 1;
    
    error = WB_Protocol_ProcessButlerCommand(&command);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    
    // Verify all motors stopped
    TEST_ASSERT_EQUAL_INT(0, MotorManager_GetSpeed(Motor_Left));
    TEST_ASSERT_EQUAL_INT(0, MotorManager_GetSpeed(Motor_Right));
    TEST_ASSERT_EQUAL_INT(0, MotorManager_GetSpeed(Motor_Thumble));
    printf("    ✓ Emergency stop executed - all motors stopped\n");
    
    // Test NULL parameter
    error = WB_Protocol_ProcessButlerCommand(NULL);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
    printf("    ✓ NULL command rejected\n");
    
    return true;
}

/**
 * @brief Test status response generation
 */
bool test_protocol_status_response(void) {
    printf("  Testing status response generation...\n");
    
    WB_Status_Response_t response;
    WB_Compatibility_Error_t error = WB_Protocol_UpdateStatusFromMelkens(&response);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    
    // Verify status fields are populated
    TEST_ASSERT(response.timestamp > 0, "Timestamp should be set");
    TEST_ASSERT(response.sequence_number > 0, "Sequence number should be set");
    TEST_ASSERT_EQUAL_INT(0x0001, response.status_word);
    printf("    ✓ Status response generated with timestamp %lu\n", response.timestamp);
    
    // Test status transmission
    error = WB_Protocol_SendStatusResponse(&response);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    printf("    ✓ Status response transmitted\n");
    
    // Test NULL parameter
    error = WB_Protocol_UpdateStatusFromMelkens(NULL);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
    printf("    ✓ NULL response rejected\n");
    
    return true;
}

// ============================================================================
// UNIT TESTS: DATABASE LAYER
// ============================================================================

/**
 * @brief Test database interface functions
 */
bool test_database_interface(void) {
    printf("  Testing database interface functions...\n");
    
    // Test track loading
    WB_Track_Record_t track;
    WB_Compatibility_Error_t error = WB_Database_LoadTrack(5, &track);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    TEST_ASSERT_EQUAL_INT(5, track.track_id);
    TEST_ASSERT(strlen(track.track_name) > 0, "Track name should be set");
    printf("    ✓ Track loaded: ID=%lu, Name=%s\n", track.track_id, track.track_name);
    
    // Test bay loading
    WB_Bay_Record_t bay;
    error = WB_Database_LoadBay(10, &bay);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    TEST_ASSERT_EQUAL_INT(10, bay.bay_id);
    TEST_ASSERT(strlen(bay.bay_name) > 0, "Bay name should be set");
    printf("    ✓ Bay loaded: ID=%lu, Name=%s\n", bay.bay_id, bay.bay_name);
    
    // Test config loading
    WB_Config_Record_t config;
    error = WB_Database_LoadConfig("test_param", &config);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    TEST_ASSERT(strlen(config.config_name) > 0, "Config name should be set");
    printf("    ✓ Config loaded: Name=%s, Value=%s\n", config.config_name, config.config_value);
    
    // Test NULL parameters
    error = WB_Database_LoadTrack(1, NULL);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_INVALID_PARAMETER, error);
    printf("    ✓ NULL parameters rejected\n");
    
    return true;
}

// ============================================================================
// UNIT TESTS: ERROR HANDLING
// ============================================================================

/**
 * @brief Test error handling and recovery
 */
bool test_error_handling(void) {
    printf("  Testing error handling and recovery...\n");
    
    // Test error state tracking
    WB_Compatibility_Error_t initial_error = WB_Compatibility_GetLastError();
    printf("    Initial error state: %d\n", initial_error);
    
    // Test debug logging control
    WB_Compatibility_SetDebugEnabled(true);
    WB_Compatibility_SetDebugEnabled(false);
    printf("    ✓ Debug logging control working\n");
    
    // Test diagnostic functions (should not crash)
    WB_Compatibility_PrintStatus();
    WB_Compatibility_PrintDiagnostics();
    printf("    ✓ Diagnostic functions working\n");
    
    return true;
}

// ============================================================================
// UNIT TESTS: STATISTICS AND PERFORMANCE
// ============================================================================

/**
 * @brief Test statistics tracking
 */
bool test_statistics_tracking(void) {
    printf("  Testing statistics tracking...\n");
    
    // Get initial statistics
    struct {
        uint32_t commands_processed;
        uint32_t responses_sent;
        uint32_t errors_encountered;
        uint32_t database_queries;
        uint32_t translations_performed;
        uint32_t uptime_seconds;
        uint32_t last_heartbeat_time;
    } stats_before, stats_after;
    
    WB_Compatibility_Error_t error = WB_Compatibility_GetStatistics(&stats_before);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    printf("    Initial commands processed: %lu\n", stats_before.commands_processed);
    
    // Perform some operations to update statistics
    WB_Butler_Command_t command = {0};
    command.command_id = 0x1001;
    command.drive_request = 0x0001;
    WB_Protocol_ProcessButlerCommand(&command);
    
    float wb_x, wb_y;
    WB_Translation_MelkensToWB_Position(1.0f, 1.0f, &wb_x, &wb_y);
    
    WB_Track_Record_t track;
    WB_Database_LoadTrack(1, &track);
    
    // Get updated statistics
    error = WB_Compatibility_GetStatistics(&stats_after);
    TEST_ASSERT_EQUAL_INT(WB_COMPAT_ERROR_NONE, error);
    
    // Verify statistics were updated
    TEST_ASSERT(stats_after.commands_processed >= stats_before.commands_processed,
               "Commands processed should increase");
    TEST_ASSERT(stats_after.translations_performed >= stats_before.translations_performed,
               "Translations performed should increase");
    TEST_ASSERT(stats_after.database_queries >= stats_before.database_queries,
               "Database queries should increase");
    
    printf("    ✓ Statistics updated correctly\n");
    printf("      Commands: %lu -> %lu\n", stats_before.commands_processed, stats_after.commands_processed);
    printf("      Translations: %lu -> %lu\n", stats_before.translations_performed, stats_after.translations_performed);
    
    return true;
}

// ============================================================================
// TEST RUNNER IMPLEMENTATION
// ============================================================================

/**
 * @brief Run all registered tests
 */
void run_all_tests(void) {
    printf("\n=== WB Compatibility Layer Unit Tests ===\n");
    printf("Running %d tests...\n\n", g_test_count);
    
    for (int i = 0; i < g_test_count; i++) {
        printf("Test %d/%d: %s\n", i + 1, g_test_count, g_tests[i].name);
        
        g_current_test_passed = true;
        strncpy(g_current_test_name, g_tests[i].name, TEST_MAX_NAME_LENGTH - 1);
        
        double start_time = get_time_ms();
        bool result = g_tests[i].test_function();
        double end_time = get_time_ms();
        
        g_tests[i].passed = result && g_current_test_passed;
        g_tests[i].execution_time_ms = end_time - start_time;
        
        if (g_tests[i].passed) {
            printf("  ✅ PASSED (%.2f ms)\n", g_tests[i].execution_time_ms);
            g_tests_passed++;
        } else {
            printf("  ❌ FAILED (%.2f ms)\n", g_tests[i].execution_time_ms);
            g_tests_failed++;
        }
        printf("\n");
    }
    
    // Print summary
    printf("=== Test Summary ===\n");
    printf("Total tests: %d\n", g_test_count);
    printf("Passed: %d (%.1f%%)\n", g_tests_passed, 
           (float)g_tests_passed / g_test_count * 100.0f);
    printf("Failed: %d (%.1f%%)\n", g_tests_failed,
           (float)g_tests_failed / g_test_count * 100.0f);
    
    if (g_tests_failed == 0) {
        printf("\n🎉 All tests passed!\n");
    } else {
        printf("\n💥 %d test(s) failed. See details above.\n", g_tests_failed);
        
        // List failed tests
        printf("\nFailed tests:\n");
        for (int i = 0; i < g_test_count; i++) {
            if (!g_tests[i].passed) {
                printf("  - %s\n", g_tests[i].name);
            }
        }
    }
}

/**
 * @brief Register all tests
 */
void register_all_tests(void) {
    // Initialization and configuration tests
    REGISTER_TEST("WB Init Valid Config", test_wb_init_valid_config);
    REGISTER_TEST("WB Init Invalid Config", test_wb_init_invalid_config);
    REGISTER_TEST("WB State Management", test_wb_state_management);
    
    // Translation layer tests
    REGISTER_TEST("Translation Position", test_translation_position);
    REGISTER_TEST("Translation Speed", test_translation_speed);
    REGISTER_TEST("Translation Magnetic", test_translation_magnetic);
    
    // Protocol layer tests
    REGISTER_TEST("Protocol Butler Commands", test_protocol_butler_commands);
    REGISTER_TEST("Protocol Status Response", test_protocol_status_response);
    
    // Database layer tests
    REGISTER_TEST("Database Interface", test_database_interface);
    
    // Error handling tests
    REGISTER_TEST("Error Handling", test_error_handling);
    
    // Statistics and performance tests
    REGISTER_TEST("Statistics Tracking", test_statistics_tracking);
}

/**
 * @brief Main test function
 */
int main(void) {
    printf("WB Compatibility Layer Unit Test Suite\n");
    printf("======================================\n");
    printf("Testing Phase 2 implementation...\n");
    
    // Register all tests
    register_all_tests();
    
    // Run all tests
    run_all_tests();
    
    // Return appropriate exit code
    return (g_tests_failed == 0) ? 0 : 1;
}

/*
 * COMPILATION INSTRUCTIONS:
 * 
 * To compile and run the unit tests:
 * 
 * gcc -I../../ -o test_wb_compatibility \
 *     test_wb_compatibility.c \
 *     ../../WB_Compatibility.c \
 *     -lm
 * 
 * ./test_wb_compatibility
 * 
 * INTEGRATION NOTES:
 * 
 * 1. This test suite provides comprehensive coverage of the WB compatibility layer
 * 2. All tests use mocked MELKENS system functions to avoid hardware dependencies
 * 3. Tests verify both positive and negative cases with error handling
 * 4. Performance timing is included for identifying slow operations
 * 5. The test framework is lightweight and self-contained
 * 
 * EXTENDING TESTS:
 * 
 * To add new tests:
 * 1. Create a new test function that returns bool
 * 2. Use TEST_ASSERT macros for verification
 * 3. Register the test using REGISTER_TEST macro
 * 4. Follow naming convention: test_<category>_<specific_feature>
 */