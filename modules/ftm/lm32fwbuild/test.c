#include "thread.h"

uint32_t deadline;
uint32_t** pBase = (uint32_t**)BASEPTR;



void setDl(uint32_t slice) {
  uint32_t dl;	
  asm ("rcsr %0, cc": "=r"(dl)); //get cycle counter
  deadline = dl + slice;
}

uint32_t entry(uint32_t cost) {
  uint32_t cc, diff; 
  asm ("rcsr %0, cc": "=r"(cc)); //get cycle counter
 
  //handle overflow
  if(cc > deadline)  	diff = (deadline -  (0xffffffff - cc));
  else            	diff = deadline - cc;

  //inside instruction budget?
  return  ((diff - cost) >> 31);
  
}
