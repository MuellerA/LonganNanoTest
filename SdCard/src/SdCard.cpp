////////////////////////////////////////////////////////////////////////////////
// SdCard.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "Longan/lcd.h"

using ::RV::Longan::Lcd ;

extern void diskioSetup() ;

Lcd &lcd{Lcd::lcd()} ;

#include "ff.h"

void Debug(char c, uint8_t v)
{
  //lcd.put(c) ;
  //lcd.put(v) ;
  //lcd.put(' ') ;
}

int main()
{
  FATFS fatFs;
  FIL f ;
  uint8_t buff[1024] ;
  UINT len ;
  FRESULT res ;

  diskioSetup() ;
  
  lcd.setup() ;
  lcd.clear() ;
  lcd.put("SD Card") ;
  lcd.txtPos(1) ;
  
  res = f_mount(&fatFs, "", 0) ;              lcd.put((uint8_t)res) ; lcd.put(' ') ;
  res = f_open(&f, "/hallo.txt", FA_READ) ;   lcd.put((uint8_t)res) ; lcd.put(' ') ;
  res = f_read(&f, buff, sizeof(buff), &len); lcd.put((uint8_t)res) ; lcd.put(' ') ;
  res = f_close(&f) ;                         lcd.put((uint8_t)res) ; lcd.put(' ') ;

  lcd.put((char*)buff, len) ;  

  if (f_open(&f, "/hallo.neu", FA_READ) == FR_OK)
  {
    f_read(&f, buff, sizeof(buff), &len);
    f_close(&f) ;

    lcd.put((char*)buff, len) ;  
  }

  f_open(&f, "/hallo.neu", FA_CREATE_ALWAYS | FA_WRITE) ;
  f_write(&f, "alles neu!", 10, &len) ;
  f_close(&f) ;

  {
    size_t size = 3000 ;
    
    std::vector<uint8_t> data(size, 'a') ;
    f_open(&f, "/hallo.data", FA_CREATE_ALWAYS | FA_WRITE) ;
    f_write(&f, data.data(), data.size(), &len) ;
    f_close(&f) ;

    std::vector<uint8_t> data2(size, 'b') ;
    f_open(&f, "/hallo.data", FA_READ) ;
    f_read(&f, data2.data(), data2.size(), &len) ;
    f_close(&f) ;

    lcd.put((data == data2) ? '=' : '!') ;
  }
  
  while (true) ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
