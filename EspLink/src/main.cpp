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

  printf("Hallo ESP-LINK!") ;
  fflush(stdout) ;
  
  uint64_t t0 = Tick::now() ;
  uint64_t tStatus = t0 ;
  
  uint32_t cnt = 0 ;
  enum class Msg
    {
     Sync,
    } msg = Msg::Sync ;

  while (true)
  {
    if (Tick::diffMs(t0) > 2000)
    {
      t0 = Tick::now() ;

      switch (msg)
      {
      case Msg::Sync:
        espLink.sync() ;
        msg = Msg::Sync ;
        break ;
      }
    }

    if (Tick::diffMs(tStatus) > 1000)
    {
      tStatus = Tick::now() ;

      {
        static uint8_t i ;
        static const char *ch = "|/-\\" ;
        uint8_t status ;
        espLink.wifiStatus(status) ;
        lcd.txtPos(17, 0) ;
        printf("%c %d", ch[i++], status) ;
        fflush(stdout) ;
        if (!ch[i])
          i = 0 ;
      }
      {
        uint32_t time ;
        espLink.unixTime(time) ;
        lcd.txtPos(0,4) ;
        lcd.txtBg(0xff0000) ;
        lcd.txtFg(0x000000) ;
        time %= 24*60*60 ;
        uint32_t h = time / (60*60) ;
        time %= 60*60 ;
        uint32_t m = time / 60 ;
        time %= 60 ;
        uint32_t s = time / 1 ;
        printf(" %02ld:%02ld:%02ld ", h,m,s) ;
        fflush(stdout) ;
        lcd.txtBg(0x000000) ;
        lcd.txtFg(0xffffff) ;
      }
    }
    
    if (espLink.poll())
    {
      const EspLink::RecvBuff &rx = espLink.recvBuff() ;
      
      lcd.txtPos(0, 1) ;
      printf("%04lu: %2u %2lx %2u", ++cnt, rx._cmd, rx._ctx, rx._argc) ;
      fflush(stdout) ;
      for (uint32_t i = 0 ; (i < rx._argc) && (i < 2) ; ++i)
      {
        uint32_t  len = rx._param[i]._len ;
        uint8_t *data = rx._param[i]._data ;
        
        lcd.txtPos(0, i+2) ;
        printf("[%lu] %2lu", i, len) ;
        for (uint32_t j = 0 ; (j < len) && (j < 4) ; ++j)
          printf(" %02x", data[j]) ;
        fflush(stdout) ;
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
