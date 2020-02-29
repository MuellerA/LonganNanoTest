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
#include "espLink.h"
#include "elWebServer.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

Usart &usart{Usart::usart0()} ;
Lcd &lcd{Lcd::lcd()} ;
EspLink::Client espLink{usart} ;

class LonganHtml : public EspLink::WebServerCallback
{
public:
  LonganHtml(const std::string ebbes)
    : _ebbes{ebbes}, _status{false},
      _lcdButton{lcd, 0, 160, 20, 16},
      _lcdEbbes {lcd, 0, 160, 40, 16}
  {
  }
  
  // WebsServerCallback
  virtual void PageLoad(const EspLink::WebServer &server, const std::string &page)
  {
    setParameter(server) ;
  }
  virtual void PageRefresh(const EspLink::WebServer &server, const std::string &page)
  {
    setParameter(server) ;
  }
  virtual void ButtonPress(const EspLink::WebServer &server, const std::string &page)
  {
    std::string button ;
    if (!server.getParameter(button))
      return ;
    if      (button == "an" ) _status = true ;
    else if (button == "aus") _status = false ;
    _lcdButton.clear() ;
    _lcdButton.put(_status ? "an" : "aus") ;
  }
  virtual void FormSubmit(const EspLink::WebServer &server, const std::string &page)
  {
    std::string ebbes ;
    if (!server.getParameter("ebbes", _ebbes))
      _ebbes = "nix" ;
    _lcdEbbes.clear() ;
    _lcdEbbes.put(_ebbes.c_str()) ;
  }
  virtual void Close()
  {
    // nothing
  }

  void setParameter(const EspLink::WebServer &server)
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


int main()
{
  EspLink::WebServer elWebServer{espLink} ;
  LonganHtml longanHtml("was") ;
  
  lcd.setup() ;
  usart.setup(115200UL) ;

  LcdArea lcdTitle{lcd, 0, 140,  0, 16, &Roboto_Bold7pt7b,      0xffffffUL, 0x4040ffUL} ;
  LcdArea lcdCmd  {lcd, 0, 140, 16, 48, &RobotoMono_Light6pt7b                        } ;
  LcdArea lcdClock{lcd, 0, 8*7, 64, 16, &RobotoMono_Light6pt7b, 0x000000UL, 0xff0000UL} ;
  
  lcdTitle.put("Hallo ESP-LINK!") ;
  
  TickTimer tSync{1000, true} ;
  TickTimer tTime{1000, true} ;
  
  // sync with ESP
  lcdCmd.put("Syncing * ") ;
  bool toggle = false ;
  while (true)
  {
    lcd.heartbeat() ;
    
    if (tSync()) // repeat every second
    {
      espLink.sync() ;
      lcdCmd.txtPos(0, 8) ;
      lcdCmd.put(toggle ? "* " : " *") ;
      toggle = !toggle ;
    }
    if (espLink.poll())
    {
      const EspLink::Pdu &rxPdu = espLink.rxPdu() ;
      if ((rxPdu._cmd == (uint16_t)EspLink::Cmd::RespV) &&
          (rxPdu._ctx == 1) &&
          (rxPdu._argc == 0))
        break ;
    }
  }
  lcdCmd.clear() ;

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
