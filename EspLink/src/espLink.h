////////////////////////////////////////////////////////////////////////////////
// espLink.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "usart.h"

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
    } param[10] ;
  } ;
  
  class Client
  {
  public:
    Client(Usart &usart) ;

    bool sync() ;     // sync with esp-link
    bool wifiStatus() ;
    bool unixTime() ;
    bool poll() ;
    const RecvBuff& recvBuff() ;
    void recvComplete() ;
    
  private:
    static void crc(uint16_t &c, uint8_t b) ;
    void putNoEsc(uint8_t b) ;
    void put(uint8_t b) ;
    void put(const uint8_t *data, uint32_t len) ;
    bool get(uint8_t &b, bool &end) ;

    void send(Cmd cmd, uint32_t ctx, uint16_t argc) ; // start
    void send(uint8_t *data, uint16_t len) ;          // start.argc times
    void send() ;                                     // end

    Usart &_usart ;
    uint16_t _crc ;
    RecvBuff _recvBuff ;
  } ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
