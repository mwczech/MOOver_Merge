/*
 * MagnetsHandler.c
 *
 *  Created on: 6 sty 2024
 *      Author: ciaptak
 */

#include "MagnetsHandler.h"
#include "UartHandler.h"
#include <string.h>
#include "stdbool.h"
//#include "Timer.h"

static char GetMagnetDataMessage[] = "S1\r\n";\



typedef union
{
  struct
  {
   uint32_t  bit0 : 1;
   uint32_t  bit1 : 1;
   uint32_t  bit2 : 1;
   uint32_t  bit3 : 1;
   uint32_t  bit4 : 1;
   uint32_t  bit5 : 1;
   uint32_t  bit6 : 1;
   uint32_t  bit7 : 1;
   uint32_t  bit8 : 1;
   uint32_t  bit9 : 1;
   uint32_t  bit10 : 1;
   uint32_t  bit11 : 1;
   uint32_t  bit12 : 1;
   uint32_t  bit13 : 1;   
   uint32_t  bit14 : 1;
   uint32_t  bit15 : 1;
   uint32_t  bit16 : 1;
   uint32_t  bit17 : 1;
   uint32_t  bit18 : 1;
   uint32_t  bit19 : 1;
   uint32_t  bit20 : 1;
   uint32_t  bit21 : 1;
   uint32_t  bit22 : 1;
   uint32_t  bit23 : 1;
   uint32_t  bit24 : 1;
   uint32_t  bit25 : 1;
   uint32_t  bit26 : 1;
   uint32_t  bit27 : 1;
   uint32_t  bit28 : 1;
   uint32_t  bit29 : 1;
   uint32_t  bit30 : 1;
   uint32_t  bit31 : 1;
  }u;
  uint32_t status;
}MagnetsStatus;



typedef struct MagnetData_t{
	bool Sampled;
	uint16_t RawX;
	uint16_t RawY;
}MagnetData;

//Timer MagnetResponseTimeout;
MagnetName CurrentMagnet;
MagnetsStatus MagnetStatus;
MagnetsStatus MagnetStatusPrev;
uint32_t MagnetsPayload;
bool MagnetDetected;

bool MagnetDataReceived;

void SendMagnetsDataRequest(void);
void MagnetsHandler_EvaluateNewData(uint32_t Data);

void MagnetsHandler_Init(void){
	//Timer_SetCounter(&MagnetResponseTimeout, dMAGNET_RESPONSE_TIMEOUT);
	CurrentMagnet = Magnet1;
}

void SendMagnetsDataRequest(void){
	UartHandler_ReloadReceiveChannel(Uart_5);
	UartHandler_SendMessage(Uart_5, &GetMagnetDataMessage[0], 4);
}
#define OFFSET_POS 3
#define OFFSET_NEG 4
uint16_t CounterToResend= 0;
uint16_t CounterToMagnetsError = 0;
uint16_t sSign = 0;
uint16_t nSign = 0;
uint16_t rSign = 0;
void MagnetsHandler_Perform1ms(void){
	if( UartHandler_IsDataReceived(Uart_5) ){
		MagnetDataReceived = true;
		CounterToMagnetsError = 0;
	}
	if( MagnetDataReceived ){
		MagnetsPayload = 0;
		MagnetDataReceived = false;
		// Zapisanie sSign = zerowy odebrany bit
		sSign=UartHandler_GetValueFromBuffer(Uart_5, 0, 1);
		// Zapisanie rSign = 10 odebrany bit
		rSign=UartHandler_GetValueFromBuffer(Uart_5, 10, 1);
		// Zapisanie nSign = 11 odebrany bit
		nSign=UartHandler_GetValueFromBuffer(Uart_5, 11, 1);

		// Sprawdzenie poprawnosci formatu transmisji
		if(sSign == 83 && rSign == 13   && nSign == 10){
			uint16_t MagnetsHigh, MagnetsLow;
			//MagnetsPayload = UartHandler_GetValueFromBuffer(Uart_5, OFFSET_POS, 4);
			MagnetsLow = UartHandler_GetValueFromBuffer(Uart_5, OFFSET_POS, 2);
			MagnetsHigh = UartHandler_GetValueFromBuffer(Uart_5, OFFSET_POS+2, 2);
			MagnetsPayload = (uint32_t)((uint32_t)MagnetsHigh<<16 | (uint32_t)MagnetsLow);

			MagnetsPayload = MagnetsHandler_FlippedSensorBarReverseBits(MagnetsPayload);
			MagnetsPayload >>= 1;
		}
			/* TODO: pawelton Dopisac negowanie wartosci */
			//Magnets.status =

		MagnetsHandler_EvaluateNewData(MagnetsPayload);
		UartHandler_ClearRXBuffer(Uart_5);
		UartHandler_ResetMessageReceived(Uart_5);
		SendMagnetsDataRequest();

	}
	else{
		CounterToResend ++;
		if(CounterToResend == 2){
			CounterToResend = 0;
			SendMagnetsDataRequest();
		}
		CounterToMagnetsError++;
		if( CounterToMagnetsError == 50 ){
		/* 10s since last message from magnets bar */
			MagnetStatus.status = 0xA5A5A5A5;
			CounterToMagnetsError = 0;
		}
	}
}

void MagnetsHandler_Perform10ms(void){
	if( 1 ){

	}
	else{

	}

}

uint32_t MagnetsHandler_GetSatus(){

	return MagnetStatus.status;
}


bool MagnetsHandler_GetDataReceivedFlag(void){
	return true;
}

void MagnetsHandler_EvaluateNewData(uint32_t Data)
{
	MagnetStatus.status = Data;
	if(MagnetStatus.status == 0){

	}
	else{
		MagnetDetected = true;
		if(MagnetStatus.status != MagnetStatusPrev.status){

		}
	}
	MagnetStatusPrev.status = Data;

}

bool MagnetsHandler_IsMagnetDetected(void){
	return MagnetDetected;
}


void MagnetsHandler_ResetDetectionFlag(void){
	MagnetDetected = false;
}

uint32_t MagnetsHandler_FlippedSensorBarReverseBits(uint32_t num)
{
	uint32_t reversed = 0;
	for (int i = 0; i < 32; i++)
	{
		reversed = (reversed << 1) | (num & 1);
		num >>= 1;
	}
	return reversed;
}

float MagnetsHandler_GetAvreageDistance()
{
	bool magnets[32] = {
			MagnetStatus.u.bit0,
			MagnetStatus.u.bit1,
			MagnetStatus.u.bit2,
			MagnetStatus.u.bit3,
			MagnetStatus.u.bit4,
			MagnetStatus.u.bit5,
			MagnetStatus.u.bit6,
			MagnetStatus.u.bit7,
			MagnetStatus.u.bit8,
			MagnetStatus.u.bit9,
			MagnetStatus.u.bit10,
			MagnetStatus.u.bit11,
			MagnetStatus.u.bit12,
			MagnetStatus.u.bit13,
			MagnetStatus.u.bit14,
			MagnetStatus.u.bit15,
			MagnetStatus.u.bit16,
			MagnetStatus.u.bit17,
			MagnetStatus.u.bit18,
			MagnetStatus.u.bit19,
			MagnetStatus.u.bit20,
			MagnetStatus.u.bit21,
			MagnetStatus.u.bit22,
			MagnetStatus.u.bit23,
			MagnetStatus.u.bit24,
			MagnetStatus.u.bit25,
			MagnetStatus.u.bit26,
			MagnetStatus.u.bit27,
			MagnetStatus.u.bit28,
			MagnetStatus.u.bit29,
			MagnetStatus.u.bit30,
			MagnetStatus.u.bit31
					};

//	for (int i = 0; i < 32; i++) //test this method
//	{
//		magnet[i] = (MagnetStatus.status >> i) & 0x1;
//	}

	uint8_t detectedPositions = 0;
	uint16_t positionSum = 0;
	float avreagePosition = 0;
	float avreageDistance = 0;

	for(int i = 0; i<32; i++)
	{
		if(magnets[i])
		{
			positionSum += i;
			detectedPositions++;
		}
	}

	avreagePosition = positionSum/detectedPositions - 16;
	avreageDistance = avreagePosition * dDISTANCE_BETWEEN_SENSORS;

	return avreageDistance;
}


