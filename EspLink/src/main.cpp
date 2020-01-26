////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "GD32VF103/usart.h"
#include "Longan/lcd.h"
#include "espLink.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;


extern "C" const uint8_t font[1520] ;

Lcd& lcd{Lcd::lcd()} ;
Usart &usart{Usart::usart0()} ; ;
EspLink::Client espLink{usart} ;

int main()
{
  lcd.setup(font, 16, 8) ;
  usart.setup(115200UL) ;

  lcd.put("Hallo ESP-LINK!") ;
  
  TickTimer tMsg{2000, true} ;
  TickTimer tStatus{1000, true} ;
  
  uint32_t cnt = 0 ;
  enum class Msg
    {
     Sync,
    } msg = Msg::Sync ;

  while (true)
  {
    if (tMsg())
    {
      switch (msg)
      {
      case Msg::Sync:
        espLink.sync() ;
        msg = Msg::Sync ;
        break ;
      }
    }

    if (tStatus())
    {
      {
        static uint8_t i ;
        static const char *ch = "|/-\\" ;
        uint8_t status ;
        espLink.wifiStatus(status) ;
        lcd.txtPos(0, 17) ;
        lcd.put(ch[i++]) ;
        lcd.put(' ') ;
        lcd.put(status) ;
        if (!ch[i])
          i = 0 ;
      }
      {
        uint32_t time ;
        espLink.unixTime(time) ;
        lcd.txtPos(4) ;
        lcd.txtBg(0xff0000) ;
        lcd.txtFg(0x000000) ;
        time %= 24*60*60 ;
        uint32_t h = time / (60*60) ;
        time %= 60*60 ;
        uint32_t m = time / 60 ;
        time %= 60 ;
        uint32_t s = time / 1 ;
        lcd.put(' ') ;
        lcd.put(h, 2, '0') ;
        lcd.put(':') ;
        lcd.put(m, 2, '0') ;
        lcd.put(':') ;
        lcd.put(s, 2, '0') ;
        lcd.put(' ') ;
        lcd.txtBg(0x000000) ;
        lcd.txtFg(0xffffff) ;
      }
    }
    
    if (espLink.poll())
    {
      const EspLink::RecvBuff &rx = espLink.recvBuff() ;
      
      lcd.txtPos(1) ;
      lcd.put(++cnt, 4) ;
      lcd.put(':') ;
      lcd.put(' ') ;
      lcd.put(rx._cmd, 2) ;
      lcd.put(' ') ;
      lcd.put(rx._ctx, 4, '0', true) ;
      lcd.put(' ') ;
      lcd.put(rx._argc, 2) ;
      for (uint32_t i = 0 ; (i < rx._argc) && (i < 2) ; ++i)
      {
        uint32_t  len = rx._param[i]._len ;
        uint8_t *data = rx._param[i]._data ;
        
        lcd.txtPos(i+2) ;
        lcd.txtPos(i+2) ;
        lcd.put('[') ;
        lcd.put(i) ;
        lcd.put(']') ; lcd.put(' ') ;
        lcd.put(len, 2) ;
        for (uint32_t j = 0 ; (j < len) && (j < 4) ; ++j)
        {
          lcd.put(' ') ;
          lcd.put(data[j], 2, '0', true) ;
        }
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
