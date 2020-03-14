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
#include "Longan/toStr.h"
#include "EspLink/client.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

Lcd& lcd{Lcd::lcd()} ;
Usart &usart{Usart::usart0()} ; ;
::RV::EspLink::Client espLink{usart} ;

std::string toIp(uint8_t val[4])
{
  char buff[10] ;
  char *b ;
  std::string ip ;
  b = ::RV::toStr(val[3], buff, 3) ; ip.append(b, buff+3-b) ;
  ip.push_back('.') ;
  b = ::RV::toStr(val[2], buff, 3) ; ip.append(b, buff+3-b) ;
  ip.push_back('.') ;
  b = ::RV::toStr(val[1], buff, 3) ; ip.append(b, buff+3-b) ;
  ip.push_back('.') ;
  b = ::RV::toStr(val[0], buff, 3) ; ip.append(b, buff+3-b) ;
  return ip ;
}

std::string toMac(uint8_t val[6])
{
  char buff[10] ;
  std::string mac ;
  mac.append(::RV::toStr(val[0], buff, 2, '0', true), 2) ;
  mac.push_back(':') ;
  mac.append(::RV::toStr(val[1], buff, 2, '0', true), 2) ;
  mac.push_back(':') ;
  mac.append(::RV::toStr(val[2], buff, 2, '0', true), 2) ;
  mac.push_back(':') ;
  mac.append(::RV::toStr(val[3], buff, 2, '0', true), 2) ;
  mac.push_back(':') ;
  mac.append(::RV::toStr(val[4], buff, 2, '0', true), 2) ;
  mac.push_back(':') ;
  mac.append(::RV::toStr(val[5], buff, 2, '0', true), 2) ;
  return mac ;
}

int main()
{
  lcd.setup() ;
  usart.setup(115200UL) ;

  LcdArea lcdTitle {lcd,   0, 130,  0, 16, &::RV::Longan::Roboto_Bold7pt7b,      0xffffffUL, 0x4040ffUL} ;
  LcdArea lcdCmd   {lcd,   0, 160, 16, 64} ;
  LcdArea lcdClock {lcd, 104,  56, 64, 16, &::RV::Longan::RobotoMono_Light6pt7b, 0x000000UL, 0xff0000UL} ;
  
  lcdTitle.put("ESP-LINK") ;

  { // initial sync
    TickTimer tSync{1000, (uint32_t)0, true} ;
    
    lcdCmd.put("   Syncing") ;
    bool toggle = true ;
    while (true)
    {
      lcd.heartbeat() ;
    
      if (tSync()) // repeat every second
      {
        espLink.sync() ;
        lcdCmd.txtPos(0) ;
        lcdCmd.put(toggle ? "* " : " *") ;
        toggle = !toggle ;
      }
      if (espLink.poll())
      {
        const ::RV::EspLink::Pdu &rxPdu = espLink.rxPdu() ;

        if ((rxPdu._cmd == (uint16_t)::RV::EspLink::Cmd::RespV) &&
            (rxPdu._ctx == 1) &&
            (rxPdu._argc == 0))
          break ;
      }
    }
  }

  TickTimer tStatus{1000, true} ;
  
  uint8_t status0{0xffU} ;
  std::string sAddr0, sMask0, sGw0, sMac0 ;
  while (true)
  {
    lcd.heartbeat() ;
    
    if (tStatus())
    {
      {
        uint8_t status ;
        espLink.wifiStatus(status) ;
        switch ((::RV::EspLink::WifiStatus)status)
        {
        default:
          if (status != status0)
          {
            lcdCmd.clear() ;
            lcdCmd.put("Status: ") ;
            lcdCmd.put(status) ;
          }
          break ;
        case ::RV::EspLink::WifiStatus::Idle:
          {
            lcdCmd.clear() ;
            lcdCmd.put("Idle") ;
          }
          break ;
        case ::RV::EspLink::WifiStatus::Connecting:
          if (status != status0)
          {
            lcdCmd.clear() ;
            lcdCmd.put("Connecting...") ;
          }
          break ;
        case ::RV::EspLink::WifiStatus::GotIp:
          {
            uint8_t addr[4] ;
            uint8_t mask[4] ;
            uint8_t gw  [4] ;
            uint8_t mac [6] ;

            if (espLink.wifiInfo(addr, mask, gw, mac))
            {
              std::string sAddr = toIp (addr) ;
              std::string sMask = toIp (mask) ;
              std::string sGw   = toIp (gw  ) ;
              std::string sMac  = toMac(mac ) ;
              
              if ((status != status0) ||
                  (sAddr  != sAddr0 ) ||
                  (sMask  != sMask0 ) ||
                  (sGw    != sGw0   ) ||
                  (sMac   != sMac0  ))
              {
                sAddr0 = sAddr ;
                sMask0 = sMask ;
                sGw0   = sGw   ;
                sMac0  = sMac  ;
                
                lcdCmd.clear() ;
                lcdCmd.txtPos(0,  0) ; lcdCmd.put("Addr:") ;
                lcdCmd.txtPos(0, 13) ; lcdCmd.put(sAddr.c_str()) ;
                lcdCmd.txtPos(1,  0) ; lcdCmd.put("Mask:") ;
                lcdCmd.txtPos(1, 13) ; lcdCmd.put(sMask.c_str()) ;
                lcdCmd.txtPos(2,  0) ; lcdCmd.put("Gw:") ;
                lcdCmd.txtPos(2, 13) ; lcdCmd.put(sGw  .c_str()) ;
                lcdCmd.txtPos(3,  0) ; lcdCmd.put("Mac:") ;
                lcdCmd.txtPos(3, 13) ; lcdCmd.put(sMac .c_str()) ;
              }
            }
            else
            {
              if (status != status0)
              {
                lcdCmd.clear() ;
                lcdCmd.put("Got IP") ;
              }
            }
          }
          break ;
        }
        status0 = status ;
      }
    }

    espLink.poll() ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
