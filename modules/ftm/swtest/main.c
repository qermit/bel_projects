#include <stdio.h>
#include <string.h>
#include "display.h"
#include "irq.h"
#include "mini_sdb.h"

volatile unsigned int* silicon_id   = (unsigned int*)0x7FFFFFF0;
volatile unsigned int* pSDB_base    = (unsigned int*)0x7FFFFFF4;
volatile unsigned int* irq_slave    = (unsigned int*)0x7FFFFE00;

volatile unsigned int* test;     
volatile unsigned int* display;     
volatile unsigned int* cores;       

volatile int xinc, yinc, timeinc, x, y, xmax, xmin, ymax, ymin, time;
volatile char color; 
unsigned int cpuID, cpuMAX;
   char buffer[12];

#define DRAW_STOP  0x01
#define DRAW_START 0x02
#define DRAW_RST   0x03

char* sprinthex(char* buffer, unsigned long val, unsigned char digits)
{
	unsigned char i,ascii;
	const unsigned long mask = 0x0000000F;

	for(i=0; i<digits;i++)
	{
		ascii= (val>>(i<<2)) & mask;
		if(ascii > 9) ascii = ascii - 10 + 'A';
	 	else 	      ascii = ascii      + '0';
		buffer[digits-1-i] = ascii;		
	}
	
	buffer[digits] = 0x00;
	return buffer;	
}


char* mat_sprinthex(char* buffer, unsigned long val)
{
   return sprinthex(buffer, val, 8);
}


void show_msi()
{
  char buffer[12];

  mat_sprinthex(buffer, global_msi.msg);
  disp_put_str("D ");
  disp_put_str(buffer);
  disp_put_c('\n');

  
  mat_sprinthex(buffer, global_msi.src);
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
   disp_put_c('\f');  
   disp_put_str("CPU ");

  disp_put_str(sprinthex(buffer, (*silicon_id & 0xff)+1, 2));
  /*
  disp_put_str(" of ");
    
  disp_put_str(sprinthex(buffer, *cores & 0xff, 2)); 
  disp_put_c('\n');
  disp_put_str(mat_sprinthex(buffer, *pSDB_base));
*/
  disp_put_c('\n');
  disp_put_str(mat_sprinthex(buffer,  (unsigned long)display));
  disp_put_c('\n');
  disp_put_str(sprinthex(buffer, *cores & 0xff, 2)); 
//pause_and_show_msi();
     
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

  irq = (unsigned int*) (0x8000000 + ((target + (*silicon_id<<2))>>2)); //send an irq to the msi queue of <target> from <silicon_id>
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

void _irq_entry(void) {

  irq_process();

   
}

const char mytext[] = "Hallo Welt!...\n\n";

void init()
{
  cores        = (unsigned int*)find_device(LM32_CLUSTER_INFO_ROM);
  display      = (unsigned int*)find_device(SCU_OLED_DISPLAY);  

  isr_table_clr();
  isr_ptr_table[0]= isr0; //eca
  isr_ptr_table[1]= isr1; //lm32
  isr_ptr_table[2]= isr2; //ilck
  isr_ptr_table[3]= isr3; //other    
  irq_set_mask(0x0f);
  irq_enable();

  disp_reset();	
  disp_put_c('\f'); 
}

void main(void) {

  

   const int xlen = 64;
   const int ylen = 48;
   int j, m, n, xl, yl;
   unsigned int addr_raw_off;

   init();
   cpuID = *silicon_id  & 0xff;
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

/*
disp_put_c('\f');

  disp_put_str("cor");
  disp_put_str(sprinthex(buffer, cpuMAX, 2));
  disp_put_str(" ID");
  disp_put_str(sprinthex(buffer, cpuID, 2));   
  disp_put_c('\n');

  disp_put_str("m ");
  disp_put_str(sprinthex(buffer, m, 2));
  disp_put_str(" n ");
  disp_put_str(sprinthex(buffer, n, 2));   
  disp_put_c('\n');

  disp_put_str("xl");
  disp_put_str(sprinthex(buffer, xl, 2));   
  disp_put_str(" yl");
  disp_put_str(sprinthex(buffer, yl, 2));
  disp_put_c('\n');
   disp_put_c('\n');    

  disp_put_str("xn");
  disp_put_str(sprinthex(buffer, xmin, 2));   
  disp_put_str(" xx");
  disp_put_str(sprinthex(buffer, xmax, 2));
  disp_put_c('\n'); 

  disp_put_str("yn");
  disp_put_str(sprinthex(buffer, ymin, 2));   
  disp_put_str(" yx");
  disp_put_str(sprinthex(buffer, ymax, 2));
  disp_put_c('\n');  
*/
/*
   for (j = 0; j < (125000000/160)*(cpuID<<3); ++j) {
        asm("# noop"); // no-op the compiler can't optimize away
      }
*/
  while (1) {
    /* Rotate the LEDs */
    //63
   //47



  disp_put_raw( get_pixcol_val((unsigned char)y), get_pixcol_addr((unsigned char)x, (unsigned char)y), color);


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
   
  }

}
