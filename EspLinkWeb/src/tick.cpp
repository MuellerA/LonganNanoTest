////////////////////////////////////////////////////////////////////////////////
// tick.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "tick.h"

namespace Tick
{
  // sys tick is SytemCoreClock/4 Hz
  
  uint64_t now()
  {
    return get_timer_value() ;
  }

  uint64_t msToTick(uint32_t ms)   { return (SystemCoreClock / 4 / 1000) * ms ; }
  uint64_t usToTick(uint32_t us)   { return (SystemCoreClock / 4       ) * us ; }
  uint32_t tickToMs(uint64_t tick) { return tick / (SystemCoreClock / 4 / 1000) ; }
  uint32_t tickToUs(uint64_t tick) { return tick / (SystemCoreClock / 4       ) ; }

  void delayMs(uint32_t ms)
  {
    uint64_t t0 = now() ;
    uint64_t d = msToTick(ms) ;

    while ((now() - t0) < d) ;
  }

  void delayUs(uint32_t us)
  {
    uint64_t t0 = now() ;
    uint64_t d = usToTick(us) ;

    while ((now() - t0) < d) ;
  }

  MsTimer::MsTimer(uint32_t ms, bool cyclic, bool exact)
    : _timeTick{now()}, _deltaTick{msToTick(ms)}, _cyclic{cyclic}, _exact{exact}
  {
  }

  bool MsTimer::operator()()
  {
    uint64_t t = now() ;
    if ((t - _timeTick) < _deltaTick)
      return false ;
    if (_cyclic)
    {
      if (_exact)
        _timeTick += _deltaTick ;
      else
        _timeTick = t ;
    }
    return true ;
  }

  uint32_t MsTimer::elapsed()
  {
    return tickToMs(now() - _timeTick) ;
  }
  
  void MsTimer::reset()
  {
    _timeTick = now() ;
  }
  
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
