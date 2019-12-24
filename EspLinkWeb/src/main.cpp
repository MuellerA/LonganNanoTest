////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include <stdio.h>

#include "tick.h"
#include "spi.h"
#include "lcd.h"
#include "usart.h"
#include "espLink.h"
#include "elWebServer.h"

extern "C" const uint8_t font[1520] ;

Spi0 spi ;
Lcd lcd{spi, font, 16, 8} ;
Usart0 usart ;
EspLink::Client espLink{usart} ;

extern "C" int _put_char(int ch) // used by printf
{
  //static char hex[] = "0123456789ABCDEF" ;
  //lcd.putChar(hex[(ch>>4) & 0x0f]) ;
  //lcd.putChar(hex[(ch>>0) & 0x0f]) ;
  //lcd.putChar(' ') ;
  lcd.putChar(ch) ;
  return ch ;
}

void heartbeat()
{
  static Tick::MsTimer tHeartBeat{250, true} ;
  static uint8_t i ;
  static const char *ch = "|/-\\" ;

  if (tHeartBeat())
  {
    lcd.txtPos(19, 0) ;
    lcd.putChar(ch[i++]) ;
    if (!ch[i])
      i = 0 ;    
  }
}

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
    lcd.txtPos(0, 1) ;
    lcd.putStr(_status ? "an" : "aus") ;
  }
  virtual void FormSubmit(const EspLink::WebServer &server, const std::string &page)
  {
    std::string ebbes ;
    if (!server.getParameter("ebbes", _ebbes))
      _ebbes = "nix" ;
    lcd.txtPos(0, 2) ;
    lcd.putStr(_ebbes.c_str()) ;
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
    sprintf(buff, "%lu", ++cnt) ;
    server.sendParameter("zahl", buff) ;
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
  
  spi.setup() ;
  lcd.setup() ;
  usart.setup(115200UL) ;

  printf("Hallo ESP-LINK!") ;
  fflush(stdout) ;
  
  Tick::MsTimer tSync{1000, true} ;
  Tick::MsTimer tTime{1000, true} ;
  
  uint32_t cnt = 0 ;

  // sync with ESP
  lcd.txtPos(0, 1) ;
  lcd.putStr("Syncing * ") ;
  bool toggle = false ;
  while (true)
  {
    heartbeat() ;
    
    if (tSync()) // repeat every second
    {
      espLink.sync() ;
      lcd.txtPos(8, 1) ;
      lcd.putStr(toggle ? "* " : " *") ;
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
  lcd.txtPos(0, 1) ;
  lcd.putStr("          ") ;

  elWebServer.addCallback("/longan.html.json", &longanHtml) ;
  elWebServer.setup() ;
  
  while (true)
  {
    heartbeat() ;
    
    if (tTime())
    {
      uint32_t time ;
      espLink.unixTime(time) ;
      lcd.txtPos(0,4) ;
      lcd.txtBg(0xff0000) ;
      lcd.txtFg(0x000000) ;
      time %= 24*60*60 ;
      uint32_t h = time / (60*60) ;
      time %= 60*60 ;
      uint32_t m = time / 60 ;
      time %= 60 ;
      uint32_t s = time / 1 ;
      printf(" %02ld:%02ld:%02ld ", h,m,s) ;
      fflush(stdout) ;
      lcd.txtBg(0x000000) ;
      lcd.txtFg(0xffffff) ;
    }
    
    if (espLink.poll() && false)
    {
      const EspLink::Pdu &rxPdu = espLink.rxPdu() ;
      
      lcd.txtPos(0, 1) ;
      printf("%04lu: %2u %2lx %2u  ", ++cnt, rxPdu._cmd, rxPdu._ctx, rxPdu._argc) ;
      fflush(stdout) ;
      for (uint32_t i = 0 ; (i < rxPdu._argc) && (i < 2) ; ++i)
      {
        uint32_t  len = rxPdu._param[i]._len ;
        uint8_t *data = rxPdu._param[i]._data ;
        
        lcd.txtPos(0, i+2) ;
        printf("[%lu] %2lu", i, len) ;
        for (uint32_t j = 0 ; (j < len) && (j < 4) ; ++j)
          printf(" %02x", data[j]) ;
        fflush(stdout) ;
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
