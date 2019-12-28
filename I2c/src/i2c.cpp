////////////////////////////////////////////////////////////////////////////////
// i2c.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "i2c.h"

I2c::I2c(uint32_t i2c, rcu_periph_enum rcuI2c, rcu_periph_enum rcuGpio, uint32_t gpio, uint32_t pinClk, uint32_t pinData)
  : _i2c{i2c}, _rcuI2c{rcuI2c}, _rcuGpio{rcuGpio}, _gpio{gpio}, _pinClk{pinClk}, _pinData{pinData}
{
}

I2c& I2c::i2c0()
{
  static I2c *i2c0 = new I2c(I2C0, RCU_I2C0, RCU_GPIOB, GPIOB, GPIO_PIN_6, GPIO_PIN_7) ;
  return *i2c0 ;
}

void I2c::setup(uint8_t address, uint32_t speed)
{
  rcu_periph_clock_enable(_rcuGpio) ; 
  rcu_periph_clock_enable(_rcuI2c) ;

  gpio_init(_gpio, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, _pinClk | _pinData) ;

  i2c_clock_config(_i2c, speed, I2C_DTCY_2) ;
  i2c_mode_addr_config(_i2c, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, address) ;
  i2c_enable(_i2c) ;
  i2c_ack_config(_i2c, I2C_ACK_ENABLE) ;
}

bool I2c::write(uint8_t i2cAddress, const uint8_t *data, size_t size)
{
  while(i2c_flag_get(_i2c, I2C_FLAG_I2CBSY)) ;   // wait until I2C bus is idle
  i2c_start_on_bus(_i2c);                        // send a start condition to I2C bus
  while(!i2c_flag_get(_i2c, I2C_FLAG_SBSEND));   // wait until SBSEND bit is set
  i2c_master_addressing(_i2c, i2cAddress << 1, I2C_TRANSMITTER);  // send slave address to I2C bus
  while(!i2c_flag_get(_i2c, I2C_FLAG_ADDSEND)) ; // wait until ADDSEND bit is set
  i2c_flag_clear(_i2c, I2C_FLAG_ADDSEND);        // clear ADDSEND bit
  while(!i2c_flag_get(_i2c, I2C_FLAG_TBE));      // wait until the transmit data buffer is empty
  for(size_t i = 0 ; i < size ; i++)
  {
    i2c_data_transmit(_i2c, data[i]) ;           // data transmission
    while(!i2c_flag_get(_i2c, I2C_FLAG_TBE));    // wait until the TBE bit is set
  }
  i2c_stop_on_bus(_i2c);                         // send a stop condition to I2C bus
  while(I2C_CTL0(_i2c) & 0x0200);                // wait until stop condition generate

  return true ;
}

bool I2c::write(uint8_t i2cAddress, uint8_t val)
{
  return write(i2cAddress, &val, 1) ;
}

bool I2c::read(uint8_t i2cAddress, uint8_t *data, size_t size)
{
  while(i2c_flag_get(_i2c, I2C_FLAG_I2CBSY)) ;   // wait until I2C bus is idle
  i2c_start_on_bus(_i2c);                        // send a start condition to I2C bus
  while(!i2c_flag_get(_i2c, I2C_FLAG_SBSEND));   // wait until SBSEND bit is set
  i2c_master_addressing(_i2c, i2cAddress << 1, I2C_RECEIVER); // send slave address to I2C bus
  while(!i2c_flag_get(_i2c, I2C_FLAG_ADDSEND)) ; // wait until ADDSEND bit is set
  i2c_flag_clear(_i2c, I2C_FLAG_ADDSEND);        // clear ADDSEND bit
  for(size_t i = 0 ; i < size ; i++)
  {
    if(i == (size - 1)) // wait until the second last data byte is received into the shift register
    {
      while(!i2c_flag_get(_i2c, I2C_FLAG_BTC));
      
      i2c_ack_config(_i2c, I2C_ACK_DISABLE);     // disable acknowledge
    }
    while(!i2c_flag_get(_i2c, I2C_FLAG_RBNE)) ;  // wait until the RBNE bit is set
    data[i] = i2c_data_receive(_i2c) ;           // read a data from I2C_DATA
  }
  i2c_stop_on_bus(_i2c);                         // send a stop condition to I2C bus
  while(I2C_CTL0(_i2c) & 0x0200) ;               // wait until stop condition generate
  i2c_ack_config(_i2c, I2C_ACK_ENABLE);          // enable acknowledge

  return true ;
}

bool I2c::read(uint8_t i2cAddress, uint8_t &val)
{
  return read(i2cAddress, &val, 1) ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
