/* 
 * File:   TimeManager.h
 * Author: ciaptak
 *
 * Created on 13 maja 2023, 12:31
 */

#ifndef TIMEMANAGER_H
#define	TIMEMANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include <stdbool.h>

void TimeManager_Init(void);
void TimeManager_DeInit(void);
void TimeManager_UpdateFlags(void);
void TimeManager_SYSTICK_Handler( void );

bool TimeManager_Is1msPassed(void);
bool TimeManager_Is10msPassed(void);
bool TimeManager_Is100msPassed(void);
bool TimeManager_Is1sPassed(void);
bool TimeManager_IsTickReloaded(void);



#ifdef	__cplusplus
}
#endif

#endif	/* TIMEMANAGER_H */

