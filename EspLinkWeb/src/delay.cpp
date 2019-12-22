////////////////////////////////////////////////////////////////////////////////
// delay.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "delay.h"

uint64_t Tick::now()
{
  return get_timer_value() ;
}

void Tick::delayMs(uint32_t ms)
{
  uint64_t t0 = now() ;
  uint64_t d = SystemCoreClock / 4 / 1000 * ms ; // sys tick is SytemCoreClock/4 Hz

  while ((now() - t0) < d) ;
}

void Tick::delayUs(uint32_t us)
{
  uint64_t t0 = now() ;
  uint64_t d = SystemCoreClock / 4  * us ;

  while ((now() - t0) < d) ;
}

uint32_t Tick::diffMs(uint64_t t)
{
  return (now() - t) * 4 / (SystemCoreClock/1000) ;
}

bool Tick::elapsedMs(uint64_t &t, uint32_t ms)
{
  uint64_t n = now() ;
  uint32_t delta = (n - t) * 4 / (SystemCoreClock/1000) ;
  if (delta < ms)
    return false ;
  t = n ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
