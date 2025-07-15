/*
 * TimeManager.c
 *
 *  Created on: Dec 14, 2022
 *      Author: pawelton
 */
#include "TimeManager.h"
#include <xc.h>

#include <stdint.h>

typedef union TimeManager_Flags
{
	struct
	{
		uint8_t	Flag1ms      : 1;
		uint8_t Flag10ms     : 1;
		uint8_t Flag100ms    : 1;
		uint8_t Flag1s       : 1;
		uint8_t Unused4      : 1;
		uint8_t Unused5      : 1;
		uint8_t Unused6      : 1;
		uint8_t Unused7      : 1;
	} Bits;
	uint8_t FlagsRegister;
} TimeManager_Flags_t;

typedef struct TimeManager
{
   uint16_t TickCount;
   bool ReloadTick;
   TimeManager_Flags_t TemporaryFlags;
   TimeManager_Flags_t CalculatedFlags;
} TimeManager_t;

static TimeManager_t TimeManager;

void TimeManager_Init(void)
{
	TimeManager.ReloadTick = false;
	TimeManager.TickCount = 0;
	TimeManager.TemporaryFlags.FlagsRegister = 0;
	TimeManager.CalculatedFlags.FlagsRegister = 0;

	/* TBD: Here SYSTICK should be started */
}

void TimeManager_DeInit(void)
{
	/* TBD: Here SYSTICK should be stopped */
}

void TimeManager_SYSTICK_Handler( void )
{

	TimeManager.TickCount++;
	TimeManager.TemporaryFlags.Bits.Flag1ms = 1;

	if (0 == (TimeManager.TickCount % 10)){
		TimeManager.TemporaryFlags.Bits.Flag10ms = 1;

		if (0 == (TimeManager.TickCount % 100)){
			TimeManager.TemporaryFlags.Bits.Flag100ms = 1;
			
            if (0 == (TimeManager.TickCount % 1000)){

                    TimeManager.TemporaryFlags.Bits.Flag1s = 1;
                    TimeManager.TickCount = 0;

                }
		}
	}

}

/* Should be called after each main loop */
void TimeManager_UpdateFlags(void)
{
    //INTERRUPT_GlobalDisable();

	TimeManager.CalculatedFlags = TimeManager.TemporaryFlags;
	TimeManager.TemporaryFlags.FlagsRegister = 0;
    //INTERRUPT_GlobalEnable();
}

bool TimeManager_Is1msPassed(void)
{
	return TimeManager.CalculatedFlags.Bits.Flag1ms;
}

bool TimeManager_Is10msPassed(void)
{
	return TimeManager.CalculatedFlags.Bits.Flag10ms;
}

bool TimeManager_Is100msPassed(void)
{
	return TimeManager.CalculatedFlags.Bits.Flag100ms;
}

bool TimeManager_Is1sPassed(void)
{
	return TimeManager.CalculatedFlags.Bits.Flag1s;
}
