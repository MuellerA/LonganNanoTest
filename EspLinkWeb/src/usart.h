////////////////////////////////////////////////////////////////////////////////
// usart.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

class Buff
{
public:
  static const uint32_t _size = 1<<8 ; // must be 2^n
  static const uint32_t _mask = _size - 1 ;

  Buff() ;
  bool empty() const ;
  bool full() const ;
  bool put(uint8_t  b) ;
  bool get(uint8_t &b) ;

private:
  volatile uint32_t _in ; 
  volatile uint32_t _out ;
  volatile uint8_t  _data[_size] ;
} ;

extern "C" void USART0_IRQHandler() ;

class Usart
{
  friend void USART0_IRQHandler() ;
protected:
  Usart(uint32_t usart, rcu_periph_enum rcuGpio, rcu_periph_enum rcuUsart, uint32_t irq, uint32_t gpio, uint32_t txPin, uint32_t rxPin) ;
  
public:
  void setup(uint32_t baud) ;
  
  bool put(uint8_t  b) ;
  bool get(uint8_t &b) ;

private:
  const uint32_t        _usart ;
  const rcu_periph_enum _rcuGpio ;
  const rcu_periph_enum _rcuUsart ;
  const uint32_t        _irq ;
  const uint32_t        _gpio ;
  const uint32_t        _txPin ;
  const uint32_t        _rxPin ;

  Buff _txData ;
  Buff _rxData ;
} ;

class Usart0 : public Usart
{
public:
  Usart0() ;
  //~Usart0() ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
