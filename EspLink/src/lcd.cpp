////////////////////////////////////////////////////////////////////////////////
// lcd.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "delay.h"
#include "spi.h"
#include "lcd.h"

struct LcdCmdData
{
  uint8_t _cmd ;
  uint8_t _size ;
  uint8_t _data[16] ; // max size
} ;

Lcd::Lcd(Spi &spi, const uint8_t *font, uint8_t fontHeight, uint8_t fontWidth)
  // LCD: B0 RS, B1 RST, B2 CS
  : _spi{spi}, _rcuGpio{RCU_GPIOB}, _gpio{GPIOB}, _pinRst{GPIO_PIN_1}, _pinRs{GPIO_PIN_0}, _pinCs{GPIO_PIN_2},
    _txtAreaXmin{0}, _txtAreaXmax{159}, _txtAreaYmin{0}, _txtAreaYmax{79}, _txtPosX{0}, _txtPosY{0},
    _font{font}, _fontHeight{fontHeight}, _fontWidth{fontWidth},
    _txtFg{0xffffff}, _txtBg{0x000000}
{
}

void Lcd::setup()
{
  rcu_periph_clock_enable(_rcuGpio);

  gpio_init(_gpio, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, _pinRst | _pinRs | _pinCs) ;
  
  csHi() ;

  // HW Reset
  rstHi() ; Tick::delayMs( 25) ;
  rstLo() ; Tick::delayMs(250) ;
  rstHi() ; Tick::delayMs( 25) ;

  // Sleep Out
  cmd(0x11) ; Tick::delayMs(120) ;

  // Display Inversion On
  cmd(0x21) ;
  // Frame Rate Control (In normal mode/ Full colors)
  cmd({0xb1,  3, { 0x05, 0x3a, 0x3a } }) ;
  // Frame Rate Control (In Idle mode/ 8-colors)
  cmd({0xb2,  3, { 0x05, 0x3a, 0x3a } }) ;
  // Frame Rate Control (In Partial mode/ full colors)
  cmd({0xB3,  6, { 0x05, 0x3A, 0x3A, 0x05, 0x3A, 0x3A } }) ;
  // Display Inversion Control
  cmd({0xB4,  1, { 0x03 } }) ;
  // Power Control 1
  cmd({0xC0,  3, { 0x62, 0x02, 0x04} }) ;
  // Power Control 2
  cmd({0xC1,  1, { 0xC0 } }) ;
  // Power Control 3 (in Normal mode/ Full colors)
  cmd({0xC2,  2, { 0x0D, 0x00 } }) ;
  // Power Control 4 (in Idle mode/ 8-colors)
  cmd({0xC3,  2, { 0x8D, 0x6A } }) ;
  // Power Control 5 (in Partial mode/ full-colors)
  cmd({0xC4,  2, { 0x8D, 0xEE } }) ;
  // VCOM Control 1
  cmd({0xC5,  1, { 0x0E } }) ;
  // Gamma (‘+’polarity) Correction Characteristics Setting
  cmd({0xE0, 16, { 0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10 } }) ;
  // Gamma ‘-’polarity Correction Characteristics Setting
  cmd({0xE1, 16, { 0x10, 0x0E, 0x03, 0x03, 0x0F, 0x06, 0x02, 0x08, 0x0A, 0x13, 0x26, 0x36, 0x00, 0x0D, 0x0E, 0x10 } }) ;
  // Interface Pixel Format
  cmd({0x3A,  1, { 0x06 } }) ; // 18 bit/pixel
  // Memory Data Access Control
  cmd({0x36,  1, { 0xa8 } }) ; // orientation 08, c8, 78, a8
  // clr
  txtArea(0, 159, 0, 79) ;
  // Display On
  cmd(0x29) ;
}

void Lcd::fill(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint32_t rgb)
{
  // Column Address Set
  cmd({0x2a, 4, { 0x00,  1+x1, 0x00,  1+x2 } }) ; // x-offset  1
  // Row Address Set
  cmd({0x2b, 4, { 0x00, 26+y1, 0x00, 26+y2 } }) ; // y-offset 26
  // Memory Write
  cmd(0x2c) ;
  
  csLo() ;
  rsHi() ;
  for (uint32_t i = x1 ; i <= x2 ; ++i)
  {
    for (uint32_t j = y1 ; j <= y2 ; ++j)
    {
      _spi.putByte(rgb >> 16) ;
      _spi.putByte(rgb >>  8) ;
      _spi.putByte(rgb >>  0) ;
    }
  }
  while (_spi.isTransmit()) ;
  csHi() ;
}

void Lcd::putChar(char ch)
{
  if ((' ' <= ch) && (ch <= '~')) // 7bit ASCII
  {
    if ((_txtPosX + _fontWidth) > _txtAreaXmax+1)
    {
      _txtPosX  = _txtAreaXmin ;
      _txtPosY += _fontHeight ;
    }
    if ((_txtPosY + _fontHeight) > _txtAreaYmax+1)
    {
      _txtPosY  = _txtAreaYmin ;
    }

    uint8_t x = (uint8_t)_txtPosX +  1 ; // x-offset  1
    uint8_t y = (uint8_t)_txtPosY + 26 ; // y-offset 26
    _txtPosX += _fontWidth ;
    cmd({0x2a, 4, { 0x00, x+0, 0x00, x+_fontWidth -1 } }) ;
    cmd({0x2b, 4, { 0x00, y+0, 0x00, y+_fontHeight-1 } }) ;

    // Memory Write
    cmd(0x2c) ;
  
    csLo() ;
    rsHi() ;

    if ((' ' <= ch) || (ch <= '~'))
      ch -= ' ' ;
    else
      ch = 0 ;

    const uint8_t *charFont = _font + (uint8_t)ch * 16 ;
  
    for (uint8_t y = 0 ; y < 16 ; ++y)
    {
      for (uint8_t x = 0, c = charFont[y] ; x < 8 ; ++x, c >>= 1)
      {
        uint32_t rgb = (c & 0x01) ? _txtFg : _txtBg ;
        _spi.putByte(rgb >> 16) ;
        _spi.putByte(rgb >>  8) ;
        _spi.putByte(rgb >>  0) ;
      }
    }  
    while (_spi.isTransmit()) ;
    csHi() ;
  }
  else
  {
    if (ch == 0x0a) // LF
    {
      _txtPosX = _txtAreaXmin ;
      _txtPosY += _fontHeight ;
      if ((_txtPosY + _fontHeight) > _txtAreaYmax+1)
        _txtPosY  = _txtAreaYmin ;
      return ;
    }
    if (ch == 0x0d) // CR
    {
      _txtPosX = _txtAreaXmin ;
      return ;
    }
    if (ch == 0x0c) // FF
    {
      _txtPosX = _txtAreaXmin ;
      _txtPosY = _txtAreaYmin ;
      fill(_txtAreaXmin, _txtAreaXmax, _txtAreaYmin, _txtAreaYmax, _txtBg) ;
      return ;
    }
  }
}

void Lcd::putStr(const char *str)
{
  while (*str)
    putChar(*(str++)) ;
}

void Lcd::txtArea(uint8_t xMin, uint8_t xMax, uint8_t yMin, uint8_t yMax)
{
  _txtAreaXmin = xMin ;
  _txtAreaXmax = xMax ;
  _txtAreaYmin = yMin ;
  _txtAreaYmax = yMax ;
  _txtPosX = xMin ;
  _txtPosY = yMin ;

  fill(_txtAreaXmin, _txtAreaXmax, _txtAreaYmin, _txtAreaYmax, _txtBg) ;
}

void Lcd::txtPos(uint8_t x, uint8_t y)
{
  _txtPosX = _txtAreaXmin + x * _fontWidth ;
  _txtPosY = _txtAreaYmin + y * _fontHeight ;
}

void Lcd::txtFg(uint32_t col)
{
  _txtFg = col ;
}
void Lcd::txtBg(uint32_t col)
{
  _txtBg = col ;
}

void Lcd::rstHi() { gpio_bit_set  (_gpio, _pinRst) ; }
void Lcd::rstLo() { gpio_bit_reset(_gpio, _pinRst) ; }
void Lcd::rsHi()  { gpio_bit_set  (_gpio, _pinRs ) ; }
void Lcd::rsLo()  { gpio_bit_reset(_gpio, _pinRs ) ; }
void Lcd::csHi()  { gpio_bit_set  (_gpio, _pinCs ) ; }
void Lcd::csLo()  { gpio_bit_reset(_gpio, _pinCs ) ; }

void Lcd::cmd(uint8_t cmd)
{
  csLo() ;

  rsLo() ;
  _spi.putByte(cmd) ;
  while (_spi.isTransmit()) ;
  
  csHi() ;
}

void Lcd::data(uint8_t data)
{
  csLo() ;

  rsHi() ;
  _spi.putByte(data) ;
  while (_spi.isTransmit()) ;
  
  csHi() ;
}

void Lcd::cmd(const LcdCmdData &cmdData)
{
  csLo() ;

  rsLo() ;
  _spi.putByte(cmdData._cmd) ;
  while (_spi.isTransmit()) ;

  rsHi() ;
  for (uint8_t i = 0 ; i < cmdData._size ; ++i)
    _spi.putByte(cmdData._data[i]) ;
  while (_spi.isTransmit()) ;
  
  csHi() ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
