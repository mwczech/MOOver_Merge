/*
 * Navigation.c
 *
 *  Created on: May 26, 2025
 *      Author: kolodzio
 */


#include "Navigation.h"
#include "RoutesDataTypes.h"
#include "math.h"
#include "IMU_func.h"
#include "main.h"
#include "MagnetsHandler.h"
#include "ConnectivityHandler.h"

float Robot_X, Robot_Y;
float Robot_Angle;

#define ROUTE_POINTS_DISTANCE 10 // each step is divided into smaller sections with distance in between of 10 cm for example

uint8_t pursuitPointIncrement = 5; //robot will pursuit a point that is located ahead for example 5 points
uint16_t closestPoint = 0; //route point that is the closest
uint16_t pursuitPoint = 0;// index of a point that robot will pursuit

uint16_t routePointsAmount;
#define ROUTE_POINTS_MAX_AMOUNT 10000
Point RoutePoints[ROUTE_POINTS_MAX_AMOUNT];

uint16_t magnetPointsAmount;
#define MAGNET_POINTS_MAX_AMOUNT 500
Point MagnetPoints[MAGNET_POINTS_MAX_AMOUNT];

int last_enco_left_val = 0;
int last_enco_right_val = 0;

int left_wheel_distance = 0;
int right_wheel_distance = 0;

float X_POSITION_ENCO = 0;
float Y_POSITION_ENCO = 0;

float moover_velocity = 0;

bool isRouteFinished;

float angleToPoint;
float DeltaAngle;

RouteData CurrentRoute;

float RouteStartAngle = 0;

void navigationInit(void){
Robot_X = 0;
Robot_Y = 0;
Robot_Angle = 0;

routePointsAmount = 0;
magnetPointsAmount = 0;

last_enco_left_val = getLeftEncoder();
last_enco_right_val = getRightEncoder();


}

void loadRoute(Route_ID RouteSelected)
{
	Route_SetRoutePointer(&CurrentRoute, RouteSelected);

	RouteStartAngle = angleWrap(getRobotAngle()-3.1415);

	Robot_X = 0;
	Robot_Y = 0;
	closestPoint = 0;
	routePointsAmount = 0;

	isRouteFinished = false;

	magnetPointsAmount = CurrentRoute.StepCount+1;
	int x = 0;
	int y = 0;

	MagnetPoints[0].X = x;
	MagnetPoints[0].Y = y;
	MagnetPoints[0].stepNumber = 0;

	for(int i = 0; i<CurrentRoute.StepCount; i++)//fill all the magnet points
	{
		x += CurrentRoute.Step[i].dX;
		y += CurrentRoute.Step[i].dY;

		MagnetPoints[i+1].X = x;
		MagnetPoints[i+1].Y = y;
		MagnetPoints[i+1].stepNumber = i;

	}

	int dx, dy;
	float stepDistance;
	int amountOfIntermediateSteps;
	int intermediateStepX, intermediateStepY;
	for(int i = 0; i<CurrentRoute.StepCount; i++)//fill all the Intermediate points
	{
		dx = MagnetPoints[i].X - MagnetPoints[i+1].X;
		dy = MagnetPoints[i].Y - MagnetPoints[i+1].Y;
		stepDistance = sqrt(dx*dx+dy*dy);
		amountOfIntermediateSteps = (int)(stepDistance / ROUTE_POINTS_DISTANCE);

		for(int a = 0; a<amountOfIntermediateSteps; a++)
		{
			intermediateStepX = MagnetPoints[i].X - (dx/amountOfIntermediateSteps) * a;
			intermediateStepY = MagnetPoints[i].Y - (dy/amountOfIntermediateSteps) * a;

			RoutePoints[routePointsAmount].X = intermediateStepX;
			RoutePoints[routePointsAmount].Y = intermediateStepY;
			RoutePoints[routePointsAmount].stepNumber = i;
			routePointsAmount++;
		}

	}

	RoutePoints[routePointsAmount].X = MagnetPoints[magnetPointsAmount-1].X;
	RoutePoints[routePointsAmount].Y = MagnetPoints[magnetPointsAmount-1].Y;
	RoutePoints[routePointsAmount].stepNumber = MagnetPoints[magnetPointsAmount-1].stepNumber;;
	routePointsAmount++;

	Robot_Angle = 3.1415;
}

void navigationPerform1ms(void)
{
	updatePosition();

	//if magnet was found
	if(MagnetsHandler_GetSatus() != 0)
	{
		Point detectionPoint = calculatePoint(Robot_Angle, Robot_X, Robot_Y, MagnetsHandler_GetAvreageDistance(), dMAGNET_BAR_OFFSET_DISTANCE);
		uint16_t closestMagnet = findNearestMagnet(detectionPoint);

		int dx = detectionPoint.X - MagnetPoints[closestMagnet].X;
		int dy = detectionPoint.Y - MagnetPoints[closestMagnet].Y;

		//Robot_X += dx;
		//Robot_Y += dy;//TODO: sprawdzić czy to jest ok

	}

	Robot_Angle = angleWrap(getRobotAngle()-RouteStartAngle);

	int dx, dy;
	float lastDistanceToPoint;
	float distanceToPoint;
	uint8_t searchRange; // closest point search range

	dx = RoutePoints[0].X - Robot_X;
	dy = RoutePoints[0].Y - Robot_Y;

	lastDistanceToPoint = sqrt(dx*dx+dy*dy);

	if(closestPoint+pursuitPointIncrement > routePointsAmount)
		searchRange = routePointsAmount;
	else
		searchRange = closestPoint+pursuitPointIncrement;

	for(int i = closestPoint; i<searchRange; i++)
	{
		dx = RoutePoints[i].X - Robot_X;
		dy = RoutePoints[i].Y - Robot_Y;

		distanceToPoint = sqrt(dx*dx+dy*dy);

		if(distanceToPoint <= lastDistanceToPoint)
		{
			closestPoint = i;
			lastDistanceToPoint = distanceToPoint;
		}
	}

	if(closestPoint < routePointsAmount - pursuitPointIncrement - 1)
		pursuitPoint = closestPoint + pursuitPointIncrement;
	else
		pursuitPoint = routePointsAmount - 1;


	//calculate angle to pursuit point
//	float angleToPoint;
//	float DeltaAngle;

	int speed = CurrentRoute.Step[RoutePoints[closestPoint].stepNumber].Speed;

	dx = RoutePoints[pursuitPoint].X - Robot_X;
	dy = RoutePoints[pursuitPoint].Y - Robot_Y;

	angleToPoint = atan2(dx, dy) - 3.1415/2;

	if(speed<0)//if going back
		angleToPoint += 3.1415;

	DeltaAngle = angleWrap(Robot_Angle - angleToPoint - 3.1415);

	//set wheel speed according to calculated position and angle

	if(DeltaAngle>3.1415/8)
		DeltaAngle = 3.1415/8;
	if(DeltaAngle<-3.1415/8)
		DeltaAngle = -3.1415/8;

	if(speed>0)
	{
		setRightWheelSpeed(speed-((DeltaAngle/(3.1415/8))*speed));
		setLeftWheelSpeed(speed+((DeltaAngle/(3.1315/8))*speed));
	}
	else
	{
		setRightWheelSpeed(speed+((DeltaAngle/(3.1415/8))*speed));
		setLeftWheelSpeed(speed-((DeltaAngle/(3.1315/8))*speed));
	}


	setThumbleSpeed(CurrentRoute.Step[RoutePoints[closestPoint].stepNumber].ThumbleSpeed);//set thumble speed

	if(closestPoint == routePointsAmount-1)//finish route
	{
		setRightWheelSpeed(0);
		setLeftWheelSpeed(0);
		setThumbleSpeed(0);
		isRouteFinished = true;
	}


}

float angleWrap(float angle)
{
	 // Zawijanie do (-pi, pi]
	    angle = fmod(angle + 3.1415, 2 * 3.1415);
	    if (angle < 0)
	    	angle += 2 * 3.1415;
	    return angle - 3.1415;
}

Point calculatePoint(float angle, int originX, int originY, float offsetX, float offsetY)
{
	Point calculatedPosition;
	calculatedPosition.X = originX + sin(angle) * offsetX + sin(angle+3.1415/2) * offsetY;
	calculatedPosition.Y = originY + cos(angle) * offsetX + cos(angle+3.1415/2) * offsetY;

	return calculatedPosition;
}

uint16_t findNearestMagnet(Point point)
{
	int dx, dy;
	float closestDistanceToMagnet;
	float distanceToMagnet;
	uint16_t closestMagnet;

	dx = MagnetPoints[0].X - Robot_X;
	dy = MagnetPoints[0].Y - Robot_Y;

	closestDistanceToMagnet = sqrt(dx*dx+dy*dy);

	for(int i = 0; i<magnetPointsAmount; i++)
	{
		dx = MagnetPoints[i].X - Robot_X;
		dy = MagnetPoints[i].Y - Robot_Y;

		distanceToMagnet = sqrt(dx*dx+dy*dy);

		if(distanceToMagnet <= closestDistanceToMagnet)
		{
			closestMagnet = i;
			closestDistanceToMagnet = distanceToMagnet;
		}
	}

	return closestMagnet; // returns closest magnet index
}

void updatePosition()
{

	int increment_L = getLeftEncoder() - last_enco_left_val;
	int increment_R = getRightEncoder() - last_enco_right_val;

	if(increment_L < -5000)
		increment_L += 10000;
	else if(increment_L > 5000)
		increment_L -= 10000;

	if(increment_R < -5000)
		increment_R += 10000;
	else if(increment_R > 5000)
		increment_R -= 10000;

	last_enco_left_val = getLeftEncoder();
	last_enco_right_val = getRightEncoder();

	moover_velocity = -(increment_R-increment_L) * 0.5 * 0.00018996; //pomnożyć przez jakąć stałą żeby się dystans zgadzał
	//moover_velocity = (getLeftWheelSpeed()+getRightWheelSpeed())*0.5*0.00001;//pomnożyć przez jakąć stałą żeby się dystans zgadzał

	left_wheel_distance += increment_L;
	right_wheel_distance += increment_R;

	int wheel_distane_diff = left_wheel_distance - right_wheel_distance;
	float moover_angle_encoders = 3.1415 / 227 * wheel_distane_diff;

	float velocity_x = moover_velocity * cos(getRobotAngle()-RouteStartAngle);
	float velocity_y = moover_velocity * sin(getRobotAngle()-RouteStartAngle);

	Robot_X -= velocity_x;
	Robot_Y += velocity_y;

	velocity_x = moover_velocity * cos(moover_angle_encoders);
	velocity_y = moover_velocity * sin(moover_angle_encoders);

	X_POSITION_ENCO += velocity_x;
	Y_POSITION_ENCO += velocity_y;



	setDebugDataPoint1((int)Robot_X, (int)Robot_Y);
}

void manualNavigation(void)
{
	updatePosition();

	int leftSpeed = getJoystickY()+getJoystickX();
	int rightSpeed = getJoystickY()-getJoystickX();

	leftSpeed *= 5;
	rightSpeed *= 5;

	setRightWheelSpeed(rightSpeed);
	setLeftWheelSpeed(leftSpeed);
	setThumbleSpeed(getThumbleSetting());
}

bool isRouteFinnished(void)
{
	return isRouteFinished;
}

uint16_t getCurrentStep(void)
{
	return RoutePoints[closestPoint].stepNumber;
}
uint8_t getRouteProgressPercentage(void)
{
	return routePointsAmount/closestPoint*100;
}
