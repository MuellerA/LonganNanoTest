////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "GD32VF103/usart.h"
#include "Longan/toStr.h"
#include "Longan/lcd.h"
#include "Longan/fonts.h"
#include "EspLink/client.h"
#include "EspLink/web.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

Usart &usart{Usart::usart0()} ;
Lcd &lcd{Lcd::lcd()} ;
::RV::EspLink::Client espLink{usart} ;

class LonganHtml : public ::RV::EspLink::WebServerCallback
{
public:
  LonganHtml(const std::string ebbes)
    : _ebbes{ebbes}, _status{false},
      _lcdButton{lcd,  0,  35, 48, 16},
      _lcdEbbes {lcd, 40, 120, 48, 16}
  {
  }
  
  // WebsServerCallback
  virtual void PageLoad(const ::RV::EspLink::WebServer &server, const std::string &page)
  {
    setParameter(server) ;
  }
  virtual void PageRefresh(const ::RV::EspLink::WebServer &server, const std::string &page)
  {
    setParameter(server) ;
  }
  virtual void ButtonPress(const ::RV::EspLink::WebServer &server, const std::string &page)
  {
    std::string button ;
    if (!server.getParameter(button))
      return ;
    if      (button == "an" ) _status = true ;
    else if (button == "aus") _status = false ;
    _lcdButton.clear() ;
    _lcdButton.put(_status ? "an" : "aus") ;
  }
  virtual void FormSubmit(const ::RV::EspLink::WebServer &server, const std::string &page)
  {
    if (!server.getParameter("ebbes", _ebbes))
      _ebbes = "nix" ;
    _lcdEbbes.clear() ;
    _lcdEbbes.put(_ebbes.c_str()) ;
  }
  virtual void Close()
  {
    // nothing
  }

  void setParameter(const ::RV::EspLink::WebServer &server)
  {
    static uint32_t cnt ;
    char buff[16] ;
    server.sendParameter("p1", "Das ist ein schoener Text!") ;
    char *para = ::RV::toStr(++cnt, buff, 16) ;
    size_t size = buff+16-para ;
    server.sendParameter("zahl", std::string{para, size}) ;
    server.sendParameter("ebbes", _ebbes) ;
    server.sendParameter("status", _status ? "An" : "Aus") ;
  }

private:
  std::string _ebbes ;
  bool _status ;
  LcdArea _lcdButton ;
  LcdArea _lcdEbbes ;
} ;

////////////////////////////////////////////////////////////////////////////////


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

void waitSync(Lcd &lcd, LcdArea &lcdCmd)
{
  TickTimer tSync{1000, (uint32_t)0, true} ;

  lcdCmd.put("   Syncing") ;
  bool toggle = true ;
  while (true)
  {
    lcd.heartbeat() ;
    
    if (tSync())
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
        return ;
    }
  }
}

std::string waitIp(Lcd &lcd, LcdArea &lcdCmd)
{
  uint8_t status0 = 0xff ;
  
  while (true)
  {
    lcd.heartbeat() ;

    uint8_t status ;
    espLink.wifiStatus(status) ;
    if (status != status0)
    {
      status0 = status ;
      switch ((::RV::EspLink::WifiStatus)status)
      {
      default:
        {
          lcdCmd.clear() ;
          lcdCmd.put("ESP Status: ") ;
          lcdCmd.put(status) ;
        }
        break ;
      case ::RV::EspLink::WifiStatus::Idle:
        {
          lcdCmd.clear() ;
          lcdCmd.put("ESP Idle...") ;
        }
        break ;
      case ::RV::EspLink::WifiStatus::Connecting:
        {
          lcdCmd.clear() ;
          lcdCmd.put("ESP Connecting...") ;
        }
        break ;
      case ::RV::EspLink::WifiStatus::GotIp:
        {
          std::string ipAddr ;
          {
            uint8_t addr[4]{ 0, 0, 0, 0 } ;
            uint8_t mask[4] ;
            uint8_t gw  [4] ;
            uint8_t mac [6] ;

            espLink.wifiInfo(addr, mask, gw, mac) ;

            ipAddr = toIp(addr) ;
          }
          return ipAddr ;
        }
      }
    }

    espLink.poll() ;
  }
}

////////////////////////////////////////////////////////////////////////////////

int main()
{
  ::RV::EspLink::WebServer elWebServer{espLink} ;
  LonganHtml longanHtml("was") ;
  
  lcd.setup() ;
  usart.setup(115200UL) ;

  LcdArea lcdTitle{lcd,   0, 140,  0, 16, &::RV::Longan::Roboto_Bold7pt7b,      0xffffffUL, 0x4040ffUL} ;
  LcdArea lcdCmd  {lcd,   0, 140, 16, 32, &::RV::Longan::RobotoMono_Light6pt7b                        } ;
  LcdArea lcdClock{lcd, 100, 8*7, 64, 16, &::RV::Longan::RobotoMono_Light6pt7b, 0x000000UL, 0xff0000UL} ;
  
  lcdTitle.put("ESP-LINK-WEB") ;
  
  TickTimer tTime{1000, (uint32_t)50, true} ;

  waitSync(lcd, lcdCmd) ;
  std::string addr = waitIp(lcd, lcdCmd) ;

  lcdCmd.clear() ;
  lcdCmd.put("http://") ;
  lcdCmd.put(addr.data(), addr.size()) ;
  lcdCmd.put("/longan.html") ;
  
  elWebServer.addCallback("/longan.html.json", &longanHtml) ;
  elWebServer.setup() ;
  
  while (true)
  {
    lcd.heartbeat() ;
    
    if (tTime())
    {
      uint32_t time ;
      espLink.unixTime(time) ;
      lcdClock.clear() ;
      time %= 24*60*60 ;
      uint32_t h = time / (60*60) ;
      time %= 60*60 ;
      uint32_t m = time / 60 ;
      time %= 60 ;
      uint32_t s = time / 1 ;
      lcdClock.put(h, 2, '0') ;
      lcdClock.put(':') ;
      lcdClock.put(m, 2, '0') ;
      lcdClock.put(':') ;
      lcdClock.put(s, 2, '0') ;
    }
    
    espLink.poll() ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
