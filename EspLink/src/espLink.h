////////////////////////////////////////////////////////////////////////////////
// espLink.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GD32VF103/usart.h"

namespace EspLink
{
  enum class Cmd
  {
    Null = 0,         // null, mainly to prevent 0 from doing something bad 
    Sync,             // Synchronize, starts the protocol 
    RespV,            // Response with a value 
    RespCb,           // Response with a callback 
    WifiStatus,       // Get the wifi status 
    CbAdd,            // Add a custom callback 
    CbEvents,         // ??? 
    GetTime,          // Get current time in seconds since the unix epoch 

    MqttSetup = 10,   // Register callback functions 
    MqttPublish,      // Publish MQTT topic 
    MqttSubscribe,    // Subscribe to MQTT topic 
    MqttLwt,          // Define MQTT last will 

    RestSetup = 20,   // Setup REST connection 
    RestRequest,      // Make request to REST server 
    RestSetHeader,    // Define HTML header 

    WebSetup = 30,    // web-server setup 
    WebData,          // used for publishing web-server data 

    SocketSetup = 40, // Setup socket connection 
    SocketSend,       // Send socket packet 
  } ;

  enum class WifiStatus
  {
    Idle = 0,         // Idle status 
    Connecting,       // Trying to connect 
    WrongPassword,    // Connection error, wrong password 
    NoApFound,        // Connection error, AP not found 
    ConnectFail,      // Connection error, connection failed 
    GotIp             // Connected, received IP 
  };

  struct RecvBuff
  {
    static const uint32_t _size = 2000 ;
    uint8_t _raw[_size] ;
    uint32_t _len ;

    uint16_t _cmd ;
    uint16_t _argc ;
    uint32_t _ctx ;
    uint16_t _crc ;
    struct
    {
      uint8_t *_data ;
      uint32_t _len ;
    } _param[10] ;
  } ;

  class Callback
  {
  public:
    virtual void Receive(const RecvBuff&) = 0 ;
    virtual void Close() = 0 ;
  } ;
  
  class Client
  {
  public:
    Client(::RV::GD32VF103::Usart &usart) ;

    bool sync() ;
    void wifiStatus(uint8_t &status) ;
    void unixTime(uint32_t &time) ;
    bool poll() ;
    const RecvBuff& recvBuff() ;
    
  private:
    friend class WifiCallback ;
    class WifiCallback : public Callback
    {
    public:
      WifiCallback(Client &client) : _client{client}
      {
      }
      virtual void Receive(const RecvBuff &rx)
      {
        _client._wifiStatus = rx._param[0]._data[0] ;
      }
      virtual void Close()
      {
      }
    private:
      Client &_client ;
    } ;
    
    static void crc(uint16_t &c, uint8_t b) ;
    void putNoEsc(uint8_t b) ;
    void put(uint8_t b) ;
    void put(const uint8_t *data, uint32_t len) ;
    bool get(uint8_t &b, bool &end) ;

    void send(Cmd cmd, uint32_t ctx, uint16_t argc) ; // start
    void send(uint8_t *data, uint16_t len) ;          // parameter * start.argc
    void send() ;                                     // end

    ::RV::GD32VF103::Usart &_usart ;
    uint16_t _crc ;
    RecvBuff _recvBuff ;

    WifiCallback _wifiCallback ;
    uint8_t  _wifiStatus{0} ;
    uint32_t _unixTime{0} ;
    ::RV::GD32VF103::TickTimer _unixTimeTick{3600000, true} ;
    Callback *_callback[32] ; // [0]: nullptr, [1]: this->_wifiCallback, other: user's
  } ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
