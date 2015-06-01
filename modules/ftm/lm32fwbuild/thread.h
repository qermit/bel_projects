#ifndef _THREAD_H_
#define _THREAD_H_
#include <inttypes.h>
#include <stdint.h>
#include "dmif.h"



//***types
typedef uint8_t t_type;

#define T_BLB     0x1
#define T_CON     0x2
#define T_SIG     0x4
#define T_MSG     0x8

#define BLOB_PT   0x1
#define BLOB_CT   0x2
#define BLOB_PPS  0x3

typedef struct {
   uint64_t start; //8
   uint64_t period; //8
   uint32_t mode; //4
} t_blob;


typedef struct {
   uint16_t   flg; //1
   void*      src; //4
   uint64_t   val; //8
   uint64_t   sto; //4
} t_con;


#define CON_SRC_STO   0x01
#define CON_SRC_64    0x02
#define CON_ACT_INC   0x40
#define CON_ACT_CLR   0x80

typedef struct {
   uint32_t flg;  //4
   void*    dst;  //4
   uint64_t val;  //8
   uint64_t offs; //8
} t_sig;


#define SIG_DST_64 0x01

typedef struct {
   uint64_t id;   //8
   uint64_t par;  //8
   uint32_t tef;  //4
   uint64_t offs; //8
} t_msg;


typedef union {
  t_blob b;
  t_con  c;
  t_sig  s;
  t_msg  m;
} t_data;

//***reference to globals
extern uint64_t sT; //cumulative blob start time
extern void*   pL;  //current label
extern t_blob* pB;  //current blob ptr
extern uint32_t** pBase; //base pointer to main fw. periphery ptrs, cmd if, etc
extern uint32_t deadline; //cycle count deadline for thread execution

//***helper functions

#define T_DUE(o) (sT + o) //preptime starttime offset
#define IS_DUE(tp) (getSysTime() > tp ? (0) : (1)) 

static inline uint32_t hiW(uint64_t dword) {return (uint32_t)(dword >> 32);}
static inline uint32_t loW(uint64_t dword) {return (uint32_t)dword;}

static inline uint64_t getSysTime()
{
   uint64_t systime;  
   systime =  ((uint64_t)*(pBase[DEV_TIME]+0))<<32;
   systime |= ((uint64_t)*(pBase[DEV_TIME]+1)) & 0x00000000ffffffff;
   return systime;  
}


static  uint8_t gotTime(uint32_t cost) {
  // global <deadline> holds deadline cycle counter value
  uint32_t cc, diff;
  asm ("rcsr %0, cc": "=r"(cc)); //get cycle counter
  
  if(cc > deadline)     diff = (deadline - (0xffffffff - cc)); //handle overflow
  else                  diff =  deadline - cc;
  return  (diff > cost); //inside cycle budget?
}

//*** the four horsemen: Signal, Condition, Message & Blob ****************************************

//FIXME: inline me again
static   void sendSig(t_data* d)
{
  t_sig* s = &d->s;
  if (s->offs) {
    if(!IS_DUE(T_DUE(s->offs))) return;
  }
  
  if(s->flg & SIG_DST_64) *(uint64_t*)s->dst   = s->val;
  else                   *((uint32_t*)s->dst)  = (uint32_t)s->val;
}

//FIXME: inline me again
static inline void checkCon(t_data* d, void* lbl)
{
  t_con* c = &d->c;
  if(c->flg & CON_SRC_STO) c->src = (void*)&c->sto;
   
  if(c->flg & CON_SRC_64) {
    if (*(uint64_t*)c->src == (uint64_t)c->val) {
      if(c->flg & CON_ACT_INC) *(uint64_t*)c->src = *(uint64_t*)c->src +1; //Action
      if(c->flg & CON_ACT_CLR) *(uint64_t*)c->src = 0;
      goto *lbl; //JUMP
    }
  } else {
    if (*(uint32_t*)c->src == (uint32_t)c->val) {
      if(c->flg & CON_ACT_INC) *(uint32_t*)c->src = *(uint32_t*)c->src +1; //Action
      if(c->flg & CON_ACT_CLR) *(uint32_t*)c->src = 0;
      goto *lbl; //JUMP
    }
  }
}

//FIXME: inline me again
static  int sendMsg(t_data* d)
{
  int     ret = 0;
  t_msg*  m   = &d->m;
  
  if(IS_DUE(T_DUE(m->offs))) { //preptime starttime offset
    
    //atomic_on();
    if (*pBase[DEV_FPQ_NOTFULL]) {
      ret = 1;
      *pBase[DEV_FPQ] = hiW(m->id);
      *pBase[DEV_FPQ] = loW(m->id);
      *pBase[DEV_FPQ] = hiW(m->par);
      *pBase[DEV_FPQ] = loW(m->par);
      *pBase[DEV_FPQ] = m->tef;
      *pBase[DEV_FPQ] = 0;
      *pBase[DEV_FPQ] = hiW(sT + m->offs);
      *pBase[DEV_FPQ] = loW(sT + m->offs);
      *pBase[REG_MSG_CNT] += ret;
    }
    //atomic_off();
  } 
  
  return ret;
  
}

//FIXME: inline me again
static   void procBlob(t_data* d)
{
   
   //switch(d->b.mode) {
//    case BLOB_PT  : sT += pB->period;                 break;
   sT += pB->period;
   // case BLOB_CT  : sT  = pB->period  + getSysTime(); break;
   // case BLOB_PPS : sT  = pB->period; break; //  + getNextPPS(); break;
   // default       : sT  = pB->period  + getSysTime();
   //}
   pB = &d->b;
}

#endif
 
