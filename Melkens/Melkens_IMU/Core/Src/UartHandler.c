/*
 * UartHandler.c
 *
 *  Created on: Dec 17, 2022
 *      Author: pawelton
 */

#include "UartHandler.h"
#include "DmaHandler.h"
#include "DataTypes.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_usart.h"

const char *Ack_Message = "INFO_ACK";
//const char *Get_Enco_Message = "GET_ENCO";

//char Send_Buff_UART1[8];
char Send_Buff_UART1[UART1_TX_MESSAGE_LEN];
char Send_Buff_UART2[UART2_TX_MESSAGE_LEN];//roll, pitch yaw, x, y
char Send_Buff_UART3[UART3_TX_MESSAGE_LEN];
char Send_Buff_UART5[UART5_TX_MESSAGE_LEN];

char Receive_Buff_UART1[UART1_RX_MESSAGE_LEN];
char Receive_Buff_UART2[UART2_RX_MESSAGE_LEN];
char Receive_Buff_UART3[UART3_RX_MESSAGE_LEN];
char Receive_Buff_UART5[UART5_RX_MESSAGE_LEN];

bool UartHandler_MessageReceivedFlag_UART1;
bool UartHandler_MessageReceivedFlag_UART2;
bool UartHandler_MessageReceivedFlag_UART3;
bool UartHandler_MessageReceivedFlag_UART5;

uint16_t TransmitErrorCnt;


void UartHandler_Check_Overrun()
{
//czy po każdym wykrytym overrunie należy wywołać UartHandler_ReloadReceiveChannel?
	 if(LL_USART_IsActiveFlag_ORE(LPUART1)){
		LL_USART_ClearFlag_ORE(LPUART1);
		TransmitErrorCnt++;
	}

	if(LL_USART_IsActiveFlag_ORE(USART2)){
		LL_USART_ClearFlag_ORE(USART2);
		TransmitErrorCnt++;
	}

	if(LL_USART_IsActiveFlag_ORE(USART3)){
		LL_USART_ClearFlag_ORE(USART3);
		TransmitErrorCnt++;
	}

	if(LL_USART_IsActiveFlag_ORE(UART5)){
		LL_USART_ClearFlag_ORE(UART5);
		TransmitErrorCnt++;
	}
}

void UartHandler_ResetMessageReceived(UartName Uart){

	switch(Uart)
		{
			case Uart_ConnectivityESP:
				UartHandler_MessageReceivedFlag_UART1 = false;
				break;
			case Uart_PMB:
				UartHandler_MessageReceivedFlag_UART2 = false;
				break;
			case Uart_3:
				UartHandler_MessageReceivedFlag_UART3 = false;
				break;
			case Uart_5:
				UartHandler_MessageReceivedFlag_UART5 = false;
				break;
			default:
				break;
		}

}

void UartHandler_SetMessageReceived(UartName Uart)
{

	switch(Uart)
	{
		case Uart_ConnectivityESP:
			UartHandler_MessageReceivedFlag_UART1 = true;
			break;
		case Uart_PMB:
			UartHandler_MessageReceivedFlag_UART2 = true;
			break;
		case Uart_3:
			UartHandler_MessageReceivedFlag_UART3 = true;
			break;
		case Uart_5:
			UartHandler_MessageReceivedFlag_UART5 = true;
			break;
		default:
			break;
	}

}

void UartHandler_ClearRXBuffer(UartName Uart){
	switch(Uart)
	{
		case Uart_ConnectivityESP:
			memset(&Receive_Buff_UART1[0], 0, UART1_RX_MESSAGE_LEN);
			break;
		case Uart_PMB:
			memset(&Receive_Buff_UART2[0], 0, UART2_RX_MESSAGE_LEN);
			break;
		case Uart_3:
			memset(&Receive_Buff_UART3[0], 0, UART3_RX_MESSAGE_LEN);
			break;
		case Uart_5:
			memset(&Receive_Buff_UART5[0], 0, UART5_RX_MESSAGE_LEN);
			break;
		default:
			break;
	}
}

void UartHandler_ReloadReceiveChannel(UartName Uart)
{

	switch(Uart)
	{
		case Uart_ConnectivityESP:
			LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_1);
			LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, UART1_RX_MESSAGE_LEN);
			LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);

			UartHandler_ResetMessageReceived(Uart_ConnectivityESP);
			break;
		case Uart_PMB:
			LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_1);
			LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_1, UART2_RX_MESSAGE_LEN);
			LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_1);

			UartHandler_ResetMessageReceived(Uart_PMB);
			break;
		case Uart_3:
			LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
			LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, UART3_RX_MESSAGE_LEN);
			LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);

			UartHandler_ResetMessageReceived(Uart_3);
			break;
		case Uart_5:
			LL_DMA_DisableChannel(DMA2, LL_DMA_CHANNEL_3);
			LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_3, UART5_RX_MESSAGE_LEN);
			LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_3);

			UartHandler_ResetMessageReceived(Uart_5);
			break;
		default:
			break;
	}


}

/*void UartHandler_SetMessage(char *Message, char *Type)
{
	 N
    memcpy(&Send_Buff_UART3[0], Type, 1);
	 N;
    memcpy(&Send_Buff_UART3[1], MESSAGE_SEPARATOR, 1);
	 N;AAA;BBB;
    memcpy(&Send_Buff_UART3[2], Message, 8);
    memcpy(&Send_Buff_UART3[UART_MESSAGE_WHEELS_LEN -3],MESSAGE_CRC,3 );

}*/

void UartHandler_SetDataRequest(UartName Uart, bool State)
{
	switch(Uart)
	{
		case Uart_ConnectivityESP:
			UartHandler_MessageReceivedFlag_UART1 = State;
			break;
		case Uart_PMB:
			UartHandler_MessageReceivedFlag_UART2 = State;
			break;
		case Uart_3:
			UartHandler_MessageReceivedFlag_UART3 = State;
			break;
		case Uart_5:
			UartHandler_MessageReceivedFlag_UART5 = State;
			break;
		default:
			break;
	}


}

bool UartHandler_IsDataReceived(UartName Uart)
{
	switch(Uart)
	{
		case Uart_ConnectivityESP:
			return UartHandler_MessageReceivedFlag_UART1;
			break;
		case Uart_PMB:
			return UartHandler_MessageReceivedFlag_UART2;
			break;
		case Uart_3:
			return UartHandler_MessageReceivedFlag_UART3;
			break;
		case Uart_5:
			return UartHandler_MessageReceivedFlag_UART5;
			break;
		default:
			return false;
			break;
	}

}

int16_t UartHandler_GetValueFromBuffer(UartName Uart, uint8_t Offset,  uint8_t Size)
{
	int16_t RetVal;
	switch(Uart)
	{
        case Uart_ConnectivityESP:
            if(Size > 1)
                RetVal = ((uint8_t)Receive_Buff_UART1[Offset+1] << 8) | (uint8_t)Receive_Buff_UART1[Offset];
            else
                RetVal = (uint8_t)Receive_Buff_UART1[Offset];
            break;
        case Uart_PMB:
            if(Size > 1)
                RetVal = ((uint8_t)Receive_Buff_UART2[Offset+1] << 8) | (uint8_t)Receive_Buff_UART2[Offset];
            else
                RetVal = (uint8_t)Receive_Buff_UART2[Offset];
            break;
        case Uart_3:
            break;
        case Uart_5:
            if(Size > 1)
                RetVal = ((uint8_t)Receive_Buff_UART5[Offset+1] << 8) | (uint8_t)Receive_Buff_UART5[Offset];
            else
                RetVal = (uint8_t)Receive_Buff_UART5[Offset];
            break;
        default:
            break;
	}
	return RetVal;
}

void UartHandler_GetRxBuffer(UartName Uart, uint8_t* dest, uint8_t Size)
{
	switch(Uart)
	{
		case Uart_ConnectivityESP:
			memcpy(dest, Receive_Buff_UART1, Size);
			break;
		case Uart_PMB:
			memcpy(dest, Receive_Buff_UART2, Size);
			break;
		case Uart_3:
			break;
		case Uart_5:
			memcpy(dest, Receive_Buff_UART2, Size);
		default:
			break;
	}
}


/* TODO: Move LL_DMA... functions into DMA handler */
void UartHandler_SendMessage(UartName UartNum, char* Message, uint8_t Len)
{
	if( UartNum < Uart_NumOf)
	{
		if( Uart_ConnectivityESP == UartNum )
		{
			memcpy(&Send_Buff_UART1, Message, Len);

			LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
			LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, Len);
			LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
		}
		else if(  Uart_PMB == UartNum )
		{
			memcpy(&Send_Buff_UART2, Message, Len);

			LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_2, Len);
			LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_2);
			LL_USART_Enable(USART2);

		}
		else if(  Uart_3 == UartNum )
		{
			memcpy(&Send_Buff_UART3, Message, Len);
			LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, Len);
			LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);

		}
		else if(  Uart_5 == UartNum )
		{
			memcpy(&Send_Buff_UART5, Message, Len);
		    LL_DMA_SetDataLength(DMA2, LL_DMA_CHANNEL_4, Len);
		    LL_DMA_EnableChannel(DMA2, LL_DMA_CHANNEL_4);
		}

	}
	else
	{
		/* UartName out of bounds */
	}

}

uint32_t UartHandler_GetSendBufferAddress(UartName UartNum)
{
	if( UartNum < Uart_NumOf)
	{
		if( Uart_ConnectivityESP == UartNum )
		{
			return (uint32_t)&Send_Buff_UART1[0];
		}
		else if ( Uart_PMB == UartNum )
		{
			return (uint32_t)&Send_Buff_UART2[0];
		}
		else if ( Uart_3 == UartNum )
		{
			return (uint32_t)&Send_Buff_UART3[0];
		}
		else if ( Uart_5 == UartNum )
		{
			return (uint32_t)&Send_Buff_UART5[0];
		}

	}
	/* TODO: remove reset address */
	return (uint32_t)0;
}

uint32_t UartHandler_GetReceiveBufferAddress(UartName UartNum)
{
	if( UartNum < Uart_NumOf)
	{
		if( Uart_ConnectivityESP == UartNum )
		{
			return (uint32_t)&Receive_Buff_UART1[0];
		}
		else if ( Uart_PMB == UartNum )
		{
			return (uint32_t)&Receive_Buff_UART2[0];
		}
		else if ( Uart_3 == UartNum )
		{
			return (uint32_t)&Receive_Buff_UART3[0];
		}
		else if ( Uart_5 == UartNum )
		{
			return (uint32_t)&Receive_Buff_UART5[0];
		}

	}
	/* TODO: remove reset address */
	return (uint32_t)0;
}

