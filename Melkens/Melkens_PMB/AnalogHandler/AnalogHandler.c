#include "AnalogHandler.h"
#include "../mcc_generated_files/adc1.h"

#define STATE_INIT     0
#define STATE_MEASURE  1
#define STATE_IDLE     2

#define ENABLE 1
#define DISABLE 0

/* How many loops before average enabled */
#define AVERAGE_FILTER_COUNT 10
#define INIT_DEBOUNCE_COUNT  10

bool SafetySwitchState;

typedef struct AnalogMeasurement_t
{
    ADC1_CHANNEL Channel;
    uint16_t CurrentRoughValue;
    uint16_t PreviousRoughValue;
    uint16_t UpperThreshold;
    uint16_t LowerThreshold;
    
    bool Initialized;
    bool UpperThresholdExceeded;
    bool LowerThresholdExceeded;
    
    uint16_t FilteredValue;
    uint32_t FilterAccumulator;
}AnalogMeasurement;

AnalogMeasurement Measurements[ADC1_ChannelNumOf];
uint8_t State;
uint8_t InitDebounce;

void AnalogHandler_CalculateAverageAll( void );
void AnalogHandler_AddToFilterAll( void );
void AnalogHandler_AddToFiter(uint16_t Measurement, ADC1_CHANNEL Name);
void AnalogHandler_DoMeasureBlocking( void );
void AnalogHandler_UpdateAndCalculateAverageAll( void );
void AnalogHandler_UpdateAndCalculateAverage(ADC1_CHANNEL Name);


void AnalogHandler_Init(void)
{
    State = STATE_INIT;
    InitDebounce = AVERAGE_FILTER_COUNT;
    
    Measurements[DC_STATUS_HS].UpperThresholdExceeded =  false;
    Measurements[StatSw2].UpperThresholdExceeded =       false;
    Measurements[StatSw1].UpperThresholdExceeded =       false;
    Measurements[DC_STATUS_LS].UpperThresholdExceeded =  false;
    Measurements[IM_SENSE].UpperThresholdExceeded =      false;
    Measurements[StatSw3].UpperThresholdExceeded =       false;
    Measurements[StatSw4].UpperThresholdExceeded =       false;
    
    Measurements[DC_STATUS_HS].LowerThresholdExceeded =  false;
    Measurements[StatSw2].LowerThresholdExceeded =       false;
    Measurements[StatSw1].LowerThresholdExceeded =       false;
    Measurements[DC_STATUS_LS].LowerThresholdExceeded =  false;
    Measurements[IM_SENSE].LowerThresholdExceeded =      false;
    Measurements[StatSw3].LowerThresholdExceeded =       false;
    Measurements[StatSw4].LowerThresholdExceeded =       false;
    
    AnalogHandler_SetChannelUpperThreshold(DC_STATUS_HS, 2500);
    AnalogHandler_SetChannelUpperThreshold(StatSw2,      2500);
    AnalogHandler_SetChannelUpperThreshold(StatSw1,      2500);
    AnalogHandler_SetChannelUpperThreshold(DC_STATUS_LS, 2500);
    AnalogHandler_SetChannelUpperThreshold(IM_SENSE,     2500);
    AnalogHandler_SetChannelUpperThreshold(StatSw3,      2500);
    AnalogHandler_SetChannelUpperThreshold(StatSw4,      2500); 
    
    AnalogHandler_SetChannelLowerThreshold(DC_STATUS_HS,      100);

    AnalogHandler_SetChannelLowerThreshold(StatSw1,      1000);
    AnalogHandler_SetChannelLowerThreshold(StatSw2,      1000);
    AnalogHandler_SetChannelLowerThreshold(StatSw3,      1000);
    AnalogHandler_SetChannelLowerThreshold(StatSw3,      1000);

    
    
}

void AnalogHandler_Perform100ms(void)
{
    switch(State)
    {
        case STATE_INIT:
            
            
            AnalogHandler_DoMeasureBlocking();
            AnalogHandler_AddToFilterAll();
            if( InitDebounce > 10 )
            {
                InitDebounce++;
                
            }
            
            else
            {
                /*  Done! Calculate average*/
                AnalogHandler_CalculateAverageAll();
                State = STATE_MEASURE;
            }
            break;
            
        case STATE_MEASURE:
            AnalogHandler_DoMeasureBlocking();
            AnalogHandler_UpdateAndCalculateAverageAll(); 
            
         
            //AnalogHandler_CalculateThresholds(dMEASUREMENT_ROUGH);
            
            
            break;
            
        case STATE_IDLE:
            break;
        default:
            break;
    }
}

void AnalogHandler_AddToFilterAll( void )
{
    ADC1_CHANNEL Idx;

    for(Idx = DC_STATUS_HS; Idx < ADC1_ChannelNumOf; Idx++ )
    {
        Measurements[Idx].FilterAccumulator += Measurements[Idx].CurrentRoughValue;
    }
}

void AnalogHandler_AddToFiter(uint16_t Measurement, ADC1_CHANNEL Name)
{
    Measurements[Name].FilterAccumulator += Measurement;
}

void AnalogHandler_DoMeasureBlocking( void )
{
    ADC1_CHANNEL Idx;
    
    ADC1_SoftwareTriggerEnable();
    /* TODO: pawelton Check where this function should be called */
    ADC1_SoftwareTriggerDisable();
    /* Go through all channels and gather results */
    for(Idx = DC_STATUS_HS; Idx < ADC1_ChannelNumOf; Idx++ )
    {
        while(!ADC1_IsConversionComplete(Idx));
                
        Measurements[Idx].CurrentRoughValue = ADC1_ConversionResultGet(Idx);
    }
}

void AnalogHandler_CalculateAverageAll( void )
{
    for(ADC1_CHANNEL CurrentName = 0; CurrentName<ADC1_ChannelNumOf; CurrentName++)
    {
        Measurements[CurrentName].FilteredValue = 
                (uint16_t)(Measurements[CurrentName].FilterAccumulator / INIT_DEBOUNCE_COUNT);
    }
}

void AnalogHandler_UpdateAndCalculateAverageAll( void )
{   
    ADC1_CHANNEL Idx;

    for(Idx = DC_STATUS_HS; Idx < ADC1_ChannelNumOf; Idx++ )
    {
        Measurements[Idx].FilterAccumulator -= Measurements[Idx].FilteredValue;
        Measurements[Idx].FilterAccumulator += Measurements[Idx].CurrentRoughValue;
        Measurements[Idx].FilteredValue = 
        (uint16_t)(Measurements[Idx].FilterAccumulator / AVERAGE_FILTER_COUNT);        
    }
}

void AnalogHandler_UpdateAndCalculateAverage(ADC1_CHANNEL Name)
{   
    Measurements[Name].FilterAccumulator -= Measurements[Name].FilteredValue;
    Measurements[Name].FilterAccumulator += Measurements[Name].CurrentRoughValue;
    Measurements[Name].FilteredValue = 
            (uint16_t)(Measurements[Name].FilterAccumulator / AVERAGE_FILTER_COUNT);
}

uint16_t AnalogHandler_GetADCFiltered(ADC1_CHANNEL Name)
{
    return Measurements[Name].FilteredValue;
}

uint16_t AnalogHandler_GetADCRough(ADC1_CHANNEL Name)
{
    return Measurements[Name].CurrentRoughValue;
}

//void AnalogHandler_CalculateThresholds(uint8_t MeasurementType)
//{
//    ADC1_CHANNEL ChannelName;
//    AnalogMeasurement *pMeasurement;
//
//    for(ChannelName= DC_STATUS_HS; ChannelName<ADC1_ChannelNumOf; ChannelName++)
//    {
//        pMeasurement = &Measurements[ChannelName];
//        if( MeasurementType == dMEASUREMENT_FILTERED )
//        {
//            if( pMeasurement.FilteredValue > pMeasurement.Threshold)
//            {
//                pMeasurement->ThresholdExceeded = true;
//            }
//            else
//            {
//            
//            }
//        }
//        else if( MeasurementType == dMEASUREMENT_ROUGH )
//        {
//        
//        }
//    }
//}

void AnalogHandler_SetChannelUpperThreshold(ADC1_CHANNEL Name, uint16_t Threshold)
{
    Measurements[Name].UpperThreshold = Threshold;
}

void AnalogHandler_SetChannelLowerThreshold(ADC1_CHANNEL Name, uint16_t Threshold)
{
    Measurements[Name].LowerThreshold = Threshold;
}


bool AnalogHandler_IsUpperThresholdExceeded(ADC1_CHANNEL Name, uint8_t MeasurementType)
{
    bool RetFlag = false;
    AnalogMeasurement *pMeasurement;
    pMeasurement = &Measurements[Name];
    
    if( MeasurementType == dMEASUREMENT_FILTERED )
    {
        if( pMeasurement->FilteredValue > pMeasurement->UpperThreshold )
        {
            RetFlag = true;
        }
        
    }
    else if( MeasurementType == dMEASUREMENT_ROUGH )
    {
        if( pMeasurement->CurrentRoughValue > pMeasurement->UpperThreshold )
        {
            RetFlag = true;
        }
    }

    return RetFlag;       
}


bool AnalogHandler_IsLowerThresholdExceeded(ADC1_CHANNEL Name, uint8_t MeasurementType)
{
    bool RetFlag = false;
    AnalogMeasurement *pMeasurement;
    pMeasurement = &Measurements[Name];
    
    if( MeasurementType == dMEASUREMENT_FILTERED )
    {
        if( pMeasurement->FilteredValue < pMeasurement->LowerThreshold )
        {
            RetFlag = true;
        }
        
    }
    else if( MeasurementType == dMEASUREMENT_ROUGH )
    {
        if( pMeasurement->CurrentRoughValue < pMeasurement->LowerThreshold )
        {
            RetFlag = true;
        }
    }

    return RetFlag;       
}

bool AnalogHandler_IsSafetyActivated(){
    bool RetFlag = false;
    if(AnalogHandler_IsLowerThresholdExceeded(StatSw1, dMEASUREMENT_ROUGH)
        || AnalogHandler_IsLowerThresholdExceeded(StatSw2, dMEASUREMENT_ROUGH)
        || AnalogHandler_IsLowerThresholdExceeded(StatSw3, dMEASUREMENT_ROUGH)
        || AnalogHandler_IsLowerThresholdExceeded(StatSw4, dMEASUREMENT_ROUGH))
        {
            RetFlag = true;
        }
    return RetFlag;
}

