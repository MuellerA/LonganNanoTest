////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "delay.h"
#include "spi.h"
#include "lcd.h"

Spi0 spi ;
Lcd lcd{spi} ;

int main()
{
  spi.setup() ;
  lcd.setup() ;

  lcd.fill(0, 159, 0, 79) ;
  lcd.fill(0, 11, 0, 21, 0x00ff00) ;
  lcd.fill(1, 10, 1, 20, 0xff0000) ;
  lcd.fill(110, 150, 35, 70, 0x0000ff) ;

  uint8_t x = 20 ;
  lcd.putChar(x, 16, 'H') ; x += 8 ;
  lcd.putChar(x, 16, 'a') ; x += 8 ;
  lcd.putChar(x, 16, 'l') ; x += 8 ;
  lcd.putChar(x, 16, 'l') ; x += 8 ;
  lcd.putChar(x, 16, 'o') ;
  lcd.fg(0xff0000) ;
  lcd.putStr(20, 32, "Longan") ;

  return 1 ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
