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
#include "usart.h"

extern "C" const uint8_t font[1520] ;

Spi0 spi ;
Lcd lcd{spi, font, 16, 8} ;
Usart0 usart ;

void putc(char ch)
{
  usart.put((uint8_t)ch) ;
  lcd.putChar(ch) ;
}

int main()
{
  spi.setup() ;
  lcd.setup() ;
  usart.setup(115200UL) ;

  for (char i : "Hallo Internet!\n")
    putc(i) ;
  
  while (true)
  {
    uint8_t ch ;
    
    if (usart.get(ch))
    {
      putc(ch) ;
      delayMs(50) ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
