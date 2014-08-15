#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <wrc.h>
#include "shell.h"
#include "uart.h"
#include "mini_sdb.h"
#include "aux.h"
#include "w1.h"
#include "onewire.h"

static int cmd_w1scan(const char *args[])
{
  
  static struct w1_dev *s_W1Dev; /* One wire structure */
  static struct w1_bus s_w1_bus; /* One wire bus structure */
  int32_t iDeviceIterator = 0;   /* Iterator for each one wire device */
  int32_t iDevicesFound = 0;     /* 1Wire devices */
  bool fConfigDone = false;      /* Configure the interface one-time-only */
  
  /* Configure interface one-time-only */
  if(!fConfigDone)
  {
    wrpc_w1_init(); 
    s_w1_bus.detail = ONEWIRE_PORT;
    fConfigDone = true;
  }
  
  /* Scan bus */
  mprintf("Scanning bus now ...\n");  
  iDevicesFound = w1_scan_bus(&s_w1_bus);
  mprintf("Devices found %d\n", iDevicesFound);  
  
  /* Iterate devices */
  for (iDeviceIterator = 0; iDeviceIterator < W1_MAX_DEVICES; iDeviceIterator++)
  {
    s_W1Dev = s_w1_bus.devs + iDeviceIterator;
    if (s_W1Dev->rom)
    {
      /* Show found device */
      mprintf("Device found [%d]: 0x%08x%08x\n", iDeviceIterator, (int)(s_W1Dev->rom >> 32), (int)s_W1Dev->rom);
    }
    else
    {
      /* Show some kind of error/warning */
      mprintf("Device not found [%d] ...\n", iDeviceIterator);
    }
  }
  
  return 0;
}

DEFINE_WRC_COMMAND(w1scan) = {
  .name = "w1scan",
  .exec = cmd_w1scan,
};
