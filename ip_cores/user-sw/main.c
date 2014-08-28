/* Includes */
/* ==================================================================================================== */
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
#include "disp-lcd.h"

/* Global variables */
/* ==================================================================================================== */
int wrc_ui_mode = UI_SHELL_MODE; /* Needed for GUI mode/wrpc files */
int wrc_ui_refperiod = TICS_PER_SECOND; /* 1 sec */

/* Prototypes */
/* ==================================================================================================== */
int  init(void);
void ui_update(void);

/* Function main(...) */
/* ==================================================================================================== */
int main(void)
{
  
  /* Initialize units and global variables */
  init();
  
  /* Start endless loop */
  for (;;)
  {
    ui_update();
  }
  
  /* Should never get here */
  return(0);
  
}

/* Function init(...) */
/* ==================================================================================================== */
int init(void)
{
  
  /* Initialize shell mode */
  wrc_ui_mode = UI_SHELL_MODE;

  /* Get units */
  discoverPeriphery();

  /* Initialize uart unit */
  uart_init_sw();
  uart_init_hw();
  
  /* Show start message */
  mprintf("User Core: Starting up...\n");
  
  /* Initialize lcdisplay */
  if (disp_init())
  {
    mprintf("Warning: Initialization of LCD failed!\n");
    return (1);
  }

  /* Done */
  return(0);
  
}

/* Function ui_update(...) */
/* ==================================================================================================== */
void ui_update(void)
{
  /* ASCII symbol 27 = ESC (Escape) */
  if (wrc_ui_mode == UI_GUI_MODE)
  {
    if (uart_read_byte() == 27 || wrc_ui_refperiod == 0)
    {
      shell_init();
      wrc_ui_mode = UI_SHELL_MODE;
    }
  } 
  else if (wrc_ui_mode == UI_STAT_MODE)
   {
    if (uart_read_byte() == 27 || wrc_ui_refperiod == 0)
    {
      shell_init();
      wrc_ui_mode = UI_SHELL_MODE;
    }
  } 
  else
  {
    shell_interactive();
  }
}

/* Function wrc_debug_printf(...) */
/* ==================================================================================================== */
void wrc_debug_printf(int subsys, const char *fmt, ...)
{
  /* Stub */
}

/* Function _irq_entry(...) */
/* ==================================================================================================== */
void _irq_entry(void)
{
  /* Stub */
}
