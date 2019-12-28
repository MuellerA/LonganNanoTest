////////////////////////////////////////////////////////////////////////////////
// i2c.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

class I2c
{
  I2c(uint32_t i2c, rcu_periph_enum rcuI2c, rcu_periph_enum rcuGpio, uint32_t gpio, uint32_t pinClk, uint32_t pinData) ;

 public:
  static I2c& i2c0() ;
  
  void setup(uint8_t address, uint32_t speed) ;
  
  bool write(uint8_t i2cAddress, const uint8_t *data, size_t len) ;
  bool write(uint8_t i2cAddress, uint8_t data) ;
  bool read(uint8_t i2cAddress, uint8_t *data, size_t len) ;
  bool read(uint8_t i2cAddress, uint8_t &data) ;

 private:
  uint32_t _i2c ;
  rcu_periph_enum _rcuI2c ;
  rcu_periph_enum _rcuGpio ;
  uint32_t _gpio ;
  uint32_t _pinClk ;
  uint32_t _pinData ;
  
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
