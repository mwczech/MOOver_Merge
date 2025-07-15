/* 
 * File:   AnalogHandler.h
 * Author: pawelton
 *
 * Created on 29 maja 2023, 19:58
 */


#ifndef ANALOGHANDLER_H
#define	ANALOGHANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include "../mcc_generated_files/adc1.h"
    
#define dMEASUREMENT_ROUGH         0
#define dMEASUREMENT_FILTERED      1    
     
extern bool SafetySwitchState;
    
uint16_t AnalogHandler_GetADCFiltered(ADC1_CHANNEL Name);
uint16_t AnalogHandler_GetADCRough(ADC1_CHANNEL Name);
void AnalogHandler_SetChannelUpperThreshold(ADC1_CHANNEL Name, uint16_t Threshold);
void AnalogHandler_SetChannelLowerThreshold(ADC1_CHANNEL Name, uint16_t Threshold);
bool AnalogHandler_IsUpperThresholdExceeded(ADC1_CHANNEL Name, uint8_t MeasurementType);
bool AnalogHandler_IsLowerThresholdExceeded(ADC1_CHANNEL Name, uint8_t MeasurementType);
bool AnalogHandler_IsSafetyActivated();
void AnalogHandler_Init(void);
void AnalogHandler_Perform100ms(void);

#ifdef	__cplusplus
}
#endif

#endif	/* ANALOGHANDLER_H */

