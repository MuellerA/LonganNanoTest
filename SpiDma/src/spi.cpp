////////////////////////////////////////////////////////////////////////////////
// spi.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "spi.h"

    Spi::Spi(uint32_t spi, rcu_periph_enum rcuSpi, rcu_periph_enum rcuGpio, uint32_t gpio, uint32_t pinClk, uint32_t pinMiso, uint32_t pinMosi)
      : _spi{spi}, _rcuSpi{rcuSpi}, _rcuGpio{rcuGpio}, _gpio{gpio}, _pinClk{pinClk}, _pinMiso{pinMiso}, _pinMosi{pinMosi}  
    {
    }

    void Spi::setup(Spi::Psc spiPsc)
    {
      rcu_periph_clock_enable(RCU_AF);
      rcu_periph_clock_enable(_rcuSpi);
      rcu_periph_clock_enable(RCU_DMA0);      
      rcu_periph_clock_enable(_rcuGpio);

      // CLK, MISO, MOSI; 
      gpio_init(_gpio, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, _pinClk | _pinMosi);
      gpio_init(_gpio, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, _pinMiso);

      dma_parameter_struct dmaInit;
      
      dma_deinit(DMA0, DMA_CH2) ;
      dma_struct_para_init(&dmaInit) ;
      dmaInit.number       = 80*80*2 ;                 // set in copy()
      dmaInit.direction    = DMA_MEMORY_TO_PERIPHERAL ;
      dmaInit.priority     = DMA_PRIORITY_MEDIUM ;
      dmaInit.memory_addr  = (uint32_t)nullptr ;     // set in copy()
      dmaInit.memory_width = DMA_MEMORY_WIDTH_8BIT ;
      dmaInit.memory_inc   = DMA_MEMORY_INCREASE_ENABLE ;
      dmaInit.periph_addr  = (uint32_t)&SPI_DATA(_spi) ;
      dmaInit.periph_width = DMA_PERIPHERAL_WIDTH_8BIT ;
      dmaInit.periph_inc   = DMA_PERIPH_INCREASE_DISABLE ;
      dma_init(DMA0, DMA_CH2, &dmaInit) ;

      dma_circulation_disable(DMA0, DMA_CH2) ;
      dma_memory_to_memory_disable(DMA0, DMA_CH2) ;

      spi_i2s_deinit(_spi);
      spi_parameter_struct spiInit;
      spi_struct_para_init(&spiInit);
      spiInit.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
      spiInit.device_mode          = SPI_MASTER;
      spiInit.frame_size           = SPI_FRAMESIZE_8BIT;
      spiInit.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
      spiInit.nss                  = SPI_NSS_SOFT;
      spiInit.prescale             = (uint32_t)spiPsc ;
      spiInit.endian               = SPI_ENDIAN_MSB;
      spi_init(_spi, &spiInit);

      spi_enable(_spi) ;

      eclic_global_interrupt_enable();
      eclic_priority_group_set(ECLIC_PRIGROUP_LEVEL3_PRIO1);
      eclic_irq_enable(DMA0_Channel2_IRQn, 1, 0);
    }

    bool Spi::isTransmit()
    {
      return spi_i2s_flag_get(_spi,SPI_FLAG_TRANS) != RESET ;
    }

    bool Spi::get(uint8_t &b)
    {
      // todo
      return false ;
    }
    
    bool Spi::put(uint8_t b)
    {
      while(spi_i2s_flag_get(_spi, SPI_FLAG_TBE) == RESET);
      spi_i2s_data_transmit(_spi, b) ;
      return true ;
    }

extern "C"
{
  void DMA0_Channel2_IRQHandler()
  {
    dma_interrupt_disable(DMA0, DMA_CH2, DMA_INT_FTF) ;
    dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_FTF) ;
    dma_interrupt_flag_clear(DMA0, DMA_CH2, DMA_INT_FLAG_G) ;
    dma_channel_disable(DMA0, DMA_CH2);
    Spi::spi0().copyEnd() ;
  }
}

    void Spi::copy(std::function<void()> begin,
                   std::function<void()> end,
                   uint8_t *data, size_t size)
    {
      switch (2)
      {
      case 0: // cpu
        begin() ;
        for (size_t i = 0 ; i < size ; ++i)
          put(*(data++)) ;
        end() ;
        break ;
      case 1: // dma poll
        begin() ;
        dma_memory_address_config(DMA0, DMA_CH2, (uint32_t)data) ;
        dma_transfer_number_config(DMA0, DMA_CH2, size) ;
        dma_channel_enable(DMA0, DMA_CH2);
        spi_dma_enable(_spi, SPI_DMA_TRANSMIT) ;
        while (!dma_flag_get(DMA0, DMA_CH2, DMA_FLAG_FTF) &&
               !dma_flag_get(DMA0, DMA_CH2, DMA_FLAG_ERR)) ;
        dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_FTF) ;
        dma_flag_clear(DMA0, DMA_CH2, DMA_FLAG_ERR) ;
        dma_channel_disable(DMA0, DMA_CH2);
        end() ;
        break ;
      case 2: // dma irq
        begin() ;
        _copyEnd = end ;
        dma_memory_address_config(DMA0, DMA_CH2, (uint32_t)data) ;
        dma_transfer_number_config(DMA0, DMA_CH2, size) ;
        dma_channel_enable(DMA0, DMA_CH2);
        spi_dma_enable(_spi, SPI_DMA_TRANSMIT) ;
        dma_interrupt_enable(DMA0, DMA_CH2, DMA_INT_FTF) ;
        break ;
      }
    }

    void Spi::copyEnd()
    {
      _copyEnd() ;
    }

    Spi& Spi::spi0()
    {
      static Spi *spi0 = new Spi{SPI0, RCU_SPI0, RCU_GPIOA, GPIOA, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7} ;
      return *spi0 ;
    }

    Spi& Spi::spi1()
    {
      static Spi *spi1 = new Spi{SPI1, RCU_SPI1, RCU_GPIOB, GPIOB, GPIO_PIN_13, GPIO_PIN_14, GPIO_PIN_15} ;
      return *spi1 ;
    }

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

