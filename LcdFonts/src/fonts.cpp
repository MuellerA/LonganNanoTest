////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include <vector>

#include "GD32VF103/time.h"
#include "GD32VF103/gpio.h"
#include "Longan/lcd.h"
#include "Longan/fonts.h"
#include "Longan/led.h"

using ::RV::GD32VF103::TickTimer ;
using ::RV::GD32VF103::GpioIrq ;
using ::RV::Longan::Lcd ;
using ::RV::Longan::LcdArea ;
using ::RV::Longan::RgbLed ;

Lcd &lcd{Lcd::lcd()} ;
RgbLed &led{RgbLed::rgbLed()} ;

////////////////////////////////////////////////////////////////////////////////
// main
////////////////////////////////////////////////////////////////////////////////

int main()
{
  struct Font
  {
    const char *_name ;
    const GFXfont *_font ;
  } ;

  std::vector<Font> fontDir
    {
     { "Roboto_Light7", &::RV::Longan::Roboto_Light7pt7b },
     { "Roboto_Bold7", &::RV::Longan::Roboto_Bold7pt7b },
     //{ "RobotoMono_Light6", &::RV::Longan::RobotoMono_Light6pt7b },

     //{ "FreeMono9", &::RV::Longan::FreeMono9pt7b },
     //{ "FreeMono12", &::RV::Longan::FreeMono12pt7b },
     { "FreeMono18", &::RV::Longan::FreeMono18pt7b },
     //{ "FreeMono24", &::RV::Longan::FreeMono24pt7b },
     //{ "FreeMonoBold9", &::RV::Longan::FreeMonoBold9pt7b },
     //{ "FreeMonoBold12", &::RV::Longan::FreeMonoBold12pt7b },
     { "FreeMonoBold18", &::RV::Longan::FreeMonoBold18pt7b },
     //{ "FreeMonoBold24", &::RV::Longan::FreeMonoBold24pt7b },
     //{ "FreeMonoBoldOblique9", &::RV::Longan::FreeMonoBoldOblique9pt7b },
     //{ "FreeMonoBoldOblique12", &::RV::Longan::FreeMonoBoldOblique12pt7b },
     { "FreeMonoBoldOblique18", &::RV::Longan::FreeMonoBoldOblique18pt7b },
     //{ "FreeMonoBoldOblique24", &::RV::Longan::FreeMonoBoldOblique24pt7b },
     //{ "FreeMonoOblique9", &::RV::Longan::FreeMonoOblique9pt7b },
     //{ "FreeMonoOblique12", &::RV::Longan::FreeMonoOblique12pt7b },
     { "FreeMonoOblique18", &::RV::Longan::FreeMonoOblique18pt7b },
     //{ "FreeMonoOblique24", &::RV::Longan::FreeMonoOblique24pt7b },
     
     //{ "FreeSans9", &::RV::Longan::FreeSans9pt7b },
     //{ "FreeSans12", &::RV::Longan::FreeSans12pt7b },
     { "FreeSans18", &::RV::Longan::FreeSans18pt7b },
     //{ "FreeSans24", &::RV::Longan::FreeSans24pt7b },
     //{ "FreeSansBold9", &::RV::Longan::FreeSansBold9pt7b },
     //{ "FreeSansBold12", &::RV::Longan::FreeSansBold12pt7b },
     { "FreeSansBold18", &::RV::Longan::FreeSansBold18pt7b },
     //{ "FreeSansBold24", &::RV::Longan::FreeSansBold24pt7b },
     //{ "FreeSansBoldOblique9", &::RV::Longan::FreeSansBoldOblique9pt7b },
     //{ "FreeSansBoldOblique12", &::RV::Longan::FreeSansBoldOblique12pt7b },
     { "FreeSansBoldOblique18", &::RV::Longan::FreeSansBoldOblique18pt7b },
     //{ "FreeSansBoldOblique24", &::RV::Longan::FreeSansBoldOblique24pt7b },
     //{ "FreeSansOblique9", &::RV::Longan::FreeSansOblique9pt7b },
     //{ "FreeSansOblique12", &::RV::Longan::FreeSansOblique12pt7b },
     { "FreeSansOblique18", &::RV::Longan::FreeSansOblique18pt7b },
     //{ "FreeSansOblique24", &::RV::Longan::FreeSansOblique24pt7b },

     //{ "FreeSerif9", &::RV::Longan::FreeSerif9pt7b },
     //{ "FreeSerif12", &::RV::Longan::FreeSerif12pt7b },
     { "FreeSerif18", &::RV::Longan::FreeSerif18pt7b },
     //{ "FreeSerif24", &::RV::Longan::FreeSerif24pt7b },
     //{ "FreeSerifBold9", &::RV::Longan::FreeSerifBold9pt7b },
     //{ "FreeSerifBold12", &::RV::Longan::FreeSerifBold12pt7b },
     { "FreeSerifBold18", &::RV::Longan::FreeSerifBold18pt7b },
     //{ "FreeSerifBold24", &::RV::Longan::FreeSerifBold24pt7b },
     //{ "FreeSerifBoldItalic9", &::RV::Longan::FreeSerifBoldItalic9pt7b },
     //{ "FreeSerifBoldItalic12", &::RV::Longan::FreeSerifBoldItalic12pt7b },
     { "FreeSerifBoldItalic18", &::RV::Longan::FreeSerifBoldItalic18pt7b },
     //{ "FreeSerifBoldItalic24", &::RV::Longan::FreeSerifBoldItalic24pt7b },
     //{ "FreeSerifItalic9", &::RV::Longan::FreeSerifItalic9pt7b },
     //{ "FreeSerifItalic12", &::RV::Longan::FreeSerifItalic12pt7b },
     { "FreeSerifItalic18", &::RV::Longan::FreeSerifItalic18pt7b },
     //{ "FreeSerifItalic24", &::RV::Longan::FreeSerifItalic24pt7b },

     //{ "Org_01", &::RV::Longan::Org_01 },
     //{ "Picopixel", &::RV::Longan::Picopixel },
     //{ "Tiny3x3a2", &::RV::Longan::Tiny3x3a2pt7b },
     //{ "TomThumb", &::RV::Longan::TomThumb },
    } ;

  GpioIrq &button{GpioIrq::gpioA8()} ;
  
  lcd.setup() ;
  led.setup() ;
  button.setup(GpioIrq::Mode::IN_FL, GpioIrq::Handler{}) ;

  LcdArea laName{lcd, 0, 160,  0, 16, &::RV::Longan::Roboto_Bold7pt7b, 0x000080, 0xc0c0c0} ;
  LcdArea laFont{lcd, 0, 160, 20, 60} ;

  laName.clear() ;
  laName.put("Font Demo") ;
  laFont.put("press button to\nstart / pause / continue") ;
  led.blue() ;
  
  while (!button.pressed()) ;
  led.green() ;
  
  TickTimer t{1000} ;
  uint32_t fgCol{0} ;
  while (true)
  {
    if (!fgCol)
      fgCol = 0xff0000 ;
    laFont.color(fgCol, 0x000000) ;
    fgCol >>= 8 ;

    for (const Font &font : fontDir)
    {
      laName.clear() ;
      laName.put(font._name) ;

      laFont.clear() ;
      laFont.font(font._font) ;
      laFont.put("XyzO0", 80, 30) ;

      t.restart() ;
      while (!t())
      {
        if (button.pressed())
        {
          led.red() ;
          while (!button.pressed()) ;
          led.green() ;
          break ;
        }
      }
    }
  }

}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
