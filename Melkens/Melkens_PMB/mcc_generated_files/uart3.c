/**
  UART3 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    uart3.c

  @Summary
    This is the generated driver implementation file for the UART3 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides implementations for driver APIs for UART3.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.1
        Device            :  dsPIC33CK256MP506
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.70
        MPLAB             :  MPLAB X v5.50
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include <xc.h>
#include "uart3.h"

/**
  Section: UART3 APIs
*/

void UART3_Initialize(void)
{
/**    
     Set the UART3 module to the options selected in the user interface.
     Make sure to set LAT bit corresponding to TxPin as high before UART initialization
*/
    // URXEN disabled; RXBIMD RXBKIF flag when Break makes low-to-high transition after being low for at least 23/11 bit periods; UARTEN enabled; MOD Asynchronous 8-bit UART; UTXBRK disabled; BRKOVR TX line driven by shifter; UTXEN disabled; USIDL disabled; WAKE disabled; ABAUD disabled; BRGH enabled; 
    // Data Bits = 8; Parity = None; Stop Bits = 1 Stop bit sent, 1 checked at RX;
    U3MODE = (0x8080 & ~(1<<15));  // disabling UARTEN bit
    // STSEL 1 Stop bit sent, 1 checked at RX; BCLKMOD disabled; SLPEN disabled; FLO Off; BCLKSEL FOSC/2; C0EN disabled; RUNOVF disabled; UTXINV disabled; URXINV disabled; HALFDPLX disabled; 
    U3MODEH = 0x80;
    // OERIE disabled; RXBKIF disabled; RXBKIE disabled; ABDOVF disabled; OERR disabled; TXCIE disabled; TXCIF disabled; FERIE disabled; TXMTIE disabled; ABDOVE disabled; CERIE disabled; CERIF disabled; PERIE disabled; 
    U3STA = 0x00;
    // URXISEL RX_ONE_WORD; UTXBE enabled; UTXISEL TX_BUF_EMPTY; URXBE enabled; STPMD disabled; TXWRE disabled; 
    U3STAH = 0x22;
    // BaudRate = 9600; Frequency = 10000000 Hz; BRG 259; 
    U3BRG = 0x15;
    // BRG 0; 
    U3BRGH = 0x00;
    // P1 0; 
    U3P1 = 0x00;
    // P2 0; 
    U3P2 = 0x00;
    // P3 0; 
    U3P3 = 0x00;
    // P3H 0; 
    U3P3H = 0x00;
    // TXCHK 0; 
    U3TXCHK = 0x00;
    // RXCHK 0; 
    U3RXCHK = 0x00;
    // T0PD 1 ETU; PTRCL disabled; TXRPT Retransmit the error byte once; CONV Direct logic; 
    U3SCCON = 0x00;
    // TXRPTIF disabled; TXRPTIE disabled; WTCIF disabled; WTCIE disabled; BTCIE disabled; BTCIF disabled; GTCIF disabled; GTCIE disabled; RXRPTIE disabled; RXRPTIF disabled; 
    U3SCINT = 0x00;
    // ABDIF disabled; WUIF disabled; ABDIE disabled; 
    U3INT = 0x00;
    
    U3MODEbits.UARTEN = 1;   // enabling UART ON bit
    //U3MODEbits.UTXEN = 1;
   // U3MODEbits.URXEN = 1;
}

uint8_t UART3_Read(void)
{
    while((U3STAHbits.URXBE == 1))
    {
        
    }

    if ((U3STAbits.OERR == 1))
    {
        U3STAbits.OERR = 0;
    }
    
    return U3RXREG;
}

void UART3_Write(uint8_t txData)
{
    while(U3STAHbits.UTXBF == 1)
    {
        
    }

    U3TXREG = txData;    // Write the data byte to the USART.
}

bool UART3_IsRxReady(void)
{
    return (U3STAHbits.URXBE == 0);
}

bool UART3_IsTxReady(void)
{
    return ((!U3STAHbits.UTXBF) && U3MODEbits.UTXEN );
}

bool UART3_IsTxDone(void)
{
    return U3STAbits.TRMT;
}


/*******************************************************************************

  !!! Deprecated API !!!
  !!! These functions will not be supported in future releases !!!

*******************************************************************************/

uint32_t __attribute__((deprecated)) UART3_StatusGet (void)
{
    uint32_t statusReg = U3STAH;
    return ((statusReg << 16 ) | U3STA);
}

void __attribute__((deprecated)) UART3_Enable(void)
{
    U3MODEbits.UARTEN = 1;
    U3MODEbits.UTXEN = 1; 
    U3MODEbits.URXEN = 1;
}

void __attribute__((deprecated)) UART3_Disable(void)
{
    U3MODEbits.UARTEN = 0;
    U3MODEbits.UTXEN = 0; 
    U3MODEbits.URXEN = 0;
}


