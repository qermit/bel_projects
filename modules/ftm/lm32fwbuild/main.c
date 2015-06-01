#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mprintf.h"
#include "mini_sdb.h"
#include "dmif.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
 
unsigned int cpuId, cpuQty, heapCap;
#define SHARED __attribute__((section(".shared")))
//extern uint32_t* _startshared;
uint32_t SHARED hub[SHARED_IF_END_/4 +1]; //allocate space for interface to host

uint32_t enable, stopreq;
extern uint32_t*       _startshared[];


void ebmInit()
{
   ebm_init();
   ebm_config_if(LOCAL,   "hw/08:00:30:e3:b0:5a/udp/192.168.0.100/port/60368");
   //ebm_config_if(REMOTE,  "hw/00:14:d1:fa:01:aa/udp/192.168.191.131/port/60368");
   //ebm_config_if(REMOTE,  "hw/00:26:7b:00:04:08/udp/192.168.191.72/port/60368");
   ebm_config_if(REMOTE,  "hw/ff:ff:ff:ff:ff:ff/udp/192.168.0.101/port/60368");
   ebm_config_meta(1500, 0x11, 255, 0x00000000 );
}

void clearCmdIf() {
  // reset cmd area
  hub[CMD_DM] = 0;
  hub[CMD_THR_START]    = 0;
  hub[CMD_THR_STOP]     = 0;
  hub[CMD_THR_ABORT]    = 0;
  hub[CMD_THR_PAGE_A]   = 0;              
  hub[CMD_THR_PAGE_B]   = 0;
}

void dmInit()
{
  //provide pointers to periphery for threads
  hub[DEV_TIME]    = pCpuSysTime;
  hub[DEV_FPQ]     = pFpqData;
  hub[DEV_SHARED]  = pSharedRam;

  //init basic interface
  clearCmdIf();

  hub[REG_STAT]     = DM_STAT_IDLE;
  hub[REG_MSG_CNT]  = 0;
  hub[REG_THR_RUN]  = 0;
  hub[REG_THR_ERR]  = 0;
  hub[REG_THR_A_B]  = 0;
  hub[REG_T_PREP]   = 0;
  hub[REG_T_TRN]    = 0;

  //init threads as empty
  int i;
  for(i = 0; i<MAX_THREADS*2; i++) {hub[REG_THR_PTRS + i*4]  = NULL;}

}

void init()
{ 
   discoverPeriphery();
   mprintf("#%02u: Hi! \n", getCpuIdx());
   irq_disable();
   cpuId = getCpuIdx();
   uart_init_hw();
   ebmInit();
   dmInit();
}




void cmdEval()
{
  uint32_t run, ab, stat, cmd;
  
  //atomically set control regs
  run = (hub[REG_THR_RUN] & ~hub[CMD_THR_ABORT]) | hub[CMD_THR_START];
  stopreq = hub[CMD_THR_STOP];
  hub[REG_THR_RUN]  = run;
  ab                = (hub[REG_THR_A_B] & ~hub[CMD_THR_PAGE_A]) | hub[CMD_THR_PAGE_B];
  hub[REG_THR_A_B]  = ab;
  
  stat = hub[REG_STAT];
  cmd  = hub[CMD_DM];
  
  clearCmdIf();
   
  // eval main cmd reg 
  switch(cmd) {
    case DM_CMD_RST:  atomic_on(); DBPRINT1("#%02u: Resetting\n", cpuId); atomic_off();
                      stat = DM_STAT_IDLE; dmInit(); break;
    case DM_CMD_ENA:  atomic_on(); DBPRINT1("#%02u: Enabled\n", cpuId); atomic_off();
                      enable = 1; stat |=  DM_STAT_RUNNING; 
                      break;
    case DM_CMD_DIS:  enable = 0; stat &= ~DM_STAT_RUNNING; break;
                      atomic_on(); DBPRINT1("#%02u: Disabled\n", cpuId); atomic_off();
                      break;
                      
    default:          atomic_on(); DBPRINT1("#%02u: Cmd %x unknown\n", cpuId, cmd); atomic_off(); 
  }  
  
  if (run) stat &= ~DM_STAT_IDLE;
  else     stat |=  DM_STAT_IDLE;
  
  //set stat
  hub[REG_STAT] = stat;

}

#define OFFSET 0x100

inline void refreshICC()
{
    uint32_t icc = 1;
    //write IRQ mask
    asm (   "wcsr icc, %0" \
            :             \
            : "r" (icc)    \
        );
}

void main(void) {

   int j, k;
   init();
   uint32_t adr = (uint32_t)&_startshared + OFFSET;
   typedef int (*fPtr)(int);
   fPtr foo = (fPtr)adr;
   //for (j = 0; j < (125000000/4); ++j) { asm("nop"); }
   atomic_on();
   mprintf("#%02u: DM Core Ready. Hi! \n", cpuId);
   #if DEBUGLEVEL != 0
      mprintf("#%02u: Debuglevel %u. Don't expect timeley delivery with console outputs on!\n", cpuId, DEBUGLEVEL);
   #endif   
   atomic_off();
   /*
   if((uint32_t)_startshared != DM_SHARED_OFFSET) {
     mprintf("#%02u: Shared Offset Wrong. I'm told it should be 0x%08x, but I have 0x%08x. Aborting.\n", cpuId, DM_SHARED_OFFSET, (uint32_t)_startshared);
     while (1) {} 
   }
   */     
    k=1;
   mprintf("#%02u: for testing func, write 0x1 to 0x%08x\n", cpuId, &hub[REG_THR_RUN]);
   while (1) {
    for (j = 0; j < (125000000); ++j) { asm("nop"); }
    mprintf("#%02u: 0x%08x\n", cpuId, hub[REG_THR_RUN]);
    if(hub[REG_THR_RUN] & 0x1) {
    hub[REG_THR_RUN] = 0;
    refreshICC();
    mprintf("#%02u: Calling function x @ %08x with k=%u. Result: %u\n", cpuId, adr, k, foo(k));   
    k++;  
    }
    
   }

}
