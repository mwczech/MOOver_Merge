/*
 * TimeManager.h
 *
 *  Created on: Dec 15, 2022
 *      Author: pawelton
 */

#ifndef INC_TIMEMANAGER_H_
#define INC_TIMEMANAGER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

void TimeManager_Init(void);
void TimeManager_DeInit(void);
void TimeManager_UpdateFlags(void);

bool TimeManager_Is1msPassed(void);
bool TimeManager_Is10msPassed(void);
bool TimeManager_Is100msPassed(void);
bool TimeManager_IsTickReloaded(void);

#endif /* INC_TIMEMANAGER_H_ */
