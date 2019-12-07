////////////////////////////////////////////////////////////////////////////////
// usart.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "usart.h"

Usart::Usart(uint32_t usart, rcu_periph_enum rcuGpio, rcu_periph_enum rcuUsart, uint32_t gpio, uint32_t tx, uint32_t rx)
  : _usart{usart}, _rcuGpio{rcuGpio}, _rcuUsart{rcuUsart}, _gpio{gpio}, _tx{tx}, _rx{rx}
{
}

void Usart::setup(uint32_t baud)
{
  rcu_periph_clock_enable(_rcuGpio);  // enable GPIO clock
  rcu_periph_clock_enable(_rcuUsart); // enable USART clock

  gpio_init(_gpio, GPIO_MODE_AF_PP,       GPIO_OSPEED_50MHZ, _tx);
  gpio_init(_gpio, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, _rx);

  // USART 115200,8N1, no-flow-control
  usart_deinit(_usart);
  usart_baudrate_set(_usart, baud);
  usart_word_length_set(_usart, USART_WL_8BIT);
  usart_stop_bit_set(_usart, USART_STB_1BIT);
  usart_parity_config(_usart, USART_PM_NONE);
  usart_hardware_flow_rts_config(_usart, USART_RTS_DISABLE);
  usart_hardware_flow_cts_config(_usart, USART_CTS_DISABLE);
  usart_receive_config(_usart, USART_RECEIVE_ENABLE);
  usart_transmit_config(_usart, USART_TRANSMIT_ENABLE);
  usart_enable(_usart);
}
  
int Usart::putc(int ch)
{
  while (usart_flag_get(_usart, USART_FLAG_TBE) == RESET) ;

  usart_data_transmit(_usart, (uint8_t) ch);
  return ch;
}

int Usart::getc()
{
  if (usart_flag_get(_usart, USART_FLAG_RBNE) == RESET)
    return -1 ;
  return usart_data_receive(_usart) ;
}

Usart0::Usart0()
  : Usart(USART0, RCU_GPIOA, RCU_USART0, GPIOA, GPIO_PIN_9, GPIO_PIN_10)
{
}
          
