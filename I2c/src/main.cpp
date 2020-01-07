////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "Longan/lcd.h"

#include "lcd1602i2c.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::Longan::Lcd ;

extern "C" const uint8_t font[1520] ;

////////////////////////////////////////////////////////////////////////////////

const static uint8_t I2Caddr_PCF8574  = 0x27 ;
const static uint8_t I2Caddr_PCF8574A = 0x3f ;
const static uint8_t I2Caddr_LCD = I2Caddr_PCF8574 ;

Lcd &lcd{Lcd::lcd()} ;
I2c &i2c{I2c::i2c0()} ;
Lcd1602I2c &lcd1602{Lcd1602I2c::lcd1602I2c(i2c)} ;

////////////////////////////////////////////////////////////////////////////////

int main()
{
  i2c.setup(42, 400000U) ;
  lcd.setup(font, 16, 8) ;
  lcd1602.setup(I2Caddr_LCD) ;

  lcd.put("Hallo I2C!") ;
  lcd.heartbeat() ;

  const char *zeilen[] =
    {
     "", ""
     "Im Anfang war", "das Wort, und", "das Wort war bei", "Gott, und das", "Wort war Gott.", "",
     "Im Anfang war es", "bei Gott.", "",
     "Alles ist durch", "das Wort", "geworden und", "ohne das Wort", "wurde nichts,", "was geworden", "ist.", "",
     "In ihm war das", "Leben und das", "Leben war das", "Licht der", "Menschen.", "",
     "Und das Licht", "leuchtet in der", "Finsternis und", "die Finsternis", "hat es nicht", "erfasst.", "",
     nullptr
    } ;
  const char **zeile0 = zeilen+0 ;
  const char **zeile1 = zeilen+1 ;

  TickTimer tUpdate1602{880, true} ;
  while (true)
  {
    lcd.heartbeat() ;

    if (tUpdate1602())
    {
      lcd1602.clearDisplay() ;
      
      lcd1602.writeData(0, 0, *zeile0) ;
      lcd1602.writeData(1, 0, *zeile1) ;
      zeile0 = zeile1++ ;
      if (!*zeile1)
        zeile1 = zeilen ;
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
