////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}
#include <stdio.h>

#include "delay.h"
#include "spi.h"
#include "lcd.h"

extern "C" const uint8_t font[1520] ;

Spi0 spi ;
Lcd lcd{spi, font, 16, 8} ;

int called = 0 ;

extern "C" int _put_char(int ch) // used by printf
{
  lcd.putChar(ch) ;
  return ch ;
}

int main()
{
  spi.setup() ;
  lcd.setup() ;

  lcd.fill(0, 159, 0, 79) ;
  lcd.fill(0, 11, 0, 15, 0x00ff00) ;
  lcd.fill(1, 10, 1, 14, 0xff0000) ;
  lcd.fill(110, 150, 35, 70, 0x0000ff) ;

  lcd.txtArea(20, 139, 0, 15) ;
  lcd.txtBg(0xffffff) ;
  lcd.txtFg(0x000000) ; lcd.putStr(" Hallo ") ;
  lcd.txtFg(0xff0000) ; lcd.putStr("Longan ") ;

  lcd.txtArea(0, 159, 16, 79) ;
  while (true)
  {
    {
      uint32_t *deviceSig = (uint32_t*)0x1FFFF7E0 ;
      uint32_t memInfo = deviceSig[0] ;
      uint32_t *deviceId = (uint32_t*)0x1FFFF7E8 ;
  
      lcd.txtFg(0xcccccc) ;
      lcd.txtBg(0x0000ff) ;
      lcd.putChar(0x0c) ;
      printf("Flash:     %3lukB\n", (memInfo >>  0) & 0xffff) ;
      printf("SRAM:      %3lukB\n", (memInfo >> 16) & 0xffff) ;
      printf("Device ID: %08lX\n           %08lX\n", deviceId[0], deviceId[1]) ; // deviceId[2]
    }
    delayMs(2000) ;
    {
      lcd.txtBg(0x000000) ;
      lcd.txtFg(0x00ff00) ;
      lcd.putChar(0x0c) ;
      for (uint8_t i = 0 ; i < 30 ; ++i)
      {
        lcd.putStr("Longan ") ;
        delayMs(200) ;
      }
    }
  }

  return 1 ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
