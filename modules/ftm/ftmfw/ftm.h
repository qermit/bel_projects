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

// Priority Queue RegisterLayout
static const struct {
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
} r_FPQ = {    .rst        =  0x00 >> 2,
               .force      =  0x04 >> 2,
               .dbgSet     =  0x08 >> 2,
               .dbgGet     =  0x0c >> 2,
               .clear      =  0x10 >> 2,
               .cfgGet     =  0x14 >> 2,
               .cfgSet     =  0x18 >> 2,
               .cfgClr     =  0x1C >> 2,
               .dstAdr     =  0x20 >> 2,
               .heapCnt    =  0x24 >> 2,
               .msgCntO    =  0x28 >> 2,
               .msgCntI    =  0x2C >> 2,
               .tTrnHi     =  0x30 >> 2,
               .tTrnLo     =  0x34 >> 2,
               .tDueHi     =  0x38 >> 2,
               .tDueLo     =  0x3C >> 2,
               .capacity   =  0x40 >> 2,
               .msgMax     =  0x44 >> 2,
               .ebmAdr     =  0x48 >> 2,
               .tsAdr      =  0x4C >> 2,
               .tsCh       =  0x50 >> 2,
               .cfg_ENA             = 1<<0,
               .cfg_FIFO            = 1<<1,    
               .cfg_IRQ             = 1<<2,
               .cfg_AUTOPOP         = 1<<3,
               .cfg_AUTOFLUSH_TIME  = 1<<4,
               .cfg_AUTOFLUSH_MSGS  = 1<<5,
               .cfg_MSG_ARR_TS      = 1<<6,
               .force_POP           = 1<<0,
               .force_FLUSH         = 1<<1
};

typedef struct {
   uint8_t sig;
   uint8_t cond;
} t_semaphore;  

typedef struct {
   uint8_t reserved [ FTM_PAGEDATA ];
} t_pageSpace;  

typedef uint64_t t_time ;


typedef struct {
   uint32_t value; 
   t_time   time;
} t_shared;

typedef struct {
   uint64_t id;
   uint64_t par;
   uint32_t tef;
   uint32_t res;
   t_time   ts;
   t_time   offs;
} t_ftmMsg;

typedef struct {
   
   t_time               tStart;  //desired start time of this chain
   t_time               tPeriod; //chain period
   t_time               tExec;   //chain execution time. if repQty > 0 or -1, this will be tStart + n*tPeriod FIXME
   uint32_t             flags; 
   uint32_t*            condSrc; //condition source adr
   uint32_t             condVal; //pattern to compare
   uint32_t             condMsk; //mask for comparison in condition
   uint32_t*            sigDst;  //dst adr for signalling
   uint32_t             sigVal;  //value for signalling
   uint32_t             repQty;  //number of desired repetitions. -1 -> infinite, 0 -> none
   uint32_t             repCnt;  //running count of repetitions
   uint32_t             msgQty;  //Number of messages
   uint32_t             msgIdx;  //idx of the currently processed msg
   t_ftmMsg*            pMsg;    //pointer to messages
   struct t_ftmChain*   pNext;   //pointer to next chain
   
} t_ftmChain;

//a plan is a linked list of chains
typedef struct {

   uint32_t       planQty;
   t_ftmChain*    plans[FTM_PLAN_MAX];
   t_ftmChain*    pBp;
   t_ftmChain*    pStart;
   t_pageSpace    pagedummy;
   
} t_ftmPage;

typedef struct {
   t_ftmPage   pPages[2];
   uint32_t    cmd;
   uint32_t    status;
   t_ftmPage*  pAct;
   t_ftmPage*  pIna;
   t_ftmChain* pNewBp;
   uint64_t    tPrep;
   uint64_t    tDue;
   uint64_t    tTrn;
   t_ftmChain  idle;
   t_shared*   pSharedMem;
   t_semaphore sema;
   uint16_t    sctr;
} t_ftmIf;

volatile t_ftmIf* pFtmIf;
t_ftmChain* pCurrentChain;

void              prioQueueInit();
void              ftmInit(void);
void              sigSend(t_ftmChain* c);
uint8_t           condValid(t_ftmChain* c);

void              processFtm();
t_ftmChain*       processChain(t_ftmChain* c);    //checks for condition and if chain is to be processed ( repQty != 0 )
t_ftmChain*       processChainAux(t_ftmChain* c); //does the actual work
int               dispatch(t_ftmMsg* pMsg);  //dispatch a message to prio queue
void              cmdEval();
void showFtmPage(t_ftmPage* pPage);
void showStatus();

extern unsigned int * pEcaAdr;
extern unsigned int * pEbmAdr;
extern unsigned int * pFPQctrl;

uint16_t    getIdFID(uint64_t id);
uint16_t    getIdGID(uint64_t id);
uint16_t    getIdEVTNO(uint64_t id);
uint16_t    getIdSID(uint64_t id);
uint16_t    getIdBPID(uint64_t id);
uint16_t    getIdSCTR(uint64_t id);
void incIdSCTR(uint64_t* id, volatile uint16_t* sctr); 
#endif
