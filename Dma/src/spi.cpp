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

void Spi::setup()
{
  rcu_periph_clock_enable(RCU_AF);
  rcu_periph_clock_enable(_rcuSpi);
  rcu_periph_clock_enable(_rcuGpio);

  // A5 CLK, A7 MISO A7 MOSI; 
  gpio_init(_gpio, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, _pinClk | _pinMosi);
  gpio_init(_gpio, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, _pinMiso);

  spi_i2s_deinit(_spi);
  spi_parameter_struct spi_init_struct;
  spi_struct_para_init(&spi_init_struct);
  spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
  spi_init_struct.device_mode          = SPI_MASTER;
  spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
  spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
  spi_init_struct.nss                  = SPI_NSS_SOFT;
  spi_init_struct.prescale             = SPI_PSC_8;
  spi_init_struct.endian               = SPI_ENDIAN_MSB;
  spi_init(_spi, &spi_init_struct);
  spi_enable(_spi) ;
}

bool Spi::isTransmit()
{
  return spi_i2s_flag_get(_spi,SPI_FLAG_TRANS) != RESET ;
}
void Spi::putByte(uint8_t b)
{
  while(spi_i2s_flag_get(_spi, SPI_FLAG_TBE) == RESET);
  spi_i2s_data_transmit(_spi, b) ;
}

Spi0::Spi0()
  : Spi(SPI0, RCU_SPI0, RCU_GPIOA, GPIOA, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7)
{
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

