////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}
#include <stdio.h>

static const uint32_t Usart    = USART0 ;
static const rcu_periph_enum UsartRcuGpio  = RCU_GPIOA ;
static const rcu_periph_enum UsartRcuUsart = RCU_USART0 ;
static const uint32_t UsartGpio = GPIOA ;
static const uint32_t UsartTx   = GPIO_PIN_9 ;
static const uint32_t UsartRx   = GPIO_PIN_10 ;


int _put_char(int ch) // used by printf
{
  while (usart_flag_get(Usart, USART_FLAG_TBE) == RESET) ;

  usart_data_transmit(Usart, (uint8_t) ch );
  return ch;
}

int getChar()
{
  if (usart_flag_get(Usart, USART_FLAG_RBNE) == RESET)
    return -1 ;
  return usart_data_receive(Usart) ;
}

void delay()
{
  volatile uint32_t delay{1000000} ;
  for (uint32_t i = 0 ; i < delay ; ++i) ;
}

int main()
{
  rcu_periph_clock_enable(UsartRcuGpio);  // enable GPIO clock
  rcu_periph_clock_enable(UsartRcuUsart); // enable USART clock

  gpio_init(UsartGpio, GPIO_MODE_AF_PP,       GPIO_OSPEED_50MHZ, UsartTx);
  gpio_init(UsartGpio, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, UsartRx);

  // USART 115200,8N1, no-flow-control
  usart_deinit(Usart);
  usart_baudrate_set(Usart, 115200U);
  usart_word_length_set(Usart, USART_WL_8BIT);
  usart_stop_bit_set(Usart, USART_STB_1BIT);
  usart_parity_config(Usart, USART_PM_NONE);
  usart_hardware_flow_rts_config(Usart, USART_RTS_DISABLE);
  usart_hardware_flow_cts_config(Usart, USART_CTS_DISABLE);
  usart_receive_config(Usart, USART_RECEIVE_ENABLE);
  usart_transmit_config(Usart, USART_TRANSMIT_ENABLE);
  usart_enable(Usart);

  uint32_t i = 0 ;
  while (true)
  {
    int ch = getChar() ;
    if (ch < 0)
      printf("Hallo Usart %ld!\n", i++);
    else
      printf("Echo %c\n", ch) ;
    delay() ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
