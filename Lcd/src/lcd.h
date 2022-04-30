////////////////////////////////////////////////////////////////////////////////
// lcd.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>

class LcdCmdData ;


// LCD library for the Sitronix ST7735S controller
// cf. https://dl.sipeed.com/fileList/LONGAN/Nano/HDK/driver%20chip%20ST7735S_V1.5_20150303.pdf
//     https://www.mipi.org/specifications/display-command-set
class Lcd
{
public:
  Lcd(Spi &spi, const uint8_t *font, uint8_t fontHeight, uint8_t fontWidth);

  void setup() ;
  void fill(uint8_t xMin, uint8_t xMax, uint8_t yMin, uint8_t yMax, uint32_t rgb = 0x000000) ;
  void putChar(char ch) ;
  void putStr(const char *str) ;
  void txtArea(uint8_t xMin, uint8_t xMax, uint8_t yMin, uint8_t yMax) ;
  void txtFg(uint32_t col) ;
  void txtBg(uint32_t col) ;
  
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

  uint32_t _txtAreaXmin ; // area in which text is printed
  uint32_t _txtAreaXmax ;
  uint32_t _txtAreaYmin ;
  uint32_t _txtAreaYmax ;
  uint32_t _txtPosX ; // pos at which next char is inserted
  uint32_t _txtPosY ;
  const uint8_t  *_font ; // ' ' .. '~'
  uint8_t  _fontHeight ; // fixed font only
  uint8_t  _fontWidth ;
  uint32_t _txtFg ;
  uint32_t _txtBg ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
