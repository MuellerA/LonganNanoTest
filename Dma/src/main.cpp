////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "Longan/lcd.h"
#include "Longan/fonts.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

Lcd& lcd{Lcd::lcd()} ;

volatile uint8_t  * const CrcBase = (uint8_t*) 0x40023000 ;
volatile uint32_t * const CrcData = (uint32_t*)(CrcBase + 0x00) ;
volatile uint32_t * const CrcCtl  = (uint32_t*)(CrcBase + 0x08) ;

int main()
{
  lcd.setup() ;

  LcdArea lcdTitle{lcd,  0, 140,  0, 16, &Roboto_Bold7pt7b, 0xffffffUL, 0x4040ffUL} ;
  LcdArea lcdLabel{lcd,  0,  40, 16, 64} ;
  LcdArea lcdData {lcd, 40, 120, 16, 64} ;
  
  const uint32_t DataSize = 4096 ;
  uint32_t data[DataSize] ;
  uint32_t crcMan ;
  uint32_t crcDma ;
  uint64_t t ;
  uint32_t usLoop ; // micro sec
  uint32_t usDma ;
  uint32_t i ;

  lcdTitle.put("  CRC/DMA Test  ") ;
  lcdLabel.put("N:\nLoop:\nDMA:\nCRC:") ;
  
  while (true)
  {
    for (uint32_t size = 256 ; size <= DataSize ; size *= 2)
    {
      rcu_periph_clock_enable(RCU_CRC);
      rcu_periph_clock_enable(RCU_DMA0);

      for (i = 0 ; i < size ; ++i)
        data[i] = i ;
 
      { // for loop
        *CrcCtl = 0x00000001 ;
  
        t = TickTimer::now() ;

        for (i = 0 ; i < size ; ++i)
          *CrcData = data[i] ;

        crcMan = *CrcData ;
  
        usLoop = TickTimer::tickToUs(TickTimer::now() - t);
      }

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

        usDma = TickTimer::tickToUs(TickTimer::now() - t);
      }

      lcdData.clear() ;
      lcdData.txtPos(0) ; lcdData.put(size) ;
      lcdData.txtPos(1) ; lcdData.put(usLoop) ; lcdData.put("us") ;
      lcdData.txtPos(2) ; lcdData.put(usDma) ; lcdData.put("us") ;
      lcdData.txtPos(3) ;
      if (crcMan == crcDma)
        lcdData.put(crcDma) ;
      else
        lcdData.put("error - Man != DMA") ;

      TickTimer::delayMs(2000) ;
    }
  }
}
