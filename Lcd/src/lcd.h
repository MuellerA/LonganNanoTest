////////////////////////////////////////////////////////////////////////////////
// lcd.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

class LcdCmdData ;

class Lcd
{
public:
  Lcd(Spi &spi);

  void setup() ;
  void fill(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint32_t rgb = 0x000000) ;
  void putChar(uint8_t x, uint8_t y, char ch) ;
  void putStr(uint8_t x, uint8_t y, const char *str) ;
  void fg(uint32_t col) ;
  void bg(uint32_t col) ;
  
private:
  void rstHi() ;
  void rstLo() ;
  void rsHi()  ;
  void rsLo()  ;
  void csHi()  ;
  void csLo()  ;

  void cmd(uint8_t cmd) ;
  void data(uint8_t data) ;
  void cmd(const LcdCmdData &cmdData) ;
  
  Spi     &_spi ;
  rcu_periph_enum _rcuGpio ;
  uint32_t _gpio ;
  uint32_t _pinRst ;
  uint32_t _pinRs ;
  uint32_t _pinCs ;
  
  uint32_t _fg ;
  uint32_t _bg ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
