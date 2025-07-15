/*
 * Navigation.h
 *
 *  Created on: May 26, 2025
 *      Author: kolodzio
 */

#ifndef INC_NAVIGATION_H_
#define INC_NAVIGATION_H_
#include "RoutesDataTypes.h"
#include "stdbool.h"

typedef struct Point_t{
   int X;
   int Y;
   uint16_t stepNumber;
} Point;



void navigationInit(void);
void loadRoute(Route_ID RouteSelected);
void navigationPerform1ms(void);
bool isRouteFinnished(void);
uint16_t getCurrentStep(void);
uint8_t getRouteProgressPercentage(void);
Point calculatePoint(float angle, int originX, int originY, float offsetX, float offsetY);
uint16_t findNearestMagnet(Point point);
void updatePosition(void);
float angleWrap(float angle);
void manualNavigation(void);

#endif /* INC_NAVIGATION_H_ */
