/* 
 * File:   CanHandler.h
 * Author: MELKENS Integration Team
 *
 * Enhanced CAN Handler with WB compatibility layer
 * 
 * Created on 15 października 2023, 16:09
 * Updated: 2024
 */

#ifndef CANHANDLER_H
#define	CANHANDLER_H

#include <stdint.h>
#include <stdbool.h>
#include "../mcc_generated_files/can_types.h"

#ifdef	__cplusplus
extern "C" {
#endif

// Function prototypes

/**
 * Initialize CAN Handler with WB compatibility layer
 */
void CANHandler_Init(void);

/**
 * Main CAN message processing task - call from main loop
 */
void CANHandler_Task(void);

/**
 * Check if received message is WB CANopen format
 * @param msg - Pointer to CAN message
 * @return true if message is WB format
 */
bool CANHandler_IsWBMessage(CAN_MSG_OBJ* msg);

/**
 * Process legacy MELKENS CAN messages
 * @param msg - Pointer to CAN message
 */
void CANHandler_ProcessLegacyMessage(CAN_MSG_OBJ* msg);

/**
 * Send WB-compatible SDO message
 * @param targetNode - Target node ID
 * @param index - Object dictionary index
 * @param subindex - Object dictionary subindex
 * @param data - Data to send
 * @return true if message sent successfully
 */
bool CANHandler_SendWBMessage(uint8_t targetNode, uint16_t index, uint8_t subindex, uint32_t data);

/**
 * Send motor command in WB CANopen format
 * @param motorNode - Motor node ID (0x7D, 0x7E, 0x7F)
 * @param speed - Motor speed (-32767 to +32767)
 * @param acceleration - Acceleration value
 * @return true if command sent successfully
 */
bool CANHandler_SendMotorCommand(uint8_t motorNode, int16_t speed, uint16_t acceleration);

/**
 * Configure servo with WB-compatible parameters
 * @param servoNode - Servo node ID
 * @return true if configuration successful
 */
bool CANHandler_ConfigureServo(uint8_t servoNode);

/**
 * Enable/disable WB compatibility mode
 * @param enable - true to enable, false to disable
 */
void CANHandler_SetWBCompatibility(bool enable);

/**
 * Get WB compatibility status
 * @return true if WB compatibility is enabled
 */
bool CANHandler_IsWBCompatibilityEnabled(void);

/**
 * Get CAN communication statistics
 * @param rxCount - Pointer to receive count
 * @param txCount - Pointer to transmit count
 */
void CANHandler_GetStatistics(uint32_t* rxCount, uint32_t* txCount);

/**
 * Periodic tasks - call from timer interrupts
 */
void CANHandler_PeriodicTasks_1ms(void);   // 1ms period
void CANHandler_PeriodicTasks_10ms(void);  // 10ms period
void CANHandler_PeriodicTasks_100ms(void); // 100ms period

// Legacy function declarations for backward compatibility
void CAN_Internal_Init(void);
void CAN_Polling(void);

#ifdef	__cplusplus
}
#endif

#endif	/* CANHANDLER_H */

