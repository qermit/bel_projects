#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include <wrc.h>
#include <w1.h>

#include "syscon.h"
#include "eeprom.h"
#include "onewire.h"
#include "shell.h"
#include "uart.h"
#include "mini_sdb.h"
#include "aux.h"
#include "w1.h"
#include "onewire.h"

#define LINK_WENT_UP 1
#define LINK_WENT_DOWN 2
#define LINK_UP 3
#define LINK_DOWN 4
#define ENDRAM_MAGIC 0xbadc0ffe

int wrc_ui_mode = UI_SHELL_MODE;
int wrc_ui_refperiod = TICS_PER_SECOND; /* 1 sec */
int wrc_phase_tracking = 1;
int wrc_man_phase = 0;

///////////////////////////////////
//Calibration data (from EEPROM if available)
int32_t sfp_alpha = 73622176;	//default values if could not read EEPROM
int32_t sfp_deltaTx = 46407;
int32_t sfp_deltaRx = 167843;
uint32_t cal_phase_transition = 2389;
extern uint32_t _endram;
extern uint32_t _fstack;

void wrc_debug_printf(int subsys, const char *fmt, ...)
{
  /* To be deleted */
}

void _irq_entry(void)
{
  /* Stub */
}

int main(void)
{

  /* Initialize shell mode */
  wrc_ui_mode = UI_SHELL_MODE;

  /* Get uart unit address */
  discoverPeriphery();

  /* Initialize uart unit */
  uart_init_hw();
  
  /* Show start message */
  mprintf("User Core: Starting up...\n");

  /* Start endless loop */
  for (;;)
  {
    shell_interactive();
  }
  
  /* Should never get here */
  return(0);
  
}
