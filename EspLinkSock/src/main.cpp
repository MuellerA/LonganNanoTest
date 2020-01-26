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
#include "espLink.h"
#include "elSocket.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;

extern "C" const uint8_t font[1520] ;

Usart &usart{Usart::usart0()} ;
Lcd &lcd{Lcd::lcd()} ;
EspLink::Client espLink{usart} ;

class TcpServer : public EspLink::SocketCallback
{
public:
  
  TcpServer(uint16_t port)
    : _elSocket{espLink}
  {
    _elSocket.setup("10.10.3.150", port, EspLink::SocketMode::TcpServer, this) ;
  }
  
  virtual void sent(uint16_t len)
  {
  }
  
  virtual void recv(uint16_t len, uint8_t *data)
  {
    _text.assign((char*)data, len) ;
  }
  
  virtual void error()
  {
  }

  virtual void connect(bool conn)
  {
  }

  void operator()()
  {
    if (_text.size())
    {
      for (char &c : _text)
      {
        if (('a' <= c) && (c <= 'z'))
        {
          c = (c <= 'm') ? c + 13 : c - 13 ;
        }
        else if (('A' <= c) && (c <= 'Z'))
        {
          c = (c <= 'M') ? c + 13 : c - 13 ;
        }
      }

      _elSocket.send((uint8_t*)_text.data(), _text.size()) ;
      _text.clear() ;
    }
  }

private:
  EspLink::Socket _elSocket ;
  std::string _text ;
  
} ;

int main()
{
  lcd.setup(font, 16, 8) ;
  usart.setup(115200UL) ;

  lcd.put("ESP-LINK-SOCK") ;

  TickTimer tSync{1000, true} ;
  TickTimer tTime{1000, true} ;

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

  uint32_t cnt = 0 ;
  TcpServer tcpServer5555(5555) ;
  TcpServer tcpServer6666(6666) ;
  
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
    
    if (espLink.poll())
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

    tcpServer5555() ;
    tcpServer6666() ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
