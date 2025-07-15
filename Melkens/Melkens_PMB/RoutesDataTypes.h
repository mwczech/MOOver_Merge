#ifndef ROUTES_DATA_TYPES
#define ROUTES_DATA_TYPES

#include <stdint.h>

#define L_FOR 1
#define L_REV 2
#define R_REV 1
#define R_FOR 2
#define TH_ON 1
#define TH_OFF 0

#define dMAGNET_NO_CORRECTION 255.0
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



typedef enum OpType_t{
    NORM = 1,
    TU_L,
    TU_R,
    L_90,
    R_90,
    DIFF,
    NORM_NOMAGNET,
    OpTypeNoOperation
}OperType;

typedef enum Route_ID_t{
    RouteA = 0,
    RouteB,
    RouteC,
    RouteD,
    RouteE,
    RouteF,
    RouteG,
    RouteH,
    RouteI,
    RouteJ,
    RouteK,
    Route_NumOf        
}Route_ID;

typedef struct RouteStep_t{
    uint8_t OperationType;  /* MODE */
    uint16_t dX;            /* X coord change */
    uint16_t dY;            /* Y coord change */
    uint16_t RightSpeed;    /* Left wheel speed */
    uint16_t LeftSpeed;     /* Right wheel speed */
    uint8_t DirectionRight; /* Wheel spin direction */
    uint8_t DirectionLeft;  /* Wheel spin direction */
    uint8_t ThumbleEnabled; /* Enable/Disable flag for thumble motor */
    float  Angle;
    float MagnetCorrection;   /* Correction of angle when magnet bar detection is not in the middle */
} RouteStep;

typedef struct RouteData_t{
    Route_ID ID;
    uint8_t RepeatCount;
    uint8_t StepCount;
    uint8_t CurrentStepCount;
    const RouteStep* Step;
}RouteData;

void Route_SetRoutePointer(RouteData* Data ,Route_ID RouteSelected, uint8_t Offset);


#endif