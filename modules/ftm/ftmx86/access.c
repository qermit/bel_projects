#include "access.h"


/// Expected Firmware Version ///
const char myVer[] = "0.2.1\n";
const char myName[] = "ftm\n";
////////////////////////////////


const uint64_t vendID_CERN       = 0x000000000000ce42;
const uint64_t vendID_GSI        = 0x0000000000000651;

const uint32_t devID_Reset       = 0x3a362063;
const uint32_t devID_RAM         = 0x66cfeb52;
const uint32_t devID_CoreRAM     = 0x54111351;
const uint32_t devID_SharedRAM   = 0x81111444;

const uint32_t devID_ClusterInfo = 0x10040086;
const uint32_t devID_ClusterCB   = 0x10041000;
const uint32_t devID_Ebm         = 0x00000815;
const uint32_t devID_Prioq       = 0x10040200;

const char     devName_RAM_pre[] = "WB4-BlockRAM_";

static uint32_t getCpuFromThr(uint64_t thr) {
  
  uint64_t mask = (pAccess->cpuQty<<1)-1;
  uint32_t res;
  
  for(i=0;i < ((64 + pAccess->cpuQty - 1)/pAccess->cpuQty),i++)
    if(thr & (mask << (i*pAccess->cpuQty))) res |= 1<<i;
  return res;  
}



uint32_t ftmOpen(const char* netaddress, t_ftmAccess* p, uint8_t overrideFWcheck)
{
  eb_cycle_t cycle;
  eb_status_t status;
  int cpuIdx, idx;
  int attempts;
  int num_devices;
  struct sdb_device devices[MAX_DEVICES];
  char              devName_RAM_post[4];
  struct sdb_bridge CluCB;
  
  uint32_t validCpus = 0;
  
  eb_data_t tmpRead[4];
  
  attempts   = 3;
  idx        = -1;

  /* open EB socket and device */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32 | EB_DATA32, &mySocket))               != EB_OK) die(status, "failed to open Etherbone socket");
  if ((status = eb_device_open(mySocket, netaddress, EB_ADDR32 | EB_DATA32, attempts, &device)) != EB_OK) die(status, "failed to open Etherbone device");

  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_ClusterCB, (struct sdb_device*)&CluCB, &num_devices)) != EB_OK)
  die(status, "failed to when searching for device");
  p->clusterAdr = CluCB.sdb_component.addr_first;
  
  //find reset ctrl
  num_devices = MAX_DEVICES;
  eb_sdb_find_by_identity(device, vendID_GSI, devID_Reset, &devices[0], &num_devices);
  if (num_devices == 0) {
    fprintf(stderr, "%s: no reset controller found\n", program);
    goto error;
  }
  p->resetAdr = (eb_address_t)devices[0].sdb_component.addr_first;

  //get clusterInfo
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity_at(device, &CluCB, vendID_GSI, devID_ClusterInfo, &devices[0], &num_devices)) != EB_OK)
  die(status, "failed to when searching for device");
  if (num_devices == 0) {
    fprintf(stderr, "%s: No lm32 clusterId rom found\n", program);
    goto error;
  }

  //get number of CPUs
  status = eb_device_read(device, (eb_address_t)devices[0].sdb_component.addr_first, EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0], 0, eb_block);
  if (status != EB_OK) die(status, "failed to create cycle");
  p->cpuQty = (uint8_t)tmpRead[0];
  p->pCores =  malloc(p->cpuQty * sizeof(t_core));

  // Get Shared RAM
  num_devices = 1;
  if ((status = eb_sdb_find_by_identity_at(device, &CluCB, vendID_GSI, devID_SharedRAM, &devices[0], &num_devices)) != EB_OK)
  die(status, "failed to when searching for Shared RAM ");
  //Old or new Gateware ?
  //FIXME: the cumbersome legacy code has to go sometime
  if(num_devices < 1) {
    //Old
    if ((status = eb_sdb_find_by_identity_at(device, &CluCB, vendID_CERN, devID_RAM, &devices[0], &num_devices)) != EB_OK)
    die(status, "failed to when searching for Shared RAM ");
  }
  p->sharedAdr = (eb_address_t)devices[0].sdb_component.addr_first;

  // Get prioq
  num_devices = 1;
  if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_Prioq, &devices[0], &num_devices)) != EB_OK)
  die(status, "failed to when searching for Priority Queue ");
  if (num_devices == 0) {
    fprintf(stderr, "%s: No Priority Queue found\n", program);
    goto error;
  }
  p->prioQAdr = (eb_address_t)devices[0].sdb_component.addr_first;
  
  // Get EBM
  num_devices = 1;
  if ((status = eb_sdb_find_by_identity(device, vendID_GSI, devID_Ebm, &devices[0], &num_devices)) != EB_OK)
  die(status, "failed to when searching for device");
  if (num_devices == 0) {
    fprintf(stderr, "%s: No Etherbone Master found\n", program);
    goto error;
  }
  p->ebmAdr = (eb_address_t)devices[0].sdb_component.addr_first;
  
  //Get RAMs 
  num_devices = MAX_DEVICES;
  if ((status = eb_sdb_find_by_identity_at(device, &CluCB, vendID_GSI, devID_CoreRAM, &devices[0], &num_devices)) != EB_OK)
  die(status, "failed to when searching for device");
  
  //Old or new Gateware ?
  //FIXME: the cumbersome legacy code has to go sometime
  //if(overrideFWcheck) printf("Gate-/firmware check disabled by override option\n");
  
  if(num_devices > 0) {
    //new
    ftm_shared_offs = FTM_SHARED_OFFSET_NEW;
    for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
      p->pCores[cpuIdx].ramAdr = devices[cpuIdx].sdb_component.addr_first;
      //check for valid firmware
      uint8_t isValid = 0;
      if(overrideFWcheck) isValid = 1;
      else                isValid = isFwValid(&devices[cpuIdx], &myVer[0], &myName[0]);
      validCpus |= (isValid << cpuIdx);
    }
  } else {
    //Old
    ftm_shared_offs = FTM_SHARED_OFFSET_OLD;
    if(!overrideFWcheck) {
      printf("ERROR: FTM is using old gateware. Sure this is the FTM you want ? Use option '-o' if you want to override\n");
      goto error;
    }
    
    for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
      devName_RAM_post[0] = '0';
      devName_RAM_post[1] = '0' + (p->cpuQty & 0xf);
      devName_RAM_post[2] = '0' + (cpuIdx  & 0xf);
      devName_RAM_post[3] =  0;

      num_devices = MAX_DEVICES;
      if ((status = eb_sdb_find_by_identity(device, vendID_CERN, devID_RAM, &devices[0], &num_devices)) != EB_OK)
      die(status, "failed to when searching for device");
      
      for (idx = 0; idx < num_devices; ++idx) {
        if(strncmp(devName_RAM_post, (const char*)&devices[idx].sdb_component.product.name[13], 3) == 0) {
          p->pCores[cpuIdx].ramAdr = devices[idx].sdb_component.addr_first;
          p->pCores[cpuIdx].hasValidFW = 1;
          validCpus |= (1 << cpuIdx);
        }
      }
    }
  } 
    
  // get the active, inactive and shared pointer values from the core RAM
  for(cpuIdx = 0; cpuIdx < p->cpuQty; cpuIdx++) {
    if (p->pCores[cpuIdx].hasValidFW) {
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
      eb_cycle_read(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_PACT_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      eb_cycle_read(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_PINA_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);
      eb_cycle_read(cycle, (eb_address_t)(p->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_SHARED_PTR_OFFSET),   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[2]); 
      
      if ((status = eb_cycle_close(cycle)) != EB_OK) die(status, "failed to close read cycle");
      p->pCores[cpuIdx].actOffs     = (uint32_t) tmpRead[0];
      p->pCores[cpuIdx].inaOffs     = (uint32_t) tmpRead[1];
      p->pCores[cpuIdx].sharedOffs  = (uint32_t) tmpRead[2];
    
    } else printf("Core #%u: Can't read schedule data offsets - no valid firmware present.\n", cpuIdx);
  }


  return validCpus;

  error:
  ftmClose();
  return 0; //dummy
}

void ftmClose(void)
{
  eb_status_t status;
  if ((status = eb_device_close(device))   != EB_OK) die(status, "failed to close Etherbone device");
  if ((status = eb_socket_close(mySocket)) != EB_OK) die(status, "failed to close Etherbone socket");
}

int die(eb_status_t status, const char* what)
{  
  fprintf(stderr, "%s: %s -- %s\n", program, what, eb_status(status));
  return -1;
}


uint8_t isFwValid(struct  sdb_device* ram, const char* sVerExp, const char* sName)
{
  uint8_t validity = 1;
  uint32_t len = FWID_LEN/4;
  eb_data_t fwdata[FWID_LEN/4];
  char cBuff[FWID_LEN];
  char* pos;
  
  eb_cycle_t cycle;
  eb_status_t status;
  
  uint8_t verExpMaj, verExpMin, verExpRev;
  uint8_t verFndMaj=0, verFndMin=0, verFndRev=0;
  uint32_t verExp, verFnd;
  verExpMaj = sVerExp[0] - '0';
  verExpMin = sVerExp[2] - '0';
  verExpRev = sVerExp[4] - '0';
  
  uint32_t i, j;
  //RAM Big enough to actually contain a FW ID?
  if ((ram->sdb_component.addr_last - ram->sdb_component.addr_first + 1) >= (FWID_LEN + BOOTL_LEN)) {
    if ((status = eb_cycle_open(device, 0, 0, &cycle)) != EB_OK)
      die(status, "eb_cycle_open");
    for (j = 0; j < len; ++j)
      eb_cycle_read(cycle, ram->sdb_component.addr_first + BOOTL_LEN + j*4, EB_DATA32|EB_BIG_ENDIAN, &fwdata[j]);
    if ((status = eb_cycle_close(cycle)) != EB_OK)
      die(status, "eb_cycle_close");

   for (j = 0; j < len; ++j) {
      for (i = 0; i < 4; i++) {
        cBuff[j*4+i] = (char)(fwdata[j] >> (8*(3-i)) & 0xff);
      }  
    }
   
  //check for magic word
  if(strncmp(cBuff, "UserLM32", 8)) {validity = 0;} 
  if(!validity) { printf("No firmware found!\n"); return 0; }

  //check project
  pos = strstr(cBuff, "Project     : ");
  if(pos != NULL) {
    pos += 14;
    if(strncmp(pos, sName, strlen(sName))) {validity = 0;} 
  } else { printf("This is no ftm firmware, name does not match!\n"); return 0;}
  
  //check version
  pos = strstr(cBuff, "Version     : ");
  if(pos != NULL) {
    pos += 14;
    verFndMaj = pos[0] - '0';
    verFndMin = pos[2] - '0';
    verFndRev = pos[4] - '0';
  } else {validity = 0;}
  } else {validity = 0;}
  
  verExp = (verExpMaj *100 + verExpMin *10 + verExpRev);
  verFnd = (verFndMaj *100 + verFndMin *10 + verFndRev);
  
  if(verExp > verFnd ) {
    validity = 0;
    printf("ERROR: Expected firmware %u.%u.%u, but found only %u.%u.%u! If you are sure, use -o to override.\n", verExpMaj, verExpMin, verExpRev, verFndMaj, verFndMin, verFndRev);
    return 0;  
  }
  if(verExp < verFnd ) {
    printf("ERROR: Expected firmware %u.%u.%u is lower than found %u.%u.%u. If you are sure, use -o to override.\n", verExpMaj, verExpMin, verExpRev, verFndMaj, verFndMin, verFndRev);
    return 0;
  }
  
  //no fwid found. try legacy
        
  return validity;
    
}




const uint8_t* ftmRamRead(uint32_t address, const uint8_t* buf, uint32_t len, uint32_t bufEndian)
{
   
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen, start;
   uint32_t* readin = (uint32_t*)buf;
   eb_data_t tmpReadin[BUF_SIZE/2];   

   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   start = 0;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK)  die(status, "failed to create cycle");
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         //printf("%4u %08x -> %p\n", i, (address+(i<<2)), &readin[i]);
         eb_cycle_read(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, &tmpReadin[i]);
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close read cycle");
      for(i= start>>2; i< (start + partLen) >>2;i++) {
        if (bufEndian == LITTLE_ENDIAN)  readin[i] = SWAP_4((uint32_t)tmpReadin[i]);
        else                             readin[i] = (uint32_t)tmpReadin[i];
         
      } //this is important caus eb_data_t is 64b wide!
      start = start + partLen;
   }
      
   return buf;
}

const uint8_t* ftmRamWrite(uint32_t address, const uint8_t* buf, uint32_t len, uint32_t bufEndian)
{
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen, start, data;
   uint32_t* writeout = (uint32_t*)buf;   
   
   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   start = 0;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
      
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         if (bufEndian == LITTLE_ENDIAN)  data = SWAP_4(writeout[i]);
         else                             data = writeout[i];
         
         eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)data); 
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close write cycle");
      start = start + partLen;
   }
   
   return buf;
}

void ftmRamClear(uint32_t address, uint32_t len)
{
   eb_status_t status;
   eb_cycle_t cycle;
   uint32_t i,j, parts, partLen, start;  
   
   //wrap frame buffer in EB packet
   parts = (len/PACKET_SIZE)+1;
   start = 0;
   
   for(j=0; j<parts; j++)
   {
      if(j == parts-1 && (len % PACKET_SIZE != 0)) partLen = len % PACKET_SIZE;
      else partLen = PACKET_SIZE;
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
      
      for(i= start>>2; i< (start + partLen) >>2;i++)  
      {
         eb_cycle_write(cycle, (eb_address_t)(address+(i<<2)), EB_BIG_ENDIAN | EB_DATA32, 0); 
      }
      if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close write cycle");
      start = start + partLen;
   }
}



int ftmRst(void) {
    status = eb_device_write(device, (eb_address_t)(pAccess->resetAdr + FTM_RST_FPGA), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)FTM_RST_FPGA_CMD, 0, eb_block);
    if (status != EB_OK) die(status, "failed to create cycle"); 
    return 0;
}

int ftmCpuRst(uint32_t dstCpus) {

  eb_status_t status;
  
  printf("Resetting CPU(s)...\n");
  status = eb_device_write(device, (eb_address_t)(pAccess->resetAdr + FTM_RST_SET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)dstCpus, 0, eb_block);
  if (status != EB_OK) die(status, "failed to create cycle");
  status = eb_device_write(device, (eb_address_t)(pAccess->resetAdr + FTM_RST_CLR), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)dstCpus, 0, eb_block);
  if (status != EB_OK) die(status, "failed to create cycle"); 
  printf("Done.\n\n");

}

int ftmFwLoad(uint32_t dstCpu, const char* filename) {
  

  FILE *file;
  uint8_t* buffer;
  unsigned long fileLen;
  eb_status_t status;
  
  //Open file
  file = fopen(filename, "rb");
  if (!file)
  {
    fprintf(stderr, "Unable to open file %s", filename);
    return -1;
  }

  //Get file length
  fseek(file, 0, SEEK_END);
  fileLen=ftell(file);
  fseek(file, 0, SEEK_SET);
  //Allocate memory
  buffer=(uint8_t *)malloc(fileLen+1);
  if (!buffer)
  {
    fprintf(stderr, "Memory error!");
    fclose(file);
    return -2;
  }
  //Read file contents into buffer
  fileLen = fread(buffer, 1, fileLen, file);
  fclose(file);
  
  printf("Putting CPU(s) into Reset for FW load\n");
  status = eb_device_write(device, (eb_address_t)(pAccess->resetAdr + FTM_RST_SET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)dstCpu, 0, eb_block);
  if (status != EB_OK) die(status, "failed to create cycle");
  
  for(cpuIdx=0;cpuIdx < pAccess->cpuQty;cpuIdx++) {
    if((dstCpu >> cpuIdx) & 0x1) {
      //Load FW
      printf("Loading %s to CPU %u @ 0x%08x\n", filename, cpuIdx, pAccess->pCores[cpuIdx].ramAdr);  
      ftmRamWrite(buffer, pAccess->pCores[cpuIdx].ramAdr, fileLen, LITTLE_ENDIAN);
    }
  }
  free(buffer);

  printf("Releasing CPU(s) from Reset\n\n");
  status = eb_device_write(device, (eb_address_t)(pAccess->resetAdr + FTM_RST_CLR), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)dstCpu, 0, eb_block);
  if (status != EB_OK) die(status, "failed to create cycle"); 

  printf("Done.\n");
  return 0;

}


int ftmThrRst(uint64_t dstBitField) {

}


int ftmCommand(uint32_t dstCpus, uint32_t command) {

  eb_status_t status;
  eb_cycle_t cycle;
  uint32_t cpuIdx;
  
  if(dstCpus) {if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle");}
  else return 0;
  
  for(cpuIdx=0;cpuIdx < pAccess->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      eb_cycle_write(cycle, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_CMD_OFFSET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)command);
    }
  }
  
  if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close write cycle");
  return 0;
}

int ftmPutString(uint32_t dstCpus, const char* sXml, uint8_t* bufWrite, uint32_t len) {
  t_ftmPage*  pPage = parseXmlString(sXml);
  return ftmPut(dstCpus, pPage, bufWrite, len);
  
int ftmPutFile(uint32_t dstCpus, const char* filename, uint8_t* bufWrite, uint32_t len) {
  
  t_ftmPage*  pPage = parseXmlFile(sXml);
  return ftmPut(dstCpus, pPage, bufWrite, len);
  
}

int ftmPut(uint32_t dstCpus, t_ftmPage*  pPage, uint8_t* bufWrite, uint32_t len) {
  t_ftmPage*  pPage = parseXmlString(sXml);
  uint32_t baseAddr, offs, cpuIdx, i; 
  uint8_t* bufRead = (uint8_t *)malloc(len);
  
  for(cpuIdx=0;cpuIdx < pAccess->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      baseAddr  = pAccess->pCores[cpuIdx].ramAdr;
      offs      = pAccess->pCores[cpuIdx].inaOffs;
      memset(bufWrite, 0, len);
      serPage (pPage, bufWrite, offs, cpuIdx);
      ftmRamWrite(bufWrite, baseAddr + offs, len, BIG_ENDIAN);
      printf("Wrote %u byte schedule to CPU %u at 0x%08x.", len, cpuIdx, baseAddr + offs);
      printf("Verify..."); 
      ftmRamRead(baseAddr + offs, len, bufRead);
      for(i = 0; i<len; i++) {
        if(!(bufRead[i] == pBufWrite[i])) { 
          fprintf(stderr, "!ERROR! \nVerify failed for CPU %u at offset 0x%08x\n", cpuIdx, baseAddr + offs +( i & ~0x3) );
          free(bufRead);
          return -1;
        }
      }
      printf("OK\n");
    }
  }
  free(bufRead);
  printf("done.\n");
  return 0;  

}

uint32_t ftmDump(uint32_t srcCpus, uint32_t len, uint8_t actIna, char* stringBuf, uint32_t lenStringBuf) {
  uint32_t baseAddr, offs, cpuIdx, i;  
  t_ftmPage* pPage;
  uint8_t* bufRead = (uint8_t *)malloc(len);
  char* bufStr = stringBuf;
  
  for(cpuIdx=0;cpuIdx < pAccess->cpuQty;cpuIdx++) {
    if((srcCpus >> cpuIdx) & 0x1) {
      baseAddr  = pAccess->pCores[cpuIdx].ramAdr;
      if(actIna == ACTIVE)  offs = pAccess->pCores[cpuIdx].actOffs; 
      else              offs = pAccess->pCores[cpuIdx].inaOffs;
      pPage = deserPage(calloc(1, sizeof(t_ftmPage)), bufRead, offs);
      if(pPage != NULL) {  
         printf("Deserialization successful.\n\n");
         if (lenStringBuf - (bufStr - stringBuf) < 2048) {printf("String buffer running too low, aborting.\n"); return (uint32_t)(bufStr - stringBuf);}
         bufStr += (sprintf("---CPU %u %s page---\n", cpuIdx, "active") -1); //don't do zero termination in between dumps
         bufStr += showFtmPage(pPage, bufStr) -1; //don't do zero termination in between dumps
      } else printf("Deserialization for CPU %u FAILED! Corrupt/No Data ?\n", cpuIdx);
    }
  }
  *bufStr++ = 0x00; // zero terminate all dumps
  return (uint32_t)(bufStr - stringBuf); // return number of characters
}

int ftmClear(uint32_t dstCpus, uint32_t len) {
  uint32_t baseAddr, offs, cpuIdx; 
  
  for(cpuIdx=0;cpuIdx < pAccess->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      baseAddr  = pAccess->pCores[cpuIdx].ramAdr;
      offs      = pAccess->pCores[cpuIdx].inaOffs;
      ftmRamClear(baseAddr + offs, len);
      printf("Cleared %u bytes in inactive page of CPU %u at 0x%08x.", len, cpuIdx, baseAddr + offs);
  }
  printf("done.\n");
  return 0;  

}

int ftmSetPreptime(uint32_t dstCpus, uint64_t tprep) {
  for(cpuIdx=0;cpuIdx < pAccess->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      baseAddr  = pAccess->pCores[cpuIdx].ramAdr;
      if ((ebstatus = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(ebstatus, "failed to create cycle"); 
      eb_cycle_write(cycle, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_TPREP_OFFSET +0), EB_BIG_ENDIAN | EB_DATA32, (uint32_t)(tprep>>32));
      eb_cycle_write(cycle, (eb_address_t)(pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs + FTM_TPREP_OFFSET +4), EB_BIG_ENDIAN | EB_DATA32, (uint32_t)tprep);
      if ((ebstatus = eb_cycle_close(cycle)) != EB_OK)  return die(ebstatus, "failed to close write cycle");
    }
  }
  return 0;    
}

int ftmSetDuetime(uint64_t tdue) {
  if ((ebstatus = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(ebstatus, "failed to create cycle"); 
  eb_cycle_write(cycle, (eb_address_t)(pAccess->prioQAdr + r_FPQ.tDueHi), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)(tdue>>32)));
  eb_cycle_write(cycle, (eb_address_t)(pAccess->prioQAdr + r_FPQ.tDueLo), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)tdue));
  if ((ebstatus = eb_cycle_close(cycle)) != EB_OK)  return die(ebstatus, "failed to close write cycle");
  return 0;    
}

int ftmSetTrntime(uint64_t ttrn) {
  if ((ebstatus = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(ebstatus, "failed to create cycle"); 
  eb_cycle_write(cycle, (eb_address_t)(pAccess->prioQAdr + r_FPQ.tTrnHi), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)(ttrn>>32)));
  eb_cycle_write(cycle, (eb_address_t)(pAccess->prioQAdr + r_FPQ.tTrnLo), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)ttrn));
  if ((ebstatus = eb_cycle_close(cycle)) != EB_OK) return die(ebstatus, "failed to close write cycle");
  return 0;    
}

int ftmSetMaxMsgs(uint64_t maxmsg) {
  if ((ebstatus =  eb_device_write(device, (eb_address_t)(pAccess->prioQAdr + r_FPQ.msgMax), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)((uint32_t)uint64val), 0, eb_block); != EB_OK) 
    return die(ebstatus, "failed to create cycle"); 
  return 0;    
}

int ftmSetBp(const char* bpStr) {

  int planIdx, planQty;
  eb_cycle_t cycle;
  eb_status_t status;
  uint32_t bp;
  eb_data_t tmpRead[3];
  
  uint32_t baseAddr, offs, cpuIdx; 
  
  if(!strcasecmp(bpStr, "idle")) planIdx = -1;
  else {planIdx = strtol(bpStr, 0, 10);}   
  
  for(cpuIdx=0;cpuIdx < pAccess->cpuQty;cpuIdx++) {
    if((dstCpus >> cpuIdx) & 0x1) {
      baseAddr  = pAccess->pCores[cpuIdx].ramAdr;
      offs      = pAccess->pCores[cpuIdx].actOffs;
      
      //user gave us a planIdx. load corresponding ptr
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(ebstatus, "failed to create cycle"); 
      eb_cycle_read(cycle, (eb_address_t)(baseAddr + offs + FTM_PAGE_PLANQTY_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      //if the user wanted to go to idle, read idle ptr from interface, else read plan ptr
      if(planIdx == -1)
      {eb_cycle_read(cycle, (eb_address_t)(baseAddr + ftm_shared_offs + FTM_IDLE_OFFSET), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);}
      else 
      {eb_cycle_read(cycle, (eb_address_t)(baseAddr + offs + FTM_PAGE_PLANS_OFFSET + 4*planIdx), EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]);}
      if((ebstatus = eb_cycle_close(cycle)) != EB_OK) return die(ebstatus, "failed to close read cycle"); 

      planQty  = (uint32_t)tmpRead[0];
      bp       = (uint32_t)tmpRead[1];
      // Check and write to BP
      if(bp != FTM_NULL && planIdx < planQty) {
        printf("Writing plan %d @ 0x%08x to BP\n", planIdx, bp);
        status = eb_device_write(device, (eb_address_t)(baseAddr + offs + FTM_PAGE_BP_OFFSET), EB_BIG_ENDIAN | EB_DATA32, (eb_data_t)bp, 0, eb_block);
        if (status != EB_OK) return die(ebstatus, "failed to create cycle");  
      } else { 
        if (planIdx >= planQty) printf ("Sorry, but the plan index is neither idle nor 0 <= %d (planIdx) <  %u (planQty)\n", planIdx, planQty);
        else printf ("Found a NULL ptr at plan idx %d, something is wrong\n", planIdx);
      }
    }  
  }
  return 0;
}



int ftmGetStatus(uint32_t srcCpus, uint32* buff) {
  uint32_t cpuIdx, offset;
  eb_address_t tmpAdr;
  eb_data_t tmpRd[4];
  
  
  // read EBM status
  ftmRamRead(pAccess->ebmAdr + EBM_REG_STATUS, EBM_REG_LAST, &buff[EBM_REG_STATUS]);
  ftmRamRead(pAccess->prioQAdr + r_FPQ.cfgGet, 4, &buff[r_FPQ.cfgGet]);
  
  // read PrioQ status
  ftmRamRead(pAccess->prioQAdr + r_FPQ.dstAdr, r_FPQ.ebmAdr - r_FPQ.dstAdr +4, &buff[r_FPQ.dstAdr]);
  offset = r_FPQ.tsCh;
  // read CPU status'
  for(cpuIdx=0;cpuIdx < pAccess->cpuQty;cpuIdx++) {
    if((srcCpus >> cpuIdx) & 0x1) {
      tmpAdr = pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs;
      
      
      if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) return die(status, "failed to create cycle"); 
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_STAT_OFFSET,         EB_BIG_ENDIAN | EB_DATA32, &tmpRead[0]);
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_SHARED_PTR_OFFSET,   EB_BIG_ENDIAN | EB_DATA32, &tmpRead[1]); 
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_TPREP_OFFSET,        EB_BIG_ENDIAN | EB_DATA32, &tmpRead[2]);
      eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_TPREP_OFFSET+4,      EB_BIG_ENDIAN | EB_DATA32, &tmpRead[3]);
      if ((status = eb_cycle_close(cycle)) != EB_OK) return die(status, "failed to close read cycle");
      for(i=0;i<4;i++) buff[offset + cpuIdx * 4 + i] = (uint32_t)tmpRead[i];
      
    } else {
      memset(&buff[offset + cpuIdx * 4 + 0], 0, 4*4);
    }
  return 0;
}

tatic void status(uint8_t cpuIdx)
{
  uint32_t ftmStatus, mySharedMem, sharedMem;
  eb_status_t status;
  uint32_t cfg;
  uint64_t tmp;
  eb_cycle_t cycle;
  eb_address_t tmpAdr = pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs;
  long long unsigned int ftmTPrep;
  uint32_t* buffEbm   = buff ;
  uint32_t* buffPrioq = buffEbm + (EBM_REG_LAST>>2);
  uint32_t* buffCpu   = buffPrioq + (r_FPQ.tsCh>>2);

  //Generate EBM Status
  printf ("EBM||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
  printf ("Status\t\t: 0x%08x\n",  buffEbm[EBM_REG_STATUS]);
  printf ("Src Mac\t\t: 0x%08x%04x\n", buffEbm[EBM_REG_SRC_MAC_HI],  buffEbm[EBM_REG_SRC_MAC_LO]);
  printf ("Src IP\t\t: 0x%08x\n", buffEbm[EBM_REG_SRC_IPV4]);
  printf ("Src Port\t: 0x%04x\n\n", buffEbm[EBM_REG_SRC_UDP_PORT]); 
  printf ("Dst Mac\t\t: 0x%08x%04x\n", buffEbm[EBM_REG_DST_MAC_HI],  buffEbm[EBM_REG_DST_MAC_LO]);
  printf ("Dst IP\t\t: 0x%08x\n", buffEbm[EBM_REG_DST_IPV4]);
  printf ("Dst Port\t: 0x%04x\n\n", buffEbm[EBM_REG_DST_UDP_PORT]);
  printf ("MTU\t\t: %u\n", buffEbm[EBM_REG_MTU]);
  printf ("Adr Hi\t\t: 0x%08x\n", buffEbm[EBM_REG_ADR_HI]);
  printf ("Ops Max\t\t: %u\n", buffEbm[EBM_REG_OPS_MAX]);
  printf ("EB Opt\t\t: 0x%08x\n\n", buffEbm[EBM_REG_EB_OPT]);

  //Generate PrioQ Status
  printf ("FPQ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
  cfg = &buffPrioq[r_FPQ.cfgGet];
  printf("-----------------------------------------------------------------------------------\n");
  if(cfg & r_FPQ.cfg_ENA)            printf("    ENA   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_FIFO)           printf("   FIFO   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOPOP)        printf("   APOP   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOFLUSH_TIME) printf(" AFL_TIME ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOFLUSH_MSGS) printf(" AFL_MSGS ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_MSG_ARR_TS)     printf("  TS_ARR  ");  else printf("     -    ");
  printf("\n");
  printf("-----------------------------------------------------------------------------------\n");
  printf ("Dst Adr\t\t: 0x%08x\n\n", buffPrioq[r_FPQ.dstAdr]);
  printf ("Heap Cnt\t: %u\n", buffPrioq[r_FPQ.heapCnt]);
  printf ("msg CntO\t: %u\n", buffPrioq[r_FPQ.msgCntO]);
  printf ("msg CntI\t: %u\n\n", buffPrioq[r_FPQ.msgCntI]);  
  tmp = (((uint64_t)buffPrioq[r_FPQ.tTrnHi]) <<32) + ((uint64_t)buffPrioq[r_FPQ.tTrnLo]);
  printf ("TTrn\t\t: %llu\n", (long long unsigned int)tmp<<3);
  tmp = (((uint64_t)buffPrioq[r_FPQ.tDueHi]) <<32) + ((uint64_t)buffPrioq[r_FPQ.tDueLo]);
  printf ("TDue\t\t: %llu\n\n", (long long unsigned int)tmp<<3);
  printf ("Capacity\t: %u\n", buffPrioq[r_FPQ.capacity]));
  printf ("msg max\t\t: %u\n\n", buffPrioq[r_FPQ.msgMax]));
  printf ("EBM Adr\t\t: 0x%08x\n", buffPrioq[r_FPQ.ebmAdr]));
  printf ("ts Adr\t\t: 0x%08x\n", buffPrioq[r_FPQ.tsAdr]));
  printf ("ts Ch\t\t: 0x%08x\n\n", buffPrioq[r_FPQ.tsCh]));



  
  ftmStatus = (uint32_t) tmpRd[0];
  sharedMem = tmpRd[1]; //convert lm32's view to pcie's view
  mySharedMem = pAccess->clusterAdr + (sharedMem & 0x3fffffff); //convert lm32's view to pcie's view
  ftmTPrep = (long long unsigned int)(((uint64_t)tmpRd[2]) << 32 | ((uint64_t)tmpRd[3]));

  uint8_t i;
     
  printf("\u2552"); for(i=0;i<79;i++) printf("\u2550"); printf("\u2555\n");
  printf("\u2502 %sCore #%02u%s                                                                      \u2502\n", KCYN, cpuIdx, KNRM);
  printf("\u251C"); for(i=0;i<24;i++) printf("\u2500"); printf("\u252C"); for(i=0;i<54;i++) printf("\u2500"); printf("\u2524\n");
  printf("\u2502 Status: %02x ErrCnt: %3u \u2502   MsgCnt: %9u       TPrep: %13llu ns    \u2502\n", \
   (uint8_t)ftmStatus, (uint8_t)(ftmStatus >> 8), (uint16_t)(ftmStatus >> 16), ftmTPrep<<3);
  printf("\u251C"); for(i=0;i<24;i++) printf("\u2500"); printf("\u253C"); for(i=0;i<54;i++) printf("\u2500"); printf("\u2524\n");
  printf("\u2502 Shared Mem: 0x%08x \u2502", mySharedMem + cpuIdx*0x0C);
  if(pAccess->pCores[cpuIdx].actOffs < pAccess->pCores[cpuIdx].inaOffs) printf("   Act Page: A 0x%08x  Inact Page: B 0x%08x", pAccess->pCores[cpuIdx].actOffs, pAccess->pCores[cpuIdx].inaOffs);
  else                      printf("   Act Page: B 0x%08x  Inact Page: A 0x%08x", pAccess->pCores[cpuIdx].actOffs, pAccess->pCores[cpuIdx].inaOffs);
  printf("   \u2502\n");
  printf("\u251C"); for(i=0;i<24;i++) printf("\u2500"); printf("\u2534"); for(i=0;i<54;i++) printf("\u2500"); printf("\u2524\n");
  printf("\u2502       ");

  if(ftmStatus & STAT_RUNNING)    printf("   %sRUNNING%s   ", KGRN, KNRM);  else printf("   %sSTOPPED%s   ", KRED, KNRM);
  if(ftmStatus & STAT_IDLE)       printf("     %sIDLE%s    ", KYEL, KNRM);  else printf("     %sBUSY%s    ", KGRN, KNRM);
  if(ftmStatus & STAT_STOP_REQ)   printf("   STOP_REQ  ");  else printf("      -      ");
  if(ftmStatus & STAT_ERROR)      printf("     %sERROR%s   ", KRED, KNRM);  else printf("     %sOK%s      ", KGRN, KNRM);
  if(ftmStatus & STAT_WAIT)       printf("  WAIT_COND  ");  else printf("      -      ");
  printf("       \u2502\n");
  printf("\u2514"); for(i=0;i<79;i++) printf("\u2500"); printf("\u2518\n");

}



 if (!strcasecmp(command, "status")) { printf("%s### FTM @ %s ####%s\n", KCYN, netaddress, KNRM); 
