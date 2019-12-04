////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "led.h"

void delay()
{
  volatile uint32_t delay{8000000} ;
  for (uint32_t i = 0 ; i < delay ; ++i) ;
}

int main()
{
  RgbLed rgb ;
  
  rgb.setup() ;
  
  while (true)
  {
    rgb.red    () ; delay() ;
    rgb.green  () ; delay() ;
    rgb.blue   () ; delay() ;

    rgb.yellow () ; delay() ;
    rgb.cyan   () ; delay() ;
    rgb.magenta() ; delay() ;
    
    rgb.white  () ; delay() ;
    rgb.black  () ; delay() ;
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
