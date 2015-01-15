#ifndef _FTM_COMMON_H_
#define _FTM_COMMON_H_
#include <inttypes.h>
#include <stdint.h>

#define FTM_PLAN_MAX        16
#define FTM_PAGEPAYLOAD     0x0600
#define FTM_PAGEMETA        (4 + 4 * FTM_PLAN_MAX + 4 + 4)   
#define FTM_PAGESIZE        (FTM_PAGEPAYLOAD + FTM_PAGEMETA)


#define FTM_PAGE_QTY        0
#define FTM_PAGE_PLANS      (FTM_PAGE_QTY    + 4)
#define FTM_PAGE_PBP        (FTM_PAGE_PLANS  + FTM_PLAN_MAX * 4)
#define FTM_PAGE_PSTART     (FTM_PAGE_PBP    + 4)
#define FTM_PAGE_DATA       (FTM_PAGE_PSTART + 4)
#define FTM_PAGE_END_       (FTM_PAGE_DATA + FTM_PAGE_PAYLOAD)


#define FTM_SHARED_OFFSET   0xC000

#define FTM_MIF_PAGES      0
#define FTM_MIF_CMD        (FTM_MIF_PAGES + (2*FTM_PAGESIZE))
#define FTM_MIF_STAT       (FTM_MIF_CMD        + 4) //
#define FTM_MIF_MSGCNT     (FTM_MIF_STAT       + 4) //     
#define FTM_MIF_PACT       (FTM_MIF_MSGCNT     + 4) //    
#define FTM_MIF_PINA       (FTM_MIF_PACT       + 4) //    
#define FTM_MIF_PBP        (FTM_MIF_PINA       + 4) //    
#define FTM_MIF_PSHARED    (FTM_MIF_PBP        + 4) //    
#define FTM_MIF_TPREP      (FTM_MIF_PSHARED    + 4) //
#define FTM_MIF_TPREP_LO    FTM_MIF_TPREP
#define FTM_MIF_TPREP_HI   (FTM_MIF_TPREP_LO   + 4)  
#define FTM_MIF_TDUE       (FTM_MIF_TPREP      + 8) //
#define FTM_MIF_TDUE_LO     FTM_MIF_TDUE
#define FTM_MIF_TDUE_HI    (FTM_MIF_TDUE_LO    + 4)    
#define FTM_MIF_TTRN       (FTM_MIF_TDUE       + 8) //
#define FTM_MIF_TTRN_LO     FTM_MIF_TTRN
#define FTM_MIF_TTRN_HI    (FTM_MIF_TTRN_LO    + 4)    
#define FTM_MIF_PIDLE      (FTM_MIF_TTRN       + 8) //
#define FTM_MIF_SCTR       (FTM_MIF_PIDLE      + 4) //
#define FTM_MIF_DEBUG_DATA (FTM_MIF_SCTR       + 4) //
#define FTM_MIF_END_       (FTM_MIF_DEBUG_DATA + 8*32) //


#define FTM_CON_PSRC       0
#define FTM_CON_CMP        (FTM_CON_PSRC    + 4)
#define FTM_CON_CMP_LO     FTM_CON_CMP
#define FTM_CON_CMP_HI     (FTM_CON_CMP_LO  + 4)
#define FTM_CON_MSK        (FTM_CON_CMP     + 8)
#define FTM_CON_MSK_LO     FTM_CON_MSK
#define FTM_CON_MSK_HI     (FTM_CON_MSK_LO  + 4)
#define FTM_CON_PJMP       (FTM_CON_MSK     + 8)
#define FTM_CON_PNEXT      (FTM_CON_PJMP    + 4)
#define FTM_CON_END_       (FTM_CON_PNEXT   + 4)

#define FTM_SIG_PDST       0
#define FTM_SIG_VAL        (FTM_SIG_PDST    + 4)
#define FTM_SIG_VAL_LO     FTM_SIG_VAL
#define FTM_SIG_VAL_HI     (FTM_SIG_VAL_LO  + 4)
#define FTM_SIG_PNEXT      (FTM_SIG_VAL     + 8)
#define FTM_SIG_END_       (FTM_SIG_PNEXT   + 4)


#define FTM_MSG_ID         0
#define FTM_MSG_ID_LO      FTM_MSG_ID
#define FTM_MSG_ID_HI      (FTM_MSG_ID    + 4)
#define FTM_MSG_PAR        (FTM_MSG_ID    + 8)
#define FTM_MSG_PAR_LO     FTM_MSG_PAR
#define FTM_MSG_PAR_HI     (FTM_MSG_PAR   + 4)
#define FTM_MSG_TEF        (FTM_MSG_PAR   + 8)
#define FTM_MSG_RES        (FTM_MSG_TEF   + 4)
#define FTM_MSG_TS         (FTM_MSG_RES   + 4)
#define FTM_MSG_TS_LO      FTM_MSG_TS
#define FTM_MSG_TS_HI      (FTM_MSG_TS    + 4)
#define FTM_MSG_OFFS       (FTM_MSG_TS    + 8)
#define FTM_MSG_END_       (FTM_MSG_OFFS  + 8)

#define FTM_CHAIN_ID       0
#define FTM_CHAIN_TSTART   (FTM_CHAIN_ID             + 4)
#define FTM_CHAIN_TPERIOD  (FTM_CHAIN_TSTART         + 8)
#define FTM_CHAIN_TEXEC    (FTM_CHAIN_TPERIOD        + 8)
#define FTM_CHAIN_FLAGS    (FTM_CHAIN_TEXEC          + 8)
#define FTM_CHAIN_REPQTY   (FTM_CHAIN_FLAGS          + 4)
#define FTM_CHAIN_REPCNT   (FTM_CHAIN_REPQTY         + 4)
#define FTM_CHAIN_MSGQTY   (FTM_CHAIN_REPCNT         + 4)
#define FTM_CHAIN_MSGIDX   (FTM_CHAIN_MSGQTY         + 4)
#define FTM_CHAIN_PMSG     (FTM_CHAIN_MSGIDX         + 4)
#define FTM_CHAIN_PCON     (FTM_CHAIN_PMSG           + 4)
#define FTM_CHAIN_PSIG     (FTM_CHAIN_PCON           + 4)
#define FTM_CHAIN_PNEXT    (FTM_CHAIN_PSIG           + 4)
#define FTM_CHAIN_END_     (FTM_CHAIN_PNEXT          + 4)

#define DBG_DISP_DUR_MIN   0
#define DBG_DISP_DUR_MAX   DBG_DISP_DUR_MIN+1
#define DBG_DISP_DUR_AVG   DBG_DISP_DUR_MAX+1


//masks & constants
#define CMD_RST               0x0001  //Reset FTM status and counters
#define CMD_START             0x0002  //Start FTM
#define CMD_IDLE   	      0x0004  //Jump into IDLE at next BP
#define CMD_STOP_REQ          0x0008  //Stop FTM if when it reaches IDLE state
#define CMD_STOP_NOW          0x0010  //Stop FTM immediately

#define CMD_COMMIT_PAGE       0x0100  //Commmit new data and validate
#define CMD_COMMIT_BP         0x0200  //Commit alt Plan pointer. Will be selected at next BP if not NULL
#define CMD_PAGE_SWAP         0x0400  //swap Page at next BP
#define CMD_SHOW_ACT          0x0800
#define CMD_SHOW_INA          0x1000
#define CMD_DBG_0             0x2000  //DBG case 0
#define CMD_DBG_1             0x4000  //DBG case 1

#define STAT_RUNNING          (1<<0)   //the FTM is running
#define STAT_IDLE             (1<<1)   //the FTM is idling  
#define STAT_STOP_REQ         (1<<2)   //alt ptr has been set to IDLE 
#define STAT_ERROR            (1<<3)   //FTM encountered an error, check error register
#define STAT_WAIT             (1<<4)   //FTM waiting on condition
#define STAT_STOPPED          (1<<5)   //FTM is Stopped - will not execute anything 


#define ID_MSK_B16            0xffff
#define ID_FID_LEN            4
#define ID_GID_LEN            12
#define ID_EVTNO_LEN          12
#define ID_SID_LEN            12
#define ID_BPID_LEN           14
#define ID_SCTR_LEN           10
#define ID_FID_POS            (ID_GID_LEN + ID_EVTNO_LEN + ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_GID_POS            (ID_EVTNO_LEN + ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_EVTNO_POS          (ID_SID_LEN + ID_BPID_LEN + ID_SCTR_LEN)
#define ID_SID_POS            (ID_BPID_LEN + ID_SCTR_LEN)
#define ID_BPID_POS           (ID_SCTR_LEN)
#define ID_SCTR_POS           0

#define FLAGS_IS_BP           (1<<0)

#define FLAGS_IS_COND         (1<<3)
#define FLAGS_IS_COND_CLR     (1<<4)
#define FLAGS_IS_COND_TIME    (1<<7)
#define FLAGS_IS_COND_ALL     (1<<8)

#define FLAGS_IS_SIG_MSI      (1<<12)
#define FLAGS_IS_SIG_SHARED   (1<<13)
#define FLAGS_IS_SIG_ADR      (1<<14)
#define FLAGS_IS_SIG_TIME     (1<<15)

#define FLAGS_IS_SIG_FIRST    (1<<16)
#define FLAGS_IS_SIG_LAST     (1<<17)
#define FLAGS_IS_SIG_ALL      (1<<18)
#define FLAGS_IS_SIG_64       (1<<19)

#define FLAGS_IS_START        (1<<20) 
#define FLAGS_IS_END          (1<<21) 
#define FLAGS_IS_ENDLOOP      (1<<22)
#define FLAGS_IS_PERS_REP_CNT (1<<23)

#define FLAGS_SEMA_CON        (1<<24)
#define FLAGS_SEMA_SIG        (1<<25)


#define FTM_TIME_SIZE         8
#define FTM_DWORD_SIZE        8
#define FTM_WORD_SIZE         4
#define FTM_PTR_SIZE          4
#define FTM_NULL              0x0




extern uint32_t*       _startshared[];
extern uint32_t*       _endshared[];


#endif
