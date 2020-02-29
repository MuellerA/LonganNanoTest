////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "Longan/lcd.h"
#include "Longan/fonts.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

#include "usart.h"

extern "C" const uint8_t font[1520] ;

Lcd& lcd{Lcd::lcd()} ;
Usart0 usart ;

void putc(LcdArea &la, char ch)
{
  usart.put((uint8_t)ch) ;
  la.put(ch) ;
}

int main()
{
  lcd.setup() ;
  usart.setup(115200UL) ;

  LcdArea la{lcd, 0, 160, 16, 64} ;
  
  for (char i : "Hallo Internet!\n")
    putc(lcd, i) ;
  
  while (true)
  {
    uint8_t ch ;
    
    if (usart.get(ch))
    {
      putc(la, ch) ;
      TickTimer::delayMs(50) ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
