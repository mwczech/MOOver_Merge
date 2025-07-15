#include "Timer.h"

void Timer_Tick( Timer* this )
{
   if ( this->Counter != 0 )
      {
      this->Counter--;
      }
}


