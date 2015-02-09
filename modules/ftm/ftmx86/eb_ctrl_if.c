#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wait.h>
#include <etherbone.h>
#include "xmlaux.h"
#include "ftmx86.h"



#define MAX_DEVICES  100
#define PACKET_SIZE  500
#define CMD_LM32_RST 0x2


const char* program;
eb_device_t device;
eb_socket_t mySocket;

const uint32_t devID_Reset       = 0x3a362063;
const uint32_t devID_RAM         = 0x66cfeb52;
const uint64_t vendID_CERN       = 0x000000000000ce42;
const uint32_t devID_ClusterInfo = 0x10040086;
const uint64_t vendID_GSI        = 0x0000000000000651;
char           devName_RAM_pre[] = "WB4-BlockRAM_";

eb_data_t tmpRead[2];
      
volatile uint32_t embeddedOffset, resetOffset, inaOffset, actOffset, targetOffset;
uint8_t error, verbose, readonly;
volatile uint32_t cpuQty;

void ebRamOpen(const char* netaddress, uint8_t cpuId);
void ebRamClose(void);

static int die(eb_status_t status, const char* what) {
  
  fprintf(stderr, "%s: %s -- %s\n", program, what, eb_status(status));
  exit(1);
}

static int getResetAdr()
{
  int idx = 0;
  int num_devices;
  struct sdb_device devices[MAX_DEVICES]; 
  
  num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, vendID_GSI, devID_Reset, &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no reset controller found\n", program);
    return 0xDEADBEEF;
  }

  if (num_devices > MAX_DEVICES) {
    fprintf(stderr, "%s: more devices found that tool supports (%d > %d)\n", program, num_devices, MAX_DEVICES);
    return 0xDEADBEEF;
  }

  if (idx > num_devices) {
    fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
    return 0xDEADBEEF;
  }

 return devices[0].sdb_component.addr_first;
}

void ebRamOpen(const char* netaddress, uint8_t cpuId)
{
   eb_cycle_t cycle;
   eb_status_t status;
   int idx;
   int attempts;
   int num_devices;
   struct sdb_device devices[MAX_DEVICES];
   char              devName_RAM_post[4];
   
   
   attempts   = 3;
   idx        = -1;

   /* open EB socket and device */
   if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32 | EB_DATA32, &mySocket))               != EB_OK) die(status, "failed to open Etherbone socket");
   if ((status = eb_device_open(mySocket, netaddress, EB_ADDR32 | EB_DATA32, attempts, &device)) != EB_OK) die(status, "failed to open Etherbone device");

   num_devices = MAX_DEVICES;
   if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_ClusterInfo, &devices[0], &num_devices)) != EB_OK)
   die(status, "failed to when searching for device");
   if (num_devices == 0) {
      fprintf(stderr, "%s: No lm32 clusterId rom found\n", program);
      goto error;
   }

   if (num_devices > MAX_DEVICES) {
      fprintf(stderr, "%s: Way too many lm32 clusterId roms found, something's wrong\n", program);
      goto error;
   }

   if (idx > num_devices) {
      fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
      goto error;
   }
      
   //get number of CPUs and create search string
   status = eb_device_read(device, (eb_address_t)devices[0].sdb_component.addr_first, EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0], 0, eb_block);
   if (status != EB_OK) die(status, "failed to create cycle");
   cpuQty = (uint8_t)tmpRead[0];
   
   devName_RAM_post[0] = '0';
   devName_RAM_post[1] = '0' + (cpuQty & 0xf);
   devName_RAM_post[2] = '0' + (cpuId  & 0xf);
   devName_RAM_post[3] =  0;

   if(cpuQty <= cpuId)
   {   
      fprintf(stderr, "The CpuId you gave me (%u) is higher than maximum (%u-1).\n", cpuId, cpuQty);
      goto error;
   }
  
   num_devices = MAX_DEVICES;
   if ((status = eb_sdb_find_by_identity(device, vendID_CERN, devID_RAM, &devices[0], &num_devices)) != EB_OK)
   die(status, "failed to when searching for device");
   if (num_devices == 0) {
      fprintf(stderr, "%s: no RAM's found\n", program);
      goto error;
   }

   if (num_devices > MAX_DEVICES) {
      fprintf(stderr, "%s: more devices found that tool supports (%d > %d)\n", program, num_devices, MAX_DEVICES);
      goto error;
   }

   if (idx > num_devices) {
      fprintf(stderr, "%s: device #%d could not be found; only %d present\n", program, idx, num_devices);
      goto error;
   }
   if (idx == -1) {
      //printf("Found %u devs\n", num_devices);
      for (idx = 0; idx < num_devices; ++idx) {
         if(strncmp(devName_RAM_post, (const char*)&devices[idx].sdb_component.product.name[13], 3) == 0)
         {
            embeddedOffset = devices[idx].sdb_component.addr_first;
         }
      }
   } else {
      printf("0x%"PRIx64"\n", devices[idx].sdb_component.addr_first);
      embeddedOffset = devices[idx].sdb_component.addr_first;
   }

   // get the active and inactive pointer value from the core

   if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
   eb_cycle_read(cycle, (eb_address_t)(embeddedOffset + FTM_PACT_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
   eb_cycle_read(cycle, (eb_address_t)(embeddedOffset + FTM_PINA_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);
   if ((status = eb_cycle_close(cycle)) != EB_OK) die(status, "failed to close read cycle");
   
   actOffset = (uint32_t) tmpRead[0];
   inaOffset = (uint32_t) tmpRead[1];
   
   return;
   
   error:
   ebRamClose();
   exit(1);
}

void ebRamClose()
{

   eb_status_t status;

   if ((status = eb_device_close(device))   != EB_OK) die(status, "failed to close Etherbone device");
   if ((status = eb_socket_close(mySocket)) != EB_OK) die(status, "failed to close Etherbone socket");
}


