////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

void delayMs(uint32_t ms = 1)
{
  uint64_t t0 = get_timer_value() ;
  uint64_t d  = SystemCoreClock / 4 / 1000 * ms ; // sys tick is SytemCoreClock/4 Hz

  while ((get_timer_value() - t0) < d) ;
}

void LcdRstHi() { gpio_bit_set  (GPIOB, GPIO_PIN_1) ; }
void LcdRstLo() { gpio_bit_reset(GPIOB, GPIO_PIN_1) ; }
void LcdRsHi()  { gpio_bit_set  (GPIOB, GPIO_PIN_0) ; }
void LcdRsLo()  { gpio_bit_reset(GPIOB, GPIO_PIN_0) ; }
void LcdCsHi()  { gpio_bit_set  (GPIOB, GPIO_PIN_2) ; }
void LcdCsLo()  { gpio_bit_reset(GPIOB, GPIO_PIN_2) ; }

void LcdPutByte(uint8_t b)
{
  while(spi_i2s_flag_get(SPI0, SPI_FLAG_TBE) == RESET);
  spi_i2s_data_transmit(SPI0, b) ;
}

void LcdCmd(uint8_t cmd)
{
  LcdCsLo() ;

  LcdRsLo() ;
  LcdPutByte(cmd) ;
  while (spi_i2s_flag_get(SPI0,SPI_FLAG_TRANS) != RESET) ;
  
  LcdCsHi() ;
}

void LcdData(uint8_t data)
{
  LcdCsLo() ;

  LcdRsHi() ;
  LcdPutByte(data) ;
  while (spi_i2s_flag_get(SPI0,SPI_FLAG_TRANS) != RESET) ;
  
  LcdCsHi() ;
}

struct LcdCmdData
{
  uint8_t _cmd ;
  uint8_t _size ;
  uint8_t _data[16] ; // max size
} ;

void LcdCmd(const LcdCmdData &cmdData)
{
  LcdCsLo() ;

  LcdRsLo() ;
  LcdPutByte(cmdData._cmd) ;
  while (spi_i2s_flag_get(SPI0,SPI_FLAG_TRANS) != RESET) ;

  LcdRsHi() ;
  for (uint8_t i = 0 ; i < cmdData._size ; ++i)
    LcdPutByte(cmdData._data[i]) ;
  while (spi_i2s_flag_get(SPI0,SPI_FLAG_TRANS) != RESET) ;
  
  LcdCsHi() ;
}

void LcdFill(uint8_t x1, uint8_t x2, uint8_t y1, uint8_t y2, uint32_t rgb = 0x00)
{
  // Column Address Set
  LcdCmd({0x2a, 4, { 0x00,  1+x1, 0x00,  1+x2 } }) ; // x-offset  1
  // Row Address Set
  LcdCmd({0x2b, 4, { 0x00, 26+y1, 0x00, 26+y2 } }) ; // y-offset 26
  // Memory Write
  LcdCmd(0x2c) ;
  
  LcdCsLo() ;
  LcdRsHi() ;
  for (uint32_t i = x1 ; i <= x2 ; ++i)
  {
    for (uint32_t j = y1 ; j <= y2 ; ++j)
    {
      LcdPutByte(rgb >> 16) ;
      LcdPutByte(rgb >>  8) ;
      LcdPutByte(rgb >>  0) ;
    }
  }
  while (spi_i2s_flag_get(SPI0,SPI_FLAG_TRANS) != RESET) ;
  LcdCsHi() ;
}

extern "C" const uint8_t font[256][12] ;

void LcdChar(uint8_t x, uint8_t y, char ch)
{
  LcdCmd({0x2a, 4, { 0x00,  1+x+0, 0x00,  1+x+ 7 } }) ; // x-offset  1
  LcdCmd({0x2b, 4, { 0x00, 26+y+0, 0x00, 26+y+11 } }) ; // y-offset 26

  // Memory Write
  LcdCmd(0x2c) ;
  
  LcdCsLo() ;
  LcdRsHi() ;

  for (uint8_t y = 0 ; y < 12 ; ++y)
  {
    for (uint8_t x = 0, c = font[ch][y] ; x < 8 ; ++x, c <<= 1)
    {
      uint32_t rgb = (c & 0x80) ? 0x7f7f00 : 0x0000ff ;
      LcdPutByte(rgb >> 16) ;
      LcdPutByte(rgb >>  8) ;
      LcdPutByte(rgb >>  0) ;
    }
  }  
  while (spi_i2s_flag_get(SPI0,SPI_FLAG_TRANS) != RESET) ;
  LcdCsHi() ;
}

int main()
{
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_GPIOB);
  rcu_periph_clock_enable(RCU_AF);
  rcu_periph_clock_enable(RCU_SPI0);

  // LCD/SPI0: A5 CLK, A7 MISO A7 MOSI; B0 RS, B1 RST, B2 CS
  gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_5 | GPIO_PIN_7);
  gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_6); // not used by lcd
  gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2) ;

  spi_i2s_deinit(SPI0);
  spi_parameter_struct spi_init_struct;
  spi_struct_para_init(&spi_init_struct);
  spi_init_struct.trans_mode           = SPI_TRANSMODE_FULLDUPLEX;
  spi_init_struct.device_mode          = SPI_MASTER;
  spi_init_struct.frame_size           = SPI_FRAMESIZE_8BIT;
  spi_init_struct.clock_polarity_phase = SPI_CK_PL_HIGH_PH_2EDGE;
  spi_init_struct.nss                  = SPI_NSS_SOFT;
  spi_init_struct.prescale             = SPI_PSC_8;
  spi_init_struct.endian               = SPI_ENDIAN_MSB;
  spi_init(SPI0, &spi_init_struct);
  spi_enable(SPI0) ;

  LcdCsHi() ;

  // HW Reset
  LcdRstHi() ; delayMs( 25) ;
  LcdRstLo() ; delayMs(250) ;
  LcdRstHi() ; delayMs( 25) ;

  // Sleep Out
  LcdCmd(0x11) ; delayMs(120) ;

  // Display Inversion On
  LcdCmd(0x21) ;
  // Frame Rate Control (In normal mode/ Full colors)
  LcdCmd({0xb1,  3, { 0x05, 0x3a, 0x3a } }) ;
  // Frame Rate Control (In Idle mode/ 8-colors)
  LcdCmd({0xb2,  3, { 0x05, 0x3a, 0x3a } }) ;
  // Frame Rate Control (In Partial mode/ full colors)
  LcdCmd({0xB3,  6, { 0x05, 0x3A, 0x3A, 0x05, 0x3A, 0x3A } }) ;
  // Display Inversion Control
  LcdCmd({0xB4,  1, { 0x03 } }) ;
  // Power Control 1
  LcdCmd({0xC0,  3, { 0x62, 0x02, 0x04} }) ;
  // Power Control 2
  LcdCmd({0xC1,  1, { 0xC0 } }) ;
  // Power Control 3 (in Normal mode/ Full colors)
  LcdCmd({0xC2,  2, { 0x0D, 0x00 } }) ;
  // Power Control 4 (in Idle mode/ 8-colors)
  LcdCmd({0xC3,  2, { 0x8D, 0x6A } }) ;
  // Power Control 5 (in Partial mode/ full-colors)
  LcdCmd({0xC4,  2, { 0x8D, 0xEE } }) ;
  // VCOM Control 1
  LcdCmd({0xC5,  1, { 0x0E } }) ;
  // Gamma (‘+’polarity) Correction Characteristics Setting
  LcdCmd({0xE0, 16, { 0x10, 0x0E, 0x02, 0x03, 0x0E, 0x07, 0x02, 0x07, 0x0A, 0x12, 0x27, 0x37, 0x00, 0x0D, 0x0E, 0x10 } }) ;
  // Gamma ‘-’polarity Correction Characteristics Setting
  LcdCmd({0xE1, 16, { 0x10, 0x0E, 0x03, 0x03, 0x0F, 0x06, 0x02, 0x08, 0x0A, 0x13, 0x26, 0x36, 0x00, 0x0D, 0x0E, 0x10 } }) ;
  // Interface Pixel Format
  LcdCmd({0x3A,  1, { 0x06 } }) ; // 18 bit/pixel
  // Memory Data Access Control
  LcdCmd({0x36,  1, { 0x78 } }) ; // orientation 08, c8, 78, a8
  // Display On
  LcdCmd(0x29) ;

  LcdFill(0, 159, 0, 79) ;
  LcdFill(0, 11, 0, 21, 0x00ff00) ;
  LcdFill(1, 10, 1, 20, 0xff0000) ;
  LcdFill(35, 70, 35, 70, 0x0000ff) ;

  uint8_t x = 20 ;
  LcdChar(x, 16, 'H') ; x += 8 ;
  LcdChar(x, 16, 'a') ; x += 8 ;
  LcdChar(x, 16, 'l') ; x += 8 ;
  LcdChar(x, 16, 'l') ; x += 8 ;
  LcdChar(x, 16, 'o') ; x += 8 ;
  LcdChar(x, 16, ' ') ; x += 8 ;
  LcdChar(x, 16, 'L') ; x += 8 ;
  LcdChar(x, 16, 'o') ; x += 8 ;
  LcdChar(x, 16, 'n') ; x += 8 ;
  LcdChar(x, 16, 'g') ; x += 8 ;
  LcdChar(x, 16, 'a') ; x += 8 ;
  LcdChar(x, 16, 'n') ; x += 8 ;
  
  return 1 ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
