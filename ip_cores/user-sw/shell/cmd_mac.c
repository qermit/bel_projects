#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <wrc.h>
#include "shell.h"

#define EP_REG_MACH 0x00000024
#define EP_REG_MACL 0x00000028

static int cmd_mac(const char *args[])
{

  /* Helper */
  uint8_t a_uMAC[6];
  uint32_t *p_uEndPoint;
  
  /* Locate end point */
  p_uEndPoint = find_device(0x650c2d4f);
  p_uEndPoint = (uint32_t*) 0x80060100; /* TBD: Fix this */
  
  /* Plausibility check */
  if(p_uEndPoint==NULL)
  { 
    mprintf("Error: Can't find end point!\n");
    return(-1);
  }
  
  /* Extract the MAC address */
  a_uMAC[5] = (uint8_t) (((*(p_uEndPoint+(EP_REG_MACL>>2))&0xff)));
  a_uMAC[4] = (uint8_t) (((*(p_uEndPoint+(EP_REG_MACL>>2))&0xff00))>>8);
  a_uMAC[3] = (uint8_t) (((*(p_uEndPoint+(EP_REG_MACL>>2))&0xff0000))>>16);
  a_uMAC[2] = (uint8_t) (((*(p_uEndPoint+(EP_REG_MACL>>2))&0xff000000))>>24);
  a_uMAC[1] = (uint8_t) (((*(p_uEndPoint+(EP_REG_MACH>>2))&0xff)));
  a_uMAC[0] = (uint8_t) (((*(p_uEndPoint+(EP_REG_MACH>>2))&0xff00)>>8));

  /* Print the MAC address */
  mprintf("MAC-address: %02x:%02x:%02x:%02x:%02x:%02x\n", a_uMAC[0], a_uMAC[1], a_uMAC[2], a_uMAC[3], a_uMAC[4], a_uMAC[5]);
  
  /* Done */
  return(0);

}

DEFINE_WRC_COMMAND(mac) = {
  .name = "mac",
  .exec = cmd_mac,
};
