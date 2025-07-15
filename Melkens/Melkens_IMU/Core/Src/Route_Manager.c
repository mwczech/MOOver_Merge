/*
 * Route_Manager.c
 *
 *  Created on: May 26, 2025
 *      Author: kolodzio
 */

#include "RoutesDataTypes.h"
#include "Navigation.h"
#include "routeManager.h"
#include "ConnectivityHandler.h"

RouteStates RouteState;

void RouteManager_Init(void)
{
    RouteState = RouteState_Idle;
    navigationInit();

}

void RouteManager_Perform1ms(void)
{

	RouteManager_StateMachine();

}

void RouteManager_StateMachine(void){


    switch (RouteState){
    case RouteState_Init:


    	loadRoute(getSelectedRoute());


    	//RouteState = RouteState_Idle;
    	RouteState = RouteState_Drive;
        break;
    case RouteState_Idle:

    	manualNavigation();

    	if(getRouteAction()==2)
    		RouteState = RouteState_Init;

        break;

    case RouteState_BuzzerLampInication:

        break;
    case RouteState_Pause:
    	manualNavigation();

    	if(getRouteAction()==2)
    	    RouteState = RouteState_Drive;
        break;
    case RouteState_Drive:

    	navigationPerform1ms();

    	if(isRouteFinnished())
    		RouteState = RouteState_Idle;
    	if(getRouteAction()==0)
    		RouteState = RouteState_Idle;
    	if(getRouteAction()==1)
    		RouteState = RouteState_Pause;

        break;
    default:
        break;
    }
}


