////////////////////////////////////////////////////////////////////////////////
// lcd1602i2c.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "lcd1602i2c.h"

using ::RV::GD32VF103::TickTimer ;

////////////////////////////////////////////////////////////////////////////////

Lcd1602I2c::Lcd1602I2c(I2c &i2c) : _i2c{i2c}, _i2cAddr{0}
{
}

Lcd1602I2c& Lcd1602I2c::lcd1602I2c(I2c &i2c)
{
  Lcd1602I2c *lcd1602I2c = new Lcd1602I2c{i2c} ;
  return *lcd1602I2c ;
}

void Lcd1602I2c::setup(uint8_t i2cAddr)
{
  _i2cAddr = i2cAddr ;
  writeCmdHi(0x30, 4100) ;
  writeCmdHi(0x30,  100) ;
  writeCmdHi(0x30) ;
  writeCmdHi(0x20) ;

  writeCmd(0x28) ;        // 0010 1000: function set 8bit, 2 lines, 5x8dot
  writeCmd(0x01) ;        // 0000 0001: clear display, ddram addr 0
  writeCmd(0x02) ;        // 0000 0010: ddram addr 0, no shift
  writeCmd(0x06) ;        // 0000 0110: entry mode: inc curser, no shift
  writeCmd(0x0c) ;        // 0000 1100: display on, cursor off, blink off
}

void Lcd1602I2c::writeCmd(uint8_t val)
{
  uint8_t buff[6] ;
  uint8_t b ;

  while (readCmd() & 0x80) ;

  b = ((val << 0) & 0xf0) | BL ;
  buff[0] = b ;
  buff[1] = b | EA ;
  buff[2] = b ;

  b = ((val << 4) & 0xf0) | BL ;
  buff[3] = b ;
  buff[4] = b | EA ;
  buff[5] = b ;

  _i2c.write(_i2cAddr, buff, 6) ;
}

void Lcd1602I2c::writeCmdHi(uint8_t val, uint16_t waitUs)
{
  uint8_t buff[3] ;
  uint8_t b ;

  b = ((val << 0) & 0xf0) | BL ;
  buff[0] = b ;
  buff[1] = b | EA ;
  buff[2] = b ;

  _i2c.write(_i2cAddr, buff, 3) ;

  TickTimer::delayUs(waitUs) ;
}

void Lcd1602I2c::writeData(uint8_t val)
{
  uint8_t buff[6] ;
  uint8_t b ;

  while (readCmd() & 0x80) ;
  
  b = ((val << 0) & 0xf0) | BL | RS ;
  buff[0] = b ;
  buff[1] = b | EA ;
  buff[2] = b ;

  b = ((val << 4) & 0xf0) | BL | RS ;
  buff[3] = b ;
  buff[4] = b | EA ;
  buff[5] = b ;
  
  _i2c.write(_i2cAddr, buff, 6) ;
}

void Lcd1602I2c::writeData(const char* str)
{
  char ch ;
  while ((ch = *str))
  {
    writeData(ch) ;
    str++ ;
  }
}

void Lcd1602I2c::writeData(const char *str, uint8_t len)
{
  for (uint8_t i = 0 ; i < len ; ++i)
    writeData(str[i]) ;
}

void Lcd1602I2c::setPos(uint8_t line, uint8_t pos)
{
  if ((line > 1) || (pos > 16))
    return ;

  uint8_t addr = 0 ;
  if (line)
    addr += 0x40 ;
  addr += pos ;

  writeCmd(0x80 | addr) ; // set ddram addr 0x00 / 0x40
}

void Lcd1602I2c::writeData(uint8_t line, uint8_t pos, char val)
{
  setPos(line, pos) ;
  writeData((uint8_t)val) ;
}

void Lcd1602I2c::writeData(uint8_t line, uint8_t pos, const char *str)
{
  setPos(line, pos) ;
  writeData(str) ;
}

void Lcd1602I2c::writeData(uint8_t line, uint8_t pos, const char *str, uint8_t len)
{
  setPos(line, pos) ;
  writeData(str, len) ;
}

void Lcd1602I2c::clearDisplay()
{
  writeCmd(0x01) ;
}

uint8_t Lcd1602I2c::readCmd()
{
  uint8_t hi, lo ;
  uint8_t b ;

  b = 0xf0 | BL | RW ;
  _i2c.write(_i2cAddr, b) ;
  _i2c.write(_i2cAddr, b | EA) ;
  _i2c.read(_i2cAddr, hi) ;
  _i2c.write(_i2cAddr, b) ;
  _i2c.write(_i2cAddr, b | EA) ;
  _i2c.read(_i2cAddr, lo) ;
  _i2c.write(_i2cAddr, b) ;

  return ((hi >> 0) & 0xf0) | ((lo >> 4) & 0xf0) ;
}

uint8_t Lcd1602I2c::readData()
{
  uint8_t hi, lo ;
  uint8_t b ;
  
  while (readCmd() & 0x80) ;

  b = 0xf0 | BL | RW | RS ;
  _i2c.write(_i2cAddr, b) ;
  _i2c.write(_i2cAddr, b | EA) ;
  _i2c.read(_i2cAddr, hi) ;
  _i2c.write(_i2cAddr, b) ;
  _i2c.write(_i2cAddr, b | EA) ;
  _i2c.read(_i2cAddr, lo) ;
  _i2c.write(_i2cAddr, b) ;

  return ((hi >> 0) & 0xf0) | ((lo >> 4) & 0xf0) ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
