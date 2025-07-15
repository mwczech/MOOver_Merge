#ifndef BATTERY_MANAGER_H
#define	BATTERY_MANAGER_H

typedef enum BatteryLevel_t{
    BatteryLevel_Init = 0, /* when in Init, delay is applied to stabilize measurement  */
    BatteryLevel_Stabilisation, //1
    BatteryLevel_Overvoltage, //2
    BatteryLevel_Good, //3
    BatteryLevel_Low, //4
    BatteryLevel_Critical, //5
    BatteryLevel_NumOf //6
}BatteryLevel;


 BatteryLevel BatteryManager_GetBatteryLevel(void);
 void BatteryManager_ResetBattery(void);
 void BatteryManager_Perform100ms(void);
 
#endif