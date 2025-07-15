/*
 * RoutesDataTypes.h
 *
 *  Created on: May 26, 2025
 *      Author: kolodzio
 */

#ifndef INC_ROUTESDATATYPES_H_
#define INC_ROUTESDATATYPES_H_

#include <stdint.h>


#define TH_ON 1500
#define TH_OFF 0

#define dMAGNET_R5 5*2.17
#define dMAGNET_R10 10*2.17
#define dMAGNET_L5 -5*2.17
#define dMAGNET_L10 -10*2.17
#define dMAGNET_MID 0
#define dMAGNET_L1 -1*2.17
#define dMAGNET_L2 -2*2.17
#define dMAGNET_L3 -3*2.17
#define dMAGNET_L4 -4*2.17
#define dMAGNET_L6 -6*2.17
#define dMAGNET_L7 -7*2.17


void RouteManager_StateMachine(void);


typedef enum Route_ID_t{
    RouteA = 0,
    RouteB,
    RouteC,
    RouteD,
    Route_NumOf
}Route_ID;

typedef enum RouteStates_t
{
    RouteState_Init = 0,
    RouteState_Idle,
    RouteState_Pause,
    RouteState_BuzzerLampInication,
    RouteState_Drive,
} RouteStates;

typedef struct RouteStep_t{
    int dX;            /* X coord change */
    int dY;            /* Y coord change */
    int16_t Speed;         /* Speed */
    uint16_t ThumbleSpeed; /* Thumble speed */
    float MagnetCorrectionOffset;   /* Correction of angle when magnet bar detection is not in the middle */
} RouteStep;

typedef struct RouteData_t{
    Route_ID ID;
    uint16_t StepCount;
    const RouteStep* Step;
}RouteData;

void Route_SetRoutePointer(RouteData* Data ,Route_ID RouteSelected);

#endif /* INC_ROUTESDATATYPES_H_ */
