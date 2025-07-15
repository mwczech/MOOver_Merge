/*
 * ConnectivityHandler.c
 *
 *  Created on: Jan 9, 2024
 *      Author: OlafKołodziejczyk(25
 */
#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "ConnectivityHandler.h"
#include "UartHandler.h"
#include "math.h"
#include "MessageTypes.h"
#include "CRC16.h"
#include "IMU_func.h"
#include "RoutesDataTypes.h"

Esp2ImuFrame_t Esp2ImuFrame;

Route_ID SelectedRoute = RouteA;


void connectivityHandlerInit()
{

}


void connectivityHandlerPerform()
{
	connectivityHandlerRecieveData();

}

void connectivityHandlerRecieveData()
{
	if (UartHandler_IsDataReceived(Uart_ConnectivityESP))
	{
		UartHandler_GetRxBuffer(Uart_ConnectivityESP, (uint8_t *)&Esp2ImuFrame, sizeof(Esp2ImuFrame_t));
		if (Esp2ImuFrame.crc == CRC16((uint8_t *)&Esp2ImuFrame, sizeof(Esp2ImuFrame_t) - sizeof(Esp2ImuFrame.crc)))
		{
			if(Esp2ImuFrame.rootNumber == 0)
				SelectedRoute = RouteA;
			else if(Esp2ImuFrame.rootNumber == 1)
				SelectedRoute = RouteB;
			else if(Esp2ImuFrame.rootNumber == 2)
				SelectedRoute = RouteC;
			else if(Esp2ImuFrame.rootNumber == 3)
				SelectedRoute = RouteD;
		}
		UartHandler_ReloadReceiveChannel(Uart_ConnectivityESP);
	}
}


int8_t getJoystickX()
{
	return Esp2ImuFrame.moveX;
}

int8_t getJoystickY()
{
	return Esp2ImuFrame.moveY;
}

int16_t getThumbleSetting()
{
	return Esp2ImuFrame.augerSpeed;
}

Route_ID getSelectedRoute()
{
	return SelectedRoute;
}

uint8_t getRouteAction(void)
{
	return Esp2ImuFrame.rootAction;
}


