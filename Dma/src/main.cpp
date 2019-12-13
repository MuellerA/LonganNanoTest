////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}
#include <stdio.h>

#include "delay.h"
#include "spi.h"
#include "lcd.h"

extern "C" const uint8_t font[1520] ;

Spi0 spi ;
Lcd lcd{spi, font, 16, 8} ;


volatile uint8_t  * const CrcBase = (uint8_t*) 0x40023000 ;
volatile uint32_t * const CrcData = (uint32_t*)(CrcBase + 0x00) ;
volatile uint32_t * const CrcCtl  = (uint32_t*)(CrcBase + 0x08) ;

extern "C" int _put_char(int ch) // used by printf
{
  lcd.putChar(ch) ;
  return ch ;
}

inline uint32_t toUs(uint64_t t)
{
  return t * 4 / (SystemCoreClock/1000000) ;
}

int main()
{
  spi.setup() ;
  lcd.setup() ;

  const uint32_t DataSize = 4096 ;
  uint32_t data[DataSize] ;
  uint32_t crcMan ;
  uint32_t crcDma ;
  uint64_t t ;
  uint32_t us ; // micro sec
  uint32_t i ;

  { // check time calculation is correct
    t = get_timer_value() ;
    delayMs() ;
    us = toUs(get_timer_value() - t);

    if (us != 1000)
    {
      printf("time base error, expected 1000 got %lu", us) ;
      fflush(stdout) ;
      while (true) ;
    }
  }
  
  while (true)
  {
    for (uint32_t size = 256 ; size <= DataSize ; size *= 2)
    {
      lcd.putChar(12) ;
      printf("CRC Test - %lu\n", size) ;

      rcu_periph_clock_enable(RCU_CRC);
      rcu_periph_clock_enable(RCU_DMA0);

      for (i = 0 ; i < size ; ++i)
        data[i] = i ;

 
      { // for loop
        *CrcCtl = 0x00000001 ;
  
        t = get_timer_value() ;

        for (i = 0 ; i < size ; ++i)
          *CrcData = data[i] ;

        crcMan = *CrcData ;
  
        us = toUs(get_timer_value() - t);
      }
      printf("Loop: %luus\n", us) ;

      { // dma
        dma_parameter_struct dmaInit ;
        dma_struct_para_init(&dmaInit) ;
        dmaInit.direction = DMA_MEMORY_TO_PERIPHERAL;
        dmaInit.memory_addr = (uint32_t)data;
        dmaInit.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
        dmaInit.memory_width = DMA_MEMORY_WIDTH_32BIT;
        dmaInit.number = size;
        dmaInit.periph_addr = (uint32_t)CrcData;
        dmaInit.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
        dmaInit.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;
        dmaInit.priority = DMA_PRIORITY_MEDIUM;
        dma_deinit(DMA0, DMA_CH1) ;
        dma_init(DMA0, DMA_CH1, &dmaInit);
        /* configure DMA mode */
        dma_circulation_disable(DMA0, DMA_CH1);
        dma_memory_to_memory_enable(DMA0, DMA_CH1);

        *CrcCtl = 0x00000001 ;
    
        t = get_timer_value() ;
    
        dma_channel_enable(DMA0, DMA_CH1);
        while (dma_transfer_number_get(DMA0, DMA_CH1)) ;
        crcDma = *CrcData ;

        us = toUs(get_timer_value() - t);
      }
      printf("DMA:  %luus\n", us) ;

      if (crcMan == crcDma)
        printf("CRC:  %lu", crcDma) ;
      else
        printf("CRC error - Man != DMA") ;
      fflush(stdout) ;
      delayMs(2000) ;
    }
  }
}
