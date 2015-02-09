#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include "mini_sdb.h"
#include "mprintf.h"
#include "aux.h"
#include "ftm.h"
#include "dbg.h"

volatile unsigned int* pEC;
volatile unsigned int* pFlush = (volatile unsigned int*)0x40000900;

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

#if CONCLICTS == 1
  #define MSK 0x3
#else
  #define MSK 0x1
#endif


char buffer[12];
 
static char* u64toStr(char* s, uint64_t val, uint8_t len)
{
   const unsigned long long divStart = 10000000000000000000ULL;
   memset(s, 0x0, len);
   char*    p     = s;
   char*    end   = p + (uintptr_t)len -1;
   uint64_t tmp   = val;
   uint64_t div   = (uint64_t)divStart;
   
   while(p <= end && div) {
      *p++   = (char)(tmp / div + '0');
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
       val = -val;
   } else *p++ = ' ';    
   u64toStr(p, (uint64_t)val, len);     
   return s;
}

static char* milPt(char* out, int64_t val)
{
   uint8_t len = 21;
   memset(out, ' ', len+3);
   char buf[len]; 
   char* s = &buf[0];
   i64toStr(s, val, len);
   char*  end = s + (uintptr_t)len -1;
   char*  p   = end;
   char* po   = out + (uintptr_t)len + 3 -1;

  
   while(p >= end-9) {
      if ((end - p) && !((end - p) % 3)) {
      // insert point
         *po-- = '.';
         *po-- = *p--;
      }
      else  *po-- = *p--; //copy 
   }
   
   while(p >= s) {
     *po-- = *p--;   
   }
   
     
   return out;
}

static char* remLz(char* s)
{
   char*  p   = s;
   char*  e   = p+1;
   uint8_t lz = 1;
  
   while(*e) {
      if (lz) {
         if((*p == '.') || (*p == '0') || (*p == ' ')) *p++ = ' ';
         else lz = 0; 
      }
      else  p++;
      e   = p+1;
   }
   return s;
}


void report(uint64_t now, uint32_t actCnt, uint32_t errCnt)
{
   const uint32_t bufSize = 24;
   char buf[bufSize];
      
 
   //acquire info on offending evt
   uint64_t id    = ((uint64_t)*(pEC + (ECA_EVT_ID_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_ID_LO >> 2));
   uint64_t par   = ((uint64_t)*(pEC + (ECA_EVT_PA_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_PA_LO >> 2));
   uint64_t time  = (((uint64_t)*(pEC + (ECA_EVT_TI_HI >> 2))) << 32 | (uint64_t)*(pEC + (ECA_EVT_TI_LO >> 2))) << 3;
   uint64_t flush = (((uint64_t)*(pFlush + (0 >> 2))) << 32 | (uint64_t)*(pFlush + (4 >> 2))) << 3;
 
   mprintf("################## %u/%u ##################", errCnt, actCnt);
   if( *(pEC + (ECA_FLAGS  >> 2)) & 0x01 ) mprintf("late ");
   if( *(pEC + (ECA_FLAGS  >> 2)) & 0x02 ) mprintf("conflict ");
   mprintf("\n");
   mprintf("id:\t0x%08x%08x\nFID:\t%u\nGID:\t%u\nEVTNO:\t%u\nSID:\t%u\nBPID:\t%u\n", 
   (uint32_t)(id>>32), (uint32_t)id, getIdFID(id), getIdGID(id), getIdEVTNO(id), getIdSID(id), getIdBPID(id));
   
   
   mprintf("par:\t0x%08x%08x\n", (uint32_t)(par>>32), (uint32_t)par);
   mprintf("tef:\t\t0x%08x\ntag:\t\t0x%08x\n", *(pEC + (ECA_EVT_TEF >> 2)), *(pEC + (ECA_EVT_TAG >> 2)) );
   mprintf("                   s   ms  us  ns\n");

   
   remLz(milPt(buf, now));
   //u64toStr(buf, now, bufSize);
   mprintf("now:     %s\n", buf);  
   //u64toStr(buf, time, bufSize);
   remLz(milPt(buf, time));
   mprintf("ts:      %s\n",  buf);
   // does par start with 0xff? if so, then 
   if((par >> 56) == 0xff) {
      uint64_t tsdbg = ((par & (~(0xffULL<<56)))<<3);
      // this is a debug timestamp
      // strip the leading 0xff and convert to decimal
      //u64toStr(buf, tsdbg, bufSize);
      remLz(milPt(buf, tsdbg));
      mprintf("tsdsp:   %s\n",  buf);
//      mprintf("flushptr: %08x val %8x%08x\n", pFlush, *(pFlush+1), *pFlush);	i
//      remLz(milPt(buf, flush));
//      mprintf("flush:   %s\n",  buf); 	
      //i64toStr(buf, ((int64_t)(now) - (int64_t)(tsdbg)), bufSize);
      remLz(milPt(buf, ((int64_t)(now) - (int64_t)(tsdbg))));
      mprintf("now-dsp: %s\n", buf);
      remLz(milPt(buf, ((int64_t)(time) - (int64_t)(tsdbg))));
      mprintf("ts-dsp:  %s\n", buf);
      remLz(milPt(buf, ((int64_t)(time) - (int64_t)(flush))));
      mprintf("ts-flush:  %s\n", buf);	    
   }
  
   
   
   remLz(milPt(buf, ((int64_t)(now) - (int64_t)(time))));
   //i64toStr(buf, ((int64_t)(now) - (int64_t)(time)), bufSize);
   mprintf("now-ts:  %s\n", buf); 
   
 }

void init()
{

   discoverPeriphery();
   pEC = find_device_adr(GSI, ECA_ACTION);
   
   uart_init_hw();
   uart_write_string("\nDebug Port\n");
 
   mprintf("\fCore #%u scanning ECA @ 0x%08x for late arrivals" , getCpuIdx(), (uint32_t)pEC);
#if CONFLICTS == 1
   mprintf("/ conflicts\n");
#else
   mprintf("\n");
#endif	
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
        if( *(pEC + (ECA_FLAGS  >> 2)) & MSK ) {    //  check for errors, if so, gather info on offending event and printf preport
          errCnt++;
          report(now<<3, actCnt, errCnt);
        }
        *(pEC + (ECA_ACT_CTL >> 2)) = ECA_CMD_POP; //... pop the element
     }
  }
}
