////////////////////////////////////////////////////////////////////////////////
// elWebServer.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "espLink.h"

namespace EspLink
{
  class WebServer ;
  
  class WebServerCallback
  {
  public:
    virtual void PageLoad(const WebServer &server, const std::string &page) = 0 ;
    virtual void PageRefresh(const WebServer &server, const std::string &page) = 0 ;
    virtual void ButtonPress(const WebServer &server, const std::string &page) = 0 ;
    virtual void FormSubmit(const WebServer &server, const std::string &page) = 0 ;
    virtual void Close() = 0 ;
  } ;
  
  class WebServer : ClientCallback
  {
  public:
    WebServer(Client &client) ;

    void setup() ;

    bool addCallback(const std::string &url, WebServerCallback *cb) ;
    bool removeCallback(const std::string &url) ;

    void sendParameter(const std::string &key, const std::string &val) const ; // only call from WebServerCallback
    
    // ClientCallback
    virtual void Receive(const RecvBuff&) ;
    virtual void Close() ;
    
  private:
    Client &_client ;
    std::map<std::string, WebServerCallback*> _callbacks ;
  } ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
