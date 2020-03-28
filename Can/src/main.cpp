////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////
/*
 needs two longans to generate CAN-ACK.
 uses CAN1[B12, B13] (pins shared with SD card SPI - remove any card)

 Button:
   send every 200ms msg id:542, len:1, data=`button state`
   on rx: display count
 Counter:
   send every 250ms msg id:345, len:4, data=`counter`
   on rx: display button state
*/

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "Longan/lcd.h"
#include "Longan/fonts.h"
#include "Longan/led.h"

#include "can.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;
using ::RV::Longan::RgbLed ;

Lcd &lcd{Lcd::lcd()} ;
Can &can1{Can::can1()} ;
RgbLed &led{RgbLed::rgbLed()} ;

static const uint32_t CanIdCount  = 0x1d22c405 ; // extended id
static const uint32_t CanIdButton = 0x345 ; // standard id

////////////////////////////////////////////////////////////////////////////////

void canErr(LcdArea &lcdErr)
{
  static uint32_t err0{~0UL} ;
  static uint32_t errRx0{~0UL} ;
  static uint32_t errTx0{~0UL} ;
      
  uint32_t err = (uint32_t)can_error_get(CAN1) ;
  uint32_t errRx = can_receive_error_number_get(CAN1) ;
  uint32_t errTx = can_transmit_error_number_get(CAN1) ;

  if ((err != err0) || (errRx != errRx0) || (errTx != errTx0))
  {
    err0 = err ; errRx0 = errRx ; errTx0 = errTx ;
    lcdErr.clear() ;
    lcdErr.put(err) ;
    lcdErr.put(' ') ;
    lcdErr.put(errRx) ;
    lcdErr.put(' ') ;
    lcdErr.put(errTx) ;
  }
}

////////////////////////////////////////////////////////////////////////////////


#if defined BUTTON

#include "GD32VF103/gpio.h"
using ::RV::GD32VF103::Gpio ;
Gpio &gpioA8{Gpio::gpioA8()} ;

int main()
{
  gpioA8.setup(Gpio::Mode::IN_FL) ;
  lcd.setup() ;
  can1.setup(CanBaud::_1M,
             {
              CanFilter{CanFilter::Fifo::Fifo1, // CanIdCount frames as mask
                        (uint32_t) CanIdCount, (uint32_t) 0x1fffffff, CanFilter::Type::Data}
             }) ;

  LcdArea lcdTitle {lcd,  0, 140,  0, 16, &::RV::Longan::Roboto_Bold7pt7b,      0xffffffUL, 0xaa0000UL} ;
  LcdArea lcdRxLbl {lcd,  0,  25, 20, 20 } ;
  LcdArea lcdRx    {lcd, 30, 160, 20, 20 } ;
  LcdArea lcdTxLbl {lcd,  0,  25, 40, 20 } ;
  LcdArea lcdTx    {lcd, 30, 160, 40, 20 } ;
  LcdArea lcdErrLbl{lcd,  0,  25, 60, 20 } ;
  LcdArea lcdErr   {lcd, 30, 160, 60, 20 } ;
  
  lcdTitle.put("  CAN BUTTON  ") ;

  lcdRxLbl.put("Rx:") ;
  lcdTxLbl.put("Tx:") ;
  lcdErrLbl.put("Err:") ;
  
  TickTimer tTx(200, true) ;

  while (true)
  {
    lcd.heartbeat() ;

    canErr(lcdErr) ;
    
    if (tTx())
    {
      uint8_t data = gpioA8.get() ? 1 : 0 ;
      can1.tx(CanIdButton, false, &data, 1) ;

      lcdTx.clear() ;
      lcdTx.put(data) ;
    }
    
    uint32_t rxId ;
    bool rxIdExt ;
    uint8_t  rxData[8] ;
    uint32_t rxSize ;

    if (can1.rx(rxId, rxIdExt, rxData, rxSize) &&
        (rxId == CanIdCount) &&
        (rxIdExt == true))
    {
      lcdRx.clear() ;
      lcdRx.put(*(uint32_t*)rxData) ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

#elif defined COUNTER

int main()
{
  uint32_t count = 0 ;
  lcd.setup() ;
  led.setup() ;
  can1.setup(CanBaud::_1M,
             {
              CanFilter{CanFilter::Fifo::Fifo1, // CanIdButton frames as mask
                        (uint16_t) CanIdButton, (uint16_t) 0x3ff, CanFilter::Type::Data,
                        (uint16_t) CanIdButton, (uint16_t) 0x3ff, CanFilter::Type::Data}
             }) ;

  LcdArea lcdTitle {lcd,  0, 140,  0, 16, &::RV::Longan::Roboto_Bold7pt7b,      0xffffffUL, 0x00aa00UL} ;
  LcdArea lcdRxLbl {lcd,  0,  25, 20, 20 } ;
  LcdArea lcdRx    {lcd, 30, 160, 20, 20 } ;
  LcdArea lcdTxLbl {lcd,  0,  25, 40, 20 } ;
  LcdArea lcdTx    {lcd, 30, 160, 40, 20 } ;
  LcdArea lcdErrLbl{lcd,  0,  25, 60, 20 } ;
  LcdArea lcdErr   {lcd, 30, 160, 60, 20 } ;

  led.red() ;
  lcdTitle.put("  CAN COUNTER  ") ;

  lcdRxLbl.put("Rx:") ;
  lcdTxLbl.put("Tx:") ;
  lcdErrLbl.put("Err:") ;
  
  TickTimer tTx(250, true) ;
  
  while (true)
  {
    lcd.heartbeat() ;

    canErr(lcdErr) ;
    
    if (tTx())
    {
      count += 1 ;
      can1.tx(CanIdCount, true, (uint8_t*)&count, 4) ;

      lcdTx.clear() ;
      lcdTx.put(count) ;
    }

    uint32_t rxId ;
    bool rxIdExt ;
    uint8_t  rxData[8] ;
    uint32_t rxSize ;
    
    if (can1.rx(rxId, rxIdExt, rxData, rxSize) &&
        (rxId == CanIdButton) &&
        (rxIdExt == false))
    {
      lcdRx.clear() ;
      lcdRx.put(rxData[0]) ;
      if (rxData[0])
        led.green() ;
      else
        led.blue() ;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

#else
  #error "undefined mode"
#endif
  
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
