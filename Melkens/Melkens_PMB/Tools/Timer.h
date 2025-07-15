#ifndef TIMER_H
#define	TIMER_H

#ifdef	__cplusplus
extern "C" {
#endif
    
#include <stdint.h>

typedef struct Timer_t
{
   uint16_t Counter;
} Timer;

#define TimerSetCounter     Timer_SetCounter
#define TimerGetCounter     Timer_GetCounter
#define TimerIsExpired      Timer_IsExpired
#define TimerTick           Timer_Tick


#define Timer_SetCounter( this, StartValue )                      \
do																								\
   {																							\
   (this)->Counter = (StartValue);												\
   } 																							\
while (0)

#define Timer_GetCounter( this )	   ( (this)->Counter )



#define Timer_IsExpired( this )                                    \
   ( ( (this)->Counter == 0 ) ? true : false )

void Timer_Tick( Timer* this );


#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */

