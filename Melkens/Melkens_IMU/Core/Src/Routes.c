/*
 * Routes.c
 *
 *  Created on: May 26, 2025
 *      Author: kolodzio
 */


#include "RoutesDataTypes.h"

#include <string.h>


#define STEPS_ROUTE_A 19
static const RouteStep RouteStepsA[STEPS_ROUTE_A] =
{
//       dx,  dy,  speed, thumble, offset
		{20, 0, 300, TH_OFF, 0},//0
		{20, 0, 300, TH_OFF, 0},//1
		{20, 0, 300, TH_OFF, 0},//2
		{50, 0, 300, TH_OFF, 0},//3
		{50, 0, 300, TH_OFF, 0},//4
		{50, 0, 300, TH_OFF, 0},//5
		{57, -9, 300, TH_OFF, 0},//6
		{57, -9, 300, TH_OFF, 0},//7
		{57, -9, 300, TH_OFF, 0},//8

		{-108, -95, -300, TH_OFF, 0},//9
		{-48, -34, -300, TH_OFF, 0},//10
		{-48, -34, -300, TH_OFF, 0},//11
		{-48, -34, -300, TH_OFF, 0},//12
		{-48, -34, -300, TH_OFF, 0},//13

		{76, -57, 300, TH_ON, 0},//14
		{60, 0, 300, TH_ON, 0},//15
		{50, 0, 300, TH_ON, 0},//16
		{50, 0, 300, TH_ON, 0},//17
		{50, 0, 300, TH_ON, 0},//18


};

#define STEPS_ROUTE_B 5
static const RouteStep RouteStepsB[STEPS_ROUTE_B] =
{
		{100, 0, 300, TH_ON, 0},
		{100, 0, 300, TH_ON, 0},
		{100, 0, 300, TH_ON, 0},
		{100, 0, 300, TH_ON, 0},
		{100, 0, 300, TH_ON, 0},

};

#define STEPS_ROUTE_C 4
static const RouteStep RouteStepsC[STEPS_ROUTE_C] =
{
		{100, 0, 200, TH_OFF, 0},
		{0, -100, 200, TH_OFF, 0},
		{-100, 0, 200, TH_OFF, 0},
		{0, 100, 200, TH_OFF, 0},

};



#define STEPS_ROUTE_D 5
static const RouteStep RouteStepsD[STEPS_ROUTE_D] =
{
		{-20, 0, -500, TH_OFF, 0},
		{-20, 0, -500, TH_OFF, 0},
		{-20, 0, -500, TH_OFF, 0},
		{-20, 0, -500, TH_OFF, 0},
		{-20, 0, -500, TH_OFF, 0},


};


static const RouteData Routes[Route_NumOf]   =
    {
     [RouteA] = {.ID = RouteA, .StepCount = STEPS_ROUTE_A, .Step = &RouteStepsA[0]},
     [RouteB] = {.ID = RouteB, .StepCount = STEPS_ROUTE_B, .Step = &RouteStepsB[0]},
     [RouteC] = {.ID = RouteC, .StepCount = STEPS_ROUTE_C, .Step = &RouteStepsC[0]},
     [RouteD] = {.ID = RouteD, .StepCount = STEPS_ROUTE_D, .Step = &RouteStepsD[0]}
    };

RouteData dummy2;

void Route_SetRoutePointer(RouteData* Data ,Route_ID RouteSelected){
    memcpy(Data, &Routes[RouteSelected], sizeof(RouteData));
//    Data->Step += Offset; /* no 0 step on display, but here steps are from 0 */
//    Data->CurrentStepCount = Offset;
}
