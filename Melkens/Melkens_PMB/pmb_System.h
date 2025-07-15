/* 
 * File:   pmb_System.h
 * Author: jackthehw
 *
 * Created on 15 pa?dziernika 2023, 19:29
 */

#ifndef PMB_SYSTEM_H
#define	PMB_SYSTEM_H

#include <xc.h> // include processor files - each processor file is guarded. 
#include "stdbool.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define COMPILE_SWITCH_MOOVER_1 1
#define COMPILE_SWITCH_MOOVER_3 0
#define COMPILE_SWITCH_MOONION 0
    
//Machine Defines
#define GEAR_SHIFT_N 100 //Ratio of the Gear Shift for wheels
#define GEAR_SHIFT_N_THUMBLE 25 //Ratio of the Gear Shift for thumble
#define CORRECTION 6

#define RADIAN 6283 //6,2832 * 1000

typedef enum PowerSequenceNames_t{
    Sequence_PowerStageOn = 0,
    Sequence_PowerStageOff,
    Sequence_ChargerOn,
    Sequence_ChargerOff,
    Sequence_NumOf
}PowerSequenceNames;

//Analog Functions
uint32_t CalculateCurrent (uint16_t ADCcnt);
uint32_t CalculateVoltage (uint16_t ADCcnt);
    
void System_Handle(void);
void System_SetInitial(void);
void System_SetDisabled(void);
void System_SetTimeTo(void);
void System_ResetTimeTo(void);
void System_Init(void);
void System_PowerStageSequence(void);
void DisableAllPowerStages(void);
void System_Perform1ms(void);
void System_PowerRailRequestSequence(PowerSequenceNames Name);
void System_DisableRail(PowerSequenceNames Name);
bool System_GetPowerRailState(void);
bool System_GetChargerState(void);

#ifdef	__cplusplus
}
#endif

#endif	/* PMB_SYSTEM_H */

