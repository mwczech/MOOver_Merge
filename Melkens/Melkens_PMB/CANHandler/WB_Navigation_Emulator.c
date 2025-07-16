/*
 * WB_Navigation_Emulator.c
 * 
 * Implementation of WB navigation emulation layer
 * Simulates WB behavior for magnet detection and navigation
 * 
 * Author: MELKENS Integration Team
 * Created: 2024
 */

#include "WB_Navigation_Emulator.h"
#include "../pmb_MotorManager.h"
#include "../pmb_System.h"
#include "../pmb_Functions.h"
#include "WB_CanOpen.h"
#include <stdio.h>
#include <string.h>

// Global navigation context
static WB_NavigationContext_t nav_context;
static WB_MagneticField_t magnetic_field;
static bool emulation_enabled = true;

// Simulated configuration data (normally loaded from database)
static WB_TrackPos_t tracks[WB_MAX_TRACKS];
static WB_Bay_t bays[WB_MAX_BAYS];
static WB_ReferencePosition_t reference_positions[WB_MAX_REFERENCE_POS];
static uint16_t track_count = 0;
static uint16_t bay_count = 0;
static uint16_t reference_count = 0;

// Statistics
static uint32_t total_distance_traveled = 0;
static uint32_t navigation_start_time = 0;
static uint16_t magnet_detections = 0;
static uint8_t error_count = 0;

// Private function prototypes
static void WB_NavEmulator_InitializeDefaultConfig(void);
static float WB_NavEmulator_CalculateDistance(WB_WorldPosition_t pos1, WB_WorldPosition_t pos2);
static float WB_NavEmulator_CalculateHeading(WB_WorldPosition_t from, WB_WorldPosition_t to);
static float WB_NavEmulator_NormalizeAngle(float angle);
static WB_ReferencePosition_t* WB_NavEmulator_FindNearestReference(WB_WorldPosition_t pos);
static WB_Bay_t* WB_NavEmulator_FindBay(uint32_t bayId);
static WB_TrackPos_t* WB_NavEmulator_FindTrack(uint32_t trackId);

/**
 * Initialize WB navigation emulator
 */
void WB_NavEmulator_Init(void) {
    printf("Initializing WB Navigation Emulator...\n");
    
    // Clear navigation context
    memset(&nav_context, 0, sizeof(WB_NavigationContext_t));
    memset(&magnetic_field, 0, sizeof(WB_MagneticField_t));
    
    // Initialize default state
    nav_context.state = WB_NAV_STATE_IDLE;
    nav_context.activeRequest = WB_DRIVE_REQ_STOP;
    nav_context.cruiseSpeed = 800.0f;
    nav_context.approachSpeed = 400.0f;
    nav_context.feedingSpeed = 200.0f;
    nav_context.maxSteeringAngle = 30.0f;
    nav_context.timeoutMs = 30000; // 30 second timeout
    
    // Initialize default configuration
    WB_NavEmulator_InitializeDefaultConfig();
    
    // Reset statistics
    total_distance_traveled = 0;
    navigation_start_time = System_GetTimeMs();
    magnet_detections = 0;
    error_count = 0;
    
    printf("WB Navigation Emulator initialized successfully\n");
}

/**
 * Update navigation emulator (call from main loop)
 */
void WB_NavEmulator_Update(void) {
    if (!emulation_enabled) return;
    
    uint32_t current_time = System_GetTimeMs();
    nav_context.lastUpdateTime = current_time;
    
    switch (nav_context.state) {
        case WB_NAV_STATE_IDLE:
            // Wait for navigation request
            break;
            
        case WB_NAV_STATE_NAVIGATING:
            WB_NavEmulator_UpdatePathFollowing();
            break;
            
        case WB_NAV_STATE_APPROACHING_BAY:
            WB_NavEmulator_UpdatePathFollowing();
            // Check if close to bay entry
            if (nav_context.remainingDistance < WB_POSITION_TOLERANCE) {
                nav_context.state = WB_NAV_STATE_IN_BAY;
                printf("WB Emulator: Entered bay %lu\n", nav_context.currentBayId);
            }
            break;
            
        case WB_NAV_STATE_IN_BAY:
            // Slow movement within bay
            WB_NavEmulator_ApplyToMelkensMotors(nav_context.feedingSpeed, 0.0f);
            break;
            
        case WB_NAV_STATE_FEEDING:
            // Stop for feeding
            WB_NavEmulator_ApplyToMelkensMotors(0.0f, 0.0f);
            // Start auger/feeding mechanism
            MotorManager_SetSpeed(Motor_Thumble, 800);
            break;
            
        case WB_NAV_STATE_EXITING_BAY:
            WB_NavEmulator_UpdatePathFollowing();
            break;
            
        case WB_NAV_STATE_PARKING:
            WB_NavEmulator_UpdatePathFollowing();
            break;
            
        case WB_NAV_STATE_ERROR:
            WB_NavEmulator_ApplyToMelkensMotors(0.0f, 0.0f);
            break;
    }
    
    // Check for timeout
    if (current_time - nav_context.navigationStartTime > nav_context.timeoutMs) {
        printf("WB Emulator: Navigation timeout\n");
        nav_context.state = WB_NAV_STATE_ERROR;
        nav_context.errorCode = 0x8001; // Timeout error
        error_count++;
    }
}

/**
 * Simulate magnet detection from MELKENS magnet array
 */
void WB_NavEmulator_SimulateMagnetDetection(MagnetName detectedMagnet) {
    if (!emulation_enabled) return;
    
    magnet_detections++;
    magnetic_field.detected = true;
    magnetic_field.timestamp = System_GetTimeMs();
    
    // Convert MELKENS magnet position to WB format
    // MELKENS: Magnet1-31, Magnet16 = center
    // WB: -15.0 to +15.0 position scale
    float position = (float)(detectedMagnet - Magnet16) * 2.17f; // 2.17cm per position
    magnetic_field.position = position;
    
    // Simulate field strength based on distance from center
    float distance_from_center = fabs(position);
    magnetic_field.strength = 100.0f - (distance_from_center * 5.0f); // Stronger near center
    if (magnetic_field.strength < 0.0f) magnetic_field.strength = 0.0f;
    
    printf("WB Emulator: Magnet detected - Position: %.1f, Strength: %.1f\n", 
           position, magnetic_field.strength);
    
    // Process magnetic field detection
    WB_NavEmulator_ProcessMagneticField(magnetic_field.strength, position);
    
    // Update position from magnetic reference
    WB_ReferencePosition_t* nearest_ref = WB_NavEmulator_FindNearestReference(nav_context.currentPos);
    if (nearest_ref != NULL) {
        WB_NavEmulator_UpdatePositionFromMagnet(nearest_ref->id, magnetic_field.strength);
    }
}

/**
 * Process magnetic field detection (WB style)
 */
void WB_NavEmulator_ProcessMagneticField(float fieldStrength, float position) {
    // Update magnetic field state
    magnetic_field.strength = fieldStrength;
    magnetic_field.position = position;
    
    // Calculate magnetic correction
    float correction = WB_NavEmulator_CalculateMagneticCorrection(fieldStrength, 0.0f);
    
    // Apply correction to navigation
    if (nav_context.state == WB_NAV_STATE_NAVIGATING || 
        nav_context.state == WB_NAV_STATE_APPROACHING_BAY) {
        
        // Update cross-track error
        nav_context.crossTrackError = position / 100.0f; // Convert cm to meters
        
        // Calculate steering correction
        float steering_correction = WB_NavEmulator_CalculateSteering(nav_context.crossTrackError, 0.0f);
        
        printf("WB Emulator: Magnetic correction - Position: %.2f, Correction: %.2f\n", 
               position, steering_correction);
        
        // Apply correction to motors (if not in feeding mode)
        if (nav_context.state != WB_NAV_STATE_FEEDING) {
            float current_speed = nav_context.cruiseSpeed;
            WB_NavEmulator_ApplyToMelkensMotors(current_speed, steering_correction);
        }
    }
    
    nav_context.magneticPositionValid = (fieldStrength > 10.0f); // Minimum threshold
}

/**
 * Calculate magnetic correction (WB style)
 */
float WB_NavEmulator_CalculateMagneticCorrection(float fieldStrength, float targetPosition) {
    // WB-style correction algorithm
    float error = magnetic_field.position - targetPosition;
    float correction = 0.0f;
    
    if (fieldStrength > 20.0f) { // Strong signal
        correction = error * 0.8f; // High correction factor
    } else if (fieldStrength > 10.0f) { // Medium signal
        correction = error * 0.5f; // Medium correction factor
    } else { // Weak signal
        correction = error * 0.2f; // Low correction factor
    }
    
    // Limit correction range
    if (correction > 15.0f) correction = 15.0f;
    if (correction < -15.0f) correction = -15.0f;
    
    return correction;
}

/**
 * Navigate to track position (WB style)
 */
bool WB_NavEmulator_NavigateToTrack(uint32_t trackId) {
    WB_TrackPos_t* track = WB_NavEmulator_FindTrack(trackId);
    if (track == NULL) {
        printf("WB Emulator: Track %lu not found\n", trackId);
        return false;
    }
    
    nav_context.currentTrackId = trackId;
    nav_context.targetPos.x = track->posX;
    nav_context.targetPos.y = track->posY;
    nav_context.targetPos.heading = track->direction;
    nav_context.state = WB_NAV_STATE_NAVIGATING;
    nav_context.navigationStartTime = System_GetTimeMs();
    
    // Calculate path
    if (!WB_NavEmulator_CalculatePath(nav_context.targetPos)) {
        printf("WB Emulator: Path calculation failed\n");
        return false;
    }
    
    printf("WB Emulator: Navigating to track %lu at (%.2f, %.2f)\n", 
           trackId, track->posX, track->posY);
    
    return true;
}

/**
 * Approach bay for feeding (WB style)
 */
bool WB_NavEmulator_ApproachBay(uint32_t bayId) {
    WB_Bay_t* bay = WB_NavEmulator_FindBay(bayId);
    if (bay == NULL) {
        printf("WB Emulator: Bay %lu not found\n", bayId);
        return false;
    }
    
    nav_context.targetBayId = bayId;
    nav_context.currentBayId = bayId;
    nav_context.targetPos.x = bay->entryNearX;
    nav_context.targetPos.y = bay->entryNearY;
    nav_context.state = WB_NAV_STATE_APPROACHING_BAY;
    nav_context.navigationStartTime = System_GetTimeMs();
    
    // Calculate path to bay entry
    if (!WB_NavEmulator_CalculatePath(nav_context.targetPos)) {
        printf("WB Emulator: Bay approach path calculation failed\n");
        return false;
    }
    
    printf("WB Emulator: Approaching bay %lu at (%.2f, %.2f)\n", 
           bayId, bay->entryNearX, bay->entryNearY);
    
    return true;
}

/**
 * Execute feeding operation (WB style)
 */
bool WB_NavEmulator_ExecuteFeeding(uint32_t bayId, float amount) {
    WB_Bay_t* bay = WB_NavEmulator_FindBay(bayId);
    if (bay == NULL) {
        printf("WB Emulator: Bay %lu not found for feeding\n", bayId);
        return false;
    }
    
    nav_context.state = WB_NAV_STATE_FEEDING;
    nav_context.targetPos.x = bay->feedPos;
    nav_context.targetPos.y = bay->entryNearY; // Assume same Y as entry
    
    printf("WB Emulator: Feeding %.2f kg at bay %lu\n", amount, bayId);
    
    // Move to feeding position
    WB_NavEmulator_CalculatePath(nav_context.targetPos);
    
    // Start feeding operation
    MotorManager_SetSpeed(Motor_Thumble, 800); // Start auger
    
    return true;
}

/**
 * Update path following (WB style)
 */
void WB_NavEmulator_UpdatePathFollowing(void) {
    // Calculate distance to target
    nav_context.remainingDistance = WB_NavEmulator_CalculateDistance(
        nav_context.currentPos, nav_context.targetPos);
    
    // Calculate heading to target
    float target_heading = WB_NavEmulator_CalculateHeading(
        nav_context.currentPos, nav_context.targetPos);
    
    // Calculate heading error
    nav_context.headingError = WB_NavEmulator_NormalizeAngle(
        target_heading - nav_context.currentPos.heading);
    
    // Calculate control commands
    float speed = WB_NavEmulator_CalculateSpeed(nav_context.remainingDistance, nav_context.state);
    float steering = WB_NavEmulator_CalculateSteering(nav_context.crossTrackError, nav_context.headingError);
    
    // Apply to motors
    WB_NavEmulator_ApplyToMelkensMotors(speed, steering);
    
    // Check if target reached
    if (nav_context.remainingDistance < WB_POSITION_TOLERANCE) {
        printf("WB Emulator: Target reached\n");
        
        switch (nav_context.state) {
            case WB_NAV_STATE_NAVIGATING:
                nav_context.state = WB_NAV_STATE_IDLE;
                break;
            case WB_NAV_STATE_APPROACHING_BAY:
                nav_context.state = WB_NAV_STATE_IN_BAY;
                break;
            case WB_NAV_STATE_EXITING_BAY:
                nav_context.state = WB_NAV_STATE_IDLE;
                break;
            case WB_NAV_STATE_PARKING:
                nav_context.state = WB_NAV_STATE_IDLE;
                WB_NavEmulator_ApplyToMelkensMotors(0.0f, 0.0f); // Stop
                break;
            default:
                break;
        }
    }
}

/**
 * Calculate steering command (WB style)
 */
float WB_NavEmulator_CalculateSteering(float crossTrackError, float headingError) {
    // WB-style PID steering controller
    const float kp_cross = 50.0f;   // Cross-track proportional gain
    const float kp_heading = 2.0f;  // Heading proportional gain
    
    float steering = (crossTrackError * kp_cross) + (headingError * kp_heading);
    
    // Limit steering angle
    if (steering > nav_context.maxSteeringAngle) {
        steering = nav_context.maxSteeringAngle;
    } else if (steering < -nav_context.maxSteeringAngle) {
        steering = -nav_context.maxSteeringAngle;
    }
    
    return steering;
}

/**
 * Calculate speed command (WB style)
 */
float WB_NavEmulator_CalculateSpeed(float distanceToTarget, WB_NavigationState_t state) {
    float speed = nav_context.cruiseSpeed;
    
    switch (state) {
        case WB_NAV_STATE_APPROACHING_BAY:
            speed = nav_context.approachSpeed;
            break;
        case WB_NAV_STATE_IN_BAY:
        case WB_NAV_STATE_FEEDING:
            speed = nav_context.feedingSpeed;
            break;
        case WB_NAV_STATE_PARKING:
            speed = nav_context.approachSpeed * 0.5f; // Slow parking
            break;
        default:
            speed = nav_context.cruiseSpeed;
            break;
    }
    
    // Slow down when approaching target
    if (distanceToTarget < 2.0f) { // 2 meter slowdown zone
        speed = speed * (distanceToTarget / 2.0f);
        if (speed < WB_MIN_SPEED) speed = WB_MIN_SPEED;
    }
    
    return speed;
}

/**
 * Apply WB navigation to MELKENS motors
 */
void WB_NavEmulator_ApplyToMelkensMotors(float speed, float steering) {
    // Convert WB commands to MELKENS differential drive
    float left_speed = speed - steering;
    float right_speed = speed + steering;
    
    // Limit speeds
    if (left_speed > WB_MAX_SPEED) left_speed = WB_MAX_SPEED;
    if (left_speed < -WB_MAX_SPEED) left_speed = -WB_MAX_SPEED;
    if (right_speed > WB_MAX_SPEED) right_speed = WB_MAX_SPEED;
    if (right_speed < -WB_MAX_SPEED) right_speed = -WB_MAX_SPEED;
    
    // Apply to MELKENS motor manager
    MotorManager_SetSpeed(Motor_Left, (int16_t)left_speed);
    MotorManager_SetSpeed(Motor_Right, (int16_t)right_speed);
}

/**
 * Convert MELKENS RouteStep to WB navigation
 */
bool WB_NavEmulator_ConvertMelkensRoute(const RouteStep* melkensStep) {
    if (melkensStep == NULL) return false;
    
    // Convert relative coordinates to world coordinates
    float target_x = nav_context.currentPos.x + (melkensStep->dX / 100.0f); // cm to meters
    float target_y = nav_context.currentPos.y + (melkensStep->dY / 100.0f);
    float target_heading = nav_context.currentPos.heading + melkensStep->Angle;
    
    // Set target
    return WB_NavEmulator_SetTarget(target_x, target_y, target_heading);
}

/**
 * Print navigation status
 */
void WB_NavEmulator_PrintStatus(void) {
    printf("=== WB Navigation Emulator Status ===\n");
    printf("State: %d\n", nav_context.state);
    printf("Position: (%.2f, %.2f) @ %.1f°\n", 
           nav_context.currentPos.x, nav_context.currentPos.y, nav_context.currentPos.heading);
    printf("Target: (%.2f, %.2f) @ %.1f°\n", 
           nav_context.targetPos.x, nav_context.targetPos.y, nav_context.targetPos.heading);
    printf("Distance to target: %.2f m\n", nav_context.remainingDistance);
    printf("Cross-track error: %.2f m\n", nav_context.crossTrackError);
    printf("Heading error: %.1f°\n", nav_context.headingError);
    printf("Magnetic position valid: %s\n", nav_context.magneticPositionValid ? "Yes" : "No");
    printf("Error code: 0x%04X\n", nav_context.errorCode);
    printf("=====================================\n");
}

// Helper functions implementation

/**
 * Initialize default configuration
 */
static void WB_NavEmulator_InitializeDefaultConfig(void) {
    // Initialize some default tracks
    tracks[0] = (WB_TrackPos_t){1, 0.0f, 0.0f, 0, 800, 800, 100};
    tracks[1] = (WB_TrackPos_t){2, 5.0f, 0.0f, 90, 800, 800, 100};
    tracks[2] = (WB_TrackPos_t){3, 5.0f, 5.0f, 180, 800, 800, 100};
    track_count = 3;
    
    // Initialize some default bays
    bays[0] = (WB_Bay_t){1, 2.0f, 1.0f, 2.0f, 0.5f, 2.5f, 1.0f, 2.5f, 0.5f, 0.1f, 0.1f, 2.25f, 5000};
    bays[1] = (WB_Bay_t){2, 4.0f, 1.0f, 4.0f, 0.5f, 4.5f, 1.0f, 4.5f, 0.5f, 0.1f, 0.1f, 4.25f, 5000};
    bay_count = 2;
    
    // Initialize reference positions
    reference_positions[0] = (WB_ReferencePosition_t){1, 1.0f, 0.0f, 0, 1, 50.0f};
    reference_positions[1] = (WB_ReferencePosition_t){2, 3.0f, 0.0f, 0, 1, 50.0f};
    reference_positions[2] = (WB_ReferencePosition_t){3, 5.0f, 0.0f, 90, 1, 50.0f};
    reference_count = 3;
    
    printf("WB Emulator: Default configuration loaded - %d tracks, %d bays, %d references\n",
           track_count, bay_count, reference_count);
}

/**
 * Calculate distance between two positions
 */
static float WB_NavEmulator_CalculateDistance(WB_WorldPosition_t pos1, WB_WorldPosition_t pos2) {
    float dx = pos2.x - pos1.x;
    float dy = pos2.y - pos1.y;
    return sqrtf(dx*dx + dy*dy);
}

/**
 * Calculate heading from one position to another
 */
static float WB_NavEmulator_CalculateHeading(WB_WorldPosition_t from, WB_WorldPosition_t to) {
    float dx = to.x - from.x;
    float dy = to.y - from.y;
    float heading = atan2f(dy, dx) * 180.0f / M_PI;
    return WB_NavEmulator_NormalizeAngle(heading);
}

/**
 * Normalize angle to 0-360 degrees
 */
static float WB_NavEmulator_NormalizeAngle(float angle) {
    while (angle < 0.0f) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;
    return angle;
}

/**
 * Find nearest reference position
 */
static WB_ReferencePosition_t* WB_NavEmulator_FindNearestReference(WB_WorldPosition_t pos) {
    if (reference_count == 0) return NULL;
    
    float min_distance = 1000.0f;
    WB_ReferencePosition_t* nearest = NULL;
    
    for (uint16_t i = 0; i < reference_count; i++) {
        WB_WorldPosition_t ref_pos = {reference_positions[i].posX, reference_positions[i].posY, 0};
        float distance = WB_NavEmulator_CalculateDistance(pos, ref_pos);
        
        if (distance < min_distance) {
            min_distance = distance;
            nearest = &reference_positions[i];
        }
    }
    
    return nearest;
}

/**
 * Find bay by ID
 */
static WB_Bay_t* WB_NavEmulator_FindBay(uint32_t bayId) {
    for (uint16_t i = 0; i < bay_count; i++) {
        if (bays[i].bayId == bayId) {
            return &bays[i];
        }
    }
    return NULL;
}

/**
 * Find track by ID
 */
static WB_TrackPos_t* WB_NavEmulator_FindTrack(uint32_t trackId) {
    for (uint16_t i = 0; i < track_count; i++) {
        if (tracks[i].trackId == trackId) {
            return &tracks[i];
        }
    }
    return NULL;
}

// Stub implementations for remaining functions

bool WB_NavEmulator_SetTarget(float x, float y, float heading) {
    nav_context.targetPos.x = x;
    nav_context.targetPos.y = y;
    nav_context.targetPos.heading = heading;
    nav_context.state = WB_NAV_STATE_NAVIGATING;
    nav_context.navigationStartTime = System_GetTimeMs();
    return WB_NavEmulator_CalculatePath(nav_context.targetPos);
}

bool WB_NavEmulator_SetTargetBay(uint32_t bayId) {
    return WB_NavEmulator_ApproachBay(bayId);
}

void WB_NavEmulator_ProcessDriveRequest(WB_DriveRequest_t request, float param) {
    nav_context.activeRequest = request;
    
    switch (request) {
        case WB_DRIVE_REQ_STOP:
            nav_context.state = WB_NAV_STATE_IDLE;
            WB_NavEmulator_ApplyToMelkensMotors(0.0f, 0.0f);
            break;
        case WB_DRIVE_REQ_START:
            nav_context.state = WB_NAV_STATE_NAVIGATING;
            break;
        case WB_DRIVE_REQ_PARK:
            WB_NavEmulator_ParkRobot();
            break;
        default:
            break;
    }
}

void WB_NavEmulator_UpdatePosition(float deltaX, float deltaY, float deltaHeading) {
    nav_context.currentPos.x += deltaX;
    nav_context.currentPos.y += deltaY;
    nav_context.currentPos.heading = WB_NavEmulator_NormalizeAngle(
        nav_context.currentPos.heading + deltaHeading);
    
    total_distance_traveled += sqrtf(deltaX*deltaX + deltaY*deltaY);
}

WB_NavigationState_t WB_NavEmulator_GetState(void) {
    return nav_context.state;
}

WB_WorldPosition_t WB_NavEmulator_GetPosition(void) {
    return nav_context.currentPos;
}

const WB_NavigationContext_t* WB_NavEmulator_GetContext(void) {
    return &nav_context;
}

bool WB_NavEmulator_ParkRobot(void) {
    // Use default parking position
    nav_context.targetPos.x = 0.0f;
    nav_context.targetPos.y = 0.0f;
    nav_context.targetPos.heading = 0.0f;
    nav_context.state = WB_NAV_STATE_PARKING;
    return true;
}

void WB_NavEmulator_EmergencyStop(void) {
    nav_context.state = WB_NAV_STATE_ERROR;
    nav_context.emergencyStop = true;
    WB_NavEmulator_ApplyToMelkensMotors(0.0f, 0.0f);
    MotorManager_Stop(Motor_Left);
    MotorManager_Stop(Motor_Right);
    MotorManager_Stop(Motor_Thumble);
}

bool WB_NavEmulator_CalculatePath(WB_WorldPosition_t target) {
    nav_context.pathDistance = WB_NavEmulator_CalculateDistance(nav_context.currentPos, target);
    nav_context.remainingDistance = nav_context.pathDistance;
    return true;
}

void WB_NavEmulator_GetStatistics(uint32_t* totalDistance, uint32_t* navigationTime, 
                                  uint16_t* magnetDetections, uint8_t* errorCount) {
    if (totalDistance) *totalDistance = total_distance_traveled;
    if (navigationTime) *navigationTime = System_GetTimeMs() - navigation_start_time;
    if (magnetDetections) *magnetDetections = magnet_detections;
    if (errorCount) *errorCount = error_count;
}

void WB_NavEmulator_SetEmulationMode(bool enabled) {
    emulation_enabled = enabled;
    printf("WB Navigation Emulator: %s\n", enabled ? "Enabled" : "Disabled");
}