////////////////////////////////////////////////////////////////////////////////
// led.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

class RgbLed
{
public:

  class Led
  {
    friend class RgbLed ;
    Led(uint32_t gpio, uint32_t pin) : _gpio{gpio}, _pin{pin}
    {
    }

  public:
    void on()  const { GPIO_BC (_gpio) = _pin ; } // GPIO port bit operation register
    void off() const { GPIO_BOP(_gpio) = _pin ; } // GPIO bit clear register

  private:
    uint32_t _gpio ;
    uint32_t _pin ;
  } ;

  const Led &r() { return _r ; }
  const Led &g() { return _g ; }
  const Led &b() { return _b ; }
    
  RgbLed()
  {
  } ;
  
  void setup()
  {
    // enable GPIO port clock
    rcu_periph_clock_enable(RCU_GPIOA) ;
    rcu_periph_clock_enable(RCU_GPIOC) ;

    // GPIO parameter initialization
    // - gpio_periph
    // - mode: gpio pin mode GPIO_MODE_OUT_PP: GPIO output with push-pull
    // - speed: gpio output max speed value GPIO_OSPEED_50MHZ: output max speed 50MHz
    // - pin: GPIO pin
    gpio_init(_r._gpio, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, _r._pin) ;
    gpio_init(_g._gpio, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, _g._pin) ;
    gpio_init(_b._gpio, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, _b._pin) ;

    _r.off() ;
    _g.off() ;
    _b.off() ;
  }

  void rgb(bool r, bool g, bool b)
  {
    if (r) _r.on() ; else _r.off() ;
    if (g) _g.on() ; else _g.off() ;
    if (b) _b.on() ; else _b.off() ;
  }

  void red()     { _r.on()  ; _g.off() ; _b.off() ; }
  void green()   { _r.off() ; _g.on()  ; _b.off() ; }
  void blue()    { _r.off() ; _g.off() ; _b.on()  ; }

  void yellow()  { _r.on()  ; _g.on()  ; _b.off() ; }
  void magenta() { _r.on()  ; _g.off() ; _b.on()  ; }
  void cyan()    { _r.off() ; _g.on()  ; _b.on()  ; }

  void white()   { _r.on()  ; _g.on()  ; _b.on()  ; }
  void black()   { _r.off() ; _g.off() ; _b.off() ; }
  
private:
  Led _r{GPIOC, GPIO_PIN_13} ;
  Led _g{GPIOA, GPIO_PIN_1} ;
  Led _b{GPIOA, GPIO_PIN_2} ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
