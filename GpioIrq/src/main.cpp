////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}
#include <stdio.h>

#include "led.h"

RgbLed led ;

void setup()
{
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_AF); ;
    
  gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
  eclic_global_interrupt_enable();
  eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO1);
  eclic_irq_enable(EXTI5_9_IRQn, 1, 1);
  gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_8);
  exti_init(EXTI_8, EXTI_INTERRUPT, EXTI_TRIG_BOTH);
  exti_interrupt_flag_clear(EXTI_8);
}

extern "C"
void EXTI5_9_IRQHandler()
{
  if (exti_interrupt_flag_get(EXTI_8) != RESET)
  {
    exti_interrupt_flag_clear(EXTI_8);

    if (gpio_input_bit_get(GPIOA, GPIO_PIN_8))
      led.red() ;
    else
      led.blue() ;
  }
}

int main()
{
  led.setup() ;
  led.blue() ;
  setup() ;

  while (true) ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
