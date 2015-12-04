#ifndef _HW_REGS_
#define _HW_REGS_

#include "prio_regs.h"

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

#define PPS_CNTR_UTCLO 0x8
#define PPS_CNTR_UTCHI 0xC
#define PPS_STATE      0x1C
#define PPS_VALID      0x2
#define TS_VALID      0x4

#endif
