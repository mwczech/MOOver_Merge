/*
 * ConnectivityHandler.h
 *
 *  Created on: Jan 9, 2024
 *      Author: OlafKołodziejczyk(25
 */

#ifndef INC_CONNECTIVITYHANDLER_H_
#define INC_CONNECTIVITYHANDLER_H_

#include "RoutesDataTypes.h"

void connectivityHandlerPerform();
void connectivityHandlerInit();
void connectivityHandlerRecieveData();
int8_t getJoystickX(void);
int8_t getJoystickY(void);
int16_t getThumbleSetting(void);
Route_ID getSelectedRoute(void);
uint8_t getRouteAction(void);



#endif /* INC_CONNECTIVITYHANDLER_H_ */
