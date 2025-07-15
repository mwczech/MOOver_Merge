/**
  PWM Generated Driver API Header File

  @Company
    Microchip Technology Inc.

  @File Name
    pwm.h

  @Summary
    This is the generated header file for the PWM driver using PIC24 / dsPIC33 / PIC32MM MCUs

  @Description
    This header file provides APIs for driver for PWM.
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

#ifndef PWM_H
#define PWM_H

/**
 Section: Included Files
*/

#include <xc.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif

/**
  Section: Data Type Definitions
*/

/** PWM Generator Definition
 
 @Summary 
   Defines the PWM generators available for PWM
 
 @Description
   This routine defines the PWM generators that are available for the module to use.
 
 Remarks:
   None
 */
typedef enum 
{
    ULTRASONIC =  1,       
    AUX =  2,       
    LED_OUT =  3,       
    LIFT =  4,
    SAFETY_PWM =  5,       
    ADC_TIMER =  6,       
} PWM_GENERATOR;
        
/** PWM Generator Interrupt Definition
 
 @Summary 
   Defines the PWM generator interrupt available for PWM
 
 @Description
   This routine defines the PWM generator interrupt that are available for the module to use.
 
 Remarks:
   None
 */
typedef enum 
{
    PWM_GENERATOR_INTERRUPT_FAULT =  1,       
    PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT =  2,
    PWM_GENERATOR_INTERRUPT_FEED_FORWARD =  3,
    PWM_GENERATOR_INTERRUPT_SYNC =  4,		
} PWM_GENERATOR_INTERRUPT;

/**
 Section: Interface Routines
*/

/**
  @Summary
    Initializes PWM module.

  @Description
    This routine initializes PWM module, using the given initialization data. 

  @Param
    None.

  @Returns
    None
 
  @Example 
    <code>
    uint16_t masterPeriod,masterDutyCycle,masterPhase;

    masterPeriod = 0xFFFF;
    masterDutyCycle = 0xFF;
    masterPhase = 0xF;

    PWM_Initialize();
 
    PWM_GeneratorDisable();

    PWM_MasterPeriodSet(masterPeriod);
    PWM_MasterDutyCycleSet(masterDutyCycle);
    PWM_MasterPhaseSet(masterPhase);
 
    PWM_GeneratorEnable(PWM_GENERATOR_1);

    </code>
*/
void PWM_Initialize (void);

/**
  @Summary
    Enables the specific PWM generator.

  @Description
    This routine is used to enable the specific PWM generator selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>
    PWM_GeneratorEnable(PWM_GENERATOR_1);
    </code>
*/
inline static void PWM_GeneratorEnable(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1CONLbits.ON = 1;              
                break;       
        case AUX:
                PG2CONLbits.ON = 1;              
                break;       
        case LED_OUT:
                PG3CONLbits.ON = 1;              
                break;       
        case LIFT:
                PG4CONLbits.ON = 1;              
                break;       
        case SAFETY_PWM:
                PG5CONLbits.ON = 1;              
                break;       
        case ADC_TIMER:
                PG6CONLbits.ON = 1;              
                break;       
        default:break;    
    }     
}

/**
  @Summary
    Disables the specific PWM generator.

  @Description
    This routine is used to disable the specific PWM generator selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>
    PWM_GeneratorDisable(PWM_GENERATOR_1);
    </code>
*/
inline static void PWM_GeneratorDisable(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1CONLbits.ON = 0;              
                break;       
        case AUX:
                PG2CONLbits.ON = 0;              
                break;       
        case LED_OUT:
                PG3CONLbits.ON = 0;              
                break;       
        case LIFT:
                PG4CONLbits.ON = 0;              
                break;       
        case SAFETY_PWM:
                PG5CONLbits.ON = 0;              
                break;       
        case ADC_TIMER:
                PG6CONLbits.ON = 0;              
                break;       
        default:break;    
    }    
}

/**
  @Summary
    Enables all the generators of PWM module.

  @Description
    This routine is used to enable all the generators of PWM module.

  @Param
    None.

  @Returns
    None
 
  @Example 
    <code>
    PWM_Enable();
    </code>
*/
inline static void PWM_Enable()
{
    PG1CONLbits.ON = 1;              
    PG2CONLbits.ON = 1;              
    PG3CONLbits.ON = 1;              
    PG4CONLbits.ON = 1;              
    PG5CONLbits.ON = 1;              
    PG6CONLbits.ON = 1;              
}

/**
  @Summary
    Disables all the generators of PWM module.

  @Description
    This routine is used to disable all the generators of PWM module.

  @Param
    None.

  @Returns
    None
 
  @Example 
    <code>
    PWM_Disable();
    </code>
*/
inline static void PWM_Disable()
{
    PG1CONLbits.ON = 0;              
    PG2CONLbits.ON = 0;              
    PG3CONLbits.ON = 0;              
    PG4CONLbits.ON = 0;              
    PG5CONLbits.ON = 0;              
    PG6CONLbits.ON = 0;              
}

/**
  @Summary
    Sets the period value in count for the Master Time Base generator.

  @Description
    This routine is used to set the period value in count for the Master Time Base generator.

  @Param
    masterPeriod - Period value in count.

  @Returns
    None
 
  @Example 
    <code>
    uint16_t masterPeriod;

    masterPeriod = 0xFF;
    
    PWM_MasterPeriodSet(masterPeriod);
    </code>
*/
inline static void PWM_MasterPeriodSet(uint16_t masterPeriod)
{
    MPER = masterPeriod;
}

/**
  @Summary
    Used to set the PWM master duty cycle register.

  @Description
    This routine is used to set the PWM master duty cycle register.

  @Param
    masterDutyCycle - Master Duty Cycle value.

  @Returns
    None
 
  @Example 
    <code>
    uint16_t masterDutyCycle;

    masterDutyCycle = 0xFF;
    
    PWM_MasterDutyCycleSet(masterDutyCycle);
    </code>
*/
inline static void PWM_MasterDutyCycleSet(uint16_t masterDutyCycle)
{
    MDC = masterDutyCycle;
}

/**
  @Summary
    Sets the phase value in count for the Master Time Base generator.

  @Description
    This routine is used to set the phase value in count for the Master Time Base generator.

  @Param
    masterPhase - Phase value in count.

  @Returns
    None
 
  @Example 
    <code>
    uint16_t masterPhase;

    masterPhase = 0xFF;
    
    PWM_MasterPhaseSet(masterPhase);
    </code>
*/
inline static void PWM_MasterPhaseSet(uint16_t masterPhase)
{
    MPHASE = masterPhase;
}

/**
  @Summary
    Sets the period value in count for the PWM generator specific Time Base.

  @Description
    This routine is used to set the period value in count for the PWM generator specific Time Base.

  @Param
    genNum - PWM generator number.
    period - PWM generator period value in count.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t period;
    
    period = 0xFFFF;
    PWM_PeriodSet(PWM_GENERATOR_1, period);
    </code>
*/
inline static void PWM_PeriodSet(PWM_GENERATOR genNum,uint16_t period)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1PER = period;              
                break;       
        case AUX:
                PG2PER = period;              
                break;       
        case LED_OUT:
                PG3PER = period;              
                break;       
        case LIFT:
                PG4PER = period;              
                break;       
        case SAFETY_PWM:
                PG5PER = period;              
                break;       
        case ADC_TIMER:
                PG6PER = period;              
                break;       
        default:break;    
    }   
}

/**
  @Summary
    Used to set the PWM generator specific duty cycle register.

  @Description
    This routine is used to set the PWM generator specific duty cycle register.

  @Param
    genNum      - PWM generator number.
    dutyCycle   - PWM generator duty cycle.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t dutyCyle;
    
    dutyCycle = 0xFF;
    PWM_DutyCycleSet(PWM_GENERATOR_1, dutyCycle);
    </code>
*/
inline static void PWM_DutyCycleSet(PWM_GENERATOR genNum,uint16_t dutyCycle)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1DC = dutyCycle;              
                break;       
        case AUX:
                PG2DC = dutyCycle;              
                break;       
        case LED_OUT:
                PG3DC = dutyCycle;              
                break;       
        case LIFT:
                PG4DC = dutyCycle;              
                break;       
        case SAFETY_PWM:
                PG5DC = dutyCycle;              
                break;       
        case ADC_TIMER:
                PG6DC = dutyCycle;              
                break;       
        default:break;    
    }  
}

/**
  @Summary
    Sets the phase value in count for the PWM generator specific Time Base.

  @Description
    This routine is used to set the phase value in count for the PWM generator specific Time Base.

  @Param
    genNum - PWM generator number.
    phase - PWM generator phase value in count.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t phase;
    
    phase = 0xFFFF;
    PWM_PhaseSet(PWM_GENERATOR_1, phase);
    </code>
*/
inline static void PWM_PhaseSet(PWM_GENERATOR genNum,uint16_t phase)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1PHASE = phase;              
                break;       
        case AUX:
                PG2PHASE = phase;              
                break;       
        case LED_OUT:
                PG3PHASE = phase;              
                break;       
        case LIFT:
                PG4PHASE = phase;              
                break;       
        case SAFETY_PWM:
                PG5PHASE = phase;              
                break;       
        case ADC_TIMER:
                PG6PHASE = phase;              
                break;       
        default:break;    
    } 
}

/**
  @Summary
    Updates PWM override data bits with the requested value for a specific PWM generator.

  @Description
    This routine is used to updates PWM override data bits with the requested 
    value for a specific PWM generator selected by the argument PWM_GENERATOR.

  @Param
    genNum          - PWM generator number.
    overrideData    - Override data

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t overrideData;

    overrideData = 0x01;
    
    PWM_OverrideDataSet(PWM_GENERATOR_1, overrideData);
    </code>
*/  
inline static void PWM_OverrideDataSet(PWM_GENERATOR genNum,uint16_t overrideData)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRDAT = overrideData;              
                break;       
        case AUX:
                PG2IOCONLbits.OVRDAT = overrideData;              
                break;       
        case LED_OUT:
                PG3IOCONLbits.OVRDAT = overrideData;              
                break;       
        case LIFT:
                PG4IOCONLbits.OVRDAT = overrideData;              
                break;       
        case SAFETY_PWM:
                PG5IOCONLbits.OVRDAT = overrideData;              
                break;       
        case ADC_TIMER:
                PG6IOCONLbits.OVRDAT = overrideData;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Updates PWM override high data bit with the requested value for a specific PWM generator.

  @Description
    This routine is used to update PWM override high data bit with the requested 
    value for a specific PWM generator selected by the argument PWM_GENERATOR.

  @Param
    genNum              - PWM generator number.
    overrideDataHigh    - Override data

  @Returns
    None
 
  @Example 
    <code>    
    bool overrideDataHigh;

    overrideDataHigh = true;
    
    PWM_OverrideDataHighSet(PWM_GENERATOR_1, overrideDataHigh);
    </code>
*/  
inline static void PWM_OverrideDataHighSet(PWM_GENERATOR genNum,bool overrideDataHigh)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRDAT |= (overrideDataHigh << 1);
                break;
        case AUX:
                PG2IOCONLbits.OVRDAT |= (overrideDataHigh << 1);
                break;
        case LED_OUT:
                PG3IOCONLbits.OVRDAT |= (overrideDataHigh << 1);
                break;
        case LIFT:
                PG4IOCONLbits.OVRDAT |= (overrideDataHigh << 1);
                break;
        case SAFETY_PWM:
                PG5IOCONLbits.OVRDAT |= (overrideDataHigh << 1);
                break;
        case ADC_TIMER:
                PG6IOCONLbits.OVRDAT |= (overrideDataHigh << 1);
                break;
        default:break;    
    }
}

/**
  @Summary
    Updates PWM override low data bit with the requested value for a specific PWM generator.

  @Description
    This routine is used to update PWM override low data bit with the requested 
    value for a specific PWM generator selected by the argument PWM_GENERATOR.

  @Param
    genNum             - PWM generator number.
    overrideDataLow    - Override data

  @Returns
    None
 
  @Example 
    <code>    
    bool overrideDataLow;

    overrideDataLow = true;
    
    PWM_OverrideDataLowSet(PWM_GENERATOR_1, overrideDataLow);
    </code>
*/  
inline static void PWM_OverrideDataLowSet(PWM_GENERATOR genNum,bool overrideDataLow)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRDAT |= overrideDataLow;             
                break;  
        case AUX:
                PG2IOCONLbits.OVRDAT |= overrideDataLow;             
                break;  
        case LED_OUT:
                PG3IOCONLbits.OVRDAT |= overrideDataLow;             
                break;  
        case LIFT:
                PG4IOCONLbits.OVRDAT |= overrideDataLow;             
                break;  
        case SAFETY_PWM:
                PG5IOCONLbits.OVRDAT |= overrideDataLow;             
                break;  
        case ADC_TIMER:
                PG6IOCONLbits.OVRDAT |= overrideDataLow;             
                break;  
        default:break;    
    }
}

/**
  @Summary
    Gets PWM override value for the PWM Generator.

  @Description
    This routine is used to get PWM override value for the PWM Generator 
    selected by the argument PWM_GENERATOR.
	
  @Param
    genNum          - PWM generator number.

  @Returns
    Override data for the PWM Generator selected by the argument PWM_GENERATOR.
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    uint16_t overrideData;
    
    genNum = PWM_GENERATOR_1;
    overrideData = PWM_OverrideDataGet(genNum, overrideData);
    </code>
*/
inline static uint16_t PWM_OverrideDataGet(PWM_GENERATOR genNum)
{
    uint16_t overrideData;
    switch(genNum) { 
        case ULTRASONIC:
                overrideData = PG1IOCONLbits.OVRDAT;             
                break;
        case AUX:
                overrideData = PG2IOCONLbits.OVRDAT;             
                break;
        case LED_OUT:
                overrideData = PG3IOCONLbits.OVRDAT;             
                break;
        case LIFT:
                overrideData = PG4IOCONLbits.OVRDAT;             
                break;
        case SAFETY_PWM:
                overrideData = PG5IOCONLbits.OVRDAT;             
                break;
        case ADC_TIMER:
                overrideData = PG6IOCONLbits.OVRDAT;             
                break;
        default:break;    
    }
    return overrideData;
}

/**
  @Summary
    Enables PWM override on PWMH output for specific PWM generator.

  @Description
    This routine is used to enables PWM override on PWMH output for specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_OverrideHighEnable(PWM_GENERATOR_1);
    </code>
*/  
inline static void PWM_OverrideHighEnable(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRENH = 1;              
                break;
        case AUX:
                PG2IOCONLbits.OVRENH = 1;              
                break;
        case LED_OUT:
                PG3IOCONLbits.OVRENH = 1;              
                break;
        case LIFT:
                PG4IOCONLbits.OVRENH = 1;              
                break;
        case SAFETY_PWM:
                PG5IOCONLbits.OVRENH = 1;              
                break;
        case ADC_TIMER:
                PG6IOCONLbits.OVRENH = 1;              
                break;
        default:break;    
    }
}

/**
  @Summary
    Enables PWM override on PWML output for specific PWM generator.

  @Description
    This routine is used to enables PWM override on PWML output for specific PWM generator
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_OverrideLowEnable(PWM_GENERATOR_1);
    </code>
*/  
inline static void PWM_OverrideLowEnable(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRENL = 1;              
                break; 
        case AUX:
                PG2IOCONLbits.OVRENL = 1;              
                break; 
        case LED_OUT:
                PG3IOCONLbits.OVRENL = 1;              
                break; 
        case LIFT:
                PG4IOCONLbits.OVRENL = 1;              
                break; 
        case SAFETY_PWM:
                PG5IOCONLbits.OVRENL = 1;              
                break; 
        case ADC_TIMER:
                PG6IOCONLbits.OVRENL = 1;              
                break; 
        default:break;    
    }
}

/**
  @Summary
    Disables PWM override on PWMH output for specific PWM generator.

  @Description
    This routine is used to disable PWM override on PWMH output for specific PWM generator 
    selected by the argument PWM_GENERATOR.


  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_OverrideHighDisable(PWM_GENERATOR_1);
    </code>
*/  
inline static void PWM_OverrideHighDisable(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRENH = 0;              
                break;
        case AUX:
                PG2IOCONLbits.OVRENH = 0;              
                break;
        case LED_OUT:
                PG3IOCONLbits.OVRENH = 0;              
                break;
        case LIFT:
                PG4IOCONLbits.OVRENH = 0;              
                break;
        case SAFETY_PWM:
                PG5IOCONLbits.OVRENH = 0;              
                break;
        case ADC_TIMER:
                PG6IOCONLbits.OVRENH = 0;              
                break;
        default:break;    
    }
}

/**
  @Summary
    Disables PWM override on PWML output for specific PWM generator.

  @Description
    This routine is used to disable PWM override on PWML output for specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_OverrideLowDisable(PWM_GENERATOR_1);
    </code>
*/  
inline static void PWM_OverrideLowDisable(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRENL = 0;              
                break;   
        case AUX:
                PG2IOCONLbits.OVRENL = 0;              
                break;   
        case LED_OUT:
                PG3IOCONLbits.OVRENL = 0;              
                break;   
        case LIFT:
                PG4IOCONLbits.OVRENL = 0;              
                break;   
        case SAFETY_PWM:
                PG5IOCONLbits.OVRENL = 0;              
                break;   
        case ADC_TIMER:
                PG6IOCONLbits.OVRENL = 0;              
                break;   
        default:break;    
    }
}

/**
  @Summary
    Updates PWM Deadtime low register with the requested value for a specific PWM generator.

  @Description
    This routine is used to updates PWM Deadtime low register with the requested
    value for a specific PWM generator selected by the argument PWM_GENERATOR.

  @Param
    genNum      - PWM generator number.
    deadtimeLow - Deadtime low value.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t deadtimeLow;

    deadtimeLow = 0x01;
    
    PWM_DeadTimeLowSet(PWM_GENERATOR_1, deadtimeLow);
    </code>
*/  
inline static void PWM_DeadTimeLowSet(PWM_GENERATOR genNum,uint16_t deadtimeLow)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1DTL = deadtimeLow;              
                break;       
        case AUX:
                PG2DTL = deadtimeLow;              
                break;       
        case LED_OUT:
                PG3DTL = deadtimeLow;              
                break;       
        case LIFT:
                PG4DTL = deadtimeLow;              
                break;       
        case SAFETY_PWM:
                PG5DTL = deadtimeLow;              
                break;       
        case ADC_TIMER:
                PG6DTL = deadtimeLow;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Updates PWM Deadtime high register with the requested value for a specific PWM generator.

  @Description
    This routine is used to updates PWM Deadtime high register with the requested
    value for a specific PWM generator selected by the argument PWM_GENERATOR.

  @Param
    genNum          - PWM generator number.
    deadtimeHigh    - Deadtime high value.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t deadtimeHigh;

    deadtimeHigh = 0x01;
    
    PWM_DeadTimeHighSet(PWM_GENERATOR_1, deadtimeHigh);
    </code>
*/  
inline static void PWM_DeadTimeHighSet(PWM_GENERATOR genNum,uint16_t deadtimeHigh)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1DTH = deadtimeHigh;              
                break;       
        case AUX:
                PG2DTH = deadtimeHigh;              
                break;       
        case LED_OUT:
                PG3DTH = deadtimeHigh;              
                break;       
        case LIFT:
                PG4DTH = deadtimeHigh;              
                break;       
        case SAFETY_PWM:
                PG5DTH = deadtimeHigh;              
                break;       
        case ADC_TIMER:
                PG6DTH = deadtimeHigh;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Updates PWM Deadtime low and high register with the requested value for a specific PWM generator.

  @Description
    This routine is used to updates PWM Deadtime low and high register with the 
    requested value for a specific PWM generator selected by the argument PWM_GENERATOR.

  @Param
    genNum          - PWM generator number.
    deadtimeHigh    - Deadtime value.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t deadtime;

    deadtime = 0x01;
    
    PWM_DeadTimeHighSet(PWM_GENERATOR_1, deadtime);
    </code>
*/  
inline static void PWM_DeadTimeSet(PWM_GENERATOR genNum,uint16_t deadtime)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1DTL = deadtime;
                PG1DTH = deadtime; 				
                break;       
        case AUX:
                PG2DTL = deadtime;
                PG2DTH = deadtime; 				
                break;       
        case LED_OUT:
                PG3DTL = deadtime;
                PG3DTH = deadtime; 				
                break;       
        case LIFT:
                PG4DTL = deadtime;
                PG4DTH = deadtime; 				
                break;       
        case SAFETY_PWM:
                PG5DTL = deadtime;
                PG5DTH = deadtime; 				
                break;       
        case ADC_TIMER:
                PG6DTL = deadtime;
                PG6DTH = deadtime; 				
                break;       
        default:break;    
    }
}

/**
  @Summary
    Sets the PWM trigger compare value in count for the 
    PWM Generator.

  @Description
    This routine is used to set the PWM trigger compare value in count
    for the PWM Generator selected by the argument PWM_GENERATOR.

  @Param
    genNum          - PWM generator number.
    trigCompValue   - Trigger compare value in count.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    uint16_t trigCompValue;

    trigCompValue = 0x01;
    
    genNum = PWM_GENERATOR_1;
    PWM_TriggerCompareValueSet(genNum, trigCompValue);
    </code>
*/  
inline static void PWM_TriggerCompareValueSet(PWM_GENERATOR genNum,uint16_t trigCompValue)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1TRIGA = trigCompValue;              
                break;      
        case AUX:
                PG2TRIGA = trigCompValue;              
                break;      
        case LED_OUT:
                PG3TRIGA = trigCompValue;              
                break;      
        case LIFT:
                PG4TRIGA = trigCompValue;              
                break;      
        case SAFETY_PWM:
                PG5TRIGA = trigCompValue;              
                break;      
        case ADC_TIMER:
                PG6TRIGA = trigCompValue;              
                break;      
        default:break;    
    }
}

/**
  @Summary
    Enables interrupt requests for the PWM Generator.

  @Description
    This routine is used to enable interrupt requests for the PWM Generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    interrupt - PWM generator interrupt source.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    PWM_GENERATOR_INTERRUPT interrupt;
    
    genNum = PWM_GENERATOR_1;
    interrupt = PWM_GENERATOR_INTERRUPT_TRIGGER;
    PWM_GeneratorInterruptEnable(genNum, interrupt);
    </code>
*/ 
inline static void PWM_GeneratorInterruptEnable(PWM_GENERATOR genNum, PWM_GENERATOR_INTERRUPT interrupt)
{
    switch(genNum) { 
        case ULTRASONIC:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG1EVTHbits.FLTIEN = true;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG1EVTHbits.CLIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG1EVTHbits.FFIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG1EVTHbits.SIEN = true;
                                        break;														
                        default:break;  
                }              
                break;   
        case AUX:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG2EVTHbits.FLTIEN = true;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG2EVTHbits.CLIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG2EVTHbits.FFIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG2EVTHbits.SIEN = true;
                                        break;														
                        default:break;  
                }              
                break;   
        case LED_OUT:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG3EVTHbits.FLTIEN = true;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG3EVTHbits.CLIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG3EVTHbits.FFIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG3EVTHbits.SIEN = true;
                                        break;														
                        default:break;  
                }              
                break;   
        case LIFT:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG4EVTHbits.FLTIEN = true;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG4EVTHbits.CLIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG4EVTHbits.FFIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG4EVTHbits.SIEN = true;
                                        break;														
                        default:break;  
                }              
                break;   
        case SAFETY_PWM:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG5EVTHbits.FLTIEN = true;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG5EVTHbits.CLIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG5EVTHbits.FFIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG5EVTHbits.SIEN = true;
                                        break;														
                        default:break;  
                }              
                break;   
        case ADC_TIMER:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG6EVTHbits.FLTIEN = true;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG6EVTHbits.CLIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG6EVTHbits.FFIEN = true;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG6EVTHbits.SIEN = true;
                                        break;														
                        default:break;  
                }              
                break;   
        default:break;  
    }
}

/**
  @Summary
    Disables interrupt requests for the PWM Generator.

  @Description
    This routine is used to disables interrupt requests for the PWM 
    Generator selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    interrupt - PWM generator interrupt source.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    PWM_GENERATOR_INTERRUPT interrupt;
    
    genNum = PWM_GENERATOR_1;
    interrupt = PWM_GENERATOR_INTERRUPT_TRIGGER;
    PWM_GeneratorInterruptDisable(genNum, interrupt);
    </code>
*/ 
inline static void PWM_GeneratorInterruptDisable(PWM_GENERATOR genNum, PWM_GENERATOR_INTERRUPT interrupt)
{
    switch(genNum) { 
        case ULTRASONIC:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG1EVTHbits.FLTIEN = false;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG1EVTHbits.CLIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG1EVTHbits.FFIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG1EVTHbits.SIEN = false;
                                        break;								
                        default:break;  
                }              
                break;  
        case AUX:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG2EVTHbits.FLTIEN = false;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG2EVTHbits.CLIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG2EVTHbits.FFIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG2EVTHbits.SIEN = false;
                                        break;								
                        default:break;  
                }              
                break;  
        case LED_OUT:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG3EVTHbits.FLTIEN = false;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG3EVTHbits.CLIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG3EVTHbits.FFIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG3EVTHbits.SIEN = false;
                                        break;								
                        default:break;  
                }              
                break;  
        case LIFT:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG4EVTHbits.FLTIEN = false;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG4EVTHbits.CLIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG4EVTHbits.FFIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG4EVTHbits.SIEN = false;
                                        break;								
                        default:break;  
                }              
                break;  
        case SAFETY_PWM:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG5EVTHbits.FLTIEN = false;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG5EVTHbits.CLIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG5EVTHbits.FFIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG5EVTHbits.SIEN = false;
                                        break;								
                        default:break;  
                }              
                break;  
        case ADC_TIMER:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG6EVTHbits.FLTIEN = false;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG6EVTHbits.CLIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG6EVTHbits.FFIEN = false;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG6EVTHbits.SIEN = false;
                                        break;								
                        default:break;  
                }              
                break;  
        default:break;  
    }
}

/**
  @Summary
    Clears PWM interrupt status for the PWM Generator.

  @Description
    This routine is used to clear PWM interrupt status for the PWM Generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    interrupt - PWM generator interrupt source.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    PWM_GENERATOR_INTERRUPT interrupt;
    
    genNum = PWM_GENERATOR_1;
    interrupt = PWM_GENERATOR_INTERRUPT_TRIGGER;
    PWM_GeneratorEventStatusClear(genNum, interrupt);
    </code>
*/
inline static void PWM_GeneratorEventStatusClear(PWM_GENERATOR genNum, PWM_GENERATOR_INTERRUPT interrupt)
{
    switch(genNum) { 
        case ULTRASONIC:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG1STATbits.FLTEVT = 0;							
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG1STATbits.CLEVT = 0;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG1STATbits.FFEVT = 0;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG1STATbits.SEVT = 0;
                                        break;							
                        default:break;  
                }              
                break; 
        case AUX:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG2STATbits.FLTEVT = 0;							
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG2STATbits.CLEVT = 0;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG2STATbits.FFEVT = 0;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG2STATbits.SEVT = 0;
                                        break;							
                        default:break;  
                }              
                break; 
        case LED_OUT:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG3STATbits.FLTEVT = 0;							
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG3STATbits.CLEVT = 0;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG3STATbits.FFEVT = 0;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG3STATbits.SEVT = 0;
                                        break;							
                        default:break;  
                }              
                break; 
        case LIFT:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG4STATbits.FLTEVT = 0;							
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG4STATbits.CLEVT = 0;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG4STATbits.FFEVT = 0;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG4STATbits.SEVT = 0;
                                        break;							
                        default:break;  
                }              
                break; 
        case SAFETY_PWM:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG5STATbits.FLTEVT = 0;							
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG5STATbits.CLEVT = 0;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG5STATbits.FFEVT = 0;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG5STATbits.SEVT = 0;
                                        break;							
                        default:break;  
                }              
                break; 
        case ADC_TIMER:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        PG6STATbits.FLTEVT = 0;							
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        PG6STATbits.CLEVT = 0;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        PG6STATbits.FFEVT = 0;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        PG6STATbits.SEVT = 0;
                                        break;							
                        default:break;  
                }              
                break; 
        default:break;  
    }
}

/**
  @Summary
    Gets PWM interrupt status for the PWM Generator.

  @Description
    This routine is used to get PWM interrupt status for the PWM Generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    interrupt - PWM generator interrupt source.

  @Returns
    true - Interrupt is pending.
    false - Interrupt is not pending.
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    PWM_GENERATOR_INTERRUPT interrupt;
    bool status;
    
    genNum = PWM_GENERATOR_1;
    interrupt = PWM_GENERATOR_INTERRUPT_TRIGGER;
    status = PWM_GeneratorEventStatusGet(genNum, interrupt);
    </code>
*/
inline static bool PWM_GeneratorEventStatusGet(PWM_GENERATOR genNum, PWM_GENERATOR_INTERRUPT interrupt)
{
    bool status = false;
    switch(genNum) { 
        case ULTRASONIC:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        status = PG1STATbits.FLTEVT;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        status = PG1STATbits.CLEVT;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        status = PG1STATbits.FFEVT;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        status = PG1STATbits.SEVT;
                                        break;							
                        default:break;  
                }              
                break; 
        case AUX:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        status = PG2STATbits.FLTEVT;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        status = PG2STATbits.CLEVT;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        status = PG2STATbits.FFEVT;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        status = PG2STATbits.SEVT;
                                        break;							
                        default:break;  
                }              
                break; 
        case LED_OUT:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        status = PG3STATbits.FLTEVT;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        status = PG3STATbits.CLEVT;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        status = PG3STATbits.FFEVT;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        status = PG3STATbits.SEVT;
                                        break;							
                        default:break;  
                }              
                break; 
        case LIFT:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        status = PG4STATbits.FLTEVT;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        status = PG4STATbits.CLEVT;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        status = PG4STATbits.FFEVT;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        status = PG4STATbits.SEVT;
                                        break;							
                        default:break;  
                }              
                break; 
        case SAFETY_PWM:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        status = PG5STATbits.FLTEVT;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        status = PG5STATbits.CLEVT;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        status = PG5STATbits.FFEVT;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        status = PG5STATbits.SEVT;
                                        break;							
                        default:break;  
                }              
                break; 
        case ADC_TIMER:
                switch(interrupt) { 
                        case PWM_GENERATOR_INTERRUPT_FAULT:
                                        status = PG6STATbits.FLTEVT;               
                                        break;       
                        case PWM_GENERATOR_INTERRUPT_CURRENT_LIMIT:
                                        status = PG6STATbits.CLEVT;
                                        break;
                        case PWM_GENERATOR_INTERRUPT_FEED_FORWARD:
                                        status = PG6STATbits.FFEVT;
                                        break;	
                        case PWM_GENERATOR_INTERRUPT_SYNC:
                                        status = PG6STATbits.SEVT;
                                        break;							
                        default:break;  
                }              
                break; 
        default:break;  
    }
	return status;
}

/**
  @Summary
    Sets the TRIGA compare value in count for a specific PWM generator.

  @Description
    This routine is used to set the TRIGA compare value in count for a specific PWM generator
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    trigA  - TRIGA compare value in count.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t trigA;
    
    trigA = 0xF;
    PWM_TriggerACompareValueSet(PWM_GENERATOR_1, trigA);
    </code>
*/
inline static void PWM_TriggerACompareValueSet(PWM_GENERATOR genNum,uint16_t trigA)
{ 
    switch(genNum) { 
        case ULTRASONIC:
                PG1TRIGA = trigA;              
                break;       
        case AUX:
                PG2TRIGA = trigA;              
                break;       
        case LED_OUT:
                PG3TRIGA = trigA;              
                break;       
        case LIFT:
                PG4TRIGA = trigA;              
                break;       
        case SAFETY_PWM:
                PG5TRIGA = trigA;              
                break;       
        case ADC_TIMER:
                PG6TRIGA = trigA;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Sets the TRIGB compare value in count for a specific PWM generator.

  @Description
    This routine is used to set the TRIGB compare value in count for a specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    trigB  - TRIGB compare value in count.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t trigB;
    
    trigB = 0xF;
    PWM_TriggerBCompareValueSet(PWM_GENERATOR_1, trigB);
    </code>
*/
inline static void PWM_TriggerBCompareValueSet(PWM_GENERATOR genNum,uint16_t trigB)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1TRIGB = trigB;              
                break;       
        case AUX:
                PG2TRIGB = trigB;              
                break;       
        case LED_OUT:
                PG3TRIGB = trigB;              
                break;       
        case LIFT:
                PG4TRIGB = trigB;              
                break;       
        case SAFETY_PWM:
                PG5TRIGB = trigB;              
                break;       
        case ADC_TIMER:
                PG6TRIGB = trigB;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Sets the TRIGC compare value in count for a specific PWM generator.

  @Description
    This routine is used to set the TRIGC compare value in count for a specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    trigC  - TRIGC compare value in count.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t trigC;
    
    trigC = 0xF;
    PWM_TriggerCCompareValueSet(PWM_GENERATOR_1, trigC);
    </code>
*/
inline static void PWM_TriggerCCompareValueSet(PWM_GENERATOR genNum,uint16_t trigC)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1TRIGC = trigC;              
                break;       
        case AUX:
                PG2TRIGC = trigC;              
                break;       
        case LED_OUT:
                PG3TRIGC = trigC;              
                break;       
        case LIFT:
                PG4TRIGC = trigC;              
                break;       
        case SAFETY_PWM:
                PG5TRIGC = trigC;              
                break;       
        case ADC_TIMER:
                PG6TRIGC = trigC;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Requests to update the data registers for specific PWM generator.

  @Description
    This routine is used to request to update the data registers for specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_SoftwareUpdateRequest(PWM_GENERATOR_1);
    </code>
*/  
inline static void PWM_SoftwareUpdateRequest(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1STATbits.UPDREQ = 1;              
                break;       
        case AUX:
                PG2STATbits.UPDREQ = 1;              
                break;       
        case LED_OUT:
                PG3STATbits.UPDREQ = 1;              
                break;       
        case LIFT:
                PG4STATbits.UPDREQ = 1;              
                break;       
        case SAFETY_PWM:
                PG5STATbits.UPDREQ = 1;              
                break;       
        case ADC_TIMER:
                PG6STATbits.UPDREQ = 1;              
                break;       
        default:break;    
    }

}

/**
  @Summary
    Gets the status of the update request for specific PWM generator.

  @Description
    This routine is used to get the status of the update request for specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    bool status;
    
    status = PWM_SoftwareUpdatePending(PWM_GENERATOR_1);
    </code>
*/  
inline static bool PWM_SoftwareUpdatePending(PWM_GENERATOR genNum)
{
    bool status = false;
    switch(genNum) { 
        case ULTRASONIC:
                status = PG1STATbits.UPDATE;              
                break;       
        case AUX:
                status = PG2STATbits.UPDATE;              
                break;       
        case LED_OUT:
                status = PG3STATbits.UPDATE;              
                break;       
        case LIFT:
                status = PG4STATbits.UPDATE;              
                break;       
        case SAFETY_PWM:
                status = PG5STATbits.UPDATE;              
                break;       
        case ADC_TIMER:
                status = PG6STATbits.UPDATE;              
                break;       
        default:break;   
    }
    return status;
}

/**
  @Summary
    Clears the status of PWM latched fault mode for the PWM Generator.

  @Description
    This routine is used to clear the status of PWM latched fault mode 
    for the PWM Generator selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    
    genNum = PWM_GENERATOR_1;
    PWM_FaultModeLatchClear(genNum);
    </code>
*/  
inline static void PWM_FaultModeLatchClear(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC: 
                PG1FPCILbits.SWTERM = 1;
                break;   
        case AUX: 
                PG2FPCILbits.SWTERM = 1;
                break;   
        case LED_OUT: 
                PG3FPCILbits.SWTERM = 1;
                break;   
        case LIFT: 
                PG4FPCILbits.SWTERM = 1;
                break;   
        case SAFETY_PWM: 
                PG5FPCILbits.SWTERM = 1;
                break;   
        case ADC_TIMER: 
                PG6FPCILbits.SWTERM = 1;
                break;   
        default:break;   
    }   
}

/**
  @Summary
    Callback for PWM1 interrupt.

  @Description
    This routine is callback for PWM1 interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_Generator1_CallBack();
    </code>
*/
void PWM_Generator1_CallBack(void);

/**
  @Summary
    Assigns a function pointer with a PWM Generator1 callback address.

  @Description
    This routine assigns a function pointer with a PWM Generator1 
    callback address.

  @Param
    Address of the callback routine.

  @Returns
    None
 
  @Example 
    <code>
    PWM_SetGenerator1InterruptHandler(&PWM_Generator1_CallBack);
    </code>
*/
void PWM_SetGenerator1InterruptHandler(void *handler);

/**
  @Summary
    Callback for PWM2 interrupt.

  @Description
    This routine is callback for PWM2 interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_Generator2_CallBack();
    </code>
*/
void PWM_Generator2_CallBack(void);

/**
  @Summary
    Assigns a function pointer with a PWM Generator2 callback address.

  @Description
    This routine assigns a function pointer with a PWM Generator2 
    callback address.

  @Param
    Address of the callback routine.

  @Returns
    None
 
  @Example 
    <code>
    PWM_SetGenerator2InterruptHandler(&PWM_Generator2_CallBack);
    </code>
*/
void PWM_SetGenerator2InterruptHandler(void *handler);

/**
  @Summary
    Callback for PWM3 interrupt.

  @Description
    This routine is callback for PWM3 interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_Generator3_CallBack();
    </code>
*/
void PWM_Generator3_CallBack(void);

/**
  @Summary
    Assigns a function pointer with a PWM Generator3 callback address.

  @Description
    This routine assigns a function pointer with a PWM Generator3 
    callback address.

  @Param
    Address of the callback routine.

  @Returns
    None
 
  @Example 
    <code>
    PWM_SetGenerator3InterruptHandler(&PWM_Generator3_CallBack);
    </code>
*/
void PWM_SetGenerator3InterruptHandler(void *handler);

/**
  @Summary
    Callback for PWM4 interrupt.

  @Description
    This routine is callback for PWM4 interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_Generator4_CallBack();
    </code>
*/
void PWM_Generator4_CallBack(void);

/**
  @Summary
    Assigns a function pointer with a PWM Generator4 callback address.

  @Description
    This routine assigns a function pointer with a PWM Generator4 
    callback address.

  @Param
    Address of the callback routine.

  @Returns
    None
 
  @Example 
    <code>
    PWM_SetGenerator4InterruptHandler(&PWM_Generator4_CallBack);
    </code>
*/
void PWM_SetGenerator4InterruptHandler(void *handler);

/**
  @Summary
    Callback for PWM5 interrupt.

  @Description
    This routine is callback for PWM5 interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_Generator5_CallBack();
    </code>
*/
void PWM_Generator5_CallBack(void);

/**
  @Summary
    Tasks routine for PWM5.

  @Description
    This is a tasks routine for PWM5.

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_Generator5_Tasks();
    </code>
*/
void PWM_Generator5_Tasks(void);

/**
  @Summary
    Callback for PWM6 interrupt.

  @Description
    This routine is callback for PWM6 interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_Generator6_CallBack();
    </code>
*/
void PWM_Generator6_CallBack(void);

/**
  @Summary
    Assigns a function pointer with a PWM Generator6 callback address.

  @Description
    This routine assigns a function pointer with a PWM Generator6 
    callback address.

  @Param
    Address of the callback routine.

  @Returns
    None
 
  @Example 
    <code>
    PWM_SetGenerator6InterruptHandler(&PWM_Generator6_CallBack);
    </code>
*/
void PWM_SetGenerator6InterruptHandler(void *handler);


/**
  @Summary
    Callback for EventA interrupt.

  @Description
    This routine is callback for EventA interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventA_CallBack();
    </code>
*/
void PWM_EventA_CallBack(void);

/**
  @Summary
    Tasks routine for EventA.

  @Description
    This is a tasks routine for EventA.

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventA_Tasks();
    </code>
*/
void PWM_EventA_Tasks(void);

/**
  @Summary
    Callback for EventB interrupt.

  @Description
    This routine is callback for EventB interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventB_CallBack();
    </code>
*/
void PWM_EventB_CallBack(void);

/**
  @Summary
    Tasks routine for EventB.

  @Description
    This is a tasks routine for EventB.

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventB_Tasks();
    </code>
*/
void PWM_EventB_Tasks(void);

/**
  @Summary
    Callback for EventC interrupt.

  @Description
    This routine is callback for EventC interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventC_CallBack();
    </code>
*/
void PWM_EventC_CallBack(void);

/**
  @Summary
    Assigns a function pointer with a PWM EventC callback address.

  @Description
    This routine assigns a function pointer with a PWM EventC 
    callback address.

  @Param
    Address of the callback routine.

  @Returns
    None
 
  @Example 
    <code>
    PWM_SetEventCInterruptHandler(&PWM_EventC_CallBack);
    </code>
*/
void PWM_SetEventCInterruptHandler(void *handler);

/**
  @Summary
    Callback for EventD interrupt.

  @Description
    This routine is callback for EventD interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventD_CallBack();
    </code>
*/
void PWM_EventD_CallBack(void);

/**
  @Summary
    Tasks routine for EventD.

  @Description
    This is a tasks routine for EventD.

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventD_Tasks();
    </code>
*/
void PWM_EventD_Tasks(void);

/**
  @Summary
    Callback for EventE interrupt.

  @Description
    This routine is callback for EventE interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventE_CallBack();
    </code>
*/
void PWM_EventE_CallBack(void);

/**
  @Summary
    Tasks routine for EventE.

  @Description
    This is a tasks routine for EventE.

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventE_Tasks();
    </code>
*/
void PWM_EventE_Tasks(void);

/**
  @Summary
    Callback for EventF interrupt.

  @Description
    This routine is callback for EventF interrupt

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventF_CallBack();
    </code>
*/
void PWM_EventF_CallBack(void);

/**
  @Summary
    Tasks routine for EventF.

  @Description
    This is a tasks routine for EventF.

  @Param
    None.

  @Returns
    None.
 
  @Example 
    <code>    
    PWM_EventF_Tasks();
    </code>
*/
void PWM_EventF_Tasks(void);


/*******************************************************************************

  !!! Deprecated Definitions and APIs !!!
  !!! These functions will not be supported in future releases !!!

*******************************************************************************/
/**
  @Summary
    Enables the specific PWM generator.

  @Description
    This routine is used to enable the specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    
    genNum = PWM_GENERATOR_1;
    PWM_ModuleEnable(genNum);
    </code>
*/
inline static void __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_GeneratorEnable instead."))) PWM_ModuleEnable(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1CONLbits.ON = 1;              
                break;       
        case AUX:
                PG2CONLbits.ON = 1;              
                break;       
        case LED_OUT:
                PG3CONLbits.ON = 1;              
                break;       
        case LIFT:
                PG4CONLbits.ON = 1;              
                break;       
        case SAFETY_PWM:
                PG5CONLbits.ON = 1;              
                break;       
        case ADC_TIMER:
                PG6CONLbits.ON = 1;              
                break;       
        default:break;    
    }     
}

/**
  @Summary
    Disables the specific PWM generator.

  @Description
    This routine is used to disable the specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_GENERATOR genNum;
    
    genNum = PWM_GENERATOR_1;
    PWM_ModuleDisable(genNum);
    </code>
*/
inline static void __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_GeneratorDisable instead."))) PWM_ModuleDisable(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1CONLbits.ON = 0;              
                break;       
        case AUX:
                PG2CONLbits.ON = 0;              
                break;       
        case LED_OUT:
                PG3CONLbits.ON = 0;              
                break;       
        case LIFT:
                PG4CONLbits.ON = 0;              
                break;       
        case SAFETY_PWM:
                PG5CONLbits.ON = 0;              
                break;       
        case ADC_TIMER:
                PG6CONLbits.ON = 0;              
                break;       
        default:break;    
    }    
}

/**
  @Summary
    Enables/Disables PWM override on PWML output for specific PWM generator.

  @Description
    This routine is used to enable/disable PWM override on PWML output for specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    enableOverride  - Data to enable or disable override low. 

  @Returns
    None
 
  @Example 
    <code>    
    PWM_OverrideLowEnableSet(PWM_GENERATOR_1, enableOverride);
    </code>
*/  
inline static void __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_OverrideLowEnable and PWM_OverrideLowDisable instead."))) PWM_OverrideLowEnableSet(PWM_GENERATOR genNum, bool enableOverride)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRENL = enableOverride;              
                break;       
        case AUX:
                PG2IOCONLbits.OVRENL = enableOverride;              
                break;       
        case LED_OUT:
                PG3IOCONLbits.OVRENL = enableOverride;              
                break;       
        case LIFT:
                PG4IOCONLbits.OVRENL = enableOverride;              
                break;       
        case SAFETY_PWM:
                PG5IOCONLbits.OVRENL = enableOverride;              
                break;       
        case ADC_TIMER:
                PG6IOCONLbits.OVRENL = enableOverride;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Enables/Disables PWM override on PWMH output for specific PWM generator.

  @Description
    This routine is used to enable/disable PWM override on PWMH output for specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    enableOverride  - Data to enable or disable override high. 

  @Returns
    None
 
  @Example 
    <code>    
    PWM_OverrideHighEnableSet(PWM_GENERATOR_1, enableOverride);
    </code>
*/  
inline static void __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_OverrideHighEnable and PWM_OverrideHighDisable instead."))) PWM_OverrideHighEnableSet(PWM_GENERATOR genNum, bool enableOverride)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1IOCONLbits.OVRENH = enableOverride;              
                break;       
        case AUX:
                PG2IOCONLbits.OVRENH = enableOverride;              
                break;       
        case LED_OUT:
                PG3IOCONLbits.OVRENH = enableOverride;              
                break;       
        case LIFT:
                PG4IOCONLbits.OVRENH = enableOverride;              
                break;       
        case SAFETY_PWM:
                PG5IOCONLbits.OVRENH = enableOverride;              
                break;       
        case ADC_TIMER:
                PG6IOCONLbits.OVRENH = enableOverride;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Sets the TRIGA compare value in count for a specific PWM generator.

  @Description
    This routine is used to set the TRIGA compare value in count for a specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    trigA  - TRIGA compare value in count.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t trigA;
    
    trigA = 0xF;
    PWM_TRIGACompareSet(PWM_GENERATOR_1, trigA);
    </code>
*/
inline static void __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_TriggerACompareValueSet instead."))) PWM_TRIGACompareSet(PWM_GENERATOR genNum,uint16_t trigA)
{ 
    switch(genNum) { 
        case ULTRASONIC:
                PG1TRIGA = trigA;              
                break;       
        case AUX:
                PG2TRIGA = trigA;              
                break;       
        case LED_OUT:
                PG3TRIGA = trigA;              
                break;       
        case LIFT:
                PG4TRIGA = trigA;              
                break;       
        case SAFETY_PWM:
                PG5TRIGA = trigA;              
                break;       
        case ADC_TIMER:
                PG6TRIGA = trigA;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Sets the TRIGB compare value in count for a specific PWM generator.

  @Description
    This routine is used to set the TRIGB compare value in count for a specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    trigB  - TRIGB compare value in count.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t trigB;
    
    trigB = 0xF;
    PWM_TRIGBCompareSet(PWM_GENERATOR_1, trigB);
    </code>
*/
inline static void __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_TriggerBCompareValueSet instead."))) PWM_TRIGBCompareSet(PWM_GENERATOR genNum,uint16_t trigB)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1TRIGB = trigB;              
                break;       
        case AUX:
                PG2TRIGB = trigB;              
                break;       
        case LED_OUT:
                PG3TRIGB = trigB;              
                break;       
        case LIFT:
                PG4TRIGB = trigB;              
                break;       
        case SAFETY_PWM:
                PG5TRIGB = trigB;              
                break;       
        case ADC_TIMER:
                PG6TRIGB = trigB;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Sets the TRIGC compare value in count for a specific PWM generator.

  @Description
    This routine is used to set the TRIGC compare value in count for a specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.
    trigC  - TRIGC compare value in count.

  @Returns
    None
 
  @Example 
    <code>    
    uint16_t trigC;
    
    trigC = 0xF;
    PWM_TRIGCCompareSet(PWM_GENERATOR_1, trigC);
    </code>
*/
inline static void __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_TriggerCCompareValueSet instead."))) PWM_TRIGCCompareSet(PWM_GENERATOR genNum,uint16_t trigC)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1TRIGC = trigC;              
                break;       
        case AUX:
                PG2TRIGC = trigC;              
                break;       
        case LED_OUT:
                PG3TRIGC = trigC;              
                break;       
        case LIFT:
                PG4TRIGC = trigC;              
                break;       
        case SAFETY_PWM:
                PG5TRIGC = trigC;              
                break;       
        case ADC_TIMER:
                PG6TRIGC = trigC;              
                break;       
        default:break;    
    }
}

/**
  @Summary
    Requests to update the data registers for specific PWM generator.

  @Description
    This routine is used to request to update the data registers for specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    PWM_DataUpdateRequestSet(PWM_GENERATOR_1);
    </code>
*/  
inline static void __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_SoftwareUpdateRequest instead."))) PWM_DataUpdateRequestSet(PWM_GENERATOR genNum)
{
    switch(genNum) { 
        case ULTRASONIC:
                PG1STATbits.UPDREQ = 1;              
                break;       
        case AUX:
                PG2STATbits.UPDREQ = 1;              
                break;       
        case LED_OUT:
                PG3STATbits.UPDREQ = 1;              
                break;       
        case LIFT:
                PG4STATbits.UPDREQ = 1;              
                break;       
        case SAFETY_PWM:
                PG5STATbits.UPDREQ = 1;              
                break;       
        case ADC_TIMER:
                PG6STATbits.UPDREQ = 1;              
                break;       
        default:break;    
    }

}

/**
  @Summary
    Gets the status of the update request for specific PWM generator.

  @Description
    This routine is used to get the status of the update request for specific PWM generator 
    selected by the argument PWM_GENERATOR.

  @Param
    genNum - PWM generator number.

  @Returns
    None
 
  @Example 
    <code>    
    bool status;
    
    status = PWM_DataUpdateStatusGet(PWM_GENERATOR_1);
    </code>
*/  
inline static bool __attribute__((deprecated("\nThis will be removed in future MCC releases. \nUse PWM_SoftwareUpdatePending instead."))) PWM_DataUpdateStatusGet(PWM_GENERATOR genNum)
{
    bool status = false;
    switch(genNum) { 
        case ULTRASONIC:
                status = PG1STATbits.UPDATE;              
                break;       
        case AUX:
                status = PG2STATbits.UPDATE;              
                break;       
        case LED_OUT:
                status = PG3STATbits.UPDATE;              
                break;       
        case LIFT:
                status = PG4STATbits.UPDATE;              
                break;       
        case SAFETY_PWM:
                status = PG5STATbits.UPDATE;              
                break;       
        case ADC_TIMER:
                status = PG6STATbits.UPDATE;              
                break;       
        default:break;   
    }
    return status;
}

#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif

#endif //PWM_H
