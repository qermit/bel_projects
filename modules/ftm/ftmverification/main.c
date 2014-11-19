#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include "mini_sdb.h"
#include "aux.h"
#include "ftm.h"
#include "dbg.h"

volatile unsigned int* pEC;


#define ECA_ACTION      0x9bfa4560

#define ECA_ACT_CTL     0x00 // RW : Control
#define ECA_CMD_POP     0x1
#define ECA_QUEACT      0x10 // R : queued actions
#define ECA_FLAGS       0x1C // R : flags
#define ECA_EVT_ID_HI   0x20
#define ECA_EVT_ID_LO   0x24
#define ECA_EVT_PA_HI   0x28
#define ECA_EVT_PA_LO   0x2C
#define ECA_EVT_TAG     0x30
#define ECA_EVT_TEF     0x34
#define ECA_EVT_TI_HI   0x38
#define ECA_EVT_TI_LO   0x3C


char buffer[12];
 
static char* u64toStr(char* s, uint64_t val, uint8_t len)
{
   const unsigned long long divStart = 10000000000000000000ULL;
   memset(s, 0x0, len);
   char*    p     = s;
   uint64_t tmp   = val;
   uint64_t div   = (uint64_t)divStart;
   uint8_t  idx;
   
   for (idx = 0; idx<(len-1); idx++) {
      p[idx]   = (char)(tmp / div + '0');
      tmp     %= div;
      div     /= 10;
   }
   return s;
}


static char* i64toStr(char* s, int64_t val, uint8_t len)
{
   
   char*    p  = s;
   
   if (val < 0) {
       *p++ = '-';
       len--;
   }    
   u64toStr(p, (uint64_t)val, 21);     
   return s;
}



void report(uint64_t now, uint32_t actCnt, uint32_t errCnt)
{
   const uint32_t bufSize = 21;
   char buf[bufSize];
   
   //acquire info on offending evt
   uint64_t id    = ((uint64_t)*(pEC + (ECA_EVT_ID_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_ID_LO >> 2));
   uint64_t par   = ((uint64_t)*(pEC + (ECA_EVT_PA_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_PA_LO >> 2));
   uint64_t time  = (((uint64_t)*(pEC + (ECA_EVT_TI_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_TI_LO >> 2))) << 3;
    
   mprintf("################## %x/%x ##################", errCnt, actCnt);
   if( *(pEC + (ECA_FLAGS  >> 2)) & 0x01 ) mprintf("late ");
   if( *(pEC + (ECA_FLAGS  >> 2)) & 0x02 ) mprintf("conflict ");
   mprintf("\n");
   mprintf("id:\t0x%08x%08x\nFID:\t%u\nGID:\t%u\nEVTNO:\t%u\nSID:\t%u\nBPID:\t%u\n", 
   (uint32_t)(id>>32), (uint32_t)id, getIdFID(id), getIdGID(id), getIdEVTNO(id), getIdSID(id), getIdBPID(id));
   
   
   mprintf("par:\t0x%08x%08x\n", (uint32_t)(par>>32), (uint32_t)par);
   // does par start with 0xff? if so, then 
   if((par >> 56) == 0xff) {
      // this is a debug timestamp
      // strip the leading 0xff and convert to decimal
      u64toStr(buf, par & (~(0xffULL<<56)), bufSize);
      mprintf("tsdgb:\t%s\n",  buf);   
   }
   mprintf("tef:\t\t0x%08x\ntag:\t\t0x%08x\n", 
    *(pEC + (ECA_EVT_TEF >> 2)), *(pEC + (ECA_EVT_TAG >> 2)) );
   
   u64toStr(buf, time, bufSize);
   mprintf("ts:\t%s\n",  buf);
   
   u64toStr(buf, now, bufSize);
   mprintf("now:\t%s\n", buf, (uint32_t)(now>>32), (uint32_t)now);
   
   i64toStr(buf, ((int64_t)(now) - (int64_t)(time)), bufSize);
   mprintf("dif:\t%s\n", buf); 
   
 }

void init()
{

   discoverPeriphery();
   pEC = find_device_adr(GSI, ECA_ACTION);
   
   uart_init_hw();
   uart_write_string("\nDebug Port\n");
 
   mprintf("\fCore #%u scanning ECA @ 0x%08x for late arrivals / conflicts\n", getCpuIdx(), (uint32_t)pEC); 
}

void main(void) {


  int j;
  uint64_t now;
  uint32_t data;
  uint32_t errCnt, actCnt; 
  
  init();
  
  for (j = 0; j < (125000000/4); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }
  
  
  while (1) {
  
     now = getSysTime(); 
     if( *(pEC + (ECA_QUEACT >> 2))) { // if there is stuff in the action queue ...
        actCnt++;
        if( *(pEC + (ECA_FLAGS  >> 2)) ) {    // check for errors, if so, gather info on offending event and printf preport
          errCnt++;
          report(now<<3, actCnt, errCnt);
        }
        *(pEC + (ECA_ACT_CTL >> 2)) = ECA_CMD_POP; //... pop the element
     }
  }
}
