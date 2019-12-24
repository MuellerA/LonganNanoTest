////////////////////////////////////////////////////////////////////////////////
// espLink.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include <functional>
#include "tick.h"
#include "espLink.h"

namespace EspLink
{
  static const uint8_t SlipEnd    = 0b11000000U ;
  static const uint8_t SlipEsc    = 0b11011011U ;
  static const uint8_t SlipEscEnd = 0b11011100U ;
  static const uint8_t SlipEscEsc = 0b11011101U ;

  Client::Client(Usart &usart)
    : _usart{usart}, _wifiCallback{*this}
  {
    for (size_t i = 0 ; i < 32 ; ++i)
      _callback[i] = nullptr ;
    _callback[1] = &_wifiCallback ;
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
    len = (4-((len)%4))%4 ;
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
    
    _rxPdu._len = 0 ; // prepare pdu

    send(Cmd::Sync, 1 /* -> _wifiCallback */, 0) ;
    send() ;

    return true ;
  }

  bool Client::webSetup(ClientCallback *cb)
  {
    uint32_t id ;
    for (id = 2 ; (id < 32) && _callback[id] ; ++id) ;
    if (id == 32)
      return false ;

    _callback[id] = cb ;
    send(Cmd::WebSetup, 0, 1) ;
    send((uint8_t*)&id, 4) ;
    send() ;
    return true ;
  }
  
  void Client::wifiStatus(uint8_t &status)
  {
    //dont call esp, use cached value instead
    //send(Cmd::WifiStatus, 0, 0) ;
    //send() ;
    status = _wifiStatus ;
  }

  void Client::unixTime(uint32_t &time)
  {
    // use local time if possible
    if ((_unixTime < 946681200) ||                // got no time yet
        (_unixTimeTick())) // refresh
    {
      send(Cmd::GetTime, 0, 0) ;
      send() ;
      while (!poll()) ; // todo timeout
      _unixTime = _rxPdu._ctx ;
    }
    time = _unixTime + _unixTimeTick.elapsed()/1000 ;
  }

  bool Client::poll()
  {
    uint8_t b ;
    bool end ;
    if (!get(b, end))
      return false ;
    if (!end)
    {
      if (_rxPdu._len == _rxPdu._size)
      {
        return false ; // todo error
      }
      _rxPdu._raw[_rxPdu._len++] = b ;
      return false ;
    }

    if (_rxPdu._len == 0) // rx sync
      return false ;
    
    if (_rxPdu._len < 10)
    {
      // todo error
      return false ;
    }
    
    // decode header
    _rxPdu._cmd  = *(uint16_t*) (_rxPdu._raw + 0x00) ;
    _rxPdu._argc = *(uint16_t*) (_rxPdu._raw + 0x02) ;
    _rxPdu._ctx  = *(uint32_t*) (_rxPdu._raw + 0x04) ;

    // check parameters
    uint8_t *offset = _rxPdu._raw + 8 ;
    uint8_t *rawEnd = _rxPdu._raw + _rxPdu._len - 2 ;
    for (uint32_t iArg = 0, eArg = _rxPdu._argc ; iArg < eArg ; ++iArg)
    {
      _rxPdu._param[iArg]._len  = *(uint16_t*)offset ;
      offset += 2 ;
      _rxPdu._param[iArg]._data = offset ; 
      offset += _rxPdu._param[iArg]._len ;
      offset += (4-((_rxPdu._param[iArg]._len+2)%4))%4 ; // padding
      if (offset > rawEnd)
        return false ;
    }
    if (offset != rawEnd)
      return false ;

    // check crc
    _rxPdu._crc = 0 ;
    for (uint32_t i = 0, e = _rxPdu._len - 2 ; i < e ; ++i)
      crc(_rxPdu._crc, _rxPdu._raw[i]) ;
    if (_rxPdu._crc != *(uint16_t*)(_rxPdu._raw + _rxPdu._len - 2))
    {
      // todo error
      return false ;
    }

    switch ((EspLink::Cmd)_rxPdu._cmd)
    {
    case Cmd::Sync:
      break ;
    case Cmd::RespV:
      break ;
    case Cmd::RespCb:
      {
        uint32_t ctx = _rxPdu._ctx ;
        if ((ctx < 32) && _callback[ctx])
          _callback[ctx]->rx(_rxPdu) ;
        // todo error
        break ;
      }
    default:
      // todo error
      break ;
    }
    _rxPdu._len = 0 ;
    
    return true ;
  }

  const Pdu& Client::rxPdu()
  {
    return _rxPdu ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
