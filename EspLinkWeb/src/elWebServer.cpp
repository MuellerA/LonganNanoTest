////////////////////////////////////////////////////////////////////////////////
// elWebServer.h
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "elWebServer.h"

namespace EspLink
{
  WebServer::WebServer(Client &client)
    : _client{client}
  {
  }

  void WebServer::setup()
  {
    _client.webSetup(this) ;
  }

  bool WebServer::addCallback(const std::string &url, WebServerCallback *cb)
  {
    auto iCb = _callbacks.find(url) ;
    if (iCb != _callbacks.end())
      return false ;
    _callbacks.insert(std::pair<std::string, WebServerCallback*>(url, cb)) ;
    return true ;
  }
  
  bool WebServer::removeCallback(const std::string &url)
  {
    auto iCb = _callbacks.find(url) ;
    if (iCb == _callbacks.end())
      return false ;
    _callbacks.erase(url) ;
    return true ;
  }

  void WebServer::sendParameter(const std::string &key, const std::string &val) const
  {
    std::string null{"\000", 1} ;
    std::string param{null + key + null + val} ;
    _client.send((uint8_t*)param.data(), param.size()) ;
  }
  
  void WebServer::Receive(const RecvBuff &rx)
  {
    uint16_t cmd{*(uint16_t*)rx._param[0]._data} ;
    uint8_t *addr{rx._param[1]._data} ;
    uint8_t *port{rx._param[2]._data} ;
    std::string url{(char*)rx._param[3]._data, rx._param[3]._len} ;

    auto iCb = _callbacks.find(url) ;
    if (iCb == _callbacks.end())
      return ;
    
    switch (cmd)
    {
    case 0: // page load
    case 1: // page refresh
      // nothing
      break ;
    case 2: // button press
      // todo
      break ;
    case 3: // form submit
      // todo
      break ;
    }

    _client.send(Cmd::WebData, 100, 255) ; // undef argc
    _client.send(addr, 4) ;
    _client.send(port, 2) ;

    if (cmd == 0)
      iCb->second->PageLoad(*this, url) ;
    else
      iCb->second->PageRefresh(*this, url) ;

    _client.send(nullptr, 0) ; // arg end marker
    _client.send() ;
  }

  void WebServer::Close()
  {
    for (auto iCb : _callbacks)
      iCb.second->Close() ;
    _callbacks.clear() ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
