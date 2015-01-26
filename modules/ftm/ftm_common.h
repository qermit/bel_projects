#ifndef _FTM_COMMON_H_
#define _FTM_COMMON_H_
#include <inttypes.h>
#include <stdint.h>

#define FTM_PROC_MAX        1
#define FTM_PLAN_MAX        2
#define FTM_PAGE_PAYLOAD    0x0600

#define FTM_PAGE_PLANS      0
#define FTM_PAGE_PBP        (FTM_PAGE_PLANS   + FTM_PLAN_MAX * 4)
#define FTM_PAGE_IDSTART    (FTM_PAGE_PBP     + 4)
#define FTM_PAGE_PCUR       (FTM_PAGE_IDSTART + 4)
#define FTM_PAGE_PLANQTY    (FTM_PAGE_PCUR + 4)
#define FTM_PAGE_DATA       (FTM_PAGE_PLANQTY + 4)
#define FTM_PAGE_END_       (FTM_PAGE_DATA + FTM_PAGE_PAYLOAD)

#define FTM_PAGESIZE        (FTM_PAGE_END_)

#define FTM_SHARED_OFFSET   0xC000

#define FTM_PROC_PAGES      0
#define FTM_PROC_MSGCNT     (FTM_PROC_PAGES + (2*FTM_PAGESIZE)) //     
#define FTM_PROC_PACT       (FTM_PROC_MSGCNT     + 4) //    
#define FTM_PROC_PINA       (FTM_PROC_PACT       + 4) //    
#define FTM_PROC_IDBP       (FTM_PROC_PINA       + 4) //    
#define FTM_PROC_TPREP      (FTM_PROC_IDBP       + 4) //
#define FTM_PROC_TPREP_LO    FTM_PROC_TPREP
#define FTM_PROC_TPREP_HI   (FTM_PROC_TPREP_LO   + 4)  
#define FTM_PROC_SCTR       (FTM_PROC_TPREP       + 8) //
#define FTM_PROC_DEBUG_DATA (FTM_PROC_SCTR       + 4) //
#define FTM_PROC_END_       (FTM_PROC_DEBUG_DATA + 8*32)  //

#define FTM_MIF_PROCS      0
#define FTM_MIF_CMD        (FTM_MIF_PROCS + FTM_PROC_MAX * FTM_PROC_END_) 
#define FTM_MIF_STAT_RUN   (FTM_MIF_CMD + 4)
#define FTM_MIF_STAT_IDLE  (FTM_MIF_STAT_RUN    + 4)
#define FTM_MIF_STAT_WAIT  (FTM_MIF_STAT_IDLE   + 4)
#define FTM_MIF_STAT_ERR   (FTM_MIF_STAT_WAIT   + 4)
#define FTM_MIF_PSHARED    (FTM_MIF_STAT_ERR    + 4)
#define FTM_MIF_PROCIDX    (FTM_MIF_PSHARED     + 4)
#define FTM_MIF_PROCQTY    (FTM_MIF_PROCIDX     + 4)
#define FTM_MIF_TDUE       (FTM_MIF_PROCQTY     + 4) //
#define FTM_MIF_TDUE_LO     FTM_MIF_TDUE
#define FTM_MIF_TDUE_HI    (FTM_MIF_TDUE_LO    + 4)    
#define FTM_MIF_TTRN       (FTM_MIF_TDUE       + 8) //
#define FTM_MIF_TTRN_LO     FTM_MIF_TTRN
#define FTM_MIF_TTRN_HI    (FTM_MIF_TTRN_LO    + 4)    
#define FTM_MIF_END_       (FTM_MIF_TTRN       + 8)

#define FTM_CON_SRC        0
#define FTM_CON_CMP        (FTM_CON_SRC    + 4)
#define FTM_CON_CMP_LO     FTM_CON_CMP
#define FTM_CON_CMP_HI     (FTM_CON_CMP_LO  + 4)
#define FTM_CON_MSK        (FTM_CON_CMP     + 8)
#define FTM_CON_MSK_LO     FTM_CON_MSK
#define FTM_CON_MSK_HI     (FTM_CON_MSK_LO  + 4)
#define FTM_CON_JMP        (FTM_CON_MSK     + 8)
#define FTM_CON_FLAGS      (FTM_CON_JMP     + 4)
#define FTM_CON_END_       (FTM_CON_FLAGS   + 4)

#define FTM_SIG_DST        0
#define FTM_SIG_VAL        (FTM_SIG_DST    + 4)
#define FTM_SIG_VAL_LO     FTM_SIG_VAL
#define FTM_SIG_VAL_HI     (FTM_SIG_VAL_LO  + 4)
#define FTM_SIG_FLAGS      (FTM_SIG_VAL     + 8)
#define FTM_SIG_END_       (FTM_SIG_FLAGS   + 4)


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

#define FTM_CH_ID       0
#define FTM_CH_TSTART   (FTM_CH_ID             + 4)
#define FTM_CH_TPERIOD  (FTM_CH_TSTART         + 8)
#define FTM_CH_TEXEC    (FTM_CH_TPERIOD        + 8)
#define FTM_CH_FLAGS    (FTM_CH_TEXEC          + 8)
#define FTM_CH_REPQTY   (FTM_CH_FLAGS          + 4)
#define FTM_CH_REPCNT   (FTM_CH_REPQTY         + 4)
#define FTM_CH_MSGQTY   (FTM_CH_REPCNT         + 4)
#define FTM_CH_MSGIDX   (FTM_CH_MSGQTY         + 2)
#define FTM_CH_CONQTY   (FTM_CH_MSGIDX         + 2)
#define FTM_CH_SIGQTY   (FTM_CH_CONQTY         + 2)
#define FTM_CH_PMSG     (FTM_CH_SIGQTY         + 2)
#define FTM_CH_PCON     (FTM_CH_PMSG           + 4)
#define FTM_CH_PSIG     (FTM_CH_PCON           + 4)
#define FTM_CH_PNEXT    (FTM_CH_PSIG           + 4)
#define FTM_CH_END_     (FTM_CH_PNEXT          + 4)

#define DBG_DISP_DUR_MIN   0
#define DBG_DISP_DUR_MAX   DBG_DISP_DUR_MIN+1
#define DBG_DISP_DUR_AVG   DBG_DISP_DUR_MAX+1


//masks & constants
#define CMD_RST               (1<<0)  //Reset FTM status and counters
#define CMD_START             (1<<1)  //Start FTM
#define CMD_STOP   	         (1<<2)  //Go Idle after completion
#define CMD_ABORT             (1<<4)  //Stop FTM immediately

#define CMD_COMMIT_PAGE       (1<<8)  //Commmit new data and validate
#define CMD_COMMIT_BP         (1<<9)  //Commit alt Plan pointer. Will be selected at next BP if not NULL
#define CMD_PAGE_SWAP         (1<<12)  //swap Page at next BP
#define CMD_SHOW_ACT          (1<<16)
#define CMD_SHOW_INA          (1<<17)
#define CMD_DBG_0             (1<<20)  //DBG case 0
#define CMD_DBG_1             (1<<21)  //DBG case 1
#define CMD_PROC_ID           0x0f000000  //process ID. if bit 28 is set, all
#define CMD_PROC_ID_ALL       (1<<28)  // all processes
#define CMD_PROC_ID_POS       24          
#define CMD_RES_0             (1<<29)  //DBG case 1
#define CMD_RES_1             (1<<30)  //DBG case 1
#define CMD_RES_2             (1<<31)  //DBG case 1




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


#define CH_FLAGBIT_BP           (1<<0)
#define CH_FLAGBIT_COND         (1<<3)
#define CH_FLAGBIT_SIG          (1<<12)
#define CH_FLAGBIT_START        (1<<20) 
#define CH_FLAGBIT_END          (1<<21) 
#define CH_FLAGBIT_ENDLOOP      (1<<22)
#define CH_FLAGBIT_PERS_REP_CNT (1<<23)

#define CON_FLAGBIT_SRC_IS_PTR  (1<<0)
#define CON_FLAGBIT_SRC_IS_ID   (1<<1)
#define CON_FLAGBIT_JMP_IS_PTR  (1<<2)
#define CON_FLAGBIT_JMP_IS_ID   (1<<3)
#define CON_FLAGBIT_JMP_TO_SRC  (1<<4)

#define CON_FLAGBIT_1ST         (1<<12)
#define CON_FLAGBIT_LAST        (1<<13)
#define CON_FLAGBIT_ONCE        (1<<14)
#define CON_FLAGBIT_DONE        (1<<15)

#define SIG_FLAGBIT_DST         (1<<0)
#define SIG_FLAGBIT_64          (1<<1)
#define SIG_FLAGBIT_1ST         (1<<4)
#define SIG_FLAGBIT_LAST        (1<<5)
#define SIG_FLAGBIT_ONCE        (1<<6)
#define SIG_FLAGBIT_DONE        (1<<8)




#define FTM_TIME_SIZE         8
#define FTM_DWORD_SIZE        8
#define FTM_WORD_SIZE         4
#define FTM_PTR_SIZE          4
#define FTM_NULL              0x0

#define CH_ID_CPU_POS      28
#define CH_ID_CPU_MSK      0xf0000000
#define CH_ID_PROC_POS     24
#define CH_ID_PROC_MSK     0x0f000000
#define CH_ID_PLAN_POS     20
#define CH_ID_PLAN_MSK     0x00f00000 
#define CH_ID_CH_POS       08    
#define CH_ID_CH_MSK       0x000fff00
#define CH_ID_TOKEN_POS    0    
#define CH_ID_TOKEN_MSK    0x000000ff

extern uint32_t*       _startshared[];
extern uint32_t*       _endshared[];


#endif
