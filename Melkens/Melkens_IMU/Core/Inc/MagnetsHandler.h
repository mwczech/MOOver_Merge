/*
 * MagnetsHandler.c
 *
 *  Created on: 6 sty 2024
 *      Author: ciaptak
 */

#ifndef INC_MAGNETSHANDLER_C_
#define INC_MAGNETSHANDLER_C_

#include <stdint.h>
#include <stdbool.h>

#define dMAGNET_RESPONSE_TIMEOUT 50U /* 50ms for not response timeout */

#define dDISTANCE_BETWEEN_SENSORS 2.5 //distnce between individual hall sensors in cm

#define dMAGNET_BAR_OFFSET_DISTANCE 20 //distance between robot axle and magnet sens bar in cm

typedef enum MagnetName_t{
	Magnet1 = 0,
	Magnet2,
	Magnet3,
	Magnet4,
	Magnet5,
	Magnet6,
	Magnet7,
	Magnet8,
	MagnetsNumOf
}MagnetName;

void MagnetsHandler_Init(void);
void MagnetsHandler_Perform1ms(void);
void MagnetsHandler_Perform10ms(void);
uint16_t MagnetsHandler_GetStatus(MagnetName Name);
bool MagnetsHandler_IsMagnetDetected(void);
void MagnetsHandler_ResetDetectionFlag(void);
uint32_t MagnetsHandler_FlippedSensorBarReverseBits(uint32_t num);
uint32_t MagnetsHandler_GetSatus();
float MagnetsHandler_GetAvreageDistance();

#endif /* INC_MAGNETSHANDLER_C_ */
