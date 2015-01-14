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
   *(pFpqCtrl + r_FPQ.tTrnHi)    = hiW(pFtmIf->tTrn);
   *(pFpqCtrl + r_FPQ.tTrnLo)    = loW(pFtmIf->tTrn);
   *(pFpqCtrl + r_FPQ.tDueHi)    = hiW(pFtmIf->tDue);
   *(pFpqCtrl + r_FPQ.tDueLo)    = loW(pFtmIf->tDue);
   
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
   pFtmIf = (t_ftmIf*)_startshared; 
   pFtmIf->status = 0x0;
   pFtmIf->pAct = (t_ftmPage*)&(pFtmIf->pPages[0]);
   pFtmIf->pIna = (t_ftmPage*)&(pFtmIf->pPages[1]);
   pFtmIf->idle = (t_ftmChain){ .tStart     = 0,
                                .tPeriod    = 5000,
                                .tExec      = 0,
                                .flags      = (FLAGS_IS_BP),
                                .condVal    = 0,
                                .condMsk    = 0,
                                .sigDst     = 0,
                                .sigVal     = 0,
                                .repQty     = -1,
                                .repCnt     = 0,
                                .msgQty     = 0,
                                .msgIdx     = 0,
                                .pMsg       = NULL,
                                .pNext      = NULL
                                };
   
   pFtmIf->pSharedMem   = (t_shared*)pSharedRam;
   pFtmIf->sema.sig     = 1;
   pFtmIf->sema.cond    = 1;
   pCurrentChain        = (t_ftmChain*)&pFtmIf->idle;
   pFtmIf->tPrep        = 100000/8;
   pFtmIf->tTrn         = 15000/8;
   pFtmIf->tDue         = 15000/8;
   prioQueueInit();
   
   pDebug[DBG_DISP_DUR_MIN] = 0xffffffff;
   pDebug[DBG_DISP_DUR_MAX] = 0x0;
   pDebug[DBG_DISP_DUR_AVG] = 0x0;
}

inline void showId()
{
   mprintf("#%02u: ", getCpuIdx()); 
}

void cmdEval()
{
   uint32_t cmd, stat;
   t_ftmPage* pTmp;
   
   cmd = pFtmIf->cmd;
   stat = pFtmIf->status;
   
   
   if(cmd)
   {
      
      
      if(cmd & CMD_RST)          { showId(); mprintf("Ftm Init done\n"); stat = 0; ftmInit(); }
      if(cmd & CMD_START)        { showId(); mprintf("Starting, IP: 0x%08x, SP: 0x%08x\n", &pFtmIf->idle, pFtmIf->pAct->pStart); 
                                   pFtmIf->pAct->pBp = pFtmIf->pAct->pStart;
                                   stat = (stat & STAT_ERROR) | STAT_RUNNING;
                                 }
      if(cmd & CMD_IDLE)         { pFtmIf->pAct->pBp = (t_ftmChain*)&pFtmIf->idle; showId(); mprintf("Going to Idle\n");}
      if(cmd & CMD_STOP_REQ)     { stat |= STAT_STOP_REQ; }
      if(cmd & CMD_STOP_NOW)     { stat = (stat & STAT_ERROR) & ~STAT_RUNNING; showId(); mprintf("Stopped (forced)\n");} 
      
      if(cmd & CMD_COMMIT_PAGE)  {//showId(); mprintf("Page Commit\n");
                                  pTmp = pFtmIf->pIna;
                                  pFtmIf->pIna = pFtmIf->pAct;
                                  pFtmIf->pAct = pTmp;
                                  pFtmIf->pAct->pBp = pFtmIf->pAct->pStart;
                                 }
      //if(cmd & CMD_COMMIT_BP)    {pFtmIf->pAct->pBp = pFtmIf->pNewBp;}
      
      if(cmd & CMD_DBG_0)        {showStatus();}
      if(cmd & CMD_DBG_1)        {showId(); mprintf("DBG1\n");}
      
      if(cmd & CMD_SHOW_ACT)     {  showId(); mprintf("Showing Active\n"); showFtmPage(pFtmIf->pAct);}
      if(cmd & CMD_SHOW_INA)     {  showId(); mprintf("Showing Inactive\n"); showFtmPage(pFtmIf->pIna);}
      
      //only zero the command reg if you found a command. otherwise this becomes race-condition-hell!
      pFtmIf->cmd = 0;                       
   }
   
   if(pCurrentChain == &pFtmIf->idle)  {stat |=  STAT_IDLE;}
   else                       {stat &= ~STAT_IDLE;}
   if(pCurrentChain == &pFtmIf->idle && (stat & STAT_STOP_REQ)) { stat = (stat & STAT_ERROR) & ~STAT_RUNNING; mprintf("Stopped\n");}
   
   pFtmIf->status = stat;
   
}



void showFtmPage(t_ftmPage* pPage)
{
   uint32_t planIdx, chainIdx, msgIdx;
   t_ftmChain* pChain  = NULL;
   t_ftmMsg*   pMsg  = NULL;
   
   mprintf("---PAGE %08x\n", pPage);
   mprintf("StartPlan:\t");
   
   if(pPage->pStart == &(pFtmIf->idle) ) mprintf("idle\n");
   else { 
          if(pPage->pStart == NULL) mprintf("NULL\n");
          else mprintf("%08x\n", pPage->pStart);
        } 
   
   mprintf("AltPlan:\t");
   if(pPage->pBp == &(pFtmIf->idle) ) mprintf("idle\n");
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
         if(pChain->flags & FLAGS_IS_BP) mprintf("-IS_BP\t");
         if(pChain->flags & FLAGS_IS_COND_MSI) mprintf("-IS_CMSI\t");
         if(pChain->flags & FLAGS_IS_COND_SHARED) mprintf("-IS_CSHA\t");
         if(pChain->flags & FLAGS_IS_SIG_SHARED) mprintf("-IS_SIG_SHARED");
         if(pChain->flags & FLAGS_IS_SIG_MSI)    mprintf("-IS_SIG_MSI");
         if(pChain->flags & FLAGS_IS_END) mprintf("-IS_END");
         if(pChain->flags & FLAGS_IS_ENDLOOP) mprintf("-IS_ENDLOOP");
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
         
   
         
         if(pChain->flags & FLAGS_IS_END) pChain = NULL;
         else pChain = (t_ftmChain*)pChain->pNext;
      }
           
   }
   uint64_t j;
  for (j = 0; j < (250000000); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }    
   
}

void showStatus()
{
   uint32_t stat = pFtmIf->status;
   mprintf("\f%08x\tStatus:\t", (uint32_t)(&(pFtmIf->cmd)) );
   if(stat & STAT_RUNNING) mprintf("\t\t-RUNNING"); else mprintf("\t\t-\t");
   if(stat & STAT_IDLE) mprintf("\t\t-IDLE"); else mprintf("\t\t-\n");
   if(stat & STAT_STOP_REQ) mprintf("\t\t-STOP_REQ"); else mprintf("\t\t-\t");
   if(stat & STAT_ERROR) mprintf("\t\t-ERROR"); else mprintf("\t\t-\t");
   mprintf("\t\tE:\t%x%08x", (uint32_t)(execCnt), (uint32_t)(execCnt>>32) );
   mprintf("\n");
   
}

inline int dispatch(void* pMsg)
{
   
   unsigned int diff;
   int ret = 1;

   diff = ( *(pFpqCtrl + r_FPQ.capacity) - *(pFpqCtrl + r_FPQ.heapCnt));
   if(diff > 1)
   {  
      //incIdSCTR(&pMsg->id, &pFtmIf->sctr); //copy sequence counter (sctr) into msg id and inc sctr
      atomic_on();
      *pFpqData = (uint32_t)pMsg[FTM_MSG_ID_HI];
      *pFpqData = (uint32_t)pMsg[FTM_MSG_ID_LO];
      #if DEBUGTIME == 1
      *pFpqData = hiW(then) | 0xff000000;
      *pFpqData = loW(then);
      #else
      *pFpqData = (uint32_t)pMsg[FTM_MSG_PAR_HI];
      *pFpqData = (uint32_t)pMsg[FTM_MSG_PAR_LO];
      #endif
      *pFpqData = (uint32_t)pMsg[FTM_MSG_TEF];
      *pFpqData = (uint32_t)pMsg[FTM_MSG_RES];
      *pFpqData = (uint32_t)pMsg[FTM_MSG_TS_HI];
      *pFpqData = (uint32_t)pMsg[FTM_MSG_TS_LO];
      atomic_off();
      
      *(uint32_t*)&pFtmIf[FTM_MIF_MSGCNT]++;
      
   } else {
      ret = 0;
      DBPRINT1("Queue full, waiting\n");
   }   

   return ret;

}

inline uint8_t condValid(void* c)
{
   uint32_t  cCondMsk   = (uin32_t)c[FTM_CHAIN_CONDMSK];
   uint32_t  cCondVal   = (uin32_t)c[FTM_CHAIN_CONDVAL] & cCondMsk; // mask right now
   uint64_t  TPrep      = (uin64_t)pFtmIf[FTM_MIF_TPREP];
   uint32_t* pcFlags    = (uin32_t*)&c[FTM_CHAIN_FLAGS];
   uint32_t* pcCondSrc  = (uin32_t*)c[FTM_CHAIN_PCONDSRC];
   uint64_t* pcTStart   = (uin64_t*)&c[FTM_CHAIN_TSTART];
   
   uint8_t ret = 0;
   
   if(*pcFlags & (FLAGS_IS_COND_MSI | FLAGS_IS_COND_SHARED | FLAGS_IS_COND_ADR) )
   {
      if(*pcFlags & FLAGS_IS_COND_MSI)
      {
         uint32_t  ip, msg;
         irq_disable();
         asm ("rcsr %0, ip": "=r"(ip)); //get pending irq flags
         if(ip & 1<<MSI_SIG)
         {
            irq_pop_msi(MSI_SIG);      //pop msg from msi queue into global_msi variable
            msg = global_msi.msg;
            irq_clear(1<<MSI_SIG);     //clear pending bit
         }      
         irq_enable();
         if(cCondVal == (msg & cCondMsk)) ret = 1;   
      }
      else
      {
         DBPRINT3("CPU %u Val: %08x Con: %08x Adr: %08x\n", getCpuID() , cCondVal,  *pcCondSrc, pcCondSrc );
         if(cCondVal == (*pcCondSrc & cCondMsk)) 
         {
            if(*pcFlags & FLAGS_IS_COND_TIME)   *pcTStart = *(uint64_t*)(pcCondSrc+1);
            else                                *pcTStart = getSysTime() + TPrep;
            *pcCondSrc  = 0;         
            ret         = 1;
            DBPRINT1("Val: %08x Src: %08x Adr: %08x\n", cCondVal,  *pcCondSrc, pcCondSrc );
         }
      }
   }
   else ret = 1;
   
   if(ret) {*(uint32_t*)&pFtmIf[FTM_MIF_STAT] &= ~STAT_WAIT; *pcFlags &= ~FLAGS_SEMA_CON;}    
   else    {*(uint32_t*)&pFtmIf[FTM_MIF_STAT] |=  STAT_WAIT; }
   
   return ret; 
} 

inline void sigSend(void* c)
{
   uint64_t  cTStart    =   (uin64_t)c[FTM_CHAIN_TSTART];
   uint64_t  cTPeriod   =   (uin64_t)c[FTM_CHAIN_TPERIOD];
   uint32_t* pcFlags    = (uin32_t*)&c[FTM_CHAIN_FLAGS];
   uint32_t* pcSigDst   = (uin32_t*)&c[FTM_CHAIN_SIGDST];
   uint32_t  cSigVal    =  (uin32_t)&c[FTM_CHAIN_SIGDST];
   
   *pcFlags &= ~FLAGS_SEMA_SIG; 
   *pcSigDst = cSigVal;
   if(*pcFlags & FLAGS_IS_SIG_TIME) *(uint64_t*)(pcSigDst+1) = cTStart + cTPeriod;
   
}

inline t_ftmChain* processChainAux(void* c)
{
   void* pCur, pCurMsg;
   
   uint64_t tMsgExec, now;

   uint64_t dbg_now, dbg_then; 
   uint32_t dbg_dur;

   DBPRINT2("Time to process Chain %08x reached\n", c);
   pCur  = c; 
   now   = getSysTime();
   
   //get everything that's constant to us by value
   uint64_t  TPrep      = (uin64_t)pFtmIf[FTM_MIF_TPREP];
   uint64_t  cTPeriod   = (uin64_t)c[FTM_CHAIN_TPERIOD];
   uint32_t  cMsgQty    = (uin32_t)c[FTM_CHAIN_MSGQTY];
   uint32_t  cRepQty    = (uin32_t)c[FTM_CHAIN_REPQTY];
   
   //everything else will be pointers if we use it often
   uint64_t* pcTStart   = (uin64_t*)&c[FTM_CHAIN_TSTART];
   uint32_t* pcMsgIdx   = (uin32_t*)&c[FTM_CHAIN_MSGIDX];
   uint32_t* pcRepCnt   = (uin32_t*)&c[FTM_CHAIN_REPCNT];
   uint32_t* pcFlags    = (uin32_t*)&c[FTM_CHAIN_FLAGS];
   uint32_t* pDebug     = (uin32_t*)&pFtmIf[FTM_MIF_DEBUG_DATA];
   
   //pointers to pointers
   void*     pcNext    = (void*)c[FTM_CHAIN_PNEXT];
   void*     pcMsg     = (void*)c[FTM_CHAIN_PMSG];
   void*     pIdle     = (void*)pFtmIf[FTM_MIF_PIDLE];
   void*     pBP       = (void*)(((void*)pFtmIf[FTM_MIF_PACT])[FTM_MIF_PBP]);
   
   
   if( now + TPrep >= *pcTStart) { // are we in the preparation phase for this chain's start time?
      //signal to send ?
      if((*pcFlags & FLAGS_SEMA_SIG) &&  (*pcFlags & (FLAGS_IS_SIG_MSI | FLAGS_IS_SIG_SHARED))) sigSend(c);

      if( *pcRepCnt < cRepQty || cRepQty == -1) //reps left ?  
      {
         if(*pcMsgIdx < cMsgQty) //msgs left to process?
         {
            pCurMsg = pcMsg + (*pcMsgIdx * FTM_CHAIN_END_); // create pointer to current message
            *(uint64_t*)&pCurMsg[FTM_MSG_TS] = *pcTstart + (uint64_t)pCurMsg[FTM_MSG_OFFS]; //set execution time for current msg 
            if( now + TPrep >= (uint64_t)pCurMsg[FTM_MSG_TS])  //### time to hand it over to prio queue ? ###
            {
               dbg_then = getSysTime();
               if(dispatch(pCurMsg)) *pcMsgIdx++;
               dbg_now = getSysTime();
      
               dbg_dur = (uint32_t)(dbg_now-dbg_then);
               if(msgCnt == 0) dbg_sum = 0;
               else dbg_sum += (uint64_t)dbg_dur;
               if(pDebug[DBG_DISP_DUR_MIN] > dbg_dur) pDebug[DBG_DISP_DUR_MIN] = dbg_dur; //min
               if(pDebug[DBG_DISP_DUR_MAX] < dbg_dur) pDebug[DBG_DISP_DUR_MAX] = dbg_dur; //max
               pDebug[DBG_DISP_DUR_AVG] = dbg_sum/(msgCnt+1);
               
            } else DBPRINT3("Too early for Msg %u", *pcMsgIdx);
         } 
         else // no msgs left. was this the last rep?
         {
//FIXME what the fuck... this is highly suspicious !!!
            if( (*pcFlags & FLAGS_IS_END) && (*pcFlags & FLAGS_IS_ENDLOOP)) 
            { 
               pCur        = pcNext;
               pcFlags*   |= (FLAGS_SEMA_SIG | FLAGS_SEMA_CON);
               DBPRINT1("Chain Loop to 0x%08x\n", pCur); 
            }
            else pCur = c;
//FIXME
            //this is fine
            *pcMsgIdx = 0; *pcRepCnt++; //repetions left, stay with this chain
            *(uin64_t*)&pCur[FTM_CHAIN_TSTART]         = *pcTStart + cTPeriod; 
            if(*pcFlags & FLAGS_IS_SIG_ALL)  pcFlags* |= FLAGS_SEMA_SIG;
            if(*pcFlags & FLAGS_IS_COND_ALL) pcFlags* |= FLAGS_SEMA_CON;
         }
      } 
      else
      {
         //done, go to next chain
//FIXME what the fuck... this is highly suspicious !!!         
         *pcMsgIdx = 0; *pcRepCnt = 0;
         //end of sequence? go to idle, else go to next
         if( ((*pcFlags & FLAGS_IS_END) && (*pcFlags & FLAGS_IS_ENDLOOP)) || (pcNext == NULL) ) pCur = pIdle;
         else                                                                                   pCur = pcNext;
//FIXME
         *(uin64_t*)&pCur[FTM_CHAIN_TSTART]   = *pcTStart + cTPeriod;   //next chain begins at current start plus current period
         pcFlags*                            |= (FLAGS_SEMA_SIG | FLAGS_SEMA_CON); //reset semaphores
      }
      
   }
   
   //is c a branchpoint? if so, jump, reset sequence counter (sctr), semaphores and BP
   if((*pcFlags & FLAGS_IS_BP) && (pBP != NULL))       
   { 
      pCur = pBP;  //BP? go to alt chain
      pBP  = NULL;
      
      *(uin32_t*)&pFtmIf[FTM_MIF_SCTR] = 0;
      pcFlags*    |= ( FLAGS_SEMA_SIG | FLAGS_SEMA_CON);
   }
    
   return pCur;    
}


inline t_ftmChain* processChain(t_ftmChain* c)
{
   t_ftmChain* pCur = c;
   t_time now = getSysTime();
   
   //if starttime is 0 or in the past, set to earliest possible time
   //   || c->tStart < now
   if ( !c->tStart ) {c->tStart = now + pFtmIf->tPrep; DBPRINT2("Adjust time\n#ST: %08x %08x \n TS: %08x %08x\n", now, c->tStart);}
   
   
   if( pcFlags* & FLAGS_SEMA_CON ) condValid(c);
   
   if(!pFtmIf->sema.cond) pCur = processChainAux(c); 
   else 
   {
      if((c->flags & FLAGS_IS_BP) && pBP != NULL)
      { 
         pCur = pFtmIf->pAct->pBp; 
         
         pFtmIf->sctr      = 0;
         pBP = NULL;
      } 
   }
      
   return pCur;    
}


void processFtm()
{
   DBPRINT3("c = %08x\n", pCurrentChain);
   if (pFtmIf->status & STAT_RUNNING) { pCurrentChain = processChain(pCurrentChain); execCnt++;} 
}

