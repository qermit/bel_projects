#ifndef _FTM_FW_H_
#define _FTM_FW_H_
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "dbg.h"
#include "mini_sdb.h"
#include "irq.h"
#include "aux.h"
#include "../ftm_common.h"

uint32_t cmdCnt;

#define MSI_SIG             0
#define TLU_CH_TEST         0x60

struct t_FPQ {
   uint32_t rst;
   uint32_t force;
   uint32_t dbgSet;
   uint32_t dbgGet;
   uint32_t clear;
   uint32_t cfgGet;
   uint32_t cfgSet;
   uint32_t cfgClr;
   uint32_t dstAdr;
   uint32_t heapCnt;
   uint32_t msgCntO;
   uint32_t msgCntI;
   uint32_t tTrnHi;
   uint32_t tTrnLo;
   uint32_t tDueHi;
   uint32_t tDueLo;
   uint32_t capacity;
   uint32_t msgMax;
   uint32_t ebmAdr;
   uint32_t tsAdr;
   uint32_t tsCh;
   uint32_t cfg_ENA;
   uint32_t cfg_FIFO;    
   uint32_t cfg_IRQ;
   uint32_t cfg_AUTOPOP;
   uint32_t cfg_AUTOFLUSH_TIME;
   uint32_t cfg_AUTOFLUSH_MSGS;
   uint32_t cfg_MSG_ARR_TS;
   uint32_t force_POP;
   uint32_t force_FLUSH;
};

extern const struct t_FPQ r_FPQ;
extern const uint8_t Idle[FTM_CHAIN_END_];
extern const uint8_t IdleCon[FTM_CON_END_];

uint32_t prioQcapacity;


volatile uint8_t* pFtmIf;
uint8_t* pCurrentChain;


void              prioQueueInit();
void              ftmInit(void);
void              processFtm();

void              cmdEval();
//void showFtmPage(t_ftmPage* pPage);
//void showStatus();

extern uint32_t * pEcaAdr;
extern uint32_t * pEbmAdr;
extern uint32_t * pFPQctrl;
extern volatile uint32_t * pSharedRam;

inline uint64_t setEvtId()     {return ();}
inline uint32_t setChId(uint32_t proc, uint32_t plan, uint32_t chain, uint32_t token) {
   return ( (uint32_t)  ((uint64_t)proc << CHAIN_ID_PROC_POS)     |
                        ((uint64_t)plan << CHAIN_ID_PLAN_POS)     |
                        ((uint64_t)chain << CHAIN_ID_CHAIN_POS)   |
                        ((uint64_t)token << CHAIN_ID_TOKEN_POS));}

inline uint32_t getBitSlice(uint32_t val, uint32_t msk, uint8_t pos) {return (val & msk)>>pos);}


inline void incIdSCTR(uint64_t* id, volatile uint16_t* sctr)   {*id = ( *id & 0xfffffffffffffc00) | *sctr; *sctr = (*sctr+1) & ~0xfc00; DBPRINT3("id: %x sctr: %x\n", *id, *sctr);}
inline uint32_t hiW(uint64_t dword) {return (uint32_t)(dword >> 32);}
inline uint32_t loW(uint64_t dword) {return (uint32_t)dword;}

#endif
