////////////////////////////////////////////////////////////////////////////////
// flash.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
  #include "gd32vf103.h"
}

#include "GD32VF103/spi.h"
#include "GD32VF103/gpio.h"
#include "GD32VF103/time.h"

#include "Longan/lcd.h"

using ::RV::GD32VF103::Spi ;
using ::RV::GD32VF103::Gpio ;
using ::RV::GD32VF103::TickTimer ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;

Gpio &button(Gpio::gpioA8()) ;
Lcd &lcd(Lcd::lcd()) ;

////////////////////////////////////////////////////////////////////////////////

class Flash
{
private:
  enum class Cmd
    {
     WriteEnable             = 0x06,
     VolatileSrWriteEnable   = 0x50,
     WriteDisable            = 0x04,
   
     ReleasePowerDownId      = 0xAB, // Dummy Dummy Dummy (ID7-ID0)
     ManufacturerDeviceId    = 0x90, // Dummy Dummy 00h   (MF7-MF0) (ID7-ID0)
     JedecId                 = 0x9F, // (MF7-MF0) (ID15-ID8) (ID7-ID0)
     UniqueId                = 0x4B, // Dummy Dummy Dummy Dummy (UID63-0)
   
     ReadData                = 0x03, // A23-A16 A15-A8 A7-A0 (D7-D0)
     FastRead                = 0x0B, // A23-A16 A15-A8 A7-A0 Dummy (D7-D0)
   
     PageProgram             = 0x02, // A23-A16 A15-A8 A7-A0 D7-D0 D7-D0
   
     SectorErase             = 0x20, // ( 4KB) A23-A16 A15-A8 A7-A0
     BlockErase32            = 0x52, // (32KB) A23-A16 A15-A8 A7-A0
     BlockErase64            = 0xD8, // (64KB) A23-A16 A15-A8 A7-A0
     ChipErase               = 0xC7, // 60h
   
     ReadStatusRegister1     = 0x05, // (S7-S0)
     WriteStatusRegister1    = 0x01, // (S7-S0)
     ReadStatusRegister2     = 0x35, // (S15-S8)
     WriteStatusRegister2    = 0x31, // (S15-S8)
     ReadStatusRegister3     = 0x15, // (S23-S16)
     WriteStatusRegister3    = 0x11, // (S23-S16)
   
     ReadSfdpRegister        = 0x5A, // 00h 00h A7–A0 Dummy (D7-D0)
     EraseSecurityRegister   = 0x44, // A23-A16 A15-A8 A7-A0
     ProgramSecurityRegister = 0x42, // A23-A16 A15-A8 A7-A0 D7-D0 D7-D0
     ReadSecurityRegister    = 0x48, // A23-A16 A15-A8 A7-A0 Dummy (D7-D0)

     GlobalBlockLock         = 0x7E,
     GlobalBlockUnlock       = 0x98,
     ReadBlockLock           = 0x3D, // A23-A16 A15-A8 A7-A0 (L7-L0)
     IndividualBlockLock     = 0x36, // A23-A16 A15-A8 A7-A0
     IndividualBlockUnlock   = 0x39, // A23-A16 A15-A8 A7-A0

     EraseProgramSuspend     = 0x75,
     EraseProgramResume      = 0x7A,
     PowerDown               = 0xB9,
   
     EnableReset             = 0x66,
     ResetDevice             = 0x99,
    } ;

public:
  Flash() : _spi(Spi::spi1()), _cs(Gpio::gpioB8())
  {
  }

  void setup()
  {
    _spi.setup(Spi::Psc::_2) ; // SPI1: 54MHz/2
    _cs.setup(Gpio::Mode::OUT_PP) ;
    _cs.high() ;
  }

  void getManufacturerDeviceId(uint8_t &mfid, uint8_t &did)
  {
    uint8_t dout[] = { 0x00, 0x00, 0x00 } ;
    uint8_t din[2] ;

    xch(Cmd::ManufacturerDeviceId, dout, sizeof(dout), din, sizeof(din)) ;

    mfid = din[0] ;
    did  = din[1] ;
  }

  void getJedecId(uint8_t &mfid, uint8_t &memoryType, uint8_t &capacity)
  {
    uint8_t din[3] ;

    xch(Cmd::JedecId, nullptr, 0, din, sizeof(din)) ;

    mfid = din[0] ;
    memoryType = din[1] ;
    capacity = din[2] ;
  }
  
  void getUniqueId(uint64_t &uid)
  {
    uint8_t dout[] = { 0x00, 0x00, 0x00, 0x00 } ;
    uint8_t din[8] ;

    xch(Cmd::UniqueId, dout, sizeof(dout), din, sizeof(din)) ;

    uid  = din[0] ; uid <<= 8 ; 
    uid |= din[1] ; uid <<= 8 ; 
    uid |= din[2] ; uid <<= 8 ; 
    uid |= din[3] ; uid <<= 8 ; 
    uid |= din[4] ; uid <<= 8 ; 
    uid |= din[5] ; uid <<= 8 ; 
    uid |= din[6] ; uid <<= 8 ; 
    uid |= din[7] ;
  }

  void read(uint32_t addr, uint8_t *data, size_t size)
  {
    uint8_t dout[3] ;
    uint8_t *o = (uint8_t*)&addr ;
    dout[0] = o[2] ;
    dout[1] = o[1] ;
    dout[2] = o[0] ;

    xch(Cmd::ReadData, dout, sizeof(dout), data, size) ;
  }

  void write(uint32_t addr, uint8_t *data, size_t size)
  {
    if ((size == 0) || (size > 256))
      return ;
    
    xch(Cmd::WriteEnable, nullptr, 0, nullptr, 0) ;

    uint8_t dout[3] ;
    uint8_t *o = (uint8_t*)&addr ;
    dout[0] = o[2] ;
    dout[1] = o[1] ;
    dout[2] = o[0] ;

    uint8_t cc = (uint8_t)Cmd::PageProgram ;
    _cs.low() ;
    _spi.xch(&cc, 1, 1) ;
    _spi.xch(dout, sizeof(dout), 1) ;
    _spi.xch(data, size, 1) ;
    _cs.high() ;
  }
  
  void erase(uint32_t addr)
  {
    xch(Cmd::WriteEnable, nullptr, 0, nullptr, 0) ;

    uint8_t dout[3] ;
    uint8_t *o = (uint8_t*)&addr ;
    dout[0] = o[2] ;
    dout[1] = o[1] ;
    dout[2] = o[0] ;

    xch(Cmd::SectorErase, dout, sizeof(dout), nullptr, 0) ;
  }
  
  void waitBusy()
  {
    uint8_t dout = (uint8_t) Cmd::ReadStatusRegister1 ;
    _cs.low() ;
    _spi.xch(&dout, sizeof(dout), 1) ;
    uint8_t status ;
    while (true)
    {
      _spi.xch(&status, sizeof(status), 2) ;
      if (!(status & 0x01))
        break ;
    }
    _cs.high() ;
  }
  
  void getStatus1(uint8_t &status)
  {
    xch(Cmd::ReadStatusRegister1, nullptr, 0, &status, sizeof(status)) ;
  }
  
  void getStatus2(uint8_t &status)
  {
    xch(Cmd::ReadStatusRegister2, nullptr, 0, &status, sizeof(status)) ;
  }
  
  void getStatus3(uint8_t &status)
  {
    xch(Cmd::ReadStatusRegister3, nullptr, 0, &status, sizeof(status)) ;
  }
  
  bool getSize(uint32_t &size)
  {
#pragma pack(push, 1)
    struct SfdpHeader
    {
      uint32_t _magic ;
      uint8_t _minor ;
      uint8_t _major ;
      uint8_t _count ; // n = _count+1
      uint8_t _unused ;
    } ;

    struct SfdpParameterHeader
    {
      uint8_t _id ;
      uint8_t _minor ;
      uint8_t _major ;
      uint8_t _size ; // in 32 bit
      uint32_t _offset ; // clear MSByte / only lower 24 Bits used
    } ;

    struct SfdpParameter0
    {
      uint32_t _1 ;
      uint32_t _flashMemoryDensity ;
      uint32_t _3 ;
      uint32_t _4 ;
      uint32_t _5 ;
      uint32_t _6 ;
      uint32_t _7 ;
      uint32_t _8 ;
      uint32_t _9 ;
    } ;
#pragma pack(pop)

    SfdpHeader sfdpHdr ;
    SfdpParameterHeader sfdpParamHdr ;
    SfdpParameter0 sfdpParam0 ;
    
    uint8_t dout[] = { 0x00, 0x00, 0x00, 0x00 } ;

    xch(Cmd::ReadSfdpRegister, dout, sizeof(dout), (uint8_t*)&sfdpHdr , sizeof(sfdpHdr)) ;

    if (sfdpHdr._magic != 'PDFS')
      return false ;

    uint32_t offset ;
    uint8_t *o = (uint8_t*)&offset ;

    offset = sizeof(SfdpHeader) ;
    dout[0] = o[2] ;
    dout[1] = o[1] ;
    dout[2] = o[0] ;

    xch(Cmd::ReadSfdpRegister, dout, sizeof(dout), (uint8_t*)&sfdpParamHdr , sizeof(sfdpParamHdr)) ;
    sfdpParamHdr._offset &= 0xffffff ;
    
    if (sfdpParamHdr._id != 0)
      return false ;
    if (sfdpParamHdr._size < 9)
      return false ;

    offset = sfdpParamHdr._offset ;
    
    dout[0] = o[2] ;
    dout[1] = o[1] ;
    dout[2] = o[0] ;

    xch(Cmd::ReadSfdpRegister, dout, sizeof(dout), (uint8_t*)&sfdpParam0 , sizeof(sfdpParam0)) ;
    
    size = sfdpParam0._flashMemoryDensity ;
    return true ;
  }

private:

  void xch(Cmd cmd, uint8_t *txData, size_t txSize, uint8_t *rxData, size_t rxSize)
  {
    uint8_t cc = (uint8_t)cmd ;
    _cs.low() ;
    _spi.xch(&cc, 1, 1) ;
    if (txSize)
      _spi.xch(txData, txSize, 1) ;
    if (rxSize)
      _spi.xch(rxData, rxSize, 2) ;
    _cs.high() ;
  }
  
private:
  Spi &_spi ;
  Gpio &_cs ;
} ;                 
  
////////////////////////////////////////////////////////////////////////////////

uint8_t buttonPressed()
{
  struct State
  {
    State() : _value{0x00}, _tick{TickTimer::now()} {}
    uint8_t  _value ;
    uint64_t _tick ;
  } ;
  static State last ;
  static State current ;

  uint64_t now = TickTimer::now() ;
  if ((now - current._tick) < TickTimer::usToTick(2500))
    return 0 ;

  current._tick = now ;
  current._value = (current._value << 1) | button.get() ;
  if ((current._value == last._value) ||
      ((current._value != 0x00) && (current._value != 0xff)))
    return 0 ;

  uint32_t ms = TickTimer::tickToMs(current._tick - last._tick) ;
  last = current ;
  if (current._value)
    return 0 ;
  return (ms < 600) ? 1 : 2 ;
}

////////////////////////////////////////////////////////////////////////////////
int main()
{
  Flash flash ;
  
  button.setup(Gpio::Mode::IN_FL) ;
  lcd.setup() ;
  flash.setup() ;

  lcd.clear() ;
  lcd.txtPos(0) ;
  lcd.put("Flash ") ;
  lcd.txtPos(4) ;
  lcd.put("press button to continue") ;

  LcdArea lcdWrk(lcd, 0, 160, 16, 48) ;

  while (true)
  {
    {
      uint8_t mfid ;
      uint8_t did ;
      flash.getManufacturerDeviceId(mfid, did) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Manufacturer ID: ") ; lcdWrk.put(mfid, 2, '0', true) ;
      lcdWrk.txtPos(1) ; lcdWrk.put("Device ID: ") ; lcdWrk.put(did, 2, '0', true) ;
    }

    while (!buttonPressed()) ;
  
    {
      uint8_t mfid ;
      uint8_t memoryType ;
      uint8_t capacity ;
      flash.getJedecId(mfid, memoryType, capacity) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Manufacturer ID: ") ; lcdWrk.put(mfid, 2, '0', true) ;
      lcdWrk.txtPos(1) ; lcdWrk.put("Memory Type: ") ; lcdWrk.put(memoryType, 2, '0', true) ;
      lcdWrk.txtPos(2) ; lcdWrk.put("Capacity: ") ; lcdWrk.put(capacity, 2, '0', true) ;
    }

    while (!buttonPressed()) ;

    {
      uint64_t uid ;
      flash.getUniqueId(uid) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Unique ID: ") ;
      lcdWrk.txtPos(1) ;
      lcdWrk.put((uint32_t)(uid>>32), 8, '0', true) ;
      lcdWrk.put((uint32_t)(uid>> 0), 8, '0', true) ;
    }

    while (!buttonPressed()) ;
  
    {
      uint32_t size ;
      uint64_t size64 ;
    
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Size: ") ;
      if (flash.getSize(size))
      {
        if (size & 0x80000000)
          size64 = 1LL << size ;
        else
          size64 = size + 1 ;

        size64 >>= 3 ; // bit to byte
        if (size64 > (1LL << 30))
        {
          size64 >>= 30 ;
          lcdWrk.put((uint32_t)size64) ; lcdWrk.put("GByte") ;
        }
        else if (size64 > (1LL << 20))
        {
          size64 >>= 20 ;
          lcdWrk.put((uint32_t)size64) ; lcdWrk.put("MByte") ;
        }
        else if (size64 > (1LL << 20))
        {
          size64 >>= 10 ;
          lcdWrk.put((uint32_t)size64) ; lcdWrk.put("kByte") ;
        }
        else
        {
          lcdWrk.put((uint32_t)size64) ; lcdWrk.put("Byte") ;
        }
      }
      else
        lcdWrk.put('?') ;
    }

    while (!buttonPressed()) ;

    {
      uint8_t status1 ;
      uint8_t status2 ;
      uint8_t status3 ;
      flash.getStatus1(status1) ;
      flash.getStatus2(status2) ;
      flash.getStatus3(status3) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Status1: ") ; lcdWrk.put(status1, 2, '0', true) ;
      lcdWrk.txtPos(1) ; lcdWrk.put("Status2: ") ; lcdWrk.put(status2, 2, '0', true) ;
      lcdWrk.txtPos(2) ; lcdWrk.put("Status3: ") ; lcdWrk.put(status3, 2, '0', true) ;
    }

    while (!buttonPressed()) ;

    {
      uint8_t data[16] ;
      flash.read(0x1000, data, sizeof(data)) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Read") ;
      lcdWrk.txtPos(1) ;
      for (uint8_t b : data)
      {
        lcdWrk.put(b, 2, '0', true) ;
        lcdWrk.put(' ') ;
      }
    }

    while (!buttonPressed()) ;

    {
      const char *txt = "Hallo Welt?!" ;
      flash.write(0x1000, (uint8_t*)txt, 11) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Write") ;
      lcdWrk.txtPos(1) ; lcdWrk.put("...") ;
      flash.waitBusy() ;
      lcdWrk.put(" done") ;
    }

    while (!buttonPressed()) ;
    
    {
      uint8_t data[16] ;
      flash.read(0x1000, data, sizeof(data)) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Read") ;
      lcdWrk.txtPos(1) ;
      for (uint8_t b : data)
      {
        lcdWrk.put(b, 2, '0', true) ;
        lcdWrk.put(' ') ;
      }
      lcdWrk.clearEOL() ;    
    }

    while (!buttonPressed()) ;

    {
      flash.erase(0x1000) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Erase") ;
      lcdWrk.txtPos(1) ; lcdWrk.put("...") ;
      flash.waitBusy() ;
      lcdWrk.put(" done") ;
    }

    while (!buttonPressed()) ;

    {
      uint8_t data[16] ;
      flash.read(0x1000, data, sizeof(data)) ;
      lcdWrk.clear() ;
      lcdWrk.txtPos(0) ; lcdWrk.put("Read") ;
      lcdWrk.txtPos(1) ;
      for (uint8_t b : data)
      {
        lcdWrk.put(b, 2, '0', true) ;
        lcdWrk.put(' ') ;
      }
      lcdWrk.clearEOL() ;    
    }

    while (!buttonPressed()) ;
  }
  
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
