////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}
#include <stdio.h>

#include "usart.h"

Usart0 usart ;

int _put_char(int ch) // used by printf
{
  return usart.putc(ch) ;
}

void delay()
{
  volatile uint32_t delay{1000000} ;
  for (uint32_t i = 0 ; i < delay ; ++i) ;
}

int main()
{
  usart.setup(115200UL) ;

  uint32_t *deviceSig = (uint32_t*)0x1FFFF7E0 ;
  uint32_t memInfo = deviceSig[0] ;

  printf("\nUSART TEST\n\n") ;
  printf("Flash:     %3lukB\n", (memInfo >>  0) & 0xffff) ;
  printf("SRAM:      %3lukB\n", (memInfo >> 16) & 0xffff) ;

  uint32_t *deviceId = (uint32_t*)0x1FFFF7E8 ;
  
  printf("Device ID: %08lX %08lX %08lX\n", deviceId[0], deviceId[1], deviceId[2]) ;
  
  while (true)
  {
    int ch = usart.getc() ;
    if (ch >= 0)
      usart.putc(ch) ;
    if (ch == 13)
      usart.putc(10) ;

    delay() ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
