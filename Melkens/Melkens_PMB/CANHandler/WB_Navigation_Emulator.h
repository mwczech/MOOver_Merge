/*
 * WB_Navigation_Emulator.h
 * 
 * Emulation layer for WB navigation behavior in MELKENS system
 * Provides stubs and functions to simulate WB magnet detection and navigation
 * 
 * Author: MELKENS Integration Team
 * Created: 2024
 */

#ifndef WB_NAVIGATION_EMULATOR_H
#define WB_NAVIGATION_EMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "../RoutesDataTypes.h"
#include "../IMUHandler/IMUHandler.h"

#ifdef __cplusplus
extern "C" {
#endif

// WB World Coordinate System
typedef struct {
    float x;           // X coordinate in meters
    float y;           // Y coordinate in meters
    float heading;     // Heading in degrees (0-360)
} WB_WorldPosition_t;

// WB Track/Bay definitions (from DDMap.cfg)
typedef struct {
    uint32_t trackId;
    float posX, posY;
    uint16_t direction;        // 0-359 degrees
    uint16_t trommelSpeed;
    uint16_t butlerSpeed;
    uint8_t power;             // Track power level
} WB_TrackPos_t;

typedef struct {
    uint32_t bayId;
    float entryNearX, entryNearY;    // Bay entry coordinates
    float entryFarX, entryFarY;      
    float exitNearX, exitNearY;      // Bay exit coordinates
    float exitFarX, exitFarY;
    float offsetFar, offsetNear;     // Position offsets
    float feedPos;                   // Feeding position
    uint16_t farNearDuration;        // Time for far-near movement
} WB_Bay_t;

// WB Reference Position (magnetic markers)
typedef struct {
    uint32_t id;
    float posX, posY;
    uint16_t direction;
    uint16_t state;                  // Active/inactive
    float fieldStrengthThreshold;    // Magnetic field threshold
} WB_ReferencePosition_t;

// WB Navigation State
typedef enum {
    WB_NAV_STATE_IDLE = 0,
    WB_NAV_STATE_NAVIGATING,
    WB_NAV_STATE_APPROACHING_BAY,
    WB_NAV_STATE_IN_BAY,
    WB_NAV_STATE_FEEDING,
    WB_NAV_STATE_EXITING_BAY,
    WB_NAV_STATE_PARKING,
    WB_NAV_STATE_ERROR
} WB_NavigationState_t;

// WB Drive Request (from Butler Control)
typedef enum {
    WB_DRIVE_REQ_STOP = 0,
    WB_DRIVE_REQ_START,
    WB_DRIVE_REQ_MANUAL,
    WB_DRIVE_REQ_AUTO,
    WB_DRIVE_REQ_TEACH_TRACK,
    WB_DRIVE_REQ_PARK,
    WB_DRIVE_REQ_CALIBRATE
} WB_DriveRequest_t;

// WB Navigation Context
typedef struct {
    WB_WorldPosition_t currentPos;
    WB_WorldPosition_t targetPos;
    WB_NavigationState_t state;
    WB_DriveRequest_t activeRequest;
    
    // Current navigation targets
    uint32_t currentTrackId;
    uint32_t targetBayId;
    uint32_t currentBayId;
    
    // Path planning
    float pathDistance;              // Total distance to target
    float remainingDistance;         // Distance remaining
    float crossTrackError;           // Cross-track error
    float headingError;              // Heading error
    
    // Magnetic positioning
    uint32_t lastReferenceId;        // Last detected reference position
    float magneticFieldStrength;     // Current field strength
    bool magneticPositionValid;      // Position fix valid
    
    // Control parameters
    float cruiseSpeed;               // Cruise speed
    float approachSpeed;             // Bay approach speed
    float feedingSpeed;              // Feeding speed
    float maxSteeringAngle;          // Maximum steering angle
    
    // Error handling
    uint16_t errorCode;
    uint8_t retryCount;
    bool emergencyStop;
    
    // Timing
    uint32_t lastUpdateTime;
    uint32_t navigationStartTime;
    uint16_t timeoutMs;
    
} WB_NavigationContext_t;

// Magnetic Field Simulation
typedef struct {
    float strength;          // Field strength (0.0 - 100.0)
    float position;          // Position relative to magnet center (-15.0 to +15.0)
    bool detected;           // Magnet detected flag
    uint32_t referenceId;    // Reference position ID
    uint32_t timestamp;      // Detection timestamp
} WB_MagneticField_t;

// Configuration limits
#define WB_MAX_TRACKS           100
#define WB_MAX_BAYS             50
#define WB_MAX_REFERENCE_POS    200

// Navigation parameters
#define WB_POSITION_TOLERANCE   0.1f    // Position tolerance in meters
#define WB_HEADING_TOLERANCE    5.0f    // Heading tolerance in degrees
#define WB_MAX_SPEED            1000    // Maximum speed
#define WB_MIN_SPEED            50      // Minimum speed
#define WB_MAGNETIC_RANGE       0.5f    // Magnetic detection range in meters

// Function prototypes

/**
 * Initialize WB navigation emulator
 */
void WB_NavEmulator_Init(void);

/**
 * Update navigation emulator (call from main loop)
 */
void WB_NavEmulator_Update(void);

/**
 * Set target position for navigation
 */
bool WB_NavEmulator_SetTarget(float x, float y, float heading);

/**
 * Set target bay for feeding operation
 */
bool WB_NavEmulator_SetTargetBay(uint32_t bayId);

/**
 * Process drive request from Butler
 */
void WB_NavEmulator_ProcessDriveRequest(WB_DriveRequest_t request, float param);

/**
 * Simulate magnet detection from MELKENS magnet array
 */
void WB_NavEmulator_SimulateMagnetDetection(MagnetName detectedMagnet);

/**
 * Update robot position from odometry
 */
void WB_NavEmulator_UpdatePosition(float deltaX, float deltaY, float deltaHeading);

/**
 * Get current navigation state
 */
WB_NavigationState_t WB_NavEmulator_GetState(void);

/**
 * Get current world position
 */
WB_WorldPosition_t WB_NavEmulator_GetPosition(void);

/**
 * Get navigation context for debugging
 */
const WB_NavigationContext_t* WB_NavEmulator_GetContext(void);

// WB-style navigation functions (emulating WB behavior)

/**
 * Navigate to track position (WB style)
 */
bool WB_NavEmulator_NavigateToTrack(uint32_t trackId);

/**
 * Approach bay for feeding (WB style)
 */
bool WB_NavEmulator_ApproachBay(uint32_t bayId);

/**
 * Execute feeding operation (WB style)
 */
bool WB_NavEmulator_ExecuteFeeding(uint32_t bayId, float amount);

/**
 * Park robot at parking position (WB style)
 */
bool WB_NavEmulator_ParkRobot(void);

/**
 * Emergency stop (WB style)
 */
void WB_NavEmulator_EmergencyStop(void);

// Magnetic positioning functions (WB emulation)

/**
 * Process magnetic field detection (WB style)
 */
void WB_NavEmulator_ProcessMagneticField(float fieldStrength, float position);

/**
 * Update position from magnetic reference (WB style)
 */
bool WB_NavEmulator_UpdatePositionFromMagnet(uint32_t referenceId, float fieldStrength);

/**
 * Calculate magnetic correction (WB style)
 */
float WB_NavEmulator_CalculateMagneticCorrection(float fieldStrength, float targetPosition);

/**
 * Interpolate position between reference points (WB style)
 */
WB_WorldPosition_t WB_NavEmulator_InterpolatePosition(uint32_t refId1, uint32_t refId2, float ratio);

// Path planning functions (WB emulation)

/**
 * Calculate path to target (WB style)
 */
bool WB_NavEmulator_CalculatePath(WB_WorldPosition_t target);

/**
 * Update path following (WB style)
 */
void WB_NavEmulator_UpdatePathFollowing(void);

/**
 * Calculate steering command (WB style)
 */
float WB_NavEmulator_CalculateSteering(float crossTrackError, float headingError);

/**
 * Calculate speed command (WB style)
 */
float WB_NavEmulator_CalculateSpeed(float distanceToTarget, WB_NavigationState_t state);

// Integration with MELKENS system

/**
 * Convert MELKENS RouteStep to WB navigation
 */
bool WB_NavEmulator_ConvertMelkensRoute(const RouteStep* melkensStep);

/**
 * Apply WB navigation to MELKENS motors
 */
void WB_NavEmulator_ApplyToMelkensMotors(float speed, float steering);

/**
 * Synchronize with MELKENS route manager
 */
void WB_NavEmulator_SyncWithMelkens(void);

// Configuration and calibration

/**
 * Load track configuration (simulated)
 */
bool WB_NavEmulator_LoadTrackConfig(void);

/**
 * Load bay configuration (simulated)
 */
bool WB_NavEmulator_LoadBayConfig(void);

/**
 * Load reference positions (simulated)
 */
bool WB_NavEmulator_LoadReferencePositions(void);

/**
 * Calibrate magnetic positioning
 */
bool WB_NavEmulator_CalibrateMagneticPositioning(void);

/**
 * Save current position as reference
 */
bool WB_NavEmulator_SaveReferencePosition(uint32_t id);

// Debugging and diagnostics

/**
 * Print navigation status
 */
void WB_NavEmulator_PrintStatus(void);

/**
 * Print magnetic field information
 */
void WB_NavEmulator_PrintMagneticInfo(void);

/**
 * Get navigation statistics
 */
void WB_NavEmulator_GetStatistics(uint32_t* totalDistance, uint32_t* navigationTime, 
                                  uint16_t* magnetDetections, uint8_t* errorCount);

/**
 * Reset navigation system
 */
void WB_NavEmulator_Reset(void);

/**
 * Enable/disable WB emulation mode
 */
void WB_NavEmulator_SetEmulationMode(bool enabled);

#ifdef __cplusplus
}
#endif

#endif /* WB_NAVIGATION_EMULATOR_H */