////////////////////////////////////////////////////////////////////////////////
// usart.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <gd32vf103_rcu.h>

class Usart
{
protected:
  Usart(uint32_t usart, rcu_periph_enum rcuGpio, rcu_periph_enum rcuUsart, uint32_t gpio, uint32_t tx, uint32_t rx) ;
  
public:
  void setup(uint32_t baud) ;
  
  int putc(int c) ;
  int getc() ; // < 0 not avail

private:
  const uint32_t        _usart ;
  const rcu_periph_enum _rcuGpio ;
  const rcu_periph_enum _rcuUsart ;
  const uint32_t        _gpio ;
  const uint32_t        _tx ;
  const uint32_t        _rx ;
} ;

class Usart0 : public Usart
{
public:
  Usart0() ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
