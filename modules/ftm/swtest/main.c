#include <stdio.h>
#include <string.h>
#include "display.h"
#include "irq.h"
#include "mini_sdb.h"
#include "timer.h"
#include "ebm.h"
#include "aux.h"

volatile unsigned int* pSDB_base    = (unsigned int*)0x7FFFFA00;

volatile unsigned int* cpu_ID;
volatile unsigned int* time_sys;
volatile unsigned int* irq_slave;
volatile unsigned int* timer;
volatile unsigned int* test;     
volatile unsigned int* display;
volatile unsigned int* ebm;     
volatile unsigned int* cores;
volatile unsigned int* atomic;        

volatile int xinc, yinc, timeinc, x, y, xmax, xmin, ymax, ymin, time;
volatile char color; 
unsigned int cpuID, cpuMAX;
   char buffer[12];
volatile unsigned long long timestamp, timestamp_old;

const unsigned int c_period = 375000000/1;


#define DRAW_STOP  0x01
#define DRAW_START 0x02
#define DRAW_RST   0x03

void show_msi()
{
  char buffer[12];

  mat_sprinthex(buffer, global_msi.msg);
  disp_put_str("D ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, global_msi.adr);
  disp_put_str("A ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, (unsigned long)global_msi.sel);
  disp_put_str("S ");
  disp_put_str(buffer);
  disp_put_c('\n');
}

void pause_and_show_msi() 
{
  const unsigned int src = (cpuID<<2);
  unsigned int dst, i;
  unsigned int* pIRQ;    
  char buffer[12];

 

   for(i = 0; i<*cores; i++) {
      if(i != cpuID )
      {

         dst = (i<<8);
         pIRQ = (unsigned int*)0x80000000 + ((dst | src)>>2);
         //mat_sprinthex(buffer, adr);
         //disp_put_str(buffer);
         //disp_put_c('\n');           
         *pIRQ = DRAW_STOP; //send an irq to the msi queue of <dst> from <src>
 
     } 
   }

   show_msi();
   for (i = 0; i < 2*125000000; ++i) {asm("# noop");}
   disp_put_c('\f');   
 for(i = 0; i<*cores; i++) {
      if(i != cpuID )
      {

         dst = (i<<8);
         pIRQ = (unsigned int*)0x80000000 + ((dst | src)>>2);
         //mat_sprinthex(buffer, adr);
         //disp_put_str(buffer);
         //disp_put_c('\n');           
         *pIRQ = DRAW_START; //send an irq to the msi queue of <dst> from <src>
 
     } 
   }
}




void isr0()
{
      
   
   char buffer[12];
   unsigned char tm_idx = global_msi.adr>>2 & 0xf;
   unsigned long long deadline = irq_tm_deadl_get(tm_idx);
   static unsigned int calls = 0;  
   static unsigned int msg_old;   
   static unsigned int deltamax;
   static unsigned int deltamin = -1;
   unsigned int delta;

   atomic_on();
   ebm_op(0x100000E0, global_msi.adr, WRITE);
   ebm_op(0x100000E4, (unsigned int)(*cpu_ID & 0xf)+1, WRITE); 
   ebm_op(0x100000E8, (unsigned int)(timestamp>>32), WRITE);
   ebm_op(0x100000EC, (unsigned int)timestamp, WRITE);
   ebm_op(0x100000F0, (((unsigned int)tm_idx)<<16) + (unsigned int)calls, WRITE);    
   ebm_flush(); 
   atomic_off();

   disp_put_c('\f');
   disp_put_str("C");
   disp_put_c('0' + (*cpu_ID & 0xf)+1); 

   disp_put_str(" TM");
   disp_put_c('0' + tm_idx);
   disp_put_c('/');
   disp_put_str(sprinthex(buffer, global_msi.msg & 0xffff, 4));
   disp_put_str("Calls: ");
   disp_put_str(sprinthex(buffer, calls++, 4));
   
   delta = timestamp - timestamp_old - c_period;

   if(calls > 2){
      if(delta>>31) { //negativ
         
         deltamax = delta<deltamax ? delta : deltamax;
         deltamin = delta>deltamin ? delta : deltamin;
      } else {        // positiv
         deltamax = delta>deltamax ? delta : deltamax;
         deltamin = delta<deltamin ? delta : deltamin;
      }
   }
   disp_put_str("D*  ");
   disp_put_str(sprinthex(buffer, delta, 5));
   disp_put_str("\nMin ");
   disp_put_str(sprinthex(buffer, deltamin, 5));
   disp_put_str("\nMax ");
   disp_put_str(sprinthex(buffer, deltamax, 5));
   disp_put_str("\nDD  ");
   disp_put_str(sprinthex(buffer, deltamax-deltamin, 5));
}

void isr1()
{
  unsigned int msg = global_msi.msg & 0xff;
  static int xincsave, yincsave;
  static char colorsave; 

   switch(msg) {
   case DRAW_STOP :  xincsave = xinc;
                     yincsave = yinc;
                     colorsave = color;
                     yinc = 0;         //stop drawing
    	               xinc = 0;
                     timeinc = 0;
                     break; 
   case DRAW_START : color = colorsave;
                     yinc = yincsave;
                     xinc = xincsave;
                     timeinc = 1;
                     break; 
   case DRAW_RST :   disp_put_c('\f'); //restart drawing
                     color = 0xFF;               
                     yinc = -1;
       	            xinc = 1;
                     timeinc = 1;
                     x = xmin+2;
	                  y = ymin+5;
                     time = 0;
                     break; 
   default   :    break;            //nothing
   }

}

void isr2()
{

  unsigned int msg = global_msi.msg & 0xff;
  unsigned int target = global_msi.msg & 0xff00;
  volatile unsigned int* irq; 
  
  disp_put_str("ILCK\n");
  show_msi();

  irq = (unsigned int*) (0x8000000 + ((target + (*cpu_ID<<2))>>2)); //send an irq to the msi queue of <target> from <cpu_ID>
  *irq = msg;
      
}

void isr3()
{
  unsigned int j;
  
  disp_put_str("OTH\n");
  show_msi();

   for (j = 0; j < 125000000; ++j) {
        asm("# noop"); /* no-op the compiler can't optimize away */
      }
   disp_put_c('\f');   
}
/*
void _irq_entry(void) {

   timestamp_old = timestamp;    
   timestamp = get_sys_time();
     
   irq_process();

   
}
*/

const char mytext[] = "Hallo Welt!...\n\n";

void init()
{

   cpu_ID       = (unsigned int*)find_device(CPU_INFO_ROM);
   cores        = (unsigned int*)find_device(LM32_CLUSTER_INFO_ROM);
   display      = (unsigned int*)find_device(SCU_OLED_DISPLAY);  
   ebm          = (unsigned int*)find_device(ETHERBONE_MASTER); 
   irq_slave    = (unsigned int*)find_device(IRQ_MSI_CTRL_IF);   
   time_sys     = (unsigned int*)find_device(SYSTEM_TIME);
   timer        = (unsigned int*)find_device(IRQ_TIMER_CTRL_IF);
   atomic       = (unsigned int*)find_device(ATOMIC_BUS_ACCESS);

   ebm_config_if(LOCAL,   "hw/08:00:30:e3:b0:5a/udp/192.168.191.254/port/60368");
   ebm_config_if(REMOTE,  "hw/00:14:d1:fa:01:aa/udp/192.168.191.131/port/60368");
   ebm_config_meta(80, 0x11, 16, 0x00000000 ); 

   isr_table_clr();
   isr_ptr_table[0]= isr0; //timer
   isr_ptr_table[1]= isr1; //lm32
   isr_ptr_table[2]= isr2; //ilck
   isr_ptr_table[3]= isr3; //other    
   irq_set_mask(0x0f);
   irq_enable();
   cpuID = irq_get_mask();
   disp_reset();	
   disp_put_c('\f'); 
}

void main(void) {

     const int xlen = 64;
   const int ylen = 48;
   int j, m, n, xl, yl;
   unsigned int addr_raw_off;

   init();
   cpuID = *cpu_ID  & 0xff;
   cpuMAX = *cores       & 0xff;
  
   //calc matrix dimension
   n = 1;   
   m = cpuMAX;
   while(m > n) 
   {
      if((m-1)*n >= cpuMAX) m--;
      else n++; 
   }
    if (m*(n-1) >= cpuMAX) n--;

       

  
   xl = xlen/m;
   yl = ylen/n;

   xmin = (cpuID % m)      * xl;
   xmax = ((cpuID % m)+1)  * xl-1;
   ymin = cpuID/m          * yl;
   ymax = (cpuID/m+1)      * yl-1;
   
     color = 0xFF;

 /*
 
  disp_put_str(mytext);
*/



  
  
   timeinc = 1;
   x = xmin+2+cpuID;
	y = ymin+3+cpuID;


	
	yinc = -1;
 	xinc = 1;
	addr_raw_off = 0;


 


disp_put_c('\f');

  disp_put_str("Timer...\n");
   
   unsigned long long systime = get_sys_time();
  
   disp_put_str("H ");
   disp_put_str(mat_sprinthex(buffer,  (unsigned int)(systime>>32)));
   disp_put_c('\n');
    disp_put_str("L ");
   disp_put_str(mat_sprinthex(buffer,  (unsigned int)(systime)));
   disp_put_c('\n');

   s_timer mytm;
   mytm.mode      = TIMER_PERIODIC;
   mytm.src       = TIMER_REL_TIME;
   mytm.cascade   = TIMER_0;
   mytm.deadline  = c_period;
   mytm.msi_dst   = 0;
   mytm.msi_msg   = 0xCAFEBABE;
   irq_tm_write_config(1, &mytm);
   irq_tm_start(1<<1);
   
   irq_tm_timer_sel(0);
   irq_tm_msi_set(0, 0xBEEF);
   irq_tm_trigger_at_time(0, get_sys_time()+375000000);
   
   disp_put_str("Go!");

   for (j = 0; j < (125000000/160)*(cpuID<<3); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }

  while (1) {
    /* Rotate the LEDs */
    //63
   //47
/*

   atomic_on();
   disp_put_raw( get_pixcol_val((unsigned char)y), get_pixcol_addr((unsigned char)x, (unsigned char)y), color);
   atomic_off();

	if(x == xmax) xinc = -1;
	if(x == xmin)  xinc = 1;

	if(y == ymax) yinc = -1;
	if(y == ymin)  yinc = 1;

	x += xinc;
	y += yinc;




     for (j = 0; j < 125000000/160; ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }

	if(time > 500) {time = 0; color = ~color; }
	time += timeinc;
*/  
  }

}
