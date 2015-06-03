#include <stdio.h>
#include "thread.h"





t_data data[10] = {
  {.b = {125, 10000, BLOB_PPS}},
  {.m = {0xcafebabedeadbee1, 0x0123456789abcdef, 5, 500}},
  {.s = {0, (void*)0xd57, 0xbabe, 0}}, //signal immediately to 0xd57/4 babe
  {.b = {125, 10000, BLOB_PT}},
  {.m = {0xcafebabedeadbee1, 0x0123456789abcdef, 5, 9000}},
  {.b = {125, 50000, BLOB_PT}},
  {.c = {CON_ACT_CLR, (void*)0x59c, 0x4712, 0}},
  {.s = {SIG_DST_64, (void*)0xd5764, 0x0123456789abcdef, 9200}}, //signal at blob offs 9200 to 0xd57/4 cafe
  {.c = {CON_SRC_STO, 0, 0, 0}},
};



t_type types[10] = {
  T_BLB,
  T_MSG,
  T_SIG,
  T_BLB,
  T_MSG,
  T_BLB,
  T_CON,
  T_CON,
  T_SIG
};

/// Generic Stuff --->

extern  uint32_t thread();
extern  uint32_t init();
extern  uint32_t dump();
extern  uint8_t gotTime(uint32_t cost);
extern  uint64_t inline getSysTime();
extern  void procBlob(t_data* data);
extern  void sendSig(t_data* data);
extern  int sendMsg(t_data* data);
extern  void checkCon(t_data* data, void* lbl);



 __attribute__((visibility("hidden")))
uint32_t entry(char exec, uint32_t dl) { //instruction count deadline
  uint32_t ret = 0;
  deadline = dl;
  switch (exec) {
    case F_INIT:    ret = init(); break;
    case F_THREAD:  ret = thread(); break;
    case F_DUMP:    ret = dump(); break;
    default: ret = thread();
  }
  return ret;
}  
