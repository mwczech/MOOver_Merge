/**
  System Interrupts Generated Driver File 

  @Company:
    Microchip Technology Inc.

  @File Name:
    interrupt_manager.h

  @Summary:
    This is the generated driver implementation file for setting up the
    interrupts using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description:
    This source file provides implementations for PIC24 / dsPIC33 / PIC32MM MCUs interrupts.
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
    Section: Includes
*/
#include <xc.h>

/**
    void INTERRUPT_Initialize (void)
*/
void INTERRUPT_Initialize (void)
{
    //    CI: CAN 1 Combined Error
    //    Priority: 1
        IPC6bits.C1IP = 1;
    //    PEVTCI: PWM EVENT C
    //    Priority: 1
        IPC42bits.PEVTCIP = 1;
    //    PWM6I: PWM Generator 6
    //    Priority: 1
        IPC18bits.PWM6IP = 1;
    //    PWM4I: PWM Generator 4
    //    Priority: 1
        IPC17bits.PWM4IP = 1;
    //    PWM3I: PWM Generator 3
    //    Priority: 1
        IPC17bits.PWM3IP = 1;
    //    PWM2I: PWM Generator 2
    //    Priority: 1
        IPC17bits.PWM2IP = 1;
    //    PWM1I: PWM Generator 1
    //    Priority: 1
        IPC16bits.PWM1IP = 1;
    //    UEVTI: UART2 Event
    //    Priority: 1
        IPC47bits.U2EVTIP = 1;
    //    UTXI: UART2 TX
    //    Priority: 1
        IPC7bits.U2TXIP = 1;
    //    UEI: UART2 Error
    //    Priority: 1
        IPC12bits.U2EIP = 1;
    //    URXI: UART2 RX
    //    Priority: 1
        IPC6bits.U2RXIP = 1;
    //    ADCAN24: ADC AN24 Convert Done
    //    Priority: 1
        IPC48bits.ADCAN24IP = 1;
    //    ADCAN25: ADC AN25 Convert Done
    //    Priority: 1
        IPC48bits.ADCAN25IP = 1;
    //    UEVTI: UART1 Event
    //    Priority: 1
        IPC47bits.U1EVTIP = 1;
    //    UTXI: UART1 TX
    //    Priority: 1
        IPC3bits.U1TXIP = 1;
    //    UEI: UART1 Error
    //    Priority: 1
        IPC12bits.U1EIP = 1;
    //    URXI: UART1 RX
    //    Priority: 1
        IPC2bits.U1RXIP = 1;
    //    INT3I: External Interrupt 3
    //    Priority: 1
        IPC6bits.INT3IP = 1;
    //    INT1I: External Interrupt 1
    //    Priority: 1
        IPC3bits.INT1IP = 1;
    //    INT2I: External Interrupt 2
    //    Priority: 1
        IPC5bits.INT2IP = 1;
    //    MICI: I2C1 Master Event
    //    Priority: 1
        IPC4bits.MI2C1IP = 1;
    //    SICI: I2C1 Slave Event
    //    Priority: 1
        IPC4bits.SI2C1IP = 1;
    //    TI: Timer 1
    //    Priority: 1
        IPC0bits.T1IP = 1;
}
