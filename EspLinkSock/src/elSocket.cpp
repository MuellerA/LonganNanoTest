////////////////////////////////////////////////////////////////////////////////
// elWebServer.h
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include <string.h>
#include "elSocket.h"

namespace EspLink
{
  Socket::Socket(Client &client)
    : _client{client}, _sockHdl{~0UL}, _callback{nullptr}
  {
  }

  void Socket::setup(const std::string &host, uint16_t port, SocketMode mode, SocketCallback *callback)
  {
    _callback = callback ;

    _client.sockSetup(this, host, port, (uint8_t)mode, _sockHdl) ;
  }

  bool Socket::send(uint8_t *data, uint32_t len)
  {
    if (_sockHdl == ~0UL)
      return false ;

    _client.send(Cmd::SocketSend, _sockHdl, 1) ;
    _client.send(data, len) ;
    _client.send() ;
    
    return true ;
  }

  void Socket::rx(const Pdu &rxPdu)
  {
    if (rxPdu._argc < 3)
      return ;

    uint8_t type ;
    uint8_t client ;
    uint16_t len ;

    if ((rxPdu._argc < 3) ||
        !rxPdu._param[0].val(type  ) ||
        !rxPdu._param[1].val(client) ||
        !rxPdu._param[2].val(len   ))
      return ;

    _sockHdl = client ; // shouldn't be
    
    switch (type)
    {
    case 0: // packet has been sent
      _callback->sent(len) ;
      break ;
    case 1: // packet has been received
      {
        if (rxPdu._argc < 4)
          return ;
        
        _callback->recv(rxPdu._param[3]._len, rxPdu._param[3]._data) ;
      }
      break ;
    case 2: // connection error
      {
        _callback->error() ;
      }
      break ;
    case 3: // connected or disconnected
      {
        _callback->connect(len) ;
      }
      break ;
    }

  }
  
  void Socket::close()
  {
    // todo
  }
  
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
