
#include "thread.h"

uint32_t** pBase = (uint32_t**)BASEPTR;



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
uint64_t sT = 0;
void*   pL;
t_blob* pB;
uint32_t deadline;

extern  uint8_t gotTime(uint32_t cost);
extern  uint64_t inline getSysTime();
extern  void procBlob(t_data* data);
extern  void sendSig(t_data* data);
extern  int sendMsg(t_data* data);
extern  void checkCon(t_data* data, void* lbl);


__attribute__((visibility("hidden")))
uint32_t entry(uint32_t dl) { //instruction count deadline
  deadline = dl;
  goto *pL;
  start:
/// ---> Generic Stuff
  
  //********** Schedule **********// 
  
  //do Action A with data X
  label1:  
  pL = &&label1;
  if (!gotTime(COST_BLB)) return T_BLB;
  procBlob(&data[0]);
  
  label2:
  pL = &&label2;
  if (!gotTime(COST_MSG)) return T_MSG;
  if (!sendMsg(&data[1])) return T_MSG;
    
  //do Action A with data Y
  label3:
  pL = &&label3;
  if (!gotTime(COST_SIG)) return T_SIG;
  sendSig(&data[2]);
  
  //do Action B with data Z
  label4:
  pL = &&label4; //remember where we were
  if (!gotTime(COST_BLB)) return T_BLB;
  procBlob(&data[3]);
  
  label5:
  pL = &&label5; //remember where we were
  if (!gotTime(COST_MSG)) return T_MSG;
  if (!sendMsg(&data[4])) return T_MSG;
  
  //if src is XYZ jump to label2
  
  label6:
  pL = &&label6; //remember where we were
  if (!gotTime(COST_BLB)) return T_BLB;
  procBlob(&data[5]); //set blob
  
  label7:
  pL = &&label7; //remember where we were
  if (!gotTime(COST_CON)) return T_CON;
  checkCon(&data[6], &&label4);
  
  //if src is ABC go idle
  label8:
  pL = &&label8; //remember where we were
  if (!gotTime(COST_CON)) return T_CON;
  checkCon(&data[7], &&idle);
  
  //do Action A with data Y
  label9:
  pL = &&label9;
  if (!gotTime(COST_SIG)) return T_SIG;
  sendSig(&data[9]);
  
  /// Generic Stuff --->
  idle:
  pL = &&idle;
  return DM_THR_IDLE;
  
  pageswap:
  return DM_THR_PAGESWAP | DM_THR_STOPPED;
  /// ---> Generic Stuff
  
}







 
