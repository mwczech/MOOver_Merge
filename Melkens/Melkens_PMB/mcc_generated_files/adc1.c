/**
  ADC1 Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    adc1.c

  @Summary
    This is the generated driver implementation file for the ADC1 driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This source file provides APIs for ADC1.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.1
        Device            :  dsPIC33CK256MP506      
    The generated drivers are tested against the following:
        Compiler          :  XC16 v1.70
        MPLAB 	          :  MPLAB X v5.50
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

#include "adc1.h"

/**
 Section: File specific functions
*/

static void (*ADC1_CommonDefaultInterruptHandler)(void);
static void (*ADC1_DC_STATUS_HSDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_StatSw2DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_StatSw1DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_DC_STATUS_LSDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_IM_SENSEDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_StatSw3DefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_CHAR_ANDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_BAT_STATUSDefaultInterruptHandler)(uint16_t adcVal);
static void (*ADC1_StatSw4DefaultInterruptHandler)(uint16_t adcVal);

/**
  Section: Driver Interface
*/

void ADC1_Initialize (void)
{
    // ADSIDL disabled; ADON enabled; 
    ADCON1L = (0x8000 & 0x7FFF); //Disabling ADON bit
    // FORM Integer; SHRRES 12-bit resolution; 
    ADCON1H = 0x60;
    // PTGEN disabled; SHRADCS 2; REFCIE disabled; SHREISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; REFERCIE disabled; EIEN disabled; 
    ADCON2L = 0x00;
    // SHRSAMC 8; 
    ADCON2H = 0x08;
    // SWCTRG disabled; SHRSAMP disabled; SUSPEND disabled; SWLCTRG disabled; SUSPCIE disabled; CNVCHSEL AN0; REFSEL disabled; 
    ADCON3L = 0x00;
    // SHREN enabled; C1EN enabled; C0EN disabled; CLKDIV 1; CLKSEL FOSC; 
    ADCON3H = (0x4082 & 0xFF00); //Disabling C0EN, C1EN, C2EN, C3EN and SHREN bits
    // SAMC0EN disabled; SAMC1EN disabled; 
    ADCON4L = 0x00;
    // C0CHS AN0; C1CHS ANA1; 
    ADCON4H = 0x04;
    // SIGN0 disabled; SIGN4 disabled; SIGN3 disabled; SIGN2 disabled; SIGN1 disabled; SIGN7 disabled; SIGN6 disabled; DIFF0 disabled; SIGN5 disabled; DIFF1 disabled; DIFF2 disabled; DIFF3 disabled; DIFF4 disabled; DIFF5 disabled; DIFF6 disabled; DIFF7 disabled; 
    ADMOD0L = 0x00;
    // DIFF15 disabled; DIFF14 disabled; SIGN8 disabled; DIFF13 disabled; SIGN14 disabled; DIFF12 disabled; SIGN15 disabled; DIFF11 disabled; DIFF10 disabled; SIGN9 disabled; DIFF8 disabled; DIFF9 disabled; SIGN10 disabled; SIGN11 disabled; SIGN12 disabled; SIGN13 disabled; 
    ADMOD0H = 0x00;
    // DIFF19 disabled; DIFF18 disabled; DIFF17 disabled; DIFF16 disabled; SIGN16 disabled; SIGN17 disabled; SIGN18 disabled; SIGN19 disabled; 
    ADMOD1L = 0x00;
    // SIGN24 disabled; DIFF25 disabled; DIFF24 disabled; SIGN25 disabled; 
    ADMOD1H = 0x00;
    // IE15 disabled; IE1 disabled; IE0 disabled; IE3 disabled; IE2 disabled; IE5 disabled; IE4 disabled; IE10 disabled; IE7 disabled; IE6 disabled; IE9 disabled; IE13 disabled; IE8 disabled; IE14 disabled; IE11 disabled; IE12 disabled; 
    ADIEL = 0x00;
    // IE17 disabled; IE18 disabled; IE16 disabled; IE19 disabled; IE24 enabled; IE25 enabled; 
    ADIEH = 0x300;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN15 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; CMPEN13 disabled; 
    ADCMP0ENL = 0x00;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN15 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; CMPEN13 disabled; 
    ADCMP1ENL = 0x00;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN15 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; CMPEN13 disabled; 
    ADCMP2ENL = 0x00;
    // CMPEN10 disabled; CMPEN11 disabled; CMPEN6 disabled; CMPEN5 disabled; CMPEN4 disabled; CMPEN3 disabled; CMPEN2 disabled; CMPEN1 disabled; CMPEN0 disabled; CMPEN14 disabled; CMPEN9 disabled; CMPEN15 disabled; CMPEN8 disabled; CMPEN12 disabled; CMPEN7 disabled; CMPEN13 disabled; 
    ADCMP3ENL = 0x00;
    // CMPEN18 disabled; CMPEN19 disabled; CMPEN16 disabled; CMPEN17 disabled; CMPEN25 disabled; CMPEN24 disabled; 
    ADCMP0ENH = 0x00;
    // CMPEN18 disabled; CMPEN19 disabled; CMPEN16 disabled; CMPEN17 disabled; CMPEN25 disabled; CMPEN24 disabled; 
    ADCMP1ENH = 0x00;
    // CMPEN18 disabled; CMPEN19 disabled; CMPEN16 disabled; CMPEN17 disabled; CMPEN25 disabled; CMPEN24 disabled; 
    ADCMP2ENH = 0x00;
    // CMPEN18 disabled; CMPEN19 disabled; CMPEN16 disabled; CMPEN17 disabled; CMPEN25 disabled; CMPEN24 disabled; 
    ADCMP3ENH = 0x00;
    // CMPLO 0; 
    ADCMP0LO = 0x00;
    // CMPLO 0; 
    ADCMP1LO = 0x00;
    // CMPLO 0; 
    ADCMP2LO = 0x00;
    // CMPLO 0; 
    ADCMP3LO = 0x00;
    // CMPHI 0; 
    ADCMP0HI = 0x00;
    // CMPHI 0; 
    ADCMP1HI = 0x00;
    // CMPHI 0; 
    ADCMP2HI = 0x00;
    // CMPHI 0; 
    ADCMP3HI = 0x00;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL0CON = 0x400;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL1CON = 0x400;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL2CON = 0x400;
    // OVRSAM 4x; MODE Oversampling Mode; FLCHSEL AN0; IE disabled; FLEN disabled; 
    ADFL3CON = 0x400;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP0CON = 0x00;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP1CON = 0x00;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP2CON = 0x00;
    // HIHI disabled; LOLO disabled; HILO disabled; BTWN disabled; LOHI disabled; CMPEN disabled; IE disabled; 
    ADCMP3CON = 0x00;
    // LVLEN9 disabled; LVLEN8 disabled; LVLEN11 disabled; LVLEN7 disabled; LVLEN10 disabled; LVLEN6 disabled; LVLEN13 disabled; LVLEN5 disabled; LVLEN12 disabled; LVLEN4 disabled; LVLEN15 disabled; LVLEN3 disabled; LVLEN14 disabled; LVLEN2 disabled; LVLEN1 disabled; LVLEN0 disabled; 
    ADLVLTRGL = 0x00;
    // LVLEN24 disabled; LVLEN25 disabled; LVLEN17 disabled; LVLEN16 disabled; LVLEN19 disabled; LVLEN18 disabled; 
    ADLVLTRGH = 0x00;
    // SAMC 0; 
    ADCORE0L = 0x00;
    // SAMC 0; 
    ADCORE1L = 0x00;
    // RES 12-bit resolution; EISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; ADCS 2; 
    ADCORE0H = 0x300;
    // RES 12-bit resolution; EISEL Early interrupt is generated 1 TADCORE clock prior to data being ready; ADCS 2; 
    ADCORE1H = 0x300;
    // EIEN9 disabled; EIEN7 disabled; EIEN8 disabled; EIEN5 disabled; EIEN6 disabled; EIEN3 disabled; EIEN4 disabled; EIEN1 disabled; EIEN2 disabled; EIEN13 disabled; EIEN0 disabled; EIEN12 disabled; EIEN11 disabled; EIEN10 disabled; EIEN15 disabled; EIEN14 disabled; 
    ADEIEL = 0x00;
    // EIEN17 disabled; EIEN16 disabled; EIEN25 disabled; EIEN19 disabled; EIEN18 disabled; EIEN24 disabled; 
    ADEIEH = 0x00;
    // C0CIE disabled; C1CIE disabled; SHRCIE disabled; WARMTIME 32768 Source Clock Periods; 
    ADCON5H = (0xF00 & 0xF0FF); //Disabling WARMTIME bit
	
    //Assign Default Callbacks
    ADC1_SetCommonInterruptHandler(&ADC1_CallBack);
    ADC1_SetDC_STATUS_HSInterruptHandler(&ADC1_DC_STATUS_HS_CallBack);
    ADC1_SetStatSw2InterruptHandler(&ADC1_StatSw2_CallBack);
    ADC1_SetStatSw1InterruptHandler(&ADC1_StatSw1_CallBack);
    ADC1_SetDC_STATUS_LSInterruptHandler(&ADC1_DC_STATUS_LS_CallBack);
    ADC1_SetIM_SENSEInterruptHandler(&ADC1_IM_SENSE_CallBack);
    ADC1_SetStatSw3InterruptHandler(&ADC1_StatSw3_CallBack);
    ADC1_SetCHAR_ANInterruptHandler(&ADC1_CHAR_AN_CallBack);
    ADC1_SetBAT_STATUSInterruptHandler(&ADC1_BAT_STATUS_CallBack);
    ADC1_SetStatSw4InterruptHandler(&ADC1_StatSw4_CallBack);
    

    // Setting WARMTIME bit
    ADCON5Hbits.WARMTIME = 0xF;
    // Enabling ADC Module
    ADCON1Lbits.ADON = 0x1;
    // Enabling Power for the Shared Core
    ADC1_SharedCorePowerEnable();
    // Enabling Power for Core1
    ADC1_Core1PowerEnable();

    //TRGSRC0 None; TRGSRC1 Common Software Trigger; 
    ADTRIG0L = 0x100;
    //TRGSRC3 Common Software Trigger; TRGSRC2 Common Software Trigger; 
    ADTRIG0H = 0x101;
    //TRGSRC4 Common Software Trigger; TRGSRC5 None; 
    ADTRIG1L = 0x01;
    //TRGSRC6 None; TRGSRC7 Common Software Trigger; 
    ADTRIG1H = 0x100;
    //TRGSRC8 Common Software Trigger; TRGSRC9 Common Software Trigger; 
    ADTRIG2L = 0x101;
    //TRGSRC11 None; TRGSRC10 None; 
    ADTRIG2H = 0x00;
    //TRGSRC13 None; TRGSRC12 Common Software Trigger; 
    ADTRIG3L = 0x01;
    //TRGSRC15 None; TRGSRC14 None; 
    ADTRIG3H = 0x00;
    //TRGSRC17 None; TRGSRC16 Common Software Trigger; 
    ADTRIG4L = 0x01;
    //TRGSRC19 None; TRGSRC18 None; 
    ADTRIG4H = 0x00;
    //TRGSRC24 Common Software Trigger; TRGSRC25 Common Software Trigger; 
    ADTRIG6L = 0x101;
}

void ADC1_Core0PowerEnable ( ) 
{
    ADCON5Lbits.C0PWR = 1; 
    while(ADCON5Lbits.C0RDY == 0);
    ADCON3Hbits.C0EN = 1;     
}

void ADC1_Core1PowerEnable ( ) 
{
    ADCON5Lbits.C1PWR = 1; 
    while(ADCON5Lbits.C1RDY == 0);
    ADCON3Hbits.C1EN = 1;     
}

void ADC1_SharedCorePowerEnable ( ) 
{
    ADCON5Lbits.SHRPWR = 1;   
    while(ADCON5Lbits.SHRRDY == 0);
    ADCON3Hbits.SHREN = 1;   
}


void __attribute__ ((weak)) ADC1_CallBack ( void )
{ 

}

void ADC1_SetCommonInterruptHandler(void* handler)
{
    ADC1_CommonDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_Tasks ( void )
{
    if(IFS5bits.ADCIF)
    {
        if(ADC1_CommonDefaultInterruptHandler) 
        { 
            ADC1_CommonDefaultInterruptHandler(); 
        }

        // clear the ADC1 interrupt flag
        IFS5bits.ADCIF = 0;
    }
}

void __attribute__ ((weak)) ADC1_DC_STATUS_HS_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetDC_STATUS_HSInterruptHandler(void* handler)
{
    ADC1_DC_STATUS_HSDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_DC_STATUS_HS_Tasks ( void )
{
    uint16_t valDC_STATUS_HS;

    if(ADSTATLbits.AN2RDY)
    {
        //Read the ADC value from the ADCBUF
        valDC_STATUS_HS = ADCBUF2;

        if(ADC1_DC_STATUS_HSDefaultInterruptHandler) 
        { 
            ADC1_DC_STATUS_HSDefaultInterruptHandler(valDC_STATUS_HS); 
        }
    }
}

void __attribute__ ((weak)) ADC1_StatSw2_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetStatSw2InterruptHandler(void* handler)
{
    ADC1_StatSw2DefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_StatSw2_Tasks ( void )
{
    uint16_t valStatSw2;

    if(ADSTATLbits.AN3RDY)
    {
        //Read the ADC value from the ADCBUF
        valStatSw2 = ADCBUF3;

        if(ADC1_StatSw2DefaultInterruptHandler) 
        { 
            ADC1_StatSw2DefaultInterruptHandler(valStatSw2); 
        }
    }
}

void __attribute__ ((weak)) ADC1_StatSw1_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetStatSw1InterruptHandler(void* handler)
{
    ADC1_StatSw1DefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_StatSw1_Tasks ( void )
{
    uint16_t valStatSw1;

    if(ADSTATLbits.AN4RDY)
    {
        //Read the ADC value from the ADCBUF
        valStatSw1 = ADCBUF4;

        if(ADC1_StatSw1DefaultInterruptHandler) 
        { 
            ADC1_StatSw1DefaultInterruptHandler(valStatSw1); 
        }
    }
}

void __attribute__ ((weak)) ADC1_DC_STATUS_LS_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetDC_STATUS_LSInterruptHandler(void* handler)
{
    ADC1_DC_STATUS_LSDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_DC_STATUS_LS_Tasks ( void )
{
    uint16_t valDC_STATUS_LS;

    if(ADSTATLbits.AN7RDY)
    {
        //Read the ADC value from the ADCBUF
        valDC_STATUS_LS = ADCBUF7;

        if(ADC1_DC_STATUS_LSDefaultInterruptHandler) 
        { 
            ADC1_DC_STATUS_LSDefaultInterruptHandler(valDC_STATUS_LS); 
        }
    }
}

void __attribute__ ((weak)) ADC1_IM_SENSE_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetIM_SENSEInterruptHandler(void* handler)
{
    ADC1_IM_SENSEDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_IM_SENSE_Tasks ( void )
{
    uint16_t valIM_SENSE;

    if(ADSTATLbits.AN8RDY)
    {
        //Read the ADC value from the ADCBUF
        valIM_SENSE = ADCBUF8;

        if(ADC1_IM_SENSEDefaultInterruptHandler) 
        { 
            ADC1_IM_SENSEDefaultInterruptHandler(valIM_SENSE); 
        }
    }
}

void __attribute__ ((weak)) ADC1_StatSw3_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetStatSw3InterruptHandler(void* handler)
{
    ADC1_StatSw3DefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_StatSw3_Tasks ( void )
{
    uint16_t valStatSw3;

    if(ADSTATLbits.AN9RDY)
    {
        //Read the ADC value from the ADCBUF
        valStatSw3 = ADCBUF9;

        if(ADC1_StatSw3DefaultInterruptHandler) 
        { 
            ADC1_StatSw3DefaultInterruptHandler(valStatSw3); 
        }
    }
}

void __attribute__ ((weak)) ADC1_CHAR_AN_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetCHAR_ANInterruptHandler(void* handler)
{
    ADC1_CHAR_ANDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_CHAR_AN_Tasks ( void )
{
    uint16_t valCHAR_AN;

    if(ADSTATLbits.AN12RDY)
    {
        //Read the ADC value from the ADCBUF
        valCHAR_AN = ADCBUF12;

        if(ADC1_CHAR_ANDefaultInterruptHandler) 
        { 
            ADC1_CHAR_ANDefaultInterruptHandler(valCHAR_AN); 
        }
    }
}

void __attribute__ ((weak)) ADC1_BAT_STATUS_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetBAT_STATUSInterruptHandler(void* handler)
{
    ADC1_BAT_STATUSDefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_BAT_STATUS_Tasks ( void )
{
    uint16_t valBAT_STATUS;

    if(ADSTATHbits.AN16RDY)
    {
        //Read the ADC value from the ADCBUF
        valBAT_STATUS = ADCBUF16;

        if(ADC1_BAT_STATUSDefaultInterruptHandler) 
        { 
            ADC1_BAT_STATUSDefaultInterruptHandler(valBAT_STATUS); 
        }
    }
}




void __attribute__ ((weak)) ADC1_StatSw4_CallBack( uint16_t adcVal )
{ 

}

void ADC1_SetStatSw4InterruptHandler(void* handler)
{
    ADC1_StatSw4DefaultInterruptHandler = handler;
}

void __attribute__ ((weak)) ADC1_StatSw4_Tasks ( void )
{
    uint16_t valStatSw4;

    if(ADSTATLbits.AN1RDY)
    {
        //Read the ADC value from the ADCBUF
        valStatSw4 = ADCBUF1;

        if(ADC1_StatSw4DefaultInterruptHandler) 
        { 
            ADC1_StatSw4DefaultInterruptHandler(valStatSw4); 
        }
    }
}



/**
  End of File
*/
