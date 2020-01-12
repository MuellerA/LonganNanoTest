////////////////////////////////////////////////////////////////////////////////
// elSocket.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "espLink.h"

namespace EspLink
{
  enum class SocketMode
  {
    TcpClient = 1,
    TcpServer = 2,
    Udp       = 3,
  } ;
  
  class Socket ;
  
  class SocketCallback
  {
  public:
    virtual void sent(uint16_t len) = 0 ; // data has been sent
    virtual void recv(uint16_t len, uint8_t *data) = 0 ; // data has been received
    virtual void error() = 0 ; // connection error
    virtual void connect(bool conn) = 0 ; // connected or disconnected
  } ;

  class Socket : ClientCallback
  {
  public:
    Socket(Client &client) ;

    void setup(const std::string &host, uint16_t port, SocketMode mode, SocketCallback *callback) ;
    bool send(uint8_t *data, uint32_t len) ;

    // ClientCallback
    virtual void rx(const Pdu&) ;
    virtual void close() ;

  private:
    Client &_client ;
    uint32_t _sockHdl ;
    SocketCallback *_callback ;
  } ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
