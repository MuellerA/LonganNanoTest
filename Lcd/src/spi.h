////////////////////////////////////////////////////////////////////////////////
// spi.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

class Spi
{
protected:
  Spi(uint32_t spi, rcu_periph_enum rcuSpi, rcu_periph_enum rcuGpio, uint32_t gpio, uint32_t pinClk, uint32_t pinMiso, uint32_t pinMosi) ;

public:
  void setup() ;

  bool isTransmit() ;
  void putByte(uint8_t c) ;

private:
  uint32_t _spi ;
  rcu_periph_enum _rcuSpi ;
  rcu_periph_enum _rcuGpio ;
  uint32_t _gpio ;
  uint32_t _pinClk ;
  uint32_t _pinMiso ;
  uint32_t _pinMosi ;
} ;

class Spi0 : public Spi
{
public:
  Spi0() ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
