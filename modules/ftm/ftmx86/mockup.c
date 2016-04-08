#include "access.h"
#include "hwregs.h"
#include "ftmx86.h"
#include "xmlaux.h"
#include "fancy.h"
#include <time.h>


uint32_t vStatus[CPU_MAX];
uint32_t vShaPtr[CPU_MAX];
uint64_t vTprep[CPU_MAX];
uint64_t vTdue;
uint64_t vTtrn;
uint32_t vMaxMsg;

uint8_t buffer[CPU_MAX * FTM_PAGESIZE * 2];
t_ftmPage page;
uint8_t* pBuf;
t_ftmPage* pPage;

uint32_t ftm_shared_offs;
t_ftmAccess* p;
t_ftmAccess ftmAccess;



static int ftmPut(uint32_t dstCpus, t_ftmPage*  pPage, uint8_t* bufWrite, uint32_t len) {
  uint32_t baseAddr, offs, cpuIdx;
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      baseAddr  = p->pCores[cpuIdx].ramAdr;
      offs      = p->pCores[cpuIdx].inaOffs;
      memset(bufWrite, 0, len);
      printf("Serializing...\n");
      if(serPage (pPage, (uint8_t*)&bufWrite[cpuIdx*(FTM_PAGESIZE * 2) + offs], offs, cpuIdx) != NULL) {
        printf("Wrote %u byte schedule to CPU %u at 0x%08x.", len, cpuIdx, baseAddr + offs);
        printf("Verify..."); 
        printf("OK\n");
      } else {
        fprintf(stderr, "There were errors, aborting PUT.\n");
        return -1;
      }
      
    }
  }
  printf("done.\n");
  return 0;  

}

int ftmOpen(const char* netaddress, uint8_t overrideFWcheck)
{
  
  int cpuIdx;
  
  pBuf  = (uint8_t*)&buffer[0];
  pPage = (t_ftmPage*)&page;
  
  p = (t_ftmAccess* )&ftmAccess;
  p->clusterAdr   = 0x400000;
  p->resetAdr     = 0x0;
  p->cpuQty       = CPU_MAX;
  p->thrQty       = THR_MAX;
  
  p->pCores       = malloc(p->cpuQty * sizeof(t_core));
  p->sharedAdr    = 0x8000;
  p->prioQAdr     = 0x200000;
  p->ebmAdr       = 0x1000000;
  ftm_shared_offs = FTM_SHARED_OFFSET_NEW;
  // get the active, inactive and shared pointer values from the core RAM
  for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
      p->pCores[cpuIdx].ramAdr      = cpuIdx * 0x10000;
      p->pCores[cpuIdx].hasValidFW  = 1;
      p->pCores[cpuIdx].actOffs     = 0;
      p->pCores[cpuIdx].inaOffs     = FTM_PAGESIZE;
      p->pCores[cpuIdx].sharedOffs  = FTM_SHARED_OFFSET + cpuIdx * CPU_SHARED_SIZE;
      vStatus[cpuIdx]   = 0;
      vShaPtr[cpuIdx]   = p->pCores[cpuIdx].sharedOffs;
      vTprep[cpuIdx]    = 100000/8;
  }
  
  vTdue   = 50000/8;
  vTtrn   = 250000/8;
  vMaxMsg = 5;
  uint32_t validCpus = (1 << p->cpuQty)-1;
  p->validCpus = validCpus;
  
  return validCpus;

}


int ftmClose(void) {
  return 0;
}


//per DM
int ftmRst(void) {
  ftmOpen("", 0);
  return 0;
}  

int ftmSetDuetime(uint64_t tdue) {
  vTdue = tdue;
  return 0;
}

int ftmSetTrntime(uint64_t ttrn) {
  vTtrn = ttrn;
  return 0;
}

int ftmSetMaxMsgs(uint64_t maxmsg) {
  vMaxMsg = maxmsg;
  return 0;
}


//per CPU
int ftmCpuRst(uint32_t dstCpus) {
  int cpuIdx;
  for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
    if(dstCpus & (1 << cpuIdx)) {
      //p->pCores[cpuIdx].ramAdr      = malloc(p->cpuQty * 2 * FTM_PAGESIZE );
      p->pCores[cpuIdx].hasValidFW  = 1;
      p->pCores[cpuIdx].actOffs     = 0;
      p->pCores[cpuIdx].inaOffs     = FTM_PAGESIZE;
      p->pCores[cpuIdx].sharedOffs  = FTM_SHARED_OFFSET + cpuIdx * CPU_SHARED_SIZE;
      vStatus[cpuIdx]   = 0;
      vShaPtr[cpuIdx]   = p->pCores[cpuIdx].sharedOffs;
      vTprep[cpuIdx]    = 100000/8;
    }
  }
  return 0;

}

int ftmFwLoad(uint32_t dstCpus, const char* filename) {
  ftmCpuRst(dstCpus);
  return 0;
}

int ftmSetPreptime(uint32_t dstCpus, uint64_t tprep) {
  int cpuIdx;
  for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
    if(dstCpus & (1 << cpuIdx)) {
      vTprep[cpuIdx] = tprep;
    }  
  }
  return 0;  
}

int ftmFetchStatus(uint32_t* buff, uint32_t len) {
  uint32_t cpuIdx, thrIdx, offset;
  uint32_t coreStateSize = (CPU_STATE_SIZE + p->thrQty * THR_STATE_SIZE);
  uint32_t returnedLen = ((EBM_REG_LAST + r_FPQ.tsCh)>>2) + WR_STATE_SIZE + p->cpuQty * coreStateSize;
  
  if (len < returnedLen) return (len - returnedLen);
  
  // read EBM status
  buff[EBM_REG_STATUS       >>2] = 0;
  buff[EBM_REG_SRC_MAC_HI   >>2] = 0x0000D15E;
  buff[EBM_REG_SRC_MAC_LO   >>2] = 0xA5EDBEE0;
  buff[EBM_REG_SRC_IPV4     >>2] = 0xCAFEBABE;
  buff[EBM_REG_SRC_UDP_PORT >>2] = 0xEBD0;
  buff[EBM_REG_DST_MAC_HI   >>2] = 0x0000ffff;
  buff[EBM_REG_DST_MAC_LO   >>2] = 0xffffffff;
  buff[EBM_REG_DST_IPV4     >>2] = 0xffffffff;
  buff[EBM_REG_DST_UDP_PORT >>2] = 0xEBD0;
  buff[EBM_REG_MTU          >>2] = 1500;
  buff[EBM_REG_ADR_HI       >>2] = 0x0;
  buff[EBM_REG_OPS_MAX      >>2] = vMaxMsg;
  buff[EBM_REG_EB_OPT       >>2] = 0x0;
  
  buff[(EBM_REG_LAST + r_FPQ.cfgGet)>>2]    = r_FPQ.cfg_ENA | r_FPQ.cfg_FIFO | r_FPQ.cfg_AUTOPOP | r_FPQ.cfg_AUTOFLUSH_TIME | r_FPQ.cfg_AUTOFLUSH_MSGS;
  buff[(EBM_REG_LAST + r_FPQ.dstAdr)>>2]    = 0x7fffffff;
  buff[(EBM_REG_LAST + r_FPQ.heapCnt)>>2]   = 0;
  
  buff[(EBM_REG_LAST + r_FPQ.msgCntO)>>2]   = 100;
  buff[(EBM_REG_LAST + r_FPQ.msgCntI)>>2]   = 100;
  buff[(EBM_REG_LAST + r_FPQ.tTrnHi)>>2]    = (uint32_t)(vTtrn >> 32);
  buff[(EBM_REG_LAST + r_FPQ.tTrnLo)>>2]    = (uint32_t)vTtrn;
  buff[(EBM_REG_LAST + r_FPQ.tDueHi)>>2]    = (uint32_t)(vTdue >> 32);
  buff[(EBM_REG_LAST + r_FPQ.tDueLo)>>2]    = (uint32_t)vTdue;
  buff[(EBM_REG_LAST + r_FPQ.capacity)>>2]  = 255;
  buff[(EBM_REG_LAST + r_FPQ.msgMax)>>2]    = vMaxMsg;
  buff[(EBM_REG_LAST + r_FPQ.ebmAdr)>>2]    = p->ebmAdr;

  offset = (EBM_REG_LAST + r_FPQ.tsCh)>>2;
  //printf("MsgCnt O is at 0x%08x\n", (EBM_REG_LAST + r_FPQ.msgCntO)>>2);
  
  buff[offset + WR_STATUS] =  0x6;
  
  uint64_t now = (uint64_t)time(NULL);
  
  buff[offset + WR_UTC_HI] =  (uint32_t)(now >> 32);
  buff[offset + WR_UTC_LO] =  (uint32_t)(now);
  
  offset += WR_STATE_SIZE; 

  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((p->validCpus >> cpuIdx) & 0x1) {
      buff[offset + cpuIdx*coreStateSize  + CPU_STATUS]   = vStatus[cpuIdx] & 0xffff;
      buff[offset + cpuIdx*coreStateSize  + CPU_MSGS]     = vStatus[cpuIdx] >> 16;
      buff[offset + cpuIdx*coreStateSize  + CPU_SHARED]   = vShaPtr[cpuIdx];
      buff[offset + cpuIdx*coreStateSize  + CPU_TPREP_HI] = (uint32_t)(vTprep[cpuIdx] >> 32);
      buff[offset + cpuIdx*coreStateSize  + CPU_TPREP_LO] = (uint32_t)vTprep[cpuIdx];
      //TODO
      //Future work: change everything to new register layout in v03. And include real threads !!!
      uint32_t tmp = buff[offset + cpuIdx*coreStateSize  + CPU_STATUS];
      if(tmp & STAT_RUNNING)  buff[offset + cpuIdx*coreStateSize  + CPU_THR_RUNNING] = 1;
      else                    buff[offset + cpuIdx*coreStateSize  + CPU_THR_RUNNING] = 0;
      if(tmp & STAT_WAIT)     buff[offset + cpuIdx*coreStateSize  + CPU_THR_WAITING] = 1;
      else                    buff[offset + cpuIdx*coreStateSize  + CPU_THR_WAITING] = 0;
      if(tmp & STAT_IDLE)     buff[offset + cpuIdx*coreStateSize  + CPU_THR_IDLE]    = 1;
      else                    buff[offset + cpuIdx*coreStateSize  + CPU_THR_IDLE]    = 0;
      if(tmp & STAT_ERROR)    buff[offset + cpuIdx*coreStateSize  + CPU_THR_ERROR]   = 1;
      else                    buff[offset + cpuIdx*coreStateSize  + CPU_THR_ERROR]   = 0;
      
      
      if(p->pCores[cpuIdx].actOffs < p->pCores[cpuIdx].inaOffs) { buff[offset + cpuIdx*coreStateSize  + CPU_THR_ACT_A] = 1;
                                                                  buff[offset + cpuIdx*coreStateSize  + CPU_THR_ACT_B] = 0;} 
      else                                                      { buff[offset + cpuIdx*coreStateSize  + CPU_THR_ACT_A] = 0;
                                                                  buff[offset + cpuIdx*coreStateSize  + CPU_THR_ACT_B] = 1;}
                                                                  
      buff[offset + cpuIdx*coreStateSize  + CPU_THR_RDY_A] = 1;
      buff[offset + cpuIdx*coreStateSize  + CPU_THR_RDY_B] = 1;
      
      thrIdx = 0;
      buff[offset + cpuIdx*coreStateSize  + CPU_STATE_SIZE + thrIdx*THR_STATE_SIZE + THR_STATUS]  = vStatus[cpuIdx] & 0xffff;  
      buff[offset + cpuIdx*coreStateSize  + CPU_STATE_SIZE + thrIdx*THR_STATE_SIZE + THR_MSGS]    = vStatus[cpuIdx] >> 16;
      
      if(p->thrQty > 1) {
        for(thrIdx=1;thrIdx < p->thrQty;thrIdx++) {
          buff[offset + cpuIdx*coreStateSize  + CPU_STATE_SIZE + thrIdx*THR_STATE_SIZE + THR_STATUS]  = 0;  
          buff[offset + cpuIdx*coreStateSize  + CPU_STATE_SIZE + thrIdx*THR_STATE_SIZE + THR_MSGS]    = 0;
        }
      }
      
    }
  }
     
  return 0;
}


/*

int ftmGetRunningThreads(uint64_t srcThrs, uint32_t* buff) {
  uint32_t srcCpus, cpuIdx, offset;
  srcCpus = thrs2cpus(srcThrs);
  

  offset = (EBM_REG_LAST + r_FPQ.tsCh)>>2;
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((srcCpus >> cpuIdx) & 0x1) {
      buff[offset + cpuIdx*4  + 0] 
    }
  }
     
  return 0;
}
*/
//this is horrible code, but harmless. Does the job for now.
//TODO: replace this with something more sensible
void ftmShowStatus(uint32_t srcCpus, uint32_t* status, uint8_t verbose) {
  uint32_t cpuIdx, i;
  uint32_t ftmStatus, ftmMsgs, mySharedMem, sharedMem;
  uint32_t cfg;
  uint64_t tmp;
  long long unsigned int ftmTPrep;
  uint32_t* buffEbm   = status;
  uint32_t* buffPrioq = (uint32_t*)&buffEbm[(EBM_REG_LAST>>2)];
  uint32_t* buffWr    = (uint32_t*)&buffPrioq[(PRIO_CNT_OUT_ALL_GET_1>>2)];
  uint32_t* buffCpu   = (uint32_t*)&buffWr[WR_STATE_SIZE];
  uint32_t coreStateSize = (CPU_STATE_SIZE + p->thrQty * THR_STATE_SIZE);
  char strBuff[65536];
  char* pSB = (char*)&strBuff;
  char sLinkState[20];
  char* pL = (char*)&sLinkState;
  char sSyncState[20];
  char* pS = (char*)&sSyncState;

  if(verbose) {
    //Generate EBM Status
    SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
    SNTPRINTF(pSB ,"\u2502 %sEBM%s                                                                           \u2502\n", KCYN, KNRM);
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 Status       \u2502 0x%08x",  buffEbm[EBM_REG_STATUS>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Src Mac      \u2502 0x%04x%08x", buffEbm[EBM_REG_SRC_MAC_HI>>2],  buffEbm[EBM_REG_SRC_MAC_LO>>2]); for(i=0;i<49;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Src IP       \u2502 0x%08x", buffEbm[EBM_REG_SRC_IPV4>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Src Port     \u2502 0x%04x", buffEbm[EBM_REG_SRC_UDP_PORT>>2]); for(i=0;i<57;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n"); 
    SNTPRINTF(pSB ,"\u2502 Dst Mac      \u2502 0x%04x%08x", buffEbm[EBM_REG_DST_MAC_HI>>2],  buffEbm[EBM_REG_DST_MAC_LO>>2]); for(i=0;i<49;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Dst IP       \u2502 0x%08x", buffEbm[EBM_REG_DST_IPV4>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Dst Port     \u2502 0x%04x", buffEbm[EBM_REG_DST_UDP_PORT>>2]); for(i=0;i<57;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 MTU          \u2502 %10u", buffEbm[EBM_REG_MTU>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Adr Hi       \u2502 0x%08x", buffEbm[EBM_REG_ADR_HI>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 Ops Max      \u2502 %10u", buffEbm[EBM_REG_OPS_MAX>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 EB Opt       \u2502 0x%08x", buffEbm[EBM_REG_EB_OPT>>2]); for(i=0;i<53;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2514"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");
    
    //Generate PrioQ Status
    SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
    SNTPRINTF(pSB ,"\u2502 %sFPQ%s                                                                           \u2502\n", KCYN, KNRM);
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    cfg = buffPrioq[PRIO_MODE_GET>>2];
    SNTPRINTF(pSB ,"\u2502 Flags        \u2502 ");
    if(cfg & 0) SNTPRINTF(pSB ,"    ENA   ");  else SNTPRINTF(pSB ,"     -    ");
    if(cfg & 1) SNTPRINTF(pSB ," AFL_MSGS ");  else SNTPRINTF(pSB ,"     -    ");    
    if(cfg & 2) SNTPRINTF(pSB ," AFL_TIME ");  else SNTPRINTF(pSB ,"     -    ");
    SNTPRINTF(pSB ,"   \u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 Dst Adr      \u2502         0x%08x", buffPrioq[PRIO_ECA_ADR_RW>>2]); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 EBM Adr      \u2502         0x%08x", buffPrioq[PRIO_EBM_ADR_RW>>2]); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    tmp = (((uint64_t)buffPrioq[PRIO_CNT_OUT_ALL_GET_1>>2]) <<32) + ((uint64_t)buffPrioq[PRIO_CNT_OUT_ALL_GET_0>>2]); 
    SNTPRINTF(pSB ,"\u2502 Msgs Out     \u2502 %18llu", (long long unsigned int)tmp); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u251C"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
    SNTPRINTF(pSB ,"\u2502 TGather      \u2502 %18u", buffPrioq[PRIO_TX_MAX_WAIT_RW>>2]); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2502 msg max      \u2502 %18u", buffPrioq[PRIO_TX_MAX_MSGS_RW>>2]); for(i=0;i<45;i++) SNTPRINTF(pSB ," "); SNTPRINTF(pSB ,"\u2502\n");
    SNTPRINTF(pSB ,"\u2514"); for(i=0;i<14;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<64;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");
  }

  //Generate WR Status
  if(buffWr[WR_STATUS] & PPS_VALID) SNTPRINTF(pL ,"  %sOK%s  ", KGRN, KNRM);
  else                              SNTPRINTF(pL ,"  %s--%s  ", KRED, KNRM);
  if(buffWr[WR_STATUS] & TS_VALID)  SNTPRINTF(pS ,"  %sOK%s  ", KGRN, KNRM);
  else                              SNTPRINTF(pS ,"  %s--%s  ", KRED, KNRM);
  uint64_t testme = (((uint64_t)buffWr[WR_UTC_HI]) << 32 | ((uint64_t)buffWr[WR_UTC_LO]));
  time_t testtime = (time_t)testme;
  SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
  SNTPRINTF(pSB ,"\u2502 %sWR %s                                                                           \u2502\n", KCYN, KNRM);
  SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
  SNTPRINTF(pSB ,"\u2502 PPS: %s TS: %s \u2502 WR-UTC: %.24s                     \u2502\n", sLinkState, sSyncState, ctime((time_t*)&testtime));
  SNTPRINTF(pSB ,"\u2514"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");
    
  //Generate CPUs Status
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((srcCpus >> cpuIdx) & 0x1) {
      ftmStatus = buffCpu[cpuIdx*coreStateSize + CPU_STATUS];  
      ftmMsgs   = buffCpu[cpuIdx*coreStateSize + CPU_MSGS];
      sharedMem = buffCpu[cpuIdx*coreStateSize + CPU_SHARED]; //convert lm32's view to pcie's view
      mySharedMem = p->clusterAdr + (sharedMem & 0x3fffffff); //convert lm32's view to pcie's view
      ftmTPrep = (long long unsigned int)(((uint64_t)buffCpu[cpuIdx*coreStateSize + CPU_TPREP_HI]) << 32 | ((uint64_t)buffCpu[cpuIdx*coreStateSize + CPU_TPREP_LO]));
      
      SNTPRINTF(pSB ,"\u2552"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2550"); SNTPRINTF(pSB ,"\u2555\n");
      SNTPRINTF(pSB ,"\u2502 %sCore #%02u%s                                                                      \u2502\n", KCYN, cpuIdx, KNRM);
      SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u252C"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
      SNTPRINTF(pSB ,"\u2502 Status: %02x ErrCnt: %3u \u2502   MsgCnt: %9u       TPrep: %13llu ns    \u2502\n", \
       (uint8_t)ftmStatus, (uint8_t)(ftmStatus >> 8), ftmMsgs, ftmTPrep);
      SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u253C"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
      SNTPRINTF(pSB ,"\u2502 Shared Mem: 0x%08x \u2502", mySharedMem + cpuIdx*CPU_SHARED_SIZE);
      if(p->pCores[cpuIdx].actOffs < p->pCores[cpuIdx].inaOffs) SNTPRINTF(pSB ,"   Act Page: A 0x%08x  Inact Page: B 0x%08x", p->pCores[cpuIdx].actOffs, p->pCores[cpuIdx].inaOffs);
      else                      SNTPRINTF(pSB ,"   Act Page: B 0x%08x  Inact Page: A 0x%08x", p->pCores[cpuIdx].actOffs, p->pCores[cpuIdx].inaOffs);
      SNTPRINTF(pSB ,"   \u2502\n");
      SNTPRINTF(pSB ,"\u251C"); for(i=0;i<24;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2534"); for(i=0;i<54;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2524\n");
      SNTPRINTF(pSB ,"\u2502       ");

      if(ftmStatus & STAT_RUNNING)    SNTPRINTF(pSB ,"   %sRUNNING%s   ", KGRN, KNRM);  else SNTPRINTF(pSB ,"   %sSTOPPED%s   ", KRED, KNRM);
      if(ftmStatus & STAT_IDLE)       SNTPRINTF(pSB ,"     %sIDLE%s    ", KYEL, KNRM);  else SNTPRINTF(pSB ,"     %sBUSY%s    ", KGRN, KNRM);
      if(ftmStatus & STAT_STOP_REQ)   SNTPRINTF(pSB ,"   STOP_REQ  ");  else SNTPRINTF(pSB ,"      -      ");
      if(ftmStatus & STAT_ERROR)      SNTPRINTF(pSB ,"     %sERROR%s   ", KRED, KNRM);  else SNTPRINTF(pSB ,"     %sOK%s      ", KGRN, KNRM);
      if(ftmStatus & STAT_WAIT)       SNTPRINTF(pSB ,"  WAIT_COND  ");  else SNTPRINTF(pSB ,"      -      ");
      SNTPRINTF(pSB ,"       \u2502\n");
      SNTPRINTF(pSB ,"\u2514"); for(i=0;i<79;i++) SNTPRINTF(pSB ,"\u2500"); SNTPRINTF(pSB ,"\u2518\n");
    }
  }
  printf("%s", (const char*)strBuff);
}


//per thread
int ftmThrRst(uint64_t dstBitField) {
  return 0;
}

int ftmCommand(uint64_t dstThr, uint32_t command) {

  uint32_t cpuIdx, tmp;

  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((thrs2cpus(dstThr) >> cpuIdx) & 0x1) {
      switch(command) {
        case CMD_START        : vStatus[cpuIdx] |=  STAT_RUNNING; break;
        case CMD_IDLE         : vStatus[cpuIdx] |=  STAT_IDLE;    break;
        case CMD_STOP_REQ     : vStatus[cpuIdx] &= ~STAT_RUNNING; break;
        case CMD_STOP_NOW     : vStatus[cpuIdx] &= ~STAT_RUNNING; break;
        case CMD_COMMIT_PAGE  : tmp = p->pCores[cpuIdx].actOffs;  
                                p->pCores[cpuIdx].actOffs = p->pCores[cpuIdx].inaOffs;
                                p->pCores[cpuIdx].inaOffs = tmp;
                                break;
        default               : return -1;
      }
    }
  }
  return 0;
}

int ftmSignal(uint64_t dstThr, uint32_t offset, uint64_t value, uint64_t mask) {
  return 0;
}

int ftmPutString(uint64_t dstThr, const char* sXml) {
  t_ftmPage*  pPage = parseXmlString(sXml);
  return ftmPut(thrs2cpus(dstThr), pPage, pBuf, BUF_SIZE);
  
}



int ftmCheckString(const char* sXml) {

  uint8_t     buff[65536];
  int         ret;
  t_ftmPage*  pPage = parseXmlString(sXml);
  
  
  if(serPage (pPage, (uint8_t*)&buff[0], 0, 0) != NULL) {
    printf("Schedule OK\n");
    ret = 0;
  } else {
    fprintf(stderr, "There were errors in the Schedule.\n");
    ret = -1;
  }
  
  if(pPage != NULL) free(pPage);

  return ret;
}

  
int ftmPutFile(uint64_t dstThr, const char* filename) {
  t_ftmPage*  pPage = parseXmlFile(filename);
  return ftmPut(thrs2cpus(dstThr), pPage, pBuf, BUF_SIZE);
  
}

int ftmClear(uint64_t dstThr) {
  uint32_t offs, cpuIdx; 
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((thrs2cpus(dstThr) >> cpuIdx) & 0x1) {
      offs      = p->pCores[cpuIdx].inaOffs;
      memset((uint8_t*)&buffer[cpuIdx*(FTM_PAGESIZE * 2) + offs], 0, FTM_PAGESIZE);
    }  
  }
  printf("done.\n");
  return 0;  
}



int ftmDump(uint64_t srcThr, uint32_t len, uint8_t actIna, char* stringBuf, uint32_t lenStringBuf) {
  uint32_t offs, cpuIdx;  
  t_ftmPage* pPage;
  char* bufStr = stringBuf;
  
  for(cpuIdx=0;cpuIdx < p->cpuQty;cpuIdx++) {
    if((thrs2cpus(srcThr) >> cpuIdx) & 0x1) {
      if(actIna == ACTIVE)  offs = p->pCores[cpuIdx].actOffs; 
      else                  offs = p->pCores[cpuIdx].inaOffs;
      

      pPage = deserPage(calloc(1, sizeof(t_ftmPage)), (uint8_t*)&buffer[cpuIdx*(FTM_PAGESIZE * 2) + offs], offs);
      if(pPage != NULL) {  
         printf("Deserialization successful.\n\n");
         if (lenStringBuf - (bufStr - stringBuf) < 2048) {printf("String buffer running too low, aborting.\n"); return (uint32_t)(bufStr - stringBuf);}
         SNTPRINTF(bufStr, "---CPU %u %s page---\n", cpuIdx, "active"); //don't do zero termination in between dumps
         bufStr += showFtmPage(pPage, bufStr); //don't do zero termination in between dumps
      } else {printf("Deserialization for CPU %u FAILED! Corrupt/No Data ?\n", cpuIdx); return -1;}
    }
  }
  *bufStr++ = 0x00; // zero terminate all dumps
  return (uint32_t)(bufStr - stringBuf); // return number of characters

}

int ftmSetBp(uint64_t dstThr, int32_t planIdx) {
  return 0;
}

uint64_t cpus2thrs(uint32_t cpus) {
  uint64_t i;
  uint64_t res=0;
  
  for(i=0;i<8;i++) {
    res |= (((cpus >> i) & 1ull) << (i*8));
  }  
  return res;
}

uint32_t thrs2cpus(uint64_t thrs) {
  uint32_t i;
  uint64_t res=0;
  
  for(i=0;i<64;i++) res |= (((thrs >> i) & 1) << (i/8));
  return res;
  
}

