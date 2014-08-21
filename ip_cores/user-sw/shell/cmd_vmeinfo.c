#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <wrc.h>
#include <sdb.h>
#include "shell.h"
#include "display.h"

extern volatile unsigned int* pVmeInfo;

static int cmd_vmeinfo(const char *args[])
{
  
  /* Helper */
  uint32_t uGetVMEAddr = 0;
  char a_cVMEAddr[] = "0x00";
  
  /* Plausibility check */
  if(pVmeInfo==NULL)
  { 
    mprintf("Error: Can't find the INFO_VME unit!\n");
    return(-1);
  }
  
  /* Get the VME address */
  uGetVMEAddr = *pVmeInfo;
  
  /* Print the VME address */
  mprintf("VME-address: 0x%02x\n", uGetVMEAddr);
  
  /* Check parameters */
  if (args[0] && !strcasecmp(args[0], "lcd"))
  {
    /* Convert address to string */
    sprintf(a_cVMEAddr, " 0x%02x", uGetVMEAddr);
    
    /* Print VME address */
    mprintf("Info: Using LCD...\n");
    lcd_disp_put_s("\f");
    lcd_disp_put_s("VME:  ");
    lcd_disp_put_s(a_cVMEAddr);
  }
  
  /* Done */
  return(0);
  
}

DEFINE_WRC_COMMAND(vmeinfo) = {
  .name = "vmeinfo",
  .exec = cmd_vmeinfo,
};
