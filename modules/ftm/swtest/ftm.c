typedef struct dw {
   unsigned int hi;
   unsigned int lo;
};


typedef union dword {
   unsigned long long v64;
                 dw   v32;               
};

typedef struct ftmMsg {
   dword id;
   unsigned int res;
   unsigned int tef;
   dword par;
   dword ts;
};


ftmMsg* addFtmMsg(eb_address_t eca_adr, ftmMsg* pMsg)
{
   atomic_on();   
   eb_op(eca_adr, pMsg->id.v32.hi,  WRITE);
   eb_op(eca_adr, pMsg->id.v32.lo,  WRITE);
   eb_op(eca_adr, pMsg->res,        WRITE);
   eb_op(eca_adr, pMsg->tef,        WRITE);
   eb_op(eca_adr, pMsg->par.v32.hi, WRITE);
   eb_op(eca_adr, pMsg->par.v32.lo, WRITE);
   eb_op(eca_adr, pMsg->ts.v32.hi,  WRITE);
   eb_op(eca_adr, pMsg->ts.v32.lo,  WRITE);
   atomic_off();   
   return pMsg;
}

void sendFtmMsgPacket()
{
   ebm_flush();
}
