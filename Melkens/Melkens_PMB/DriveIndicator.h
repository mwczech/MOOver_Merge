#include "stdbool.h"
#include "stdint.h"

typedef enum IndicationType_t{
    dIndicationType_Buzzer = 0,
    dIndicationType_Lamp,
    dIndicationType_Both,
    dIndicationType_NumOf,

}IndicationType;

void DriveIndicator_Init(void);
void DriveIndicator_1msPerform(void);
bool IsIndicatorEnable(void);
bool DriveIndicator_IsFinishedIndication(void);
void DriveIndicator_SetEnable(void);
void DriveIndicator_BuzzerOnly_SetEnable(void);
void DriveIndicator_LampOnly_SetEnable(void);
void DriveIndicator_BuzzerOnly_Init(void);
void DriveIndicator_LampOnly_Init(void);

void DriveIndicator_SetIndication(uint32_t BuzzerMs, uint32_t LampMs);
void DriveIndicator_SetDisable(IndicationType Indication);