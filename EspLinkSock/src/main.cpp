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

#include "espLink.h"
#include "elSocket.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

Usart &usart{Usart::usart0()} ;
Lcd &lcd{Lcd::lcd()} ;
EspLink::Client espLink{usart} ;

class TcpServer : public EspLink::SocketCallback
{
public:
  
  TcpServer(uint16_t port)
    : _elSocket{espLink}
  {
    _elSocket.setup("0.0.0.0", port, EspLink::SocketMode::TcpServer, this) ;
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
  lcd.setup() ;
  usart.setup(115200UL) ;

  LcdArea lcdTitle{lcd, 0, 140,  0, 16, &Roboto_Bold7pt7b,      0xffffffUL, 0x4040ffUL} ;
  LcdArea lcdCmd  {lcd, 0, 160, 16, 48, &RobotoMono_Light6pt7b                        } ;
  LcdArea lcdClock{lcd, 0, 160, 64, 16, &RobotoMono_Light6pt7b, 0x000000UL, 0xff0000UL} ;
  
  lcdTitle.put("ESP-LINK-SOCK") ;

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
      time %= 24*60*60 ;
      uint32_t h = time / (60*60) ;
      time %= 60*60 ;
      uint32_t m = time / 60 ;
      time %= 60 ;
      uint32_t s = time / 1 ;
      lcdClock.txtPos(0) ;
      lcdClock.put(h, 2, '0') ;
      lcdClock.put(':') ;
      lcdClock.put(m, 2, '0') ;
      lcdClock.put(':') ;
      lcdClock.put(s, 2, '0') ;
    }
    
    if (espLink.poll())
    {
      const EspLink::Pdu &rxPdu = espLink.rxPdu() ;

      lcdCmd.clear() ;
      lcdCmd.put(++cnt, 4) ;
      lcdCmd.put(':') ;
      lcdCmd.put(' ') ;
      lcdCmd.put(rxPdu._cmd, 2) ;
      lcdCmd.put(' ') ;
      lcdCmd.put(rxPdu._ctx, 4, '0', true) ;
      lcdCmd.put(' ') ;
      lcdCmd.put(rxPdu._argc, 2) ;
      for (uint32_t i = 0 ; (i < rxPdu._argc) && (i < 2) ; ++i)
      {
        uint32_t  len = rxPdu._param[i]._len ;
        uint8_t *data = rxPdu._param[i]._data ;
        
        lcdCmd.put('\n') ;
        lcdCmd.put('[') ;
        lcdCmd.put(i) ;
        lcdCmd.put(']') ;
        lcdCmd.put(' ') ;
        lcdCmd.put(len, 2) ;
        for (uint32_t j = 0 ; (j < len) && (j < 4) ; ++j)
        {
          lcdCmd.put(' ') ;
          lcdCmd.put(data[j], 2, '0', true) ;
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
