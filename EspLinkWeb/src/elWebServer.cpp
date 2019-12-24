////////////////////////////////////////////////////////////////////////////////
// elWebServer.h
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include <string.h>
#include "elWebServer.h"

namespace EspLink
{
  WebServer::WebServer(Client &client)
    : _client{client}, _rxPdu{nullptr}
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

  bool WebServer::sendParameter(const std::string &key, const std::string &val) const
  {
    if (!_rxPdu)
      return false ;
    
    std::string null{"\000", 1} ;
    std::string param{null + key + null + val} ;
    _client.send((uint8_t*)param.data(), param.size()) ;
    return true ;
  }

  bool WebServer::getParameter(std::string &val) const
  {
    if (!_rxPdu)
      return false ;

    if (_rxPdu->_argc < 4)
      return false ;

    val.assign((char*)_rxPdu->_param[4]._data, _rxPdu->_param[4]._len) ;

    return true ;
  }
  
  bool WebServer::getParameter(const std::string &key, std::string &val) const
  {
    if (!_rxPdu)
      return false ;
    
    for (uint16_t iParam = 4 ; iParam < _rxPdu->_argc ; ++iParam)
    {
      char *paramKey = (char*)_rxPdu->_param[iParam]._data+1 ;
      if (!key.compare(paramKey))
      {
        char *paramVal = paramKey + strlen(paramKey) + 1 ;
        uint16_t paramLen = _rxPdu->_param[iParam]._len - (paramVal - paramKey) - 1 ;
        val.assign(paramVal, paramLen) ;
        return true ;
      }
    }
    return false ;
  }
  
  void WebServer::rx(const Pdu &rxPdu)
  {
    _rxPdu = &rxPdu ;
    uint16_t cmd{*(uint16_t*)rxPdu._param[0]._data} ;
    uint8_t *addr{rxPdu._param[1]._data} ;
    uint8_t *port{rxPdu._param[2]._data} ;
    std::string url{(char*)rxPdu._param[3]._data, rxPdu._param[3]._len} ;

    auto iCb = _callbacks.find(url) ;
    if (iCb == _callbacks.end())
    {
      _rxPdu = nullptr ;
      return ;
    }

    switch (cmd)
    {
    case 0: // page load
    case 1: // page refresh
      // nothing
      break ;
    case 2: // button press
      iCb->second->ButtonPress(*this, url) ;
      break ;
    case 3: // form submit
      iCb->second->FormSubmit(*this, url) ;
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

    _rxPdu = nullptr ;
  }

  void WebServer::close()
  {
    for (auto iCb : _callbacks)
      iCb.second->Close() ;
    _callbacks.clear() ;
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
