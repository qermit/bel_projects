#define EBM_REG_CLEAR         0                         
#define EBM_REG_FLUSH         (EBM_REG_CLEAR        +4)        
#define EBM_REG_STATUS        (EBM_REG_FLUSH        +4)         
#define EBM_REG_SRC_MAC_HI    (EBM_REG_STATUS       +4)       
#define EBM_REG_SRC_MAC_LO    (EBM_REG_SRC_MAC_HI   +4)    
#define EBM_REG_SRC_IPV4      (EBM_REG_SRC_MAC_LO   +4)    
#define EBM_REG_SRC_UDP_PORT  (EBM_REG_SRC_IPV4     +4)   
#define EBM_REG_DST_MAC_HI    (EBM_REG_SRC_UDP_PORT +4)  
#define EBM_REG_DST_MAC_LO    (EBM_REG_DST_MAC_HI   +4)   
#define EBM_REG_DST_IPV4      (EBM_REG_DST_MAC_LO   +4)  
#define EBM_REG_DST_UDP_PORT  (EBM_REG_DST_IPV4     +4)   
#define EBM_REG_MTU           (EBM_REG_DST_UDP_PORT +4)  
#define EBM_REG_ADR_HI        (EBM_REG_MTU          +4)    
#define EBM_REG_OPS_MAX       (EBM_REG_ADR_HI       +4) 
#define EBM_REG_EB_OPT        (EBM_REG_OPS_MAX      +4) 
#define EBM_REG_LAST          (EBM_REG_EB_OPT)



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
} r_FPQ = {    .rst        =  0x00,
               .force      =  0x04,
               .dbgSet     =  0x08,
               .dbgGet     =  0x0c,
               .clear      =  0x10,
               .cfgGet     =  0x14,
               .cfgSet     =  0x18,
               .cfgClr     =  0x1C,
               .dstAdr     =  0x20,
               .heapCnt    =  0x24,
               .msgCntO    =  0x28,
               .msgCntI    =  0x2C,
               .tTrnHi     =  0x30,
               .tTrnLo     =  0x34,
               .tDueHi     =  0x38,
               .tDueLo     =  0x3C,
               .capacity   =  0x40,
               .msgMax     =  0x44,
               .ebmAdr     =  0x48,
               .tsAdr      =  0x4C,
               .tsCh       =  0x50,
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
