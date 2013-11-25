#ifndef _FTM_H_
#define _FTM_H_

#include "ebm.h"

//masks & constants
#define MSK_PAGE              (1<<0)

#define CMD_RST           		(1<<0)	//Reset FTM status and counters
#define CMD_PAGESWAP      		(1<<1)	//Use mempage A/B

#define CMD_CYC_START_ABS     (1<<4)	//Start timing cycle at time specified
#define CMD_CYC_START_REL     (1<<5)	//Start timing cycle at now + time specifed
#define CMD_CYC_DBG           (1<<6)	//Run Cycle in  debug mode (start time will be corrected if in the past, no error detection)
#define CMD_CYC_STOP          (1<<7)	//Stop timing msg program safely
#define CMD_CYC_STOP_I        (1<<8)	//Stop timing msg program immediately

#define STAT_CYC_WAITING      (1<<0)	//shows if cycle is waiting for condition
#define STAT_CYC_DBG          (1<<1)	//shows cycle debug mode is active/inactive
#define STAT_CYC_ACTIVE       (1<<2)	//shows cycle is active/inactive
#define STAT_CYC_MEMPAGE_B    (1<<3)	//using mem page A when 0, B when 1
#define STAT_CYC_ERROR      	(1<<4)	//error occured during cycle execution

//control & status registers
typedef struct {
   unsigned int hi;
   unsigned int lo;
} t_dw;


typedef union {
   unsigned long long v64;
                 t_dw v32;               
} u_dword;

typedef struct {
   u_dword id;
   unsigned int res;
   unsigned int tef;
   u_dword par;
   u_dword ts;
} t_ftmMsg;

typedef struct {
   unsigned long long t_trn;
   unsigned long long t_margin;
   unsigned long long t_exec;
   unsigned long long t_period;
   unsigned int       rep;
} t_ftmCycle;


typedef struct {
   unsigned int*  pThisPage;
   unsigned int   msgChStat;
   unsigned int   cycCnt;
   unsigned int   msgCnt;
   unsigned int   msgChInst;
   t_ftmCycle     cycle;
   t_ftmMsg       msgs[10];
} t_fesaPage;

typedef struct {
   unsigned int cmd;
   unsigned int pageAct; 
   t_fesaPage page[2];
} t_fesaFtmIf;

extern unsigned int* _startshared[];
extern unsigned int* _endshared[];
volatile t_fesaFtmIf* fesaFtmIf = (t_fesaFtmIf*)_startshared; 


t_ftmMsg* addFtmMsg(unsigned int* eca_adr, t_ftmMsg* pMsg);

void sendFtmMsgPacket();

#endif
