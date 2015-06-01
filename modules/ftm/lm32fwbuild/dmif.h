#ifndef _DMIF_H_
#define _DMIF_H_

#define MAX_THREADS     8

//***interface to thread - periphery pointers
#define BASEPTR 0x500
#define DEV_TIME        ( 0x00 )
#define DEV_FPQ         ( DEV_TIME        +4 )
#define DEV_FPQ_NOTFULL ( DEV_FPQ         +4 )
#define DEV_SHARED      ( DEV_FPQ_NOTFULL +4 )

//***interface to host
#define CMD_DM          ( DEV_SHARED      +4 )
#define CMD_THR_START   ( CMD_DM          +4 )
#define CMD_THR_STOP    ( CMD_THR_START   +4 )
#define CMD_THR_ABORT   ( CMD_THR_STOP    +4 )
#define CMD_THR_PAGE_A  ( CMD_THR_ABORT   +4 )
#define CMD_THR_PAGE_B  ( CMD_THR_PAGE_A  +4 )

#define REG_STAT        ( CMD_THR_PAGE_B  +4 )
#define REG_MSG_CNT     ( REG_STAT        +4 )
#define REG_THR_RUN     ( REG_MSG_CNT     +4 )
#define REG_THR_ERR     ( REG_THR_RUN     +4 )
#define REG_THR_A_B     ( REG_THR_ERR     +4 )
#define REG_T_PREP      ( REG_THR_A_B     +4 )
#define REG_T_TRN       ( REG_T_PREP      +4 )

#define REG_THR_PTRS    ( REG_T_TRN       +4 )

#define SHARED_IF_END_        ( REG_THR_PTRS + MAX_THREADS*2*4)

//misc
#define COST_BLB 10
#define COST_CON 20
#define COST_MSG 20
#define COST_SIG 10
#define COST_MSC 10

//command words
#define DM_CMD_RST  0x1
#define DM_CMD_ENA  0x2
#define DM_CMD_DIS  0x3
#define DM_CMD_HI   0x4

//***states
#define DM_STAT_IDLE    0x1
#define DM_STAT_ERRORS  0x2
#define DM_STAT_ABORTED 0x4
#define DM_STAT_RUNNING 0x8

#define DM_THR_IDLE      0x20
#define DM_THR_PAGESWAP  0x40
#define DM_THR_STOPPED   0x80

#endif
