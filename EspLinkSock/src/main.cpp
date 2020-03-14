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
#include "EspLink/sock.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Usart ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

Usart &usart{Usart::usart0()} ;
Lcd &lcd{Lcd::lcd()} ;
::RV::EspLink::Client espLink{usart} ;

class TcpServer : public ::RV::EspLink::SocketCallback
{
public:
  
  TcpServer(uint16_t port, bool encode)
    : _elSocket{espLink}, _rot{encode ? 10 : (26-10) }
  {
    _elSocket.setup("0.0.0.0", port, ::RV::EspLink::SocketMode::TcpServer, this) ;
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
          c -= 'a' ;
          c += _rot ;
          c %= 26 ;
          c += 'a' ;
        }
        else if (('A' <= c) && (c <= 'Z'))
        {
          c -= 'A' ;
          c += _rot ;
          c %= 26 ;
          c += 'A' ;
        }
      }

      _elSocket.send((uint8_t*)_text.data(), _text.size()) ;
      _text.clear() ;
    }
  }

private:
  ::RV::EspLink::Socket _elSocket ;
  std::string _text ;
  uint8_t _rot ;
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
  lcd.setup() ;
  usart.setup(115200UL) ;

  LcdArea lcdTitle{lcd, 0, 140,  0, 16, &::RV::Longan::Roboto_Bold7pt7b,      0xffffffUL, 0x4040ffUL} ;
  LcdArea lcdCmd  {lcd, 0, 160, 16, 64, &::RV::Longan::RobotoMono_Light6pt7b                        } ;
  
  lcdTitle.put("ESP-LINK-SOCK") ;

  waitSync(lcd, lcdCmd) ;
  std::string addr = waitIp(lcd, lcdCmd) ;

  lcdCmd.clear() ;
  lcdCmd.put("ROT 10 Codec\nAddr: ") ;
  lcdCmd.put(addr.data(), addr.size()) ;
  lcdCmd.put("\nencode port: 5555\ndecode port: 6666") ;  

  TcpServer tcpServer5555(5555, true ) ;
  TcpServer tcpServer6666(6666, false) ;
  
  while (true)
  {
    lcd.heartbeat() ;
    
    espLink.poll() ;

    tcpServer5555() ;
    tcpServer6666() ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
