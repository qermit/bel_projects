#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include "mprintf.h"
#include "mini_sdb.h"
#include "ebm.h"
#include "aux.h"
#include "dbg.h"
 
unsigned int cpuId, cpuQty;


void show_msi()
{
  mprintf(" Msg:\t%08x\nAdr:\t%08x\nSel:\t%01x\n", global_msi.msg, global_msi.adr, global_msi.sel);

}

void isr0()
{
   mprintf("Interrupt Service Routine 0\n");   
   show_msi();
}

void isr1()
{
   mprintf("Interrupt Service Routine 1\n");   
   show_msi();
}


void ebmInit()
{
   ebm_init();
   ebm_config_if(LOCAL,   "hw/cb:a9:87:65:43:21/udp/192.168.191.250/port/60000");
   ebm_config_if(REMOTE,  "hw/ff:ff:ff:ff:ff:ff/udp/192.168.191.255/port/60000");
   ebm_config_meta(1500, 0x11, 255, 0x00000000 );
}

void init()
{ 
   discoverPeriphery();
   uart_init_hw();
   ebmInit();
   cpuId = getCpuIdx() + 1;
   cpuQty = getCores();
   
   isr_table_clr();
   isr_ptr_table[0] = isr0; //timer
   isr_ptr_table[1] = isr1;   
   irq_set_mask(0x03);
   irq_enable();
   
}




void main(void) {
   
   int j, k;
   
   init();
   
   mprintf("Hello World, user LM32 #%02u/%02u here!\n", cpuId, cpuQty);
   for (j = 0; j < (125000000/4); ++j) { asm("nop"); }
      
   while (1) {
      
      for (j = 0; j < 125000000; ++j) { asm("nop"); }
      atomic_on();
      mprintf("%u: User LM32 #%02u/%02u is still alive\n", k++, cpuId, cpuQty);
      atomic_off();
   }

}
