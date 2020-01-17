////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}
#include <stdio.h>

#include "GD32VF103/time.h"
#include "lcd.h"

using ::RV::GD32VF103::TickTimer ;

extern "C" const uint8_t font[1520] ;

Lcd &lcd{Lcd::lcd()} ;

extern "C" int _put_char(int ch) // used by printf
{
  lcd.put(ch) ;
  return ch ;
}

struct Frame
{
  uint16_t _pixel[80][80] ;
} ;

class Fixed
{
  //  1bit sign
  //  7bit int
  // 24bit frac
  static const int FRAC = 24 ;
  using FixedType = int32_t ;
  
public:
  explicit Fixed(int32_t v, int32_t div=1) ;
  Fixed() ;
  
  inline Fixed operator*(const Fixed &that) const ;
  inline Fixed operator+(const Fixed &that) const ;
  inline Fixed operator-(const Fixed &that) const ;
  inline Fixed operator*(int32_t i) const ;
  inline Fixed operator/(int32_t i) const ;
  inline bool operator>(const Fixed &that) const ;

  FixedType _v ;
} ;
Fixed::Fixed()
{
}
Fixed::Fixed(int32_t v, int32_t div) : _v{(v << FRAC) / div}
{
}
Fixed Fixed::operator*(const Fixed &that) const
{
  Fixed f ;
  f._v = (int32_t)(((int64_t)_v * (int64_t)that._v) >> FRAC) ;
  return f ;
}
Fixed Fixed::operator+(const Fixed &that) const
{
  Fixed f ;
  f._v = _v + that._v ;
  return f ;
}
Fixed Fixed::operator-(const Fixed &that) const 
{
  Fixed f ;
  f._v = _v - that._v ;
  return f ;
}
Fixed Fixed::operator*(int32_t i) const
{
  Fixed f ;
  f._v = _v * i ;
  return f ;
}
Fixed Fixed::operator/(int32_t i) const
{
  Fixed f ;
  f._v = _v / i ;
  return f ;
}
bool Fixed::operator>(const Fixed &that) const
{
  return _v > that._v ;
}


Frame frame1 ;
//Frame frame2 ;
Frame *frame = &frame1 ;

const Fixed    MaxValue{4} ;

uint32_t iterate(Fixed re0, Fixed im0, uint32_t maxIter) // holzhammer
{
  Fixed rere = re0 * re0 ;
  Fixed imim = im0 * im0 ;
  Fixed reim = re0 * im0 ;

  for (uint32_t iter = 0 ; iter < maxIter ; ++iter)
  {
    if ((rere + imim) > Fixed{4})
      return iter ;

    Fixed re = rere - imim + re0 ;
    Fixed im = reim + reim + im0 ;
    rere = re * re ;
    imim = im * im ;
    reim = re * im ;
  }

  return maxIter ;
}

uint16_t col(uint32_t iter, uint32_t maxIter)
{
  if (iter == maxIter)
    return 0x0000 ;

  uint16_t c = (iter % 0b111111) + 1 ;
  return
    ((c & 0b110000) << 10) |
    ((c & 0b001100) <<  7) |
    ((c & 0b000011) <<  3) ;
}


void apfel(Fixed reMin, Fixed reMax, Fixed imMin, Fixed imMax,
           uint32_t xCnt, uint32_t yCnt, uint32_t maxIter)
{
  Fixed reStep = (reMax - reMin) / xCnt ;
  Fixed imStep = (imMax - imMin) / yCnt ;
  
  for (uint32_t y = 0 ; y < yCnt ; ++y)
  {
    Fixed im = imMin + (imStep*y) ;

    for (uint32_t x = 0 ; x < xCnt ; ++x)
    {
      Fixed re = reMin + (reStep*x) ;

      uint32_t iter = iterate(re, im, maxIter) ;

      frame1._pixel[y][x] = col(iter, maxIter) ;
    }
  }
}

bool checkPixel(bool pixel0, int32_t i, int32_t j, uint32_t &x, uint32_t &y, Frame *frame)
{
  bool pixel = frame->_pixel[j][i] != 0 ;
  if (pixel == pixel0)
    return false ;

  x = (uint32_t)i ;
  y = (uint32_t)j ;

  return true ;
}

bool calcNextPoint(uint32_t &xU, uint32_t &yU, Frame *frame)
{
  bool pixel = frame->_pixel[yU][xU] != 0 ;

  int32_t i, j ;
  int32_t x = (int32_t)xU ;
  int32_t y = (int32_t)yU ;
  
  for (int32_t d = 1 ; d < 80 ; d += 2)
  {
    i = x - d ;
    if (i >= 0)
    {
      for (j = y - d ; j < y + d ; ++j)
      {
        if ((0 <= j) && (j <= 79) &&
            checkPixel(pixel, i, j, xU, yU, frame))
          return true ;
      }
    }
    j = y - d ;
    if (j >= 0)
    {
      for (i = x - d ; i < x + d ; ++i)
      {
        if ((0 <= i) && (i <= 79) &&
            checkPixel(pixel, i, j, xU, yU, frame))
          return true ;
      }
    }
    i = x + d ;
    if (i <= 79)
    {
      for (j = y + d ; j > y - d ; --j)
      {
        if ((0 <= j) && (j <= 79) &&
            checkPixel(pixel, i, j, xU, yU, frame))
          return true ;
      }
    }
    j = y + d ;
    if (j <= 79)
    {
      for (i = x + d ; i > x - d ; --i)
      {
        if ((0 <= i) && (79 <= i) &&
            checkPixel(pixel, i, j, xU, yU, frame))
          return true ;
      }
    }
  }
  return false ;
}

int main()
{
  lcd.setup(font, 16, 8) ;

  lcd.put("Spi Dma") ;
  
  Fixed dd{9, 10} ;

  TickTimer t(300, true) ;
  bool first = true ;

  while (true)
  {
    Fixed re{-5, 10} ;
    Fixed im{0} ;
    Fixed  d{1} ;

    uint32_t x, y, cnt ;
    uint32_t xy = TickTimer::now() % (80 * 80) ; // ultimate random generator
    x = xy / 80 ;
    y = xy % 80 ;
    
    for (cnt = 0 ; cnt < 48 ; ++cnt)
    {
      uint64_t t0 = TickTimer::now() ;
      apfel(re - d, re + d, im - d, im + d, 80, 80, 320) ;
      uint64_t t1 = TickTimer::now() ;

      if (!calcNextPoint(x, y, &frame1))
        break ;

      while (!first && !t()) ;
      first = false ;

      uint64_t t2 = TickTimer::now() ;
      lcd.copy(80, 159, 0, 79, &frame1._pixel[0][0]) ;
      uint64_t t3 = TickTimer::now() ;

      lcd.txtPos(2) ; printf("%5lu", TickTimer::tickToMs(t1-t0)) ; fflush(stdout) ;
      lcd.txtPos(3) ; printf("%5lu", TickTimer::tickToMs(t3-t2)) ; fflush(stdout) ;
      lcd.txtPos(4) ; printf("%5lu", cnt                       ) ; fflush(stdout) ;

      re = re - d + (d*2 * x / 80) ;
      im = im - d + (d*2 * y / 80) ;
      d = d * dd ;

      x = 80/2 ;
      y = 80/2 ;
    }

    if (cnt == 48)
      TickTimer::delayMs(2500) ;
  }
}  
  
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
