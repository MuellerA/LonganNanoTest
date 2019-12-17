////////////////////////////////////////////////////////////////////////////////
// usart.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "usart.h"

////////////////////////////////////////////////////////////////////////////////

Buff::Buff() : _in{0}, _out{0}
{
}

bool Buff::empty() const
{
  return _in == _out ;
}

bool Buff::full() const
{
  return ((_in+1) & _mask) == _out ;
}

bool Buff::get(uint8_t &b)
{
  if (empty())
    return false ;
  b = _data[_out++] ;
  _out &= _mask ;
  return true ;
}
bool Buff::put(uint8_t b)
{
  if (full())
    return false ;
  _data[_in++] = b ;
  _in &= _mask ;
  return true ;
}

////////////////////////////////////////////////////////////////////////////////

Usart::Usart(uint32_t usart, rcu_periph_enum rcuGpio, rcu_periph_enum rcuUsart, uint32_t irq, uint32_t gpio, uint32_t txPin, uint32_t rxPin)
  : _usart{usart}, _rcuGpio{rcuGpio}, _rcuUsart{rcuUsart}, _irq{irq}, _gpio{gpio}, _txPin{txPin}, _rxPin{rxPin}
{
}

void Usart::setup(uint32_t baud)
{
  rcu_periph_clock_enable(_rcuGpio);  // enable GPIO clock
  rcu_periph_clock_enable(_rcuUsart); // enable USART clock

  gpio_init(_gpio, GPIO_MODE_AF_PP,       GPIO_OSPEED_50MHZ, _txPin);
  gpio_init(_gpio, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, _rxPin);

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

  eclic_global_interrupt_enable();
  eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO1);
  eclic_irq_enable(_irq, 1, 0);

  usart_interrupt_enable(_usart, USART_INT_TBE);
  usart_interrupt_enable(_usart, USART_INT_RBNE);
}

bool Usart::put(uint8_t b)
{
  usart_interrupt_disable(_usart, USART_INT_TBE);
  bool res = _txData.put(b) ;
  usart_interrupt_enable(_usart, USART_INT_TBE);
  return res ;
}

bool Usart::get(uint8_t &b)
{
  usart_interrupt_disable(_usart, USART_INT_RBNE);
  bool res = _rxData.get(b) ;
  usart_interrupt_enable(_usart, USART_INT_RBNE);
  return res ;
}  

static Usart *usart0 ;

Usart0::Usart0()
  : Usart(USART0, RCU_GPIOA, RCU_USART0, USART0_IRQn, GPIOA, GPIO_PIN_9, GPIO_PIN_10)
{
  usart0 = this ;
}

//Usart0::~Usart0()
//{
//  usart0 = nullptr ;
//}

extern "C" void USART0_IRQHandler()
{
  if (!usart0)
    return ;

  if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) != RESET) // RX
  {
    usart_interrupt_disable(USART0, USART_INT_RBNE);
    uint8_t b = usart_data_receive(USART0) ;
    usart0->_rxData.put(b) ;
    usart_interrupt_enable(USART0, USART_INT_RBNE);
  }
  if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_TBE) != RESET)
  {
    usart_interrupt_disable(USART0, USART_INT_TBE);    
    uint8_t b ;

    if (usart0->_txData.get(b))
    {
      usart_data_transmit(USART0, b) ;
      usart_interrupt_enable(USART0, USART_INT_TBE);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
