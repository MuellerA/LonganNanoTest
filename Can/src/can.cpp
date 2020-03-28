////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "can.h"

////////////////////////////////////////////////////////////////////////////////

CanFilter::CanFilter(Fifo fifo, uint32_t id1, Type type1, uint32_t id2, Type type2)
  : _fifo{fifo}, _mask{false}, _format{CanFilter::Format::Extended}, _id1{id1}, _id2{id2}, _type1{type1}, _type2{type2}
{
}

CanFilter::CanFilter(Fifo fifo, uint32_t id, uint32_t mask, CanFilter::Type type)
  : _fifo{fifo}, _mask{true}, _format{CanFilter::Format::Extended}, _id1{id}, _id2{mask}, _type1{type}
{
}

CanFilter::CanFilter(Fifo fifo, uint16_t id1, Type type1, uint16_t id2, Type type2, uint16_t id3, Type type3, uint16_t id4, Type type4)
  : _fifo{fifo}, _mask{false}, _format{CanFilter::Format::Standard}, _id1{id1}, _id2{id2}, _id3{id3}, _id4{id4}, _type1{type1}, _type2{type2}, _type3{type3}, _type4{type4}
{
}

CanFilter::CanFilter(Fifo fifo, uint16_t id1, uint16_t mask1, CanFilter::Type type1, uint16_t id2, uint16_t mask2, CanFilter::Type type2)
  : _fifo{fifo}, _mask{true}, _format{CanFilter::Format::Standard}, _id1{id1}, _id2{mask1}, _id3{id2}, _id4{mask2}, _type1{type1}, _type3{type2}
{
}

uint32_t frameType(bool mask, CanFilter::Type type)
{
  if (mask)
  {
    switch (type)
    {
    case CanFilter::Type::Data  : return 1 ;
    case CanFilter::Type::Remote: return 1 ;
    case CanFilter::Type::Any   : return 0 ;
    }
  }
  else
  {
    switch (type)
    {
    case CanFilter::Type::Data  : return 0 ;
    case CanFilter::Type::Remote: return 1 ;
    case CanFilter::Type::Any   : return 0 ; // invalid: any && !mask
    }
  }

  return 0 ;
}

uint32_t CanFilter::fXData0() const
{
  uint32_t fData{0} ;
  const uint32_t stdIdMask{0x7ff} ;

  switch (_format)
  {
  case CanFilter::Format::Extended:
    fData |= _id1 << 3 ;
    fData |= 1 << 2 ; // FF extended
    fData |= frameType(false, _type1) << 1 ;
    break ;
  case CanFilter::Format::Standard:
    // id1
    fData |= (_id1 & stdIdMask) << (5 + 0) ;
    fData |= 0 << (3 + 0) ; // FF standard
    fData |= frameType(false, _type1) << (4 + 0) ;
    // id2
    fData |= (_id2 & stdIdMask) << (5 + 16) ;
    if (_mask)
    {
      fData |= 1 << (3 + 16) ; // FF standard
      fData |= frameType(true, _type1) << (4 + 16) ;
    }
    else
    {
      fData |= 0 << (3 + 16) ; // FF standard
      fData |= frameType(false, _type2) << (4 + 16) ;
    }
    break ;
  }

  return fData ;
}

uint32_t CanFilter::fXData1() const
{
  uint32_t fData{0} ;
  const uint32_t stdIdMask{0x7ff} ;

  switch (_format)
  {
  case CanFilter::Format::Extended:
    fData |= _id2 << 3 ;
    fData |= 1 << 2 ; // FF extended
    fData |= frameType(_mask, _mask ? _type1 : _type2) ;
    break ;
  case CanFilter::Format::Standard:
    // id3
    fData |= (_id3 & stdIdMask) << (5 + 0) ;
    fData |= 0 << (3 + 0) ; // FF standard
    fData |= frameType(false, _type3) << (4 + 0) ;
    // id4
    fData |= (_id4 & stdIdMask) << (5 + 16) ;
    if (_mask)
    {
      fData |= 1 << (3 + 16) ; // FF standard
      fData |= frameType(true, _type3) << (4 + 16) ;
    }
    else
    {
      fData |= 0 << (3 + 16) ; // FF standard
      fData |= frameType(false, _type4) << (4 + 16) ;
    }
    break ;
  }

  return fData ;
}

bool CanFilter::mask() const
{
  return _mask ;
}

bool CanFilter::extended() const
{
  return _format == CanFilter::Format::Extended ;
}

bool CanFilter::fifo1() const
{
  return _fifo == CanFilter::Fifo::Fifo1 ;
}

////////////////////////////////////////////////////////////////////////////////

Can::Can(uint32_t can, rcu_periph_enum rcu)
  : _can{can}, _rcu{rcu}
{
}

Can& Can::can1()
{
  Can *can = new Can(CAN1, RCU_CAN1) ;
  return *can ;
}

Can& Can::can0()
{
  Can *can = new Can(CAN0, RCU_CAN0) ;
  return *can ;
}

void Can::setup(CanBaud baud, const std::vector<CanFilter> /*max 14*/ &rxFilter, uint8_t remap0)
{
  rcu_periph_enum rcuGpio ;
  uint32_t gpio ;
  uint32_t pinRx ;
  uint32_t pinTx ;
  uint32_t remap ;

  switch (_can)
  {
  default:
  case CAN0:
    {
      switch (remap0)
      {
      default:
      case 0: rcuGpio = RCU_GPIOA ; gpio = GPIOA ; pinRx = GPIO_PIN_11 ; pinTx = GPIO_PIN_12 ; remap = 0                       ; break ;
      case 1: rcuGpio = RCU_GPIOB ; gpio = GPIOB ; pinRx = GPIO_PIN_8  ; pinTx = GPIO_PIN_9  ; remap = GPIO_CAN0_PARTIAL_REMAP ; break ;
      case 2: rcuGpio = RCU_GPIOD ; gpio = GPIOD ; pinRx = GPIO_PIN_0  ; pinTx = GPIO_PIN_1  ; remap = GPIO_CAN0_FULL_REMAP    ; break ;
      }
    }
    break ;
  case CAN1:
    rcu_periph_clock_enable(RCU_CAN0) ; // rx-filter associated with CAN0
    {
      switch (remap0)
      {
      default:
      case 0: rcuGpio = RCU_GPIOB ; gpio = GPIOB ; pinRx = GPIO_PIN_12 ; pinTx = GPIO_PIN_13 ; remap = 0                       ; break ;
      case 1: rcuGpio = RCU_GPIOB ; gpio = GPIOB ; pinRx = GPIO_PIN_5  ; pinTx = GPIO_PIN_6  ; remap = GPIO_CAN1_REMAP         ; break ;
      }
    }
    break ;
  }

  rcu_periph_clock_enable(_rcu) ;

  rcu_periph_clock_enable(rcuGpio) ;
  rcu_periph_clock_enable(RCU_AF) ;
  gpio_init(gpio, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, pinRx) ;
  gpio_init(gpio, GPIO_MODE_AF_PP      , GPIO_OSPEED_50MHZ, pinTx) ;

  if (remap)
    gpio_pin_remap_config(remap, ENABLE) ;

  can_deinit(_can) ;

  uint32_t prescaler = 48 ;
  switch (baud)
  {
  case CanBaud::_20k : prescaler = 300 ; break ;
  case CanBaud::_50k : prescaler = 120 ; break ;
  case CanBaud::_100k: prescaler =  60 ; break ;
  case CanBaud::_125k: prescaler =  48 ; break ;
  case CanBaud::_250k: prescaler =  24 ; break ;
  case CanBaud::_500k: prescaler =  12 ; break ;
  case CanBaud::_1M  : prescaler =   6 ; break ;
  }

  // initialize CAN
  can_parameter_struct can_parameter ;
  can_struct_para_init(CAN_INIT_STRUCT, &can_parameter) ;

  can_parameter.time_triggered        = DISABLE ;
  can_parameter.auto_bus_off_recovery = ENABLE ;
  can_parameter.auto_wake_up          = ENABLE ;
  can_parameter.auto_retrans          = ENABLE ;
  can_parameter.rec_fifo_overwrite    = ENABLE ;
  can_parameter.trans_fifo_order      = DISABLE ;
  can_parameter.working_mode          = CAN_NORMAL_MODE ;
  can_parameter.resync_jump_width     = CAN_BT_SJW_1TQ ;
  can_parameter.time_segment_1        = CAN_BT_BS1_5TQ ;
  can_parameter.time_segment_2        = CAN_BT_BS2_3TQ ;
  can_parameter.prescaler             = prescaler ;

  can_init(_can, &can_parameter) ;

  can1_filter_start_bank(14) ;

  uint32_t filterNumber{(_can == CAN0) ? 0U : 14U} ;
  uint32_t filterNumberEnd{filterNumber + 14U} ;

  uint8_t  *can0   {(uint8_t*)0x40006400} ;
  uint32_t *fctl   {(uint32_t*)(can0 + 0x200)} ;
  uint32_t *fmcfg  {(uint32_t*)(can0 + 0x204)} ;
  uint32_t *fscfg  {(uint32_t*)(can0 + 0x20c)} ;
  uint32_t *fafifo {(uint32_t*)(can0 + 0x214)} ;  
  uint32_t *fw     {(uint32_t*)(can0 + 0x21c)} ;

  *fctl |= 0x01UL ;
  for (const CanFilter &filter : rxFilter)
  {
    if (filterNumber == filterNumberEnd)
      break ;

    uint32_t filterMask = 1 << filterNumber++ ;
      
    uint32_t *fxdata0{(uint32_t*)(can0 + 0x240 + 8*14 + 4*0)} ;
    uint32_t *fxdata1{(uint32_t*)(can0 + 0x240 + 8*14 + 4*1)} ;

    if (filter.mask())
      *fmcfg &= ~filterMask ;
    else
      *fmcfg |= filterMask ;

    if (filter.extended())
      *fscfg |= filterMask ;
    else
      *fscfg &= ~filterMask ;

    if (filter.fifo1())
      *fafifo |= filterMask ;
    else
      *fafifo &= ~filterMask ;

    *fw |= filterMask ;

    *fxdata0 = filter.fXData0() ;
    *fxdata1 = filter.fXData1() ;
  }
  *fctl &= ~0x01UL ;
}

bool Can::tx(uint32_t id, bool extId, const uint8_t *data, uint32_t size)
{
  if (size > 8)
    return false ;

  can_trasnmit_message_struct transmit_message ;

  can_struct_para_init(CAN_TX_MESSAGE_STRUCT, &transmit_message) ;
  if (extId)
  {
  transmit_message.tx_efid    = id ;
  transmit_message.tx_ff      = CAN_FF_EXTENDED ;
  }
  else
  {
  transmit_message.tx_sfid    = id ;
  transmit_message.tx_ff      = CAN_FF_STANDARD ;
  }

  transmit_message.tx_ft      = CAN_FT_DATA ;
  transmit_message.tx_dlen    = size ;
  for (uint8_t i = 0 ; i < size ; ++i)
    transmit_message.tx_data[i] = data[i] ;

  uint8_t mailbox = CAN_NOMAILBOX ;
  while (mailbox == CAN_NOMAILBOX)
    mailbox = can_message_transmit(_can, &transmit_message) ;

  return true ;
}

bool Can::rx(uint32_t &id, bool &extId, uint8_t *data, uint32_t &size)
{
  can_receive_message_struct receive_message ;

  if (can_receive_message_length_get(_can, CAN_FIFO1) == 0)
    return false ;

  can_struct_para_init(CAN_RX_MESSAGE_STRUCT, &receive_message) ;

  can_message_receive(_can, CAN_FIFO1, &receive_message) ;
  if (receive_message.rx_ff == CAN_FF_EXTENDED)
  {
    id = receive_message.rx_efid ;
    extId = true ;
  }
  else
  {
    id = receive_message.rx_sfid ;
    extId = false ;
  }
  size = receive_message.rx_dlen ;
  for (uint8_t i = 0 ; i < size ; ++i)
    data[i] = receive_message.rx_data[i] ;

  return true ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
