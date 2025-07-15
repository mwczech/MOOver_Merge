#include "pmb_Scheduler.h"
#include "RoutesDataTypes.h"
#include "Tools/Tools.h"
#include "mcc_generated_files/memory/flash.h"
#include <stdbool.h>

#define dFLASH_PAGE_LOCATION 0x7000
#define dFLASH_PAGE_SIZE 2048
Time CurrentTime;
WeekDay CurrentWeekday;
Scheduler ScheduleTimer[dTimer_NumOf];
Route_ID RootActivated[dTimer_NumOf];
uint8_t SchedulerBlockTimer = 0;
bool IsRTCInitialized = false;

uint8_t flashSchedulerPage[FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS] __attribute__((space(prog), address(dFLASH_PAGE_LOCATION), section(".scheduler")));

bool IsScheduledTimeEqualCurrent(uint8_t TimerID);
void Scheduler_Count1s(void);

void Scheduler_Init(void){
    uint32_t addressCounter;
    uint32_t dummyByteFirst;
    uint32_t dummyByteLast;
    uint32_t flash_storage_address;
    uint32_t *pData;
    uint32_t timersSize = sizeof(ScheduleTimer);
    
    RootActivated[dTimer_1] = Route_NumOf;
    RootActivated[dTimer_2] = Route_NumOf;
    RootActivated[dTimer_3] = Route_NumOf;
    RootActivated[dTimer_4] = Route_NumOf;
    
    pData = &ScheduleTimer[dTimer_1].startTime.Hour;

    flash_storage_address = dFLASH_PAGE_LOCATION;
    /* Check if timer settings are set in Flash memory */
    dummyByteFirst = FLASH_ReadWord24(flash_storage_address);
    /* Offset to get value from last possible page address */
    dummyByteLast = FLASH_ReadWord24(flash_storage_address + dFLASH_PAGE_SIZE - 2);
    flash_storage_address = flash_storage_address + 4;
    if((dummyByteFirst == 0xA5) && (dummyByteLast = 0xA5)){    
        for(addressCounter = 0; addressCounter < timersSize/2; addressCounter++){
            *pData = FLASH_ReadWord24(flash_storage_address );
            pData++;
            *pData = FLASH_ReadWord24(flash_storage_address +2);
            pData++;
            flash_storage_address = flash_storage_address + 4;
        }
    }
}

void Scheduler_Perform1s(void){
    /* In case of moving keeping time to PMB, uncomment clock counter function below */
    Scheduler_Count1s();
    if(!IsRTCInitialized){
        /* Do not take any scheduler actions, if current time is not set */
    }
    else{
        uint8_t i;
        /* Current time set, check if route should start */
        for(i=0; i<dTimer_NumOf; i++){
            if( ScheduleTimer[i].enabled ){
                if(IsScheduledTimeEqualCurrent(i)){
                    if( SchedulerBlockTimer > 0 ){
                    }
                    else{
                        RootActivated[i] = ScheduleTimer[i].routeName;
                        SchedulerBlockTimer = SCHEDULER_BLOCK_COUNT;
                    }                    
                }
            }
        }
    }
}

void Scheduler_Count1s(void){
//    CurrentTime.Second = CurrentTime.Second + 1;
//    if(CurrentTime.Second >= 60){
//        CurrentTime.Second = 0;
//        CurrentTime.Minute = CurrentTime.Minute + 1;
//        if(CurrentTime.Minute >= 60){
//            CurrentTime.Minute = 0;
//            CurrentTime.Hour = CurrentTime.Hour + 1;
//            if(CurrentTime.Hour >= 24){
//                CurrentTime.Hour = 0;
//                CurrentWeekday = CurrentWeekday + 1;
//                if( CurrentWeekday >= 6 ){
//                    CurrentWeekday = 0;
//                }
//            }
//        }
//    }
    if( SchedulerBlockTimer > 0 ){
        SchedulerBlockTimer = SchedulerBlockTimer - 1;
    }
}

void Scheduler_SetSchedule(Time TimeToSet, TimerName TimerID, Route_ID RouteID, uint8_t Days){
    ScheduleTimer[TimerID].startTime.Hour = TimeToSet.Hour;
    ScheduleTimer[TimerID].startTime.Minute = TimeToSet.Minute;
    ScheduleTimer[TimerID].startTime.Second = 0;
    ScheduleTimer[TimerID].routeName = RouteID;
    ScheduleTimer[TimerID].days = (uint32_t)Days;

    ScheduleTimer[TimerID].enabled = true;
}

Scheduler Scheduler_GetSchedule(TimerName TimerID){
    return ScheduleTimer[TimerID];
}

void Scheduler_DisableSchedule(TimerName TimerID){
    ScheduleTimer[TimerID].startTime.Hour = 0;
    ScheduleTimer[TimerID].startTime.Minute = 0;
    ScheduleTimer[TimerID].startTime.Second = 0;
    ScheduleTimer[TimerID].routeName = Route_NumOf;
    ScheduleTimer[TimerID].days = (uint32_t)0;
    ScheduleTimer[TimerID].enabled = 0;
}

void Scheduler_SetCurrentTime(uint8_t Day, uint8_t Hour, uint8_t Minute){
    CurrentTime.Hour = (uint32_t)Hour;
    CurrentTime.Minute = (uint32_t)Minute;
       
    CurrentWeekday = (WeekDay)Day;

    if(!IsRTCInitialized){
        IsRTCInitialized = true;
    }
}

Route_ID Scheduler_GetRouteFromScheduler(void){
    Route_ID Ret = Route_NumOf;
    uint8_t i;

    for(i=0; i<dTimer_NumOf; i++){
        if(RootActivated[i] != Route_NumOf){
            Ret = RootActivated[i];
            RootActivated[i] = Route_NumOf;
            break;
        }
    }
    return Ret;
}

bool IsScheduledTimeEqualCurrent(uint8_t TimerID){
    bool Ret = false;
    
    if(isBitSet(ScheduleTimer[TimerID].days ,(uint8_t)CurrentWeekday ))
        /* Today's day matches corresponding true flag */
        if(CurrentTime.Hour == ScheduleTimer[TimerID].startTime.Hour
        && CurrentTime.Minute == ScheduleTimer[TimerID].startTime.Minute)
            Ret = true;

    return Ret;
}

bool Scheduler_EraseFlash(void){
    bool Ret;
    Ret = FLASH_ErasePage(dFLASH_PAGE_LOCATION);
    return Ret;
}

static void FlashError()
{
    while (1) 
    { }
}

bool Scheduler_SaveToFlash(void){
    uint32_t DataSize = sizeof(ScheduleTimer);
    uint32_t *pData = &ScheduleTimer[dTimer_1].startTime.Hour;
    uint16_t currentFlashAddress;
    uint16_t flashOffset;
    bool success;
    
    FLASH_Unlock(FLASH_UNLOCK_KEY);
    
    success = FLASH_ErasePage(dFLASH_PAGE_LOCATION);
    if (success == false)
    {
        FlashError();
    }
    currentFlashAddress = dFLASH_PAGE_LOCATION;
    /* Write pattern at the beginning of page */
    success  = FLASH_WriteDoubleWord24(dFLASH_PAGE_LOCATION, 0xA5, 0x08);//    
    if(!success){
        return success;
    }
   
    /* Set addres next to first delimiter bytes -4 addresess, because one 24-bit value fits to one address */
    currentFlashAddress = currentFlashAddress + 4;
    for (flashOffset= 0U; flashOffset < DataSize/2  ; flashOffset += 4U){
       success = FLASH_WriteDoubleWord24(currentFlashAddress+flashOffset, *pData, *(pData+1U));
        //success = FLASH_WriteDoubleWord24(currentFlashAddress+flashOffset, 0x12, 0x34);
        //success = FLASH_WriteDoubleWord24(dFLASH_PAGE_LOCATION + FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS - 4, 0x23, 0x45);
        if(!success){
            return success;
        }
       pData+=2;
   }
    

    success  = FLASH_WriteDoubleWord24(currentFlashAddress + dFLASH_PAGE_SIZE - 8, 0x5A, 0xA5);
    if(!success){
        return success;
    }
    
    /* Write pattern at the end of page */
    //success  = FLASH_WriteDoubleWord24(dFLASH_PAGE_LOCATION + FLASH_ERASE_PAGE_SIZE_IN_PC_UNITS - 4, 0xA5, 0x08);//    
 
 
    
   FLASH_Lock();
    
   return true;
}