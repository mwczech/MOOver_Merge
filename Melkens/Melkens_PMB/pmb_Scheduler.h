#include <stdint.h>
#include <stdbool.h>
#include "RoutesDataTypes.h"

#define SCHEDULER_BLOCK_COUNT 120

typedef struct Time_t{
    uint32_t Hour;
    uint32_t Minute;
    uint32_t Second;
}Time;

typedef enum TimerName_t{
    dTimer_1 = 0,
    dTimer_2,
    dTimer_3,
    dTimer_4,
    dTimer_NumOf
}TimerName;

typedef enum WeekDay_t{
    dSunday = 0,
    dMonday,
    dTuesday,
    dWednesday,
    dThursday,
    dFriday,
    dSaturday,
}WeekDay;

typedef struct Scheduler_t{
    Time startTime;
    uint32_t routeName;
    uint32_t days;
    uint32_t enabled;
}Scheduler;

void Scheduler_Init(void);
void Scheduler_Perform1s(void);
void Scheduler_SetCurrentTime(uint8_t Day, uint8_t Hour, uint8_t Minute);
Route_ID Scheduler_GetRouteFromScheduler(void);
bool Scheduler_SaveToFlash(void);
bool Scheduler_EraseFlash(void);
 void PageWritexample();
WeekDay Scheduler_CalculateWeekday(uint16_t Year, uint8_t Month, uint8_t Day);
void Scheduler_SetSchedule(Time TimeToSet, TimerName TimerID, Route_ID RouteID, uint8_t Days);
Scheduler Scheduler_GetSchedule(TimerName TimerID);
