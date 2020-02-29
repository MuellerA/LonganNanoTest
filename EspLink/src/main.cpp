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
#include "Longan/fonts.h"
#include "espLink.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

Lcd& lcd{Lcd::lcd()} ;
Usart &usart{Usart::usart0()} ; ;
EspLink::Client espLink{usart} ;

int main()
{
  lcd.setup() ;
  usart.setup(115200UL) ;

  LcdArea lcdTitle {lcd,   0, 120,  0, 16, &Roboto_Bold7pt7b,      0xffffffUL, 0x4040ffUL} ;
  LcdArea lcdStatus{lcd, 120,  40,  0, 16, &RobotoMono_Light6pt7b                        } ;
  LcdArea lcdCmd   {lcd,   0, 160, 16, 48, &RobotoMono_Light6pt7b                        } ;
  LcdArea lcdClock {lcd,   0, 160, 64, 16, &RobotoMono_Light6pt7b, 0x000000UL, 0xff0000UL} ;

  lcdTitle.put("Hallo ESP-LINK!") ;
  
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
        lcdStatus.txtPos(0) ;
        lcdStatus.put(ch[i++]) ;
        lcdStatus.put(' ') ;
        lcdStatus.put(status) ;
        if (!ch[i])
          i = 0 ;
      }
      {
        uint32_t time ;
        espLink.unixTime(time) ;
        time %= 24*60*60 ;
        uint32_t h = time / (60*60) ;
        time %= 60*60 ;
        uint32_t m = time / 60 ;
        time %= 60 ;
        uint32_t s = time / 1 ;
        lcdClock.txtPos(0) ;
        lcdClock.put(' ') ;
        lcdClock.put(h, 2, '0') ;
        lcdClock.put(':') ;
        lcdClock.put(m, 2, '0') ;
        lcdClock.put(':') ;
        lcdClock.put(s, 2, '0') ;
        lcdClock.put(' ') ;
      }
    }
    
    if (espLink.poll())
    {
      const EspLink::RecvBuff &rx = espLink.recvBuff() ;
      
      lcdCmd.clear() ;
      lcdCmd.put(++cnt, 4) ;
      lcdCmd.put(':') ;
      lcdCmd.put(' ') ;
      lcdCmd.put(rx._cmd, 2) ;
      lcdCmd.put(' ') ;
      lcdCmd.put(rx._ctx, 4, '0', true) ;
      lcdCmd.put(' ') ;
      lcdCmd.put(rx._argc, 2) ;
      for (uint32_t i = 0 ; (i < rx._argc) && (i < 2) ; ++i)
      {
        uint32_t  len = rx._param[i]._len ;
        uint8_t *data = rx._param[i]._data ;
        
        lcdCmd.put('\n') ;
        lcdCmd.put('[') ;
        lcdCmd.put(i) ;
        lcdCmd.put(']') ;
        lcdCmd.put(' ') ;
        lcdCmd.put(len, 2) ;
        for (uint32_t j = 0 ; (j < len) && (j < 4) ; ++j)
        {
          lcdCmd.put(' ') ;
          lcdCmd.put(data[j], 2, '0', true) ;
        }
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
