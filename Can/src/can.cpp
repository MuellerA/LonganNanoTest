////////////////////////////////////////////////////////////////////////////////
// main.cpp
////////////////////////////////////////////////////////////////////////////////

extern "C"
{
#include "gd32vf103.h"
}

#include "GD32VF103/time.h"
#include "can.h"

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

void Can::setup(uint8_t remap0)
{
  rcu_periph_enum rcuGpio ;
  uint32_t gpio ;
  uint32_t pinRx ;
  uint32_t pinTx ;
  uint32_t remap ;

  switch (_can)
  {
  case CAN0:
    {
      switch (remap0)
      {
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
  can_parameter.prescaler             = 6 ; // 1Mbps
  //can_parameter.prescaler           = 48 ; // 125kbps
  
  can_init(_can, &can_parameter) ;

  can1_filter_start_bank(14) ;
  
  // input filter (accept all, use FIFO1)
  can_filter_parameter_struct can_filter ;
  can_struct_para_init(CAN_FILTER_STRUCT, &can_filter) ;

  can_filter.filter_number      = (_can == CAN0) ? 0 : 14 ;
  can_filter.filter_mode        = CAN_FILTERMODE_MASK ;
  can_filter.filter_bits        = CAN_FILTERBITS_32BIT ;
  can_filter.filter_list_high   = 0x0000 ;
  can_filter.filter_list_low    = 0x0000 ;
  can_filter.filter_mask_high   = 0x0000 ;
  can_filter.filter_mask_low    = 0x0000 ;
  can_filter.filter_fifo_number = CAN_FIFO1 ;
  can_filter.filter_enable      = ENABLE ;

  can_filter_init(&can_filter) ;
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
