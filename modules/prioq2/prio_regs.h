/** @file        prio_regs.h
  * DesignUnit   prio
  * @author      M. Kreider <m.kreider@gsi.de>
  * @date        16/11/2015
  * @version     0.0.1
  * @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
  *
  * @brief       Register map for Wishbone interface of VHDL entity <prio_auto>
  */

#ifndef _PRIO_H_
#define _PRIO_H_

   #define CTRL_RESET_OWR           0x000 //wo,          1 b, Resets the Priority Queue
   #define CTRL_MODE_GET            0x004 //ro,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   #define CTRL_MODE_CLR            0x008 //wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   #define CTRL_MODE_SET            0x00c //wo,          3 b, b2: Time limit, b1: Msg limit, b0 enable
   #define CTRL_CLEAR_OWR           0x010 //wo,          1 b, Clears counters and status
   #define CTRL_ST_FULL_GET         0x014 //ro, g_channels b, Channel Full flag (n..0) 
   #define CTRL_ST_LATE_GET         0x018 //ro,          1 b, Late message detected
   #define CTRL_EBM_ADR_RW          0x01c //rw,         32 b, Etherbone Master address
   #define CTRL_ECA_ADR_RW          0x020 //rw,         32 b, Event Condition Action Unit address
   #define CTRL_TX_MAX_MSGS_RW      0x024 //rw,          8 b, Max msgs per packet
   #define CTRL_TX_MAX_WAIT_RW      0x028 //rw,         32 b, Max wait time for non empty packet
   #define CTRL_TX_RATE_LIMIT_RW    0x02c //rw,         32 b, Max msgs per milliseconds
   #define CTRL_OFFS_LATE_RW_0      0x030 //rw,         32 b, Time offset before message is late
   #define CTRL_OFFS_LATE_RW_1      0x034 //rw,         32 b, Time offset before message is late
   #define CTRL_CNT_LATE_GET        0x038 //ro,         32 b, Sum of all late messages
   #define CTRL_TS_LATE_GET_0       0x03c //ro,         32 b, First late Timestamp
   #define CTRL_TS_LATE_GET_1       0x040 //ro,         32 b, First late Timestamp
   #define CTRL_CNT_OUT_ALL_GET_0   0x044 //ro,         32 b, Sum of all outgoing messages
   #define CTRL_CNT_OUT_ALL_GET_1   0x048 //ro,         32 b, Sum of all outgoing messages
   #define CTRL_CH_SEL_RW           0x100 //rw,          4 b, Channel select
   #define CTRL_CNT_OUT_GET         0x104 //ro,         32 b, Outgoing messages per Channel
   #define CTRL_CNT_IN_GET          0x108 //ro,         32 b, Incoming messages per Channel

#endif
