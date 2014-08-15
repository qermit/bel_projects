#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <wrc.h>
#include <sdb.h>
#include "shell.h"

#define EB_REG_IP 0x00000018

static int cmd_ip(const char *args[])
{

  /* Helpers */
  uint8_t a_uIP[4];
  uint32_t *p_uEtherBoneBase;
  
  /* Locate EtherBone base */
  p_uEtherBoneBase = find_device(0x68202b22);
  
  /* Plausibility check */
  if(p_uEtherBoneBase==NULL)
  { 
    mprintf("Error: Can't find EtherBone base!\n");
    return(-1);
  }

  /* Extract the IP address */
  a_uIP[3] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff)));
  a_uIP[2] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff00))>>8);
  a_uIP[1] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff0000))>>16);
  a_uIP[0] = (uint8_t) (((*(p_uEtherBoneBase+(EB_REG_IP>>2))&0xff000000))>>24); 
  
  /* Print the IP address */
  mprintf("IP-address: %d.%d.%d.%d\n", a_uIP[0], a_uIP[1], a_uIP[2], a_uIP[3]);
  
  /* Done */
  return(0);
  
}

DEFINE_WRC_COMMAND(ip) = {
  .name = "ip",
  .exec = cmd_ip,
};
