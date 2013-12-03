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



unsigned int 


//unsigned int sendMsgIdxHigh(t_ftmCycle* cyc

void setupFtmCycMsgTimer(t_time tOffset, int cycTimerIdx, unsigned int msiMsg)
{
   //make sure tOffset < tPeriod of cycle !!!
   
   //timer timerIdx
   //cascade to Timer timerIdx
   tm1.mode      = TIMER_1TIME;
   tm1.src       = TIMER_REL_TIME;  //Cycle Messages send times are relative to their cycle start
   tm1.cascade   = timerIdx;        //cascade the message timer to the cycle timer
   tm1.deadline  = tOffset;
   tm1.msi_dst   = 0;
   tm1.msi_msg   = TIMER_MSG_PREP_MSK | (msiMsg & 0xff);
 
   atomic_on();
   irq_tm_write_config(timerIdx, tm);
   irq_tm_set_arm(1<<timerIdx);
   atomic_off();
   
}

t_time getMsgSendOffset(t_ftmCycle* cyc, unsigned int ftmMsgIdx)
{
   tExec = cyc->msg[ftmMsgIdx].tOffs;
   tPrep = cyc->tMargin + cyc->tTrn;
   
   if(tExec < tPrep) return 0; //too close to cycle start, must be sent with cycle start
   else return tExec - tPrep;
}






t_status setupFtmCycleTimer(t_ftmCycle* cyc)
{
   t_time   tPrep, tExec, tSend, tPeriod;
   t_timer  tm0, tm1;
      
   //timer0
   //calculate transmission time for start
   tExec = cyc->tExec;
   tPrep = cyc->tMargin + cyc->tTrn;
   
   //if abs time, check if we can actually make it
   if(cyc->info & CYC_ABS_TIME)
   {if (getSysTime() + tPrep > tExec) return TIMER_CFG_ERROR_0;}
   
   tSend = tExec - tPrep;
   //
   tm0.mode      = TIMER_1TIME; 
   tm0.src       = (bool)(cyc->info & CYC_TIME_TYPE); //absolute or relative value
   tm0.cascade   = TIMER_NO_CASCADE;
   tm0.deadline  = tSend;
   tm0.msi_dst   = 0;
   tm0.msi_msg   = TIMER_CYC_PREP | CYC_0;
   
   // if the cycle duration is shorter than preparation time, pack multiple cycles in one packet
   if(cyc->tPeriod > tPrep) { tPeriod = cyc->tPeriod; factor = 1;}
   else  {  factor = tPrep/cyc->tPeriod + ((tPrep % cyc->tPeriod) ? 1 : 0);
            tPeriod = factor * cyc->tPeriod;     
   }                        
   //timer1
   //cascade to Timer1, periodic with cycle period
   tm1.mode      = TIMER_PERIODIC;
   tm1.src       = TIMER_REL_TIME;
   tm1.cascade   = TM_0;
   tm1.deadline  = tPeriod;
   tm1.msi_dst   = 0;
   tm1.msi_msg   = TIMER_CYC_START | CYC_0 | factor;
 
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


