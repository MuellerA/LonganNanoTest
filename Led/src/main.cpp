////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "led.h"


int main()
{
  RgbLed rgb ;
  volatile uint32_t delay{7000000} ;
  
  rgb.setup() ;
  
  while (true)
  {
    rgb.red() ;
    for (uint32_t i = 0 ; i < delay ; ++i) ;

    rgb.green() ;
    for (uint32_t i = 0 ; i < delay ; ++i) ;

    rgb.blue() ;
    for (uint32_t i = 0 ; i < delay ; ++i) ;

    rgb.white() ;
    for (uint32_t i = 0 ; i < delay ; ++i) ;

    rgb.black() ;
    for (uint32_t i = 0 ; i < delay ; ++i) ;
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
