/*
 * UartHandler.h
 *
 *  Created on: Dec 17, 2022
 *      Author: pawelton
 */

#ifndef INC_UARTHANDLER_H_
#define INC_UARTHANDLER_H_

#define UART_MESSAGE_ACK 0
#define UART_MESSAGE_ENCODER 1

typedef enum
{
	Uart_ConnectivityESP = 0, //UART1 do esp
	Uart_PMB = 1, //UART2 do pmb
	Uart_3 = 2, //UART3 do debugowania
	Uart_5 = 3, //UART5 do belki
	Uart_NumOf
}UartName;

#include <stdint.h>
#include <stdbool.h>

//void UartHandler_SendMessage(uint8_t MessageID);
void UartHandler_SendMessage(UartName UartNum, char* Message, uint8_t Len);
int16_t UartHandler_GetValueFromBuffer(UartName Uart, uint8_t Offset,  uint8_t Size);
void UartHandler_GetRxBuffer(UartName Uart, uint8_t* dest, uint8_t Size);
void UartHandler_SetMessage(char *Message, char *Type);
void UartHandler_SetMessageReceived(UartName Uart);
void UartHandler_ResetMessageReceived(UartName Uart);
bool UartHandler_IsDataReceived(UartName Uart);
void UartHandler_SetDataRequest(UartName Uart, bool State);
uint32_t UartHandler_GetSendBufferAddress(UartName UartNum);
uint32_t UartHandler_GetReceiveBufferAddress(UartName UartNum);
void UartHandler_ReloadReceiveChannel(UartName Uart);
void UartHandler_ClearRXBuffer(UartName Uart);
void UartHandler_Check_Overrun();


#endif /* INC_UARTHANDLER_H_ */
