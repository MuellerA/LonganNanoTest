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
#include "usart.h"
#include "espLink.h"

extern "C" const uint8_t font[1520] ;

Spi0 spi ;
Lcd lcd{spi, font, 16, 8} ;
Usart0 usart ;
EspLink::Client espLink{usart} ;

extern "C" int _put_char(int ch) // used by printf
{
  //static char hex[] = "0123456789ABCDEF" ;
  //lcd.putChar(hex[(ch>>4) & 0x0f]) ;
  //lcd.putChar(hex[(ch>>0) & 0x0f]) ;
  //lcd.putChar(' ') ;
  lcd.putChar(ch) ;
  return ch ;
}

int main()
{
  spi.setup() ;
  lcd.setup() ;
  usart.setup(115200UL) ;

  printf("Hallo ESP-LINK!\n") ;
  
  uint64_t t0 = Tick::now() ;
  uint32_t cnt = 0 ;
  uint32_t msg = 0 ;
  
  while (true)
  {
    if (Tick::diffMs(t0) > 2000)
    {
      t0 = Tick::now() ;
      if (!msg)
        lcd.txtPos(0, 0) ;

      switch (msg)
      {
      case 0: espLink.sync()       ; msg += 1 ; break ;
      case 1: espLink.wifiStatus() ; msg += 1 ; break ;
      case 2: espLink.unixTime()   ; msg  = 0 ; break ;
      }
    }
    
    if (espLink.poll())
    {
      const EspLink::RecvBuff &rx = espLink.recvBuff() ;

      printf("%u[%lu]", rx._cmd, ++cnt) ;
      //printf(" %u", rx._argc) ;
      printf(" %04lx  \n", rx._ctx) ;

      if (msg == 0) // time
      {
        lcd.txtBg(0xff0000) ;
        lcd.txtFg(0x000000) ;
        uint32_t d = rx._ctx ;
        d %= 24*60*60 ;
        uint32_t h = d / (60*60) ;
        d %= 60*60 ;
        uint32_t m = d / 60 ;
        d %= 60 ;
        uint32_t s = d / 1 ;
        printf(" %02ld:%02ld:%02ld \n", h,m,s) ;        
        lcd.txtBg(0x000000) ;
        lcd.txtFg(0xffffff) ;
      }
      espLink.recvComplete() ;
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
