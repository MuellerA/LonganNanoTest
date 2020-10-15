////////////////////////////////////////////////////////////////////////////////
// diskio.c
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "ff.h"
#include "diskio.h"
}

extern "C"
{
#include "gd32vf103.h"
}

extern void Debug(char c, uint8_t v) ;


#include "GD32VF103/time.h"
#include "GD32VF103/gpio.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::Gpio ;

Gpio  &gpioDO{Gpio::gpioB14()} ;
Gpio  &gpioDI{Gpio::gpioB15()} ;
Gpio  &gpioCK{Gpio::gpioB13()} ;
Gpio  &gpioCS{Gpio::gpioB12()} ;

////////////////////////////////////////////////////////////////////////////////

static DSTATUS Stat = STA_NOINIT;       // Disk status
static BYTE    CardType;                // b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing

////////////////////////////////////////////////////////////////////////////////

// MMC/SD command (SPI mode)
static const uint8_t CMD0   = 0      ;       // GO_IDLE_STATE
static const uint8_t CMD1   = 1      ;       // SEND_OP_COND
static const uint8_t ACMD41 = 0x80+41;       // SEND_OP_COND (SDC)
static const uint8_t CMD8   = 8      ;       // SEND_IF_COND
static const uint8_t CMD9   = 9      ;       // SEND_CSD
static const uint8_t CMD10  = 10     ;       // SEND_CID
static const uint8_t CMD12  = 12     ;       // STOP_TRANSMISSION
static const uint8_t CMD13  = 13     ;       // SEND_STATUS
static const uint8_t ACMD13 = 0x80+13;       // SD_STATUS (SDC)
static const uint8_t CMD16  = 16     ;       // SET_BLOCKLEN
static const uint8_t CMD17  = 17     ;       // READ_SINGLE_BLOCK
static const uint8_t CMD18  = 18     ;       // READ_MULTIPLE_BLOCK
static const uint8_t CMD23  = 23     ;       // SET_BLOCK_COUNT
static const uint8_t ACMD23 = 0x80+23;       // SET_WR_BLK_ERASE_COUNT (SDC)
static const uint8_t CMD24  = 24     ;       // WRITE_BLOCK
static const uint8_t CMD25  = 25     ;       // WRITE_MULTIPLE_BLOCK
static const uint8_t CMD32  = 32     ;       // ERASE_ER_BLK_START
static const uint8_t CMD33  = 33     ;       // ERASE_ER_BLK_END
static const uint8_t CMD38  = 38     ;       // ERASE
static const uint8_t CMD55  = 55     ;       // APP_CMD
static const uint8_t CMD58  = 58     ;       // READ_OCR

////////////////////////////////////////////////////////////////////////////////

void diskioSetup()
{
  gpioDO.setup(Gpio::Mode::IN_FL ) ;
  gpioDI.setup(Gpio::Mode::OUT_PP) ;
  gpioCK.setup(Gpio::Mode::OUT_PP) ; gpioCK.low() ;
  gpioCS.setup(Gpio::Mode::OUT_PP) ; gpioCS.high() ;
}

static inline void gpioCkToggle()
{
  gpioCK.high() ;
  gpioCK.low() ;
}

////////////////////////////////////////////////////////////////////////////////

static void tx(const BYTE* buff, // Data to be sent
               UINT size)          //Number of bytes to send
{
  BYTE d;

  for (UINT i = 0 ; i < size ; ++i)
  {
    d = *buff++;
    gpioDI.set(d & 0x80) ; gpioCkToggle() ;
    gpioDI.set(d & 0x40) ; gpioCkToggle() ;
    gpioDI.set(d & 0x20) ; gpioCkToggle() ;
    gpioDI.set(d & 0x10) ; gpioCkToggle() ;
    gpioDI.set(d & 0x08) ; gpioCkToggle() ;
    gpioDI.set(d & 0x04) ; gpioCkToggle() ;
    gpioDI.set(d & 0x02) ; gpioCkToggle() ;
    gpioDI.set(d & 0x01) ; gpioCkToggle() ;
  }
}

static void rx(BYTE *buff,  // Pointer to read buffer
               UINT size)     // Number of bytes to receive
{
  gpioDI.high(); // Send 0xFF

  for (UINT i = 0 ; i < size ; ++i)
  {
    BYTE r = 0 ;
    if (gpioDO.get()) { r |= 0x80 ; } gpioCkToggle() ;
    if (gpioDO.get()) { r |= 0x40 ; } gpioCkToggle() ;
    if (gpioDO.get()) { r |= 0x20 ; } gpioCkToggle() ;
    if (gpioDO.get()) { r |= 0x10 ; } gpioCkToggle() ;
    if (gpioDO.get()) { r |= 0x08 ; } gpioCkToggle() ;
    if (gpioDO.get()) { r |= 0x04 ; } gpioCkToggle() ;
    if (gpioDO.get()) { r |= 0x02 ; } gpioCkToggle() ;
    if (gpioDO.get()) { r |= 0x01 ; } gpioCkToggle() ;
    *buff++ = r;
  }
}

static bool wait_ready(UINT timeoutMs = 500)
{
  BYTE d;
  TickTimer timeout(timeoutMs) ;

  while (!timeout())
  {
    rx(&d, 1) ;
    if (d == 0xff)
      return true ;
    TickTimer::delayUs(100) ;
  }
  return false ;
}

static void deselect()
{
  BYTE d = 0xff ;

  gpioCS.high();
  tx(&d, 1) ;     // Dummy clock (force DO hi-z for multiple slave SPI)
}

static bool select()
{
  BYTE d = 0xff ;

  gpioCS.low();
  tx(&d, 1); // Dummy clock (force DO enabled)
  if (!wait_ready())
  {
    deselect() ;
    return false ;
  }

  return true ;
}

static bool rx_datablock(BYTE *buff,  // Data buffer to store received data
                         UINT btr)    // Byte count
{
  BYTE d[2];
  TickTimer timeout(100) ;

  while (true)
  {
    rx(d, 1);
    if (d[0] != 0xFF)
      break;
    if (timeout())
      return false ;
    TickTimer::delayUs(100);
  }
  if (d[0] != 0xFE)
    return false;           // If not valid data token, return with error

  rx(buff, btr);            // Receive the data block into buffer
  rx(d, 2);                 // Discard CRC

  return true;              // Return with success
}

#if FF_FS_READONLY == 0
static bool tx_datablock(const BYTE *buff,  //512 byte data block to be transmitted
                         BYTE token)        // Data/Stop token
{
  BYTE d[2];

  if (!wait_ready())
    return false;

  d[0] = token;
  tx(d, 1);                 // tx a token
  if (token != 0xFD)        // Is it data token?
  {
    tx(buff, 512);          // Xmit the 512 byte data block to MMC
    rx(d, 2);               // Xmit dummy CRC (0xFF,0xFF)
    rx(d, 1);               // Receive data response
    if ((d[0] & 0x1F) != 0x05)  // If not accepted, return with error
      return false;
  }

  return true;
}
#endif

// Returns command response (bit7==1:Send failed)*/
static BYTE send_cmd (BYTE cmd,         // Command byte
                      DWORD arg)        // Argument
{
  BYTE n, d, buf[6];

  if (cmd & 0x80)       // ACMD<n> is the command sequense of CMD55-CMD<n>
  {
    cmd &= 0x7F;
    n = send_cmd(CMD55, 0);
    if (n > 1)
      return n;
  }

  // Select the card and wait for ready except to stop multiple block read
  if (cmd != CMD12)
  {
    deselect();
    if (!select())
      return 0xFF;
  }

  // Send a command packet
  buf[0] = 0x40 | cmd;                  // Start + Command index
  buf[1] = (BYTE)(arg >> 24);           // Argument[31..24]
  buf[2] = (BYTE)(arg >> 16);           // Argument[23..16]
  buf[3] = (BYTE)(arg >> 8);            // Argument[15..8]
  buf[4] = (BYTE)arg;                   // Argument[7..0]
  n = 0x01;                             // Dummy CRC + Stop
  if (cmd == CMD0) n = 0x95;            // (valid CRC for CMD0(0))
  if (cmd == CMD8) n = 0x87;            // (valid CRC for CMD8(0x1AA))
  buf[5] = n;
  tx(buf, 6);

  // Receive command response
  if (cmd == CMD12)
    rx(&d, 1);      // Skip a stuff byte when stop reading
  n = 10;               // Wait for a valid response in timeout of 10 attempts
  for (BYTE i = 0 ; i < 10 ; ++i)
  {
    rx(&d, 1);
    if (!(d & 0x80))
      break ;
  }

  return d;   // Return with the response value
}

////////////////////////////////////////////////////////////////////////////////

DSTATUS disk_initialize (BYTE drv) // [IN] Physical drive number
{
  Debug('i', drv) ;
  BYTE type, cmd, buf[10];

  if (drv)
    return RES_NOTRDY;

  TickTimer::delayMs(10);

  rx(buf, 10);                        // Apply 80 dummy clocks and the card gets ready to receive command

  type = 0;
  if (send_cmd(CMD0, 0) == 1)         // Enter Idle state
  {
    if (send_cmd(CMD8, 0x1AA) == 1)   // SDv2?
    {
      rx(buf, 4);                     // Get trailing return value of R7 resp
      if (buf[2] == 0x01 && buf[3] == 0xAA)   // The card can work at vdd range of 2.7-3.6V
      {
        TickTimer timeout(1000) ;
        while (!timeout())
        {
          if (send_cmd(ACMD41, 1UL << 30) == 0)
          {
            if (send_cmd(CMD58, 0) == 0)   // Check CCS bit in the OCR
            {
              rx(buf, 4);
              type = (buf[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;  // SDv2
            }
            break ;
          }
          TickTimer::delayMs(1) ;
        }
      }
    }
    else              // SDv1 or MMCv3
    {
      if (send_cmd(ACMD41, 0) <= 1)
      {
        type = CT_SD1;
        cmd = ACMD41;  // SDv1
      }
      else
      {
        type = CT_MMC;
        cmd = CMD1;  // MMCv3
      }
      TickTimer timeout(1000) ;
      while (true)
      {
        if (send_cmd(cmd, 0) == 0)
        {
          if (send_cmd(CMD16, 512) != 0)
            type = 0 ;
          break;
        }
        if (timeout())
        {
          type = 0 ;
          break ;
        }
      }
    }
  }
  CardType = type;
  Stat = type ? 0 : STA_NOINIT;

  deselect();

  return Stat;
}

DSTATUS disk_status (BYTE drv)     // [IN] Physical drive number
{
  Debug('s', drv) ;
  if (drv)
    return STA_NOINIT ;

  return Stat ;
}

DRESULT disk_read(BYTE drv,      // [IN] Physical drive number
                  BYTE* buff,    // [OUT] Pointer to the read data buffer
                  LBA_t sector,  // [IN] Start sector number
                  UINT count)    // [IN] Number of sectros to read
{
  Debug('r', drv) ;
  DWORD sect = (DWORD)sector;

  if (drv || !count)
    return RES_PARERR ;

  if (Stat & STA_NOINIT)
    return RES_NOTRDY;

  if (!(CardType & CT_BLOCK))      // Convert LBA to byte address if needed
    sect *= 512;

  UINT iRx = 0 ;
  if (count == 1)
  {
    if ((send_cmd(CMD17, sect) == 0) &&
        rx_datablock(buff, 512))
      iRx = 1 ;
  }
  else
  {
    if (send_cmd(CMD18, sect) == 0)
    {
      for (iRx = 0 ; iRx < count ; ++iRx)
      {
        if (!rx_datablock(buff, 512))
          break;
        buff += 512;
      }
      send_cmd(CMD12, 0);       // STOP_TRANSMISSION
    }
  }

  deselect();

  return (iRx == count) ? RES_OK : RES_ERROR ;
}

#if FF_FS_READONLY == 0
DRESULT disk_write (BYTE drv,         // [IN] Physical drive number
                    const BYTE* buff, // [IN] Pointer to the data to be written
                    LBA_t sector,     // [IN] Sector number to write from
                    UINT count)       // [IN] Number of sectors to write
{
  Debug('w', drv) ;

  DWORD sect = (DWORD)sector;

  if (drv || !count)
    return RES_PARERR ;
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;
  if (!(CardType & CT_BLOCK))
    sect *= 512;                        // Convert LBA to byte address if needed

  UINT iTx = 0 ;
  if (count == 1)                       // Single block write
  {
    if ((send_cmd(CMD24, sect) == 0) && // WRITE_BLOCK
        tx_datablock(buff, 0xFE))
      iTx = 1;
  }
  else                                  // Multiple block write
  {
    if (CardType & CT_SDC)
      send_cmd(ACMD23, count);
    if (send_cmd(CMD25, sect) == 0)     // WRITE_MULTIPLE_BLOCK
    {
      for (iTx = 0 ; iTx < count ; ++iTx)
      {
        if (!tx_datablock(buff, 0xFC))
          break;
        buff += 512;
      }
      if (!tx_datablock(0, 0xFD))       // STOP_TRAN token
        iTx = 0 ;
    }
  }
  deselect();

  return (count == iTx) ? RES_OK : RES_ERROR;
}
#endif

DRESULT disk_ioctl (BYTE drv,       // [IN] Drive number
                    BYTE ctrl,      // [IN] Control command code
                    void* buff)     // [I/O] Parameter and data buffer
{
  Debug('i', drv) ;

  DRESULT res;
  BYTE n, csd[16];
  DWORD cs;
  BYTE dummy ;

  if (drv)
    return RES_PARERR ;
  if (Stat & STA_NOINIT)
    return RES_NOTRDY;          // Check if card is in the socket

  res = RES_ERROR;
  switch (ctrl)
  {
  case CTRL_SYNC :              // Make sure that no pending write process
    if (select())
      res = RES_OK;
    break;

  case GET_SECTOR_COUNT :       // Get number of sectors on the disk (DWORD)
    if ((send_cmd(CMD9, 0) == 0) && rx_datablock(csd, 16))
    {
      if ((csd[0] >> 6) == 1)   // SDC ver 2.00
      {
        cs = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
        *(LBA_t*)buff = cs << 10;
      }
      else                                      // SDC ver 1.XX or MMC
      {
        n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
        cs = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
        *(LBA_t*)buff = cs << (n - 9);
      }
      res = RES_OK;
    }
    break;

  case GET_BLOCK_SIZE :    // Get erase block size in unit of sector (DWORD)
    if (CardType & CT_SD2) // SDC ver 2.00
    {
      if (send_cmd(ACMD13, 0) == 0) // Read SD status
      {
        rx(&dummy, 1);
        if (rx_datablock(csd, 16)) // Read partial block
        {
          for (n = 64 - 16; n; n--)
            rx(&dummy, 1);  // Purge trailing data
          *(DWORD*)buff = 16UL << (csd[10] >> 4);
          res = RES_OK;
        }
      }
    }
    else // SDC ver 1.XX or MMC
    {
      if ((send_cmd(CMD9, 0) == 0) && rx_datablock(csd, 16)) // Read CSD
      {
        if (CardType & CT_SD1) // SDC ver 1.XX
        {
          *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
        }
        else // MMC
        {
          *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
        }
        res = RES_OK;
      }
    }

    break;

  case CTRL_TRIM :            // Erase a block of sectors (used when _USE_ERASE == 1)
    {
      LBA_t *dp;
      DWORD st, ed ;
      
      if (!(CardType & CT_SDC)) // Check if the card is SDC
        break;
      if (disk_ioctl(drv, MMC_GET_CSD, csd))   // Get CSD
        break;
      if (!(csd[0] >> 6) && !(csd[10] & 0x40)) // Check if sector erase can be applied to the card
        break;
      
      dp = (LBA_t*)buff;
      st = (DWORD)dp[0];
      ed = (DWORD)dp[1];                     // Load sector block
      if (!(CardType & CT_BLOCK))
      {
        st *= 512;
        ed *= 512;
      }
      
      if (send_cmd(CMD32, st) == 0 &&  // Erase sector block
          send_cmd(CMD33, ed) == 0 &&
          send_cmd(CMD38, 0) == 0 &&
          wait_ready(30000))
      {
        res = RES_OK;   // FatFs does not check result of this command */
      }
    }
    break;

  case MMC_GET_CSD:             // Receive CSD as a data block (16 bytes)
    if (send_cmd(CMD9, 0) == 0 && rx_datablock((BYTE*)buff, 16))       // READ_CSD
      res = RES_OK;
    deselect();
    break;

  default:
    res = RES_PARERR;
  }

  deselect();

  return res;
}

DWORD get_fattime (void)
{
  return // 2020-01-01 00:00:00
    ((2020 - 1980) << 25) | // year
    ((1          ) << 21) | // month
    ((1          ) << 15) | // day
    ((0          ) << 11) | // hour
    ((0          ) <<  5) | // minute
    ((0/2        ) <<  0) ; // second/2
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
