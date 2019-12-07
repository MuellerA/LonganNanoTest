////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

static const rcu_periph_enum FreqRcu  = RCU_GPIOA ;
static const uint32_t FreqGpio = GPIOA ;
static const uint32_t FreqPin  = GPIO_PIN_8 ;

inline uint32_t tick32(void)
{
  return *(volatile uint32_t *)(TIMER_CTRL_ADDR + TIMER_MTIME);
}

void delayMs(uint32_t ms = 1)
{
  uint64_t t0 = get_timer_value() ;
  uint64_t d  = SystemCoreClock / 4 / 1000 * ms ; // sys tick is SytemCoreClock/4 Hz

  while ((get_timer_value() - t0) < d) ;
}

void test_Manual(uint8_t mode)
{
  uint32_t t1, t2 ;

  rcu_periph_clock_enable(FreqRcu) ;
  gpio_init(FreqGpio, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, FreqPin) ;

  switch (mode)
  {
  case 1:
    while (true)
    {
      GPIO_BC (FreqGpio) = FreqPin ;
      GPIO_BOP(FreqGpio) = FreqPin ;
    }

  case 2:
    t1 = tick32() ;

    while(true)
    {
      GPIO_BC (FreqGpio) = FreqPin ;
      while ((t2 = tick32()) == t1) ;
      GPIO_BOP(FreqGpio) = FreqPin ;
      while ((t1 = tick32()) == t2) ;
    }

  case 3:
    while (true)
    {
      if (tick32() & 0x10)
        GPIO_BC (FreqGpio) = FreqPin ;
      else
        GPIO_BOP(FreqGpio) = FreqPin ;
    }

  case 4:
    while (true)
    {
      if (tick32() & 0x80)
        GPIO_BC (FreqGpio) = FreqPin ;
      else
        GPIO_BOP(FreqGpio) = FreqPin ;
    }

  case 5:
    while (true)
    {
      GPIO_BOP(FreqGpio) = FreqPin ;
      delayMs(10) ;
      GPIO_BC (FreqGpio) = FreqPin ;
      delayMs(990) ;
    }
  }

}

void test_System(uint32_t clk)
{
  rcu_periph_clock_enable(FreqRcu) ;
  rcu_ckout0_config(clk) ;

  gpio_init(FreqGpio, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, FreqPin) ;

  while (true) ;
}


int main()
{
  // test_Manual(1) ; // 18MHz - awful slope
  // test_Manual(2) ; // 10.8MHz - overshoot
  // test_Manual(3) ; // 844kHz - overshoot
  test_Manual(4) ; // 106kHz - overshoot
  // test_Manual(5) ; // 1Hz (10ms pulse)
  
  // test_System(RCU_CKOUT0SRC_CKSYS)       ; // 104MHz - sine, too fast for analyzer
  // test_System(RCU_CKOUT0SRC_IRC8M)       ; // 8MHz - overshoot
  // test_System(RCU_CKOUT0SRC_HXTAL)       ; // 8MHz - asym, overshoot
  // test_System(RCU_CKOUT0SRC_CKPLL_DIV2)  ; // 54MHz - sine, overshoot
  // test_System(RCU_CKOUT0SRC_CKPLL1)      ; // 80MHz - sine
  // test_System(RCU_CKOUT0SRC_CKPLL2_DIV2) ; // 40MHz - sine, overshoot
  // test_System(RCU_CKOUT0SRC_CKPLL2)      ; // 80Mhz - sine
  // test_System(RCU_CKOUT0SRC_EXT1)        ; // 8MHz - asym, overshoot

  while (true) ;
}


////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
