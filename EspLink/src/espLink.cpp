////////////////////////////////////////////////////////////////////////////////
// espLink.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include <functional>
#include "delay.h"
#include "espLink.h"

namespace EspLink
{
  static const uint8_t SlipEnd    = 0b11000000U ;
  static const uint8_t SlipEsc    = 0b11011011U ;
  static const uint8_t SlipEscEnd = 0b11011100U ;
  static const uint8_t SlipEscEsc = 0b11011101U ;

  Client::Client(Usart &usart)
    : _usart{usart}
  {
    
  }

  void Client::putNoEsc(uint8_t b)
  {
    while (!_usart.put(b))
      Tick::delayMs() ;
  }

  void Client::crc(uint16_t &c, uint8_t b)
  {
    c ^= b ;
    c  = (c >> 8) | (c << 8);
    c ^= (c & 0xff00) << 4;
    c ^= (c >> 8) >> 4;
    c ^= (c & 0xff00) >> 5;
  }
  
  void Client::put(uint8_t b)
  {
    crc(_crc, b) ;
    
    switch (b)
    {
    case SlipEnd: putNoEsc(SlipEsc) ; putNoEsc(SlipEscEnd) ; break ;
    case SlipEsc: putNoEsc(SlipEsc) ; putNoEsc(SlipEscEsc) ; break ;
    default:      putNoEsc(b) ;                              break ;
    }
  }

  void Client::put(const uint8_t *data, uint32_t len)
  {
    for (uint32_t i = 0 ; i < len ; ++i)
      put(*(data++)) ;
  }

  bool Client::get(uint8_t &b, bool &end)
  {
    static bool isEsc = false ;
    end = false ;
    if (!_usart.get(b))
      return false ;
    if (isEsc)
    {
      isEsc = false ;
      switch (b)
      {
      case SlipEscEnd: b = SlipEnd ; break ;
      case SlipEscEsc: b = SlipEsc ; break ;
      default: /* todo error */ ; break ;
      }
    }
    else if (b == SlipEsc)
    {
      isEsc = true ;
      return false ;
    }
    else if (b == SlipEnd)
    {
      end = true ;
    }
    return true ;
  }

  void Client::send(Cmd cmd, uint32_t ctx, uint16_t argc)
  {
    _crc = 0 ;
    putNoEsc(SlipEnd) ;
    put((uint8_t*)&cmd, 2) ;
    put((uint8_t*)&argc, 2) ;
    put((uint8_t*)&ctx, 4) ;
  }
  
  void Client::send(uint8_t *data, uint16_t len)
  {
    uint8_t padding[4] = { 0x00, 0x00, 0x00, 0x00 } ;
    put((uint8_t*)&len, 2) ;
    put(data, len) ;
    len = (4-((len+2)%4))%4 ;
    put(padding, len) ;
  }
  
  void Client::send()
  {
    uint16_t crc = _crc ; // _crc gets modified in put()
    put((uint8_t*)&crc, 2) ;
    putNoEsc(SlipEnd) ;
  }
  
  bool Client::sync()
  {
    _usart.put(SlipEnd) ; // new start
    
    _recvBuff._len = 0 ; // prepare recvBuff
    
    send(Cmd::Sync, 0x4711, 0) ;
    send() ;

    return true ;
  }

  bool Client::wifiStatus()
  {
    _recvBuff._len = 0 ;
    send(Cmd::WifiStatus, 0x1804, 0) ;
    send() ;
    return true ;
  }

  bool Client::unixTime()
  {
    _recvBuff._len = 0 ;
    send(Cmd::GetTime, 0x0815, 0) ;
    send() ;
    return true ;
  }

  bool Client::poll()
  {
    uint8_t b ;
    bool end ;
    if (!get(b, end))
      return false ;
    if (!end)
    {
      if (_recvBuff._len == _recvBuff._size)
      {
        return false ; // todo error
      }
      _recvBuff._raw[_recvBuff._len++] = b ;
      return false ;
    }

    if (_recvBuff._len == 0) // recv sync
      return false ;
    
    if (_recvBuff._len < 10)
    {
      // todo error
      return false ;
    }
    
    // decode header
    _recvBuff._cmd  = *(uint16_t*) (_recvBuff._raw + 0x00) ;
    _recvBuff._argc = *(uint16_t*) (_recvBuff._raw + 0x02) ;
    _recvBuff._ctx  = *(uint32_t*) (_recvBuff._raw + 0x04) ;

    // check parameters
    uint8_t *offset = _recvBuff._raw + 8 ;
    uint8_t *rawEnd = _recvBuff._raw + _recvBuff._len - 2 ;
    for (uint32_t iArg = 0, eArg = _recvBuff._argc ; iArg < eArg ; ++iArg)
    {
      _recvBuff.param[iArg]._len  = *(uint16_t*)offset ;
      offset += 2 ;
      _recvBuff.param[iArg]._data = offset ; 
      offset += _recvBuff.param[iArg]._len ;
      offset += (4-((_recvBuff.param[iArg]._len+2)%4))%4 ; // padding
      if (offset > rawEnd)
        return false ;
    }
    if (offset != rawEnd)
      return false ;

    // check crc
    _recvBuff._crc = 0 ;
    for (uint32_t i = 0, e = _recvBuff._len - 2 ; i < e ; ++i)
      crc(_recvBuff._crc, _recvBuff._raw[i]) ;
    if (_recvBuff._crc != *(uint16_t*)(_recvBuff._raw + _recvBuff._len - 2))
    {
      // todo error
      return false ;
    }

    return true ;
  }

  const RecvBuff& Client::recvBuff()
  {
    return _recvBuff ;
  }

  void Client::recvComplete()
  {
    _recvBuff._len = 0 ;
  }
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
