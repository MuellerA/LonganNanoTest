////////////////////////////////////////////////////////////////////////////////
// can.h
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

class CanFilter
{
public:
  enum class Format
    {
     Standard,
     Extended,
    } ;
  enum class Type
    {
     Data,
     Remote,
     Any, // only for mask mode
    } ;
  enum class Fifo
    {
     Fifo0,
     Fifo1,
    } ;

  // extendend id, list
  CanFilter(Fifo fifo, uint32_t id1, Type type1, uint32_t id2, Type type2) ;
  // extendend id, mask
  CanFilter(Fifo fifo, uint32_t id, uint32_t mask, Type type = Type::Data) ;

  // standard id, list
  CanFilter(Fifo fifo, uint16_t id1, Type type1, uint16_t id2, Type type2, uint16_t id3, Type type3, uint16_t id4, Type type4) ;
  // standard id, mask
  CanFilter(Fifo fifo, uint16_t id1, uint16_t mask1, Type type1, uint16_t id2, uint16_t mask2, Type type2) ;

  uint32_t fXData0() const ;
  uint32_t fXData1() const ;
  bool mask() const ;
  bool extended() const ;
  bool fifo1() const ;
  
private:
  Fifo _fifo ;
  bool _mask ;
  Format _format ;
  uint32_t _id1 ;
  uint32_t _id2 ;
  uint32_t _id3 ;
  uint32_t _id4 ;
  Type _type1 ;
  Type _type2 ;
  Type _type3 ;
  Type _type4 ;
} ;

enum class CanBaud
  {
   _20k,
   _50k,
   _100k,
   _125k,
   _250k,
   _500k,
   _1M,
  } ;

class Can
{
 private:
  Can(uint32_t can, rcu_periph_enum rcu) ;
 public:
  static Can& can1() ;
  static Can& can0() ;

  void setup(CanBaud baud, const std::vector<CanFilter> /*max 14*/ &rxFilter, uint8_t remap = 0) ;

  bool tx(uint32_t id, bool extId, const uint8_t *data, uint32_t size) ;
  bool rx(uint32_t &id, bool &extId, uint8_t *data, uint32_t &size) ;

 private:
  uint32_t        _can ;
  rcu_periph_enum _rcu ;
} ;

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
