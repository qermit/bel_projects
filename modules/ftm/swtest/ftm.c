#include "ftm.h"

ftmMsg* addFtmMsg(eb_address_t eca_adr, ftmMsg* pMsg)
{
   atomic_on();   
   eb_op(eca_adr, pMsg->id.v32.hi,  WRITE);
   eb_op(eca_adr, pMsg->id.v32.lo,  WRITE);
   eb_op(eca_adr, pMsg->par.v32.hi, WRITE);
   eb_op(eca_adr, pMsg->par.v32.lo, WRITE);
   eb_op(eca_adr, pMsg->res,        WRITE);
   eb_op(eca_adr, pMsg->tef,        WRITE);
   eb_op(eca_adr, pMsg->ts.v32.hi,  WRITE);
   eb_op(eca_adr, pMsg->ts.v32.lo,  WRITE);
   atomic_off();   
   return pMsg;
}

void sendFtmMsgPacket()
{
   ebm_flush();
}

fesa_init()
{ fesaFtmIf = (fesaFtmIf*) }


void ISR_timer()
{
   updateExecTimes();
   
   if(commit) {
      udpateDataSrc();
      updateTimers();
   }
   
   if( (global_msi.msg & (TIMER_CYC_START | TIMER_CYC_PREP)) ) 
      if (pPageAct->cycles[cycleSel].info & CYC_ACTIVE)
      && (pPageAct->cycles[cycleSel].repCnt <= pPageAct->cycles[cycleSel].rep) 
         pPageAct->cycles[cycleSel].repCnt++;
   } else {
   
   }
         
}

void ISR_Cmd()
{

}

void processDueMsgs()
{
   t_ftmCycle* cyc = pPageAct->cycles[cycleSel];
   unsigned char dueIdx = cyc->procMsg;
   bool  dispatch = false;
   
   if( ((c->rep == -1) || (c->rep > c->repCnt)) && (cyc->info & CYC_ACTIVE) )
   {
      t_due = cyc->msgs[cyc->procMsg].ts;
      if(get_system_time() >= tDue - (cyc->tMargin + cyc->tTrn) ) // 
      
      for(dueIdx = cyc->procMsg; dueIdx <  cyc->qtyMsgs; dueIdx++)
      {
         if ( (tDue + tProc > cyc->msgs[dueIdx].ts) // diff between msgs less than time to process...
         ||   (cyc->msgs[dueIdx].ts + tProc >= cyc->tExec + cyc->tPeriod) ) // or diff to cycle end less than time to process? 
         {
            //dispatch msg
            addFtmMsg( , cyc->msgs+dueIdx); 
            dispatch = true;
            tDue = cyc->msgs[dueIdx].ts;
            updateMsgExecTime(cyc->msgs + dueIdx, cyc); 
         } else {
         
            if(dispatch) sendFtmMsgPacket();
            nextIdx = (dueIdx == cyc->qtyMsgs-1) 
            {
               if(cycleSwap || pageSwap) 
               updateCycExecTime(pPageAct->cycles + cycleSel);
               
            } ? 0 : dueIdx+1;
            cyc->procMsg = nextIdx;
         } 
      }
   } 
}


inline void updateCycExecTime(t_ftmCycle* c)
{
   if((c->rep == -1) || (c->rep > c->repCnt)) c->tExec = c->tStart + c->repCnt * c->tPeriod; 
}


inline void updateMsgExecTime(t_ftmMsg* m, t_ftmCycle* c)
{
   m->ts = c->tExec + m->offs;
}



t_status setMsgTimer(t_time tDeadline, unsigned int timerIdx)
{
   t_timer  tm;
  
   if (getSysTime() + tProc > tDeadline) return TIMER_CFG_ERROR_0;
   
   tm.mode      = TIMER_1TIME;
   tm.src       = TIMER_ABS_TIME;  
   tm.cascade   = TIMER_NO_CASCADE;
   tm.deadline  = tDeadline;
   tm.msi_dst   = 0;
   tm.msi_msg   = (unsigned int)*mg;
 
   atomic_on();
   irq_tm_write_config(timerIdx, tm);
   irq_tm_set_arm(1<<timerIdx);
   atomic_off();
   
   return TIMER_CFG_SUCCESS;
   
}




t_status setCycleTimer(t_ftmCycle* cyc)
{
   t_time   tPrep, tExec, tPeriod;
   t_timer  tm0, tm1;
      
   //timer0
   //calculate due time for start
   tExec = cyc->tStart;
   tPrep = cyc->tMargin + cyc->tTrn;
   
   if (getSysTime() + tPrep > tExec) return TIMER_CFG_ERROR_0;
   
   tm0.mode      = TIMER_1TIME; 
   tm0.src       = TIMER_ABS_TIME; //absolute or relative value
   tm0.cascade   = TIMER_NO_CASCADE;
   tm0.deadline  = tExec - tPrep;
   tm0.msi_dst   = 0;
   tm0.msi_msg   = TIMER_CYC_PREP;
   
   // if the cycle duration is shorter than processing time, pack multiple cycles in one packet
   if(cyc->tPeriod > tProc) { tPeriod = cyc->tPeriod; factor = 1;}
   else  {  factor = tProc/cyc->tPeriod + ((tProc % cyc->tPeriod) ? 1 : 0);
            tPeriod = factor * cyc->tPeriod;     
   }                        
   //timer1
   //cascade to Timer1, periodic with cycle period
   tm1.mode      = TIMER_PERIODIC;
   tm1.src       = TIMER_REL_TIME;
   tm1.cascade   = TM_0;
   tm1.deadline  = tPeriod;
   tm1.msi_dst   = 0;
   tm1.msi_msg   = TIMER_CYC_START;
 
   atomic_on();
   irq_tm_write_config(0, tm0);
   irq_tm_write_config(1, tm1);
   irq_tm_set_arm(1<<TM_1 | 1<<TM_0);
   atomic_off();
   
   return TIMER_CFG_SUCCESS;
}



void fesaCmdEval()
{
   unsigned int cmd;   
   
   cmd         = fesaFtmIf->cmd;
   
   if(cmd)
   {
      if(cmd & CMD_RST)       ftmInit();
      if(cmd & CMD_FTM_RUN) 
      if(cmd & (CMD_PAGESWAP | CMD_PAGESWAP_I)
      {
         ftmRun = (pFesaFtmIf->status & FTM_RUNNING);
         swap |= ( (pPageAct->cycles[cycleSel].status   & CYC_ACTIVE) && !(cmd & CMD_PAGESWAP_I)   && ftmRun) <<0;
         swap |= ( (pPageInAct->cycles[cycleSel].status & CYC_ACTIVE)                              && ftmRun) <<1;
         
         switch (swap) {
            case 0   : //nothing running right now and nothing to run on other page. toggle page index right now
                        break;   
            case 1   : //something running right now but nothing to run on other page, schedule swap to cycle end
                        break;
            case 2   : //immeditate swap or nothing running right now and something to run other page, standard setup
                        break;
            case 3   : //something running right now and something to run on other page, schedule swap to meet both cycle constraints
                        break;
            default  :  break;
         }
         //update page pointers
         pPageAct    = fesaFtmIf->page[pageSel & 1];
         pPageInAct  = fesaFtmIf->page[~pageSel & 1];   
      }//if pageswap
      
      
         
   }
}





