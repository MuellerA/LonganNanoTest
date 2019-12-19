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
  uint64_t d = SystemCoreClock / 4 / 1000 * ms ;

  while ((now() - t0) < d) ;
}

uint32_t Tick::diffMs(uint64_t t)
{
  return (now() - t) * 4 / (SystemCoreClock/1000) ;
}

void delayMs(uint32_t ms)
{
  uint64_t t0 = get_timer_value() ;
  uint64_t d  = SystemCoreClock / 4 / 1000 * ms ; // sys tick is SytemCoreClock/4 Hz

  while ((get_timer_value() - t0) < d) ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
