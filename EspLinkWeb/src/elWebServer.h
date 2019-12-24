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

    bool sendParameter(const std::string &key, const std::string &val) const ; // only call from WebServerCallback::PageLoad/Refresh
    bool getParameter(std::string &val) const ; // only call from WebServerCallback::ButtonPress
    bool getParameter(const std::string &key, std::string &val) const ; // only call from WebServerCallback::FormSubmit
    
    // ClientCallback
    virtual void rx(const Pdu&) ;
    virtual void close() ;
    
  private:
    Client &_client ;
    std::map<std::string, WebServerCallback*> _callbacks ;
    const Pdu *_rxPdu ;
  } ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
