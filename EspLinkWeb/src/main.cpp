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
#include "espLink.h"
#include "elWebServer.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;

extern "C" const uint8_t font[1520] ;

Usart &usart{Usart::usart0()} ;
Lcd &lcd{Lcd::lcd()} ;
EspLink::Client espLink{usart} ;

class LonganHtml : public EspLink::WebServerCallback
{
public:
  LonganHtml(const std::string ebbes)
    : _ebbes{ebbes}, _status{false}
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
    lcd.txtPos(1) ;
    lcd.put(_status ? "an" : "aus") ;
  }
  virtual void FormSubmit(const EspLink::WebServer &server, const std::string &page)
  {
    std::string ebbes ;
    if (!server.getParameter("ebbes", _ebbes))
      _ebbes = "nix" ;
    lcd.txtPos(2) ;
    lcd.put(_ebbes.c_str()) ;
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
} ;


int main()
{
  EspLink::WebServer elWebServer{espLink} ;
  LonganHtml longanHtml("was") ;
  
  lcd.setup(font, 16, 8) ;
  usart.setup(115200UL) ;

  lcd.put("Hallo ESP-LINK!") ;
  
  TickTimer tSync{1000, true} ;
  TickTimer tTime{1000, true} ;
  
  uint32_t cnt = 0 ;

  // sync with ESP
  lcd.txtPos(1) ;
  lcd.put("Syncing * ") ;
  bool toggle = false ;
  while (true)
  {
    lcd.heartbeat() ;
    
    if (tSync()) // repeat every second
    {
      espLink.sync() ;
      lcd.txtPos(1, 8) ;
      lcd.put(toggle ? "* " : " *") ;
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
  lcd.txtPos(1) ;
  lcd.put("          ") ;

  elWebServer.addCallback("/longan.html.json", &longanHtml) ;
  elWebServer.setup() ;
  
  while (true)
  {
    lcd.heartbeat() ;
    
    if (tTime())
    {
      uint32_t time ;
      espLink.unixTime(time) ;
      lcd.txtPos(4) ;
      lcd.txtBg(0xff0000) ;
      lcd.txtFg(0x000000) ;
      time %= 24*60*60 ;
      uint32_t h = time / (60*60) ;
      time %= 60*60 ;
      uint32_t m = time / 60 ;
      time %= 60 ;
      uint32_t s = time / 1 ;
      lcd.put(h, 2, '0') ;
      lcd.put(':') ;
      lcd.put(m, 2, '0') ;
      lcd.put(':') ;
      lcd.put(s, 2, '0') ;
      lcd.txtBg(0x000000) ;
      lcd.txtFg(0xffffff) ;
    }
    
    if (espLink.poll() && false)
    {
      const EspLink::Pdu &rxPdu = espLink.rxPdu() ;
      
      lcd.txtPos(1) ;
      lcd.put(++cnt, 4) ;
      lcd.put(':') ;
      lcd.put(' ') ;
      lcd.put(rxPdu._cmd, 2) ;
      lcd.put(' ') ;
      lcd.put(rxPdu._ctx, 4, '0', true) ;
      lcd.put(' ') ;
      lcd.put(rxPdu._argc, 2) ;
      for (uint32_t i = 0 ; (i < rxPdu._argc) && (i < 2) ; ++i)
      {
        uint32_t  len = rxPdu._param[i]._len ;
        uint8_t *data = rxPdu._param[i]._data ;
        
        lcd.txtPos(i+2) ;
        lcd.put('[') ;
        lcd.put(i) ;
        lcd.put(']') ; lcd.put(' ') ;
        lcd.put(len, 2) ;
        for (uint32_t j = 0 ; (j < len) && (j < 4) ; ++j)
        {
          lcd.put(' ') ;
          lcd.put(data[j], 2, '0', true) ;
        }
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
