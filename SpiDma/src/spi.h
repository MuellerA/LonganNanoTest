////////////////////////////////////////////////////////////////////////////////
// spi.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

    #include <functional>

    class Spi
    {
    private:
      Spi(uint32_t spi, rcu_periph_enum rcuSpi, rcu_periph_enum rcuGpio, uint32_t gpio, uint32_t pinClk, uint32_t pinMiso, uint32_t pinMosi) ;
      Spi(const Spi&) = delete ;
      
    public:
      enum class Psc
        {
         _2   = SPI_PSC_2,
         _4   = SPI_PSC_4,
         _8   = SPI_PSC_8,
         _16  = SPI_PSC_16,
         _32  = SPI_PSC_32,
         _64  = SPI_PSC_64,
         _128 = SPI_PSC_128,
         _256 = SPI_PSC_256,
         
        } ;
      void setup(Psc spiPsc = Psc::_4) ;

      virtual bool get(uint8_t &b) ;
      virtual bool put(uint8_t  b) ;
      
      bool isTransmit() ;

      void copy(std::function<void()> begin,
                std::function<void()> end,
                uint8_t *data, size_t size) ;
      void copyEnd() ;
      
      static Spi& spi0() ;
      static Spi& spi1() ;
      
    private:
      uint32_t _spi ;
      rcu_periph_enum _rcuSpi ;
      rcu_periph_enum _rcuGpio ;
      uint32_t _gpio ;
      uint32_t _pinClk ;
      uint32_t _pinMiso ;
      uint32_t _pinMosi ;
      std::function<void()> _copyEnd ;
    } ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
