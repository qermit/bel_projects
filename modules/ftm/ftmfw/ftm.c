#include "ftm.h"
#include "dbg.h"



const struct t_FPQ r_FPQ = {    .rst        =  0x00 >> 2,
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


uint64_t dbg_sum = 0;

uint64_t execCnt; 

void prioQueueInit()
{
   volatile uint8_t* pIf   = pFtmIf;
   
   *(pFpqCtrl + r_FPQ.rst)       = 1;
   *(pFpqCtrl + r_FPQ.cfgClr)    = 0xffffffff;
   *(pFpqCtrl + r_FPQ.clear)     = 1;
   *(pFpqCtrl + r_FPQ.dstAdr)    = (uint32_t)pEca & ~0x80000000;
#ifndef DEBUGPRIOQDST
#define DEBUGPRIOQDST 0
#endif
   *(pFpqCtrl + r_FPQ.tsAdr)     = (uint32_t)(DEBUGPRIOQDST);
   *(pFpqCtrl + r_FPQ.ebmAdr)    = ((uint32_t)pEbm & ~0x80000000);
   *(pFpqCtrl + r_FPQ.msgMax)    = 5;
   *(pFpqCtrl + r_FPQ.tTrnHi)    = *(uint32_t*)&pIf[FTM_MIF_TTRN_HI];
   *(pFpqCtrl + r_FPQ.tTrnLo)    = *(uint32_t*)&pIf[FTM_MIF_TTRN_LO];
   *(pFpqCtrl + r_FPQ.tDueHi)    = *(uint32_t*)&pIf[FTM_MIF_TDUE_HI];
   *(pFpqCtrl + r_FPQ.tDueLo)    = *(uint32_t*)&pIf[FTM_MIF_TDUE_LO];
   
   *(pFpqCtrl + r_FPQ.cfgClr)    = 0xffffffff;
   *(pFpqCtrl + r_FPQ.cfgSet)    = r_FPQ.cfg_AUTOFLUSH_TIME | 
                                   r_FPQ.cfg_AUTOFLUSH_MSGS |
                                   r_FPQ.cfg_AUTOPOP | 
                                   r_FPQ.cfg_FIFO |
                                   r_FPQ.cfg_ENA;
#if DEBUGPRIOQ == 1                               
   *(pFpqCtrl + r_FPQ.cfgSet)    = r_FPQ.cfg_MSG_ARR_TS;
#endif                                      
}

void ftmInit()
{
  
   
   pFtmIf   = (uint8_t*)_startshared;

   *(uint32_t*)&pFtmIf[FTM_MIF_PROCQTY] = FTM_PROC_MAX;
   
   *(volatile uint32_t**)&pFtmIf[FTM_MIF_PSHARED]  = pSharedRam;
   *(uint32_t*)&pFtmIf[FTM_MIF_CMD]                = 0;
   *(uint32_t*)&pFtmIf[FTM_MIF_STAT_RUN]           = 0;
    *(uint32_t*)&pFtmIf[FTM_MIF_STAT_IDLE]         = 0;
   *(uint32_t*)&pFtmIf[FTM_MIF_STAT_WAIT]          = 0;
   *(uint32_t*)&pFtmIf[FTM_MIF_STAT_ERR]           = 0;
   *(uint32_t*)&pFtmIf[FTM_MIF_PROCQTY]            = 0;
   *(uint64_t*)&pFtmIf[FTM_MIF_TTRN]               = 150000/8;
   *(uint64_t*)&pFtmIf[FTM_MIF_TDUE]               = 150000/8;
   

   uint32_t* pDebug = (uint32_t*)&pFtmIf[FTM_PROC_DEBUG_DATA];
   pDebug[DBG_DISP_DUR_MIN] = 0xffffffff;
   pDebug[DBG_DISP_DUR_MAX] = 0x0;
   pDebug[DBG_DISP_DUR_AVG] = 0x0;

   // init all proc page ptrs and sensible default values   
   uint8_t* pCurProc;
   uint32_t procQty  =  *(uint32_t*)&pFtmIf[FTM_MIF_PROCQTY];
   int i;
   for(i = 0; i < procQty; pCurProc = pProc[i++])
   {
      *(uint32_t*)&pCurProc[FTM_PROC_MSGCNT]    = 0;
      *(uint64_t*)&pCurProc[FTM_PROC_TPREP]     = 100000/8;
      *(uint8_t**)&pCurProc[FTM_PROC_PACT]      = (uint8_t*)&pCurProc[FTM_PROC_PAGES + 0*FTM_PAGESIZE];
      *(uint8_t**)&pCurProc[FTM_PROC_PINA]      = (uint8_t*)&pCurProc[FTM_PROC_PAGES + 1*FTM_PAGESIZE];
   }
   
  prioQueueInit();
  
}

inline void showId()
{
   mprintf("#%02u: ", getCpuIdx()); 
}

void cmdEval()
{
   uint32_t* pCmd    = (uint32_t*)&pFtmIf[FTM_MIF_CMD];
   uint8_t* pCurProc;
   uint32_t procIdx, procQty, idStart;
   
   
   uint8_t** pAct;
   uint8_t** pIna;
   uint8_t* pTmp;
   uint32_t* pStatRun   = (uint32_t*)&pFtmIf[FTM_MIF_STAT_RUN];
   uint32_t* pStatIdle  = (uint32_t*)&pFtmIf[FTM_MIF_STAT_IDLE];
   uint32_t* pStatWait  = (uint32_t*)&pFtmIf[FTM_MIF_STAT_WAIT];
   uint32_t* pStatErr   = (uint32_t*)&pFtmIf[FTM_MIF_STAT_ERR];
   
   if(*pCmd)
   {
      if(*pCmd & CMD_PROC_ID_ALL) {procIdx = 0; procQty = *(uint32_t*)&pFtmIf[FTM_MIF_PROCQTY];}
      else                        {procIdx = getBitSlice(*pCmd, CMD_PROC_ID, CMD_PROC_ID_POS); procQty = procIdx+1;}
      
      while(procIdx < procQty) {
         pCurProc    = pProc[procIdx * FTM_PROC_END_];
         pAct        = (uint8_t**)&pCurProc[FTM_PROC_PACT]; 
         pIna        = (uint8_t**)&pCurProc[FTM_PROC_PINA];
         
         idStart     = *(uint32_t*)&((*pAct)[FTM_PAGE_IDSTART]);
         
         if(*pCmd & CMD_RST)          { showId(); mprintf("Ftm Init done\n"); ftmInit(); }
         if(*pCmd & CMD_START)        { showId(); 
                                        mprintf(" Starting Process %02u, Plan: %02u\n",
                                        procIdx, getBitSlice(idStart, CH_ID_PLAN_MSK, CH_ID_PLAN_POS));
                                         
                                        *pStatIdle &= ~(1<<procIdx);
                                        *pStatRun  |=  (1<<procIdx);
                                        *(uint8_t**)&((*pAct)[FTM_PAGE_PCUR]) = getPtrByID(idStart);
                                      }
         if(*pCmd & CMD_STOP)       { *pStatIdle |=  (1<<procIdx); }
         if(*pCmd & CMD_ABORT)      { *pStatIdle &= ~(1<<procIdx);
                                        *pStatRun  &= ~(1<<procIdx); 
                                        showId(); mprintf("Aborted Process %02u\n", procIdx);} 
         
         if(*pCmd & CMD_COMMIT_PAGE)  { //swap active and inactive pointers. set current plan ptr to plan indicated by start ID
                                        pTmp = *pIna; *pIna = *pAct; *pAct = pTmp;
                                        *(uint8_t**)&((*pAct)[FTM_PAGE_PCUR]) = getPtrByID(idStart);
                                       }
         if(*pCmd & CMD_DBG_1)        {showId(); mprintf("DBG1\n");}
         
         //if(*pCmd & CMD_SHOW_ACT)     {  showId(); mprintf("Showing Active\n"); showFtmPage(pAct);}
         //if(*pCmd & CMD_SHOW_INA)     {  showId(); mprintf("Showing Inactive\n"); showFtmPage(pIna);}
         procIdx++;
      }
     
      //only zero the command reg if you found a command. otherwise this becomes race-condition-hell!
      *pCmd = 0;                       
   }
   
   
}


/*
void showFtmPage(t_ftmPage* pPage)
{
   uint32_t planIdx, chainIdx, msgIdx;
   t_ftmChain* pChain  = NULL;
   t_ftmMsg*   pMsg  = NULL;
   
   mprintf("---PAGE %08x\n", pPage);
   mprintf("StartPlan:\t");
   
   if(pPage->pStart == &(pActProc->idle) ) mprintf("idle\n");
   else { 
          if(pPage->pStart == NULL) mprintf("NULL\n");
          else mprintf("%08x\n", pPage->pStart);
        } 
   
   mprintf("AltPlan:\t");
   if(pPage->pBp == &(pActProc->idle) ) mprintf("idle\n");
   else { 
          if(pPage->pBp == NULL) mprintf("NULL\n");
          else mprintf("%08x\n", pPage->pBp);
        }  
   mprintf("PlanQty:\t%u\t%08x\n", pPage->planQty, &(pPage->planQty));
    
   for(planIdx = 0; planIdx < pPage->planQty; planIdx++)
   {
      mprintf("\t---PLAN %c\n", planIdx+'A');
      chainIdx = 0;
      pChain = pPage->plans[planIdx];
      while(pChain != NULL)
      {
         mprintf("\t\t---CHAIN %c%u\n", planIdx+'A', chainIdx);
         mprintf("\t\tpNext: 0x%08x\n", pChain->pNext);
         mprintf("\t\tStart:\t\t%08x%08x\n\t\tperiod:\t\t%08x%08x\n\t\trep:\t\t\t%08x\nrepcnt:\t\t%08x\n\t\tmsg:\t\t\t%08x\nmsgIdx:\t\t%08x\n", 
         (uint32_t)(pChain->tStart>>32), (uint32_t)pChain->tStart, 
         (uint32_t)(pChain->tPeriod>>32), (uint32_t)pChain->tPeriod,
         pChain->repQty,
         pChain->repCnt,
         pChain->msgQty,
         pChain->msgIdx);
         
         mprintf("\t\tFlags:\t");
         if(pChain->flags & CH_FLAGBIT_BP) mprintf("-IS_BP\t");
         if(pChain->flags & CH_FLAGBIT_COND_MSI) mprintf("-IS_CMSI\t");
         if(pChain->flags & CH_FLAGBIT_COND_SHARED) mprintf("-IS_CSHA\t");
         if(pChain->flags & CH_FLAGBIT_SIG_SHARED) mprintf("-IS_SIG_SHARED");
         if(pChain->flags & CH_FLAGBIT_SIG_MSI)    mprintf("-IS_SIG_MSI");
         if(pChain->flags & CH_FLAGBIT_END) mprintf("-IS_END");
         if(pChain->flags & CH_FLAGBIT_ENDLOOP) mprintf("-IS_ENDLOOP");
         mprintf("\n");
         
         mprintf("\t\tCondSrc:\t%08x\n\t\tCondVal:\t%08x\n\t\tCondMsk:\t%08x\n\t\tSigDst:\t\t\t%08x\n\t\tSigVal:\t\t\t%08x\n", 
         (uint32_t)pChain->condSrc,
         (uint32_t)pChain->condVal, 
         (uint32_t)pChain->condMsk,
         pChain->sigDst,
         pChain->sigVal);  
         
         pMsg = pChain->pMsg;
         
         for(msgIdx = 0; msgIdx < pChain->msgQty; msgIdx++)
         {
            mprintf("\t\t\t---MSG %u\n", msgIdx);
            mprintf("\t\t\tid:\t%08x%08x\n\t\t\tFID:\t%u\n\t\t\tGID:\t%u\n\t\t\tEVTNO:\t%u\n\t\t\tSID:\t%u\n\t\t\tBPID:\t%u\n\t\t\tpar:\t%08x%08x\n\t\t\ttef:\t\t%08x\n\t\t\toffs:\t%08x%08x\n", 
            (uint32_t)(pMsg[msgIdx].id>>32), (uint32_t)pMsg[msgIdx].id,
            getIdFID(pMsg[msgIdx].id),
            getIdGID(pMsg[msgIdx].id),
            getIdEVTNO(pMsg[msgIdx].id),
            getIdSID(pMsg[msgIdx].id),
            getIdBPID(pMsg[msgIdx].id), 
            (uint32_t)(pMsg[msgIdx].par>>32), (uint32_t)pMsg[msgIdx].par,
            pMsg[msgIdx].tef,
            (uint32_t)(pMsg[msgIdx].offs>>32), (uint32_t)pMsg[msgIdx].offs);   
         }
         
   
         
         if(pChain->flags & CH_FLAGBIT_END) pChain = NULL;
         else pChain = (t_ftmChain*)pChain->pNext;
      }
           
   }
   uint64_t j;
  for (j = 0; j < (250000000); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }    
   
}
*/
/*
void showStatus()
{
   uint32_t stat = pActProc->status;
   mprintf("\f%08x\tStatus:\t", (uint32_t)(&(pActProc->cmd)) );
   if(stat & STAT_RUNNING) mprintf("\t\t-RUNNING"); else mprintf("\t\t-\t");
   if(stat & STAT_IDLE) mprintf("\t\t-IDLE"); else mprintf("\t\t-\n");
   if(stat & STAT_STOP_REQ) mprintf("\t\t-STOP_REQ"); else mprintf("\t\t-\t");
   if(stat & STAT_ERROR) mprintf("\t\t-ERROR"); else mprintf("\t\t-\t");
   mprintf("\t\tE:\t%x%08x", (uint32_t)(execCnt), (uint32_t)(execCnt>>32) );
   mprintf("\n");
   
}
*/
inline int dispatch(uint8_t* pMsg)
{
   
   unsigned int diff;
   int ret = 1;

   diff = ( *(pFpqCtrl + r_FPQ.capacity) - *(pFpqCtrl + r_FPQ.heapCnt));
   if(diff > 1)
   {  
      //incIdSCTR(&pMsg->id, &pActProc->sctr); //copy sequence counter (sctr) into msg id and inc sctr
      atomic_on();
      *pFpqData = *(uint32_t*)&pMsg[FTM_MSG_ID_HI];
      *pFpqData = *(uint32_t*)&pMsg[FTM_MSG_ID_LO];
      #if DEBUGTIME == 1
      *pFpqData = hiW(then) | 0xff000000;
      *pFpqData = loW(then);
      #else
      *pFpqData = *(uint32_t*)&pMsg[FTM_MSG_PAR_HI];
      *pFpqData = *(uint32_t*)&pMsg[FTM_MSG_PAR_LO];
      #endif
      *pFpqData = *(uint32_t*)&pMsg[FTM_MSG_TEF];
      *pFpqData = *(uint32_t*)&pMsg[FTM_MSG_RES];
      *pFpqData = *(uint32_t*)&pMsg[FTM_MSG_TS_HI];
      *pFpqData = *(uint32_t*)&pMsg[FTM_MSG_TS_LO];
      atomic_off();
      
      
      
   } else {
      ret = 0;
      DBPRINT1("Queue full, waiting\n");
   }   

   return ret;

}

uint8_t* condValid(uint8_t* c)
{
   uint8_t* pConBase = (uint8_t*)&c[FTM_CH_PCON];
   uint32_t* flags;
   uint8_t* pJmp;
   uint8_t* pCon;
   uint64_t val, cmp, msk ;
   uint64_t* pSrc = NULL;
   uint32_t cpuIdx, procIdx, conIdx, tmpID;
   uint32_t conQty = *(uint32_t*)&c[FTM_CH_CONQTY];

    
   
   //1. Is there a condition left to check?
   while(conIdx < conQty) {
      pCon     = pConBase + conIdx * FTM_CON_END_;
      flags    = (uint32_t*)&pCon[FTM_CON_FLAGS];
      //set pSrc correctly
      if(*flags & CON_FLAGBIT_SRC_IS_PTR) pSrc  = (uint64_t*)&pCon[FTM_CON_SRC]; 
      else {
         if(*flags & CON_FLAGBIT_SRC_IS_ID)  { tmpID = *(uint32_t*)&pCon[FTM_CON_SRC];}                    // get stuff from con ID
         else                                { tmpID = *(uint32_t*)&c[FTM_CH_ID];}   // get stuff from chain ID
         procIdx  = getBitSlice(tmpID, CH_ID_PROC_MSK, CH_ID_PROC_POS);
         cpuIdx   = getBitSlice(tmpID, CH_ID_CPU_MSK,  CH_ID_CPU_POS);
         pSrc     = (uint64_t*)(pSharedRam + cpuIdx * procIdx * 12); //get the shared region for cpu x, process y         
      }
      msk   = *(uint64_t*)&pCon[FTM_CON_MSK];
      cmp   = *(uint64_t*)&pCon[FTM_CON_CMP];
         
      //2. Get value from source (Address or MSI)
      if(pSrc != NULL) {val = *pSrc; *pSrc = 0;}
      else {
         uint32_t ip;
         irq_disable();
         asm ("rcsr %0, ip": "=r"(ip)); //get pending irq flags
         if(ip & 1<<MSI_SIG)
         {
            irq_pop_msi(MSI_SIG);      //pop msg from msi queue into global_msi variable
            val = (uint64_t)global_msi.adr << 32 | (uint64_t)global_msi.msg;
            irq_clear(1<<MSI_SIG);     //clear pending bit
         }      
         irq_enable();
      }
      
      //3. Check condition
      if((cmp & msk) == (val & msk)) {
         if(*flags & CON_FLAGBIT_JMP_TO_SRC)  pJmp = (uint8_t*)pSrc;
         if(*flags & CON_FLAGBIT_JMP_IS_ID)   pJmp = getPtrByID(*(uint32_t*)&pCon[FTM_CON_JMP]);
         else                                 pJmp = (uint8_t*)&pCon[FTM_CON_JMP];
         
         //*(uint64_t*)&pCur[FTM_CH_TSTART] = *pcTStart + cTPeriod;
         return pJmp;
      }
      conIdx++;
   }
   *(uint32_t*)&pFtmIf[FTM_MIF_STAT_WAIT] |= (1<<getBitSlice(*(uint32_t*)&c[FTM_CH_ID], CH_ID_PROC_MSK, CH_ID_PROC_POS));
   return NULL;

} 

inline void sigSend(uint8_t* c)
{
   uint8_t** pSigBase = (uint8_t**)&c[FTM_CH_PSIG];
   uint8_t** pSig     = (uint8_t**)&((*pSigBase)[FTM_SIG_FLAGS]);
   
   uint32_t sigQty = *(uint32_t*)&c[FTM_CH_SIGQTY];
   uint32_t sigIdx;
   uint32_t* flags = (uint32_t*)&((*pSig)[FTM_SIG_FLAGS]);
   uint64_t val;
   
   while(sigIdx < sigQty) {
      pSig     = pSigBase + sigIdx * FTM_SIG_END_;
      flags    = (uint32_t*)&pSig[FTM_SIG_FLAGS];
      
      if( 
         ((*flags & SIG_FLAGBIT_1ST)  && (*(uint32_t*)&c[FTM_CH_REPCNT] == 0)) 
      || ((*flags & SIG_FLAGBIT_LAST) && (*(uint32_t*)&c[FTM_CH_REPCNT] == *(uint32_t*)&c[FTM_CH_REPQTY] -1) )
      || (!((*flags & SIG_FLAGBIT_1ST) || (*flags & SIG_FLAGBIT_LAST)))
        ) {  
         
         //32 or 64 bit write
         if(*flags & SIG_FLAGBIT_64)   *(uint64_t*)&((*pSig)[FTM_SIG_DST]) = (uint64_t)val;
         else                          *(uint32_t*)&((*pSig)[FTM_SIG_DST]) = (uint32_t)val;
           
         if(*flags & SIG_FLAGBIT_ONCE) *flags |= SIG_FLAGBIT_DONE;
      }
      
      sigIdx++; 
  }
  
}

uint8_t* processChain(uint8_t* c)
{
   uint8_t* pCur;
   uint8_t* pCurMsg;
   
   uint64_t tMsgExec, now;

   uint64_t dbg_now, dbg_then; 
   uint32_t dbg_dur;

   pCur  = c; 
   now   = getSysTime();
   
   //values
   uint32_t* procIdx    =  (uint32_t*)&pFtmIf[FTM_MIF_PROCIDX];
   uint8_t** pActProc   =  (uint8_t**)&pFtmIf[FTM_MIF_PROCS + *procIdx * FTM_PROC_END_];
   uint64_t  TPrep      = *(uint64_t*)&((*pActProc)[FTM_PROC_TPREP]);
   uint64_t  cTPeriod   = *(uint64_t*)&c[FTM_CH_TPERIOD];
   uint32_t  cMsgQty    = *(uint32_t*)&c[FTM_CH_MSGQTY];
   uint32_t  cRepQty    = *(uint32_t*)&c[FTM_CH_REPQTY];
   
   
   //pointers
   uint32_t* pcMsgCnt   =  (uint32_t*)&c[FTM_PROC_MSGCNT];
   uint64_t* pcTStart   =  (uint64_t*)&c[FTM_CH_TSTART];
   uint32_t* pcMsgIdx   =  (uint32_t*)&c[FTM_CH_MSGIDX];
   uint32_t* pcRepCnt   =  (uint32_t*)&c[FTM_CH_REPCNT];
   uint32_t* pcFlags    =  (uint32_t*)&c[FTM_CH_FLAGS];
   uint32_t* pDebug     =  (uint32_t*)&((*pActProc)[FTM_PROC_DEBUG_DATA]);
   
   uint8_t** pcNext     =  (uint8_t**)&c[FTM_CH_PNEXT];
   uint8_t** pcMsg      =  (uint8_t**)&c[FTM_CH_PMSG];
   uint8_t** pcCon      =  (uint8_t**)&c[FTM_CH_PCON];

   
   //2. Check: Running and Chain PrepTime reached ? 
   if(now + TPrep >= *pcTStart) { // are we in the preparation phase for this chain's start time?

      //1. Is this chain conditional? Condition already fulfilled?
      pCur = condValid(c);
      if(pCur == NULL) return c; //conditions present but none met, wait
         
      //4. Check: signal to send ?
      sigSend(c);
      
      //5. Check: Got Msgs? If so, Msg PrepTime reached ?
      if(cMsgQty != 0) {
      
         pCurMsg = (uint8_t*)&((*pcMsg)[*pcMsgIdx * FTM_CH_END_]); // create pointer to current message
         *(uint64_t*)&pCurMsg[FTM_MSG_TS] = *pcTStart + *(uint64_t*)&pCurMsg[FTM_MSG_OFFS]; //set execution time for current msg 
         if( now + TPrep >= (uint64_t)pCurMsg[FTM_MSG_TS]) { //### time to hand it over to prio queue ? ###
            dbg_then = getSysTime();
            if(dispatch(pCurMsg)) {*pcMsgIdx++; (*(uint32_t*)&((*pActProc)[FTM_PROC_MSGCNT]))++;}
            dbg_now = getSysTime();
   
            dbg_dur = (uint32_t)(dbg_now-dbg_then);
            if(*pcMsgCnt == 0) dbg_sum = 0;
            else dbg_sum += (uint64_t)dbg_dur;
            if(pDebug[DBG_DISP_DUR_MIN] > dbg_dur) pDebug[DBG_DISP_DUR_MIN] = dbg_dur; //min
            if(pDebug[DBG_DISP_DUR_MAX] < dbg_dur) pDebug[DBG_DISP_DUR_MAX] = dbg_dur; //max
            pDebug[DBG_DISP_DUR_AVG] = dbg_sum/(*pcMsgCnt+1);
         };
      };
      
      //6. Check: All Msgs processed?
      if(*pcMsgIdx == cMsgQty) {
         *pcMsgIdx = 0;
         //a rep is complete, an update of the start time is necessary
         *(uint64_t*)&pCur[FTM_CH_TSTART] = *pcTStart + cTPeriod;
         if(cRepQty != -1) *pcRepCnt++;
         
         //7. Check: All Reps processed?
         if(*pcRepCnt == cRepQty) {
            *pcRepCnt = 0;
            //8. update chain ptr
            pCur = *pcNext;
         }
         //9. Update Chain (this or next) start time
                                 

      } // if msgs processed
   } // if chain preptime       
   
   //if the chain ptr changed, an update of the start time is necessary
   if(pCur != c) *(uint64_t*)&pCur[FTM_CH_TSTART] = *pcTStart + cTPeriod;
   
   return pCur;    
}

uint8_t* getPtrByID(uint32_t ID)
{
   
   uint32_t cpu         = (ID & CH_ID_CPU_MSK)   >> CH_ID_CPU_POS;
   uint32_t proc        = (ID & CH_ID_PROC_MSK)  >> CH_ID_PROC_POS;
   uint32_t plan        = (ID & CH_ID_PLAN_MSK)  >> CH_ID_PLAN_POS;
   
   uint32_t* procIdx    =  (uint32_t*)&pFtmIf[FTM_MIF_PROCIDX];
   uint32_t  procQty    = *(uint32_t*)&pFtmIf[FTM_MIF_PROCQTY];
   
   uint8_t** pActProc   =  (uint8_t**)&pFtmIf[FTM_MIF_PROCS + *procIdx * FTM_PROC_END_];
   uint8_t** pAct       =  (uint8_t**)&((*pActProc)[FTM_PROC_PACT]); 
   uint32_t  planQty    = *(uint32_t*)&((*pAct)[FTM_PAGE_PLANQTY]);
   uint8_t** pPlan      =  (uint8_t**)&((*pAct)[FTM_PAGE_PLANS]);   
   
   if(cpu  != getCpuIdx()) return NULL;
   if(proc >= procQty)     return NULL;
   if(plan >= planQty)     return NULL;
   
   return  (uint8_t*)&((*pPlan)[plan * 4]);  
}



