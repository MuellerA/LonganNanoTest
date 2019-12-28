////////////////////////////////////////////////////////////////////////////////
// lcd1602i2c.h
////////////////////////////////////////////////////////////////////////////////

#ifndef _LCD1602I2C_H_
#define _LCD1602I2C_H_

////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "i2c.h"

////////////////////////////////////////////////////////////////////////////////

class Lcd1602I2c
{
  static const uint8_t BL = 0x08 ; // back light
  static const uint8_t EA = 0x04 ; // enable
  static const uint8_t RW = 0x02 ; // read, /write
  static const uint8_t RS = 0x01 ; // data register, /command register

  Lcd1602I2c(I2c &i2c) ;

public:
  static Lcd1602I2c& lcd1602I2c(I2c &i2c) ;

  void setup(uint8_t i2cAddr) ;
  
  void writeCmd(uint8_t val) ;
  void writeCmdHi(uint8_t val, uint16_t waitUs = 37) ;
  void writeData(uint8_t val) ;
  void writeData(const char *str) ;
  void writeData(const char *str, uint8_t len) ;

  void setPos   (uint8_t line, uint8_t pos) ;
  void writeData(uint8_t line, uint8_t pos, char val) ;
  void writeData(uint8_t line, uint8_t pos, const char *str) ;
  void writeData(uint8_t line, uint8_t pos, const char *str, uint8_t len) ;

  void clearDisplay() ;
  
  uint8_t readCmd() ;
  uint8_t readData() ;

private:
  I2c &_i2c ;
  uint8_t _i2cAddr ;
  
} ;

////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
