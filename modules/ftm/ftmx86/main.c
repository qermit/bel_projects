#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <wait.h>
#include <etherbone.h>
#include "access.h"
#include "xmlaux.h"
#include "ftmx86.h"
#include "fancy.h"


#define FILENAME_LEN   256
#define UINT64_STR_LEN 24
#define DUMP_STR_LEN 65536

const char* program;
eb_device_t device;
eb_socket_t mySocket;
t_ftmAccess ftmAccess;
t_ftmAccess* pAccess = &ftmAccess;


volatile uint32_t targetOffset, clusterOffset;
uint8_t error, verbose, readonly;

static uint32_t bytesToUint32(uint8_t* pBuf)
{
   uint8_t i;
   uint32_t val=0;
   
   for(i=0;i<FTM_WORD_SIZE;   i++) val |= (uint32_t)pBuf[i] << (8*i);
   return val;
}

static void ebPeripheryStatus()
{
  uint8_t buff[2048];
  uint32_t cfg;
  uint64_t tmp;
   
  ftmRamRead(pAccess->ebmAdr + EBM_REG_STATUS, EBM_REG_LAST, &buff[EBM_REG_STATUS]);
  printf ("EBM||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
  printf ("Status\t\t: 0x%08x\n",  bytesToUint32(&buff[EBM_REG_STATUS]));
  printf ("Src Mac\t\t: 0x%08x%04x\n", bytesToUint32(&buff[EBM_REG_SRC_MAC_HI]),  bytesToUint32(&buff[EBM_REG_SRC_MAC_LO]));
  printf ("Src IP\t\t: 0x%08x\n", bytesToUint32(&buff[EBM_REG_SRC_IPV4]));
  printf ("Src Port\t: 0x%04x\n\n", bytesToUint32(&buff[EBM_REG_SRC_UDP_PORT])); 
  printf ("Dst Mac\t\t: 0x%08x%04x\n", bytesToUint32(&buff[EBM_REG_DST_MAC_HI]),  bytesToUint32(&buff[EBM_REG_DST_MAC_LO]));
  printf ("Dst IP\t\t: 0x%08x\n", bytesToUint32(&buff[EBM_REG_DST_IPV4]));
  printf ("Dst Port\t: 0x%04x\n\n", bytesToUint32(&buff[EBM_REG_DST_UDP_PORT]));
  printf ("MTU\t\t: %u\n", bytesToUint32(&buff[EBM_REG_MTU]));
  printf ("Adr Hi\t\t: 0x%08x\n", bytesToUint32(&buff[EBM_REG_ADR_HI]));
  printf ("Ops Max\t\t: %u\n", bytesToUint32(&buff[EBM_REG_OPS_MAX]));
  printf ("EB Opt\t\t: 0x%08x\n\n", bytesToUint32(&buff[EBM_REG_EB_OPT]));

  ftmRamRead(pAccess->prioQAdr + r_FPQ.cfgGet, 4, &buff[r_FPQ.cfgGet]);
  ftmRamRead(pAccess->prioQAdr + r_FPQ.dstAdr, r_FPQ.ebmAdr - r_FPQ.dstAdr +4, &buff[r_FPQ.dstAdr]);
  printf ("FPQ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||\n");
  cfg = bytesToUint32(&buff[r_FPQ.cfgGet]);
  printf("-----------------------------------------------------------------------------------\n");
  if(cfg & r_FPQ.cfg_ENA)            printf("    ENA   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_FIFO)           printf("   FIFO   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOPOP)        printf("   APOP   ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOFLUSH_TIME) printf(" AFL_TIME ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_AUTOFLUSH_MSGS) printf(" AFL_MSGS ");  else printf("     -    ");
  if(cfg & r_FPQ.cfg_MSG_ARR_TS)     printf("  TS_ARR  ");  else printf("     -    ");
  printf("\n");
  printf("-----------------------------------------------------------------------------------\n");
  printf ("Dst Adr\t\t: 0x%08x\n\n", bytesToUint32(&buff[r_FPQ.dstAdr]));
  printf ("Heap Cnt\t: %u\n", bytesToUint32(&buff[r_FPQ.heapCnt]));
  printf ("msg CntO\t: %u\n", bytesToUint32(&buff[r_FPQ.msgCntO]));
  printf ("msg CntI\t: %u\n\n", bytesToUint32(&buff[r_FPQ.msgCntI]));  
  tmp = (((uint64_t)bytesToUint32(&buff[r_FPQ.tTrnHi])) <<32) + ((uint64_t)bytesToUint32(&buff[r_FPQ.tTrnLo]));
  printf ("TTrn\t\t: %llu\n", (long long unsigned int)tmp<<3);
  tmp = (((uint64_t)bytesToUint32(&buff[r_FPQ.tDueHi])) <<32) + ((uint64_t)bytesToUint32(&buff[r_FPQ.tDueLo]));
  printf ("TDue\t\t: %llu\n\n", (long long unsigned int)tmp<<3);
  printf ("Capacity\t: %u\n", bytesToUint32(&buff[r_FPQ.capacity]));
  printf ("msg max\t\t: %u\n\n", bytesToUint32(&buff[r_FPQ.msgMax]));
  printf ("EBM Adr\t\t: 0x%08x\n", bytesToUint32(&buff[r_FPQ.ebmAdr]));
  printf ("ts Adr\t\t: 0x%08x\n", bytesToUint32(&buff[r_FPQ.tsAdr]));
  printf ("ts Ch\t\t: 0x%08x\n\n", bytesToUint32(&buff[r_FPQ.tsCh]));
   
   
   return;
   
}






static void status(uint8_t cpuIdx)
{
  uint32_t ftmStatus, mySharedMem, sharedMem;
  eb_status_t status;
  eb_data_t tmpRd[4];
  eb_cycle_t cycle;
  eb_address_t tmpAdr = pAccess->pCores[cpuIdx].ramAdr + ftm_shared_offs;
  long long unsigned int ftmTPrep;

  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) die(status, "failed to create cycle"); 
  eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_STAT_OFFSET,         EB_BIG_ENDIAN | EB_DATA32, &tmpRd[0]);
  eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_SHARED_PTR_OFFSET,   EB_BIG_ENDIAN | EB_DATA32, &tmpRd[1]); 
  eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_TPREP_OFFSET,        EB_BIG_ENDIAN | EB_DATA32, &tmpRd[2]);
  eb_cycle_read(cycle, tmpAdr + (eb_address_t)FTM_TPREP_OFFSET+4,      EB_BIG_ENDIAN | EB_DATA32, &tmpRd[3]);
  if ((status = eb_cycle_close(cycle)) != EB_OK)  die(status, "failed to close read cycle");
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

static void help(void) {
  fprintf(stderr, "\nUsage: %s [OPTION] <etherbone-device> [command]\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  duetime <Time / ns>       Set due time for priority queue\n");
  fprintf(stderr, "  trntime <Time / ns>       Set transmission delay for priority queue\n");
  fprintf(stderr, "  maxmsg <Message Quantity> Set maximum messages in a packet for priority queue\n\n");
  fprintf(stderr, "  -c <core-idx>             select a core by index, -1 selects all\n");
  fprintf(stderr, "  -v                        verbose operation, print more details\n");
  fprintf(stderr, "  -h                        display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  status                    (default) report core status\n");
  fprintf(stderr, "  run                       start this core\n");
  fprintf(stderr, "  stop                      request stop on this core\n");
  fprintf(stderr, "  fstop                     force stop on this core\n");
  fprintf(stderr, "  bpset                     set branchpoint. accepts 0..n or 'idle'\n");
  fprintf(stderr, "  idle                      request idle state on this core\n");
  fprintf(stderr, "  swap                      swap active and inactive page on this core\n");
  fprintf(stderr, "  put    <filename>         puts ftm data from xml file to inactive page on this core\n");
  fprintf(stderr, "  clear                     clears all pages on this core\n");
  fprintf(stderr, "  get                       gets ftm data from inactive page and displays it\n");
  fprintf(stderr, "  dump                      gets ftm data from active page and displays it\n");
  fprintf(stderr, "  loadfw <filename>         puts firmware from bin file to core\n");
  fprintf(stderr, "  condump                   debug feature. make this core output its data to console\n");
  fprintf(stderr, "  preptime <Time / ns>      Set preparation time on this core\n");
  fprintf(stderr, "\n");

  //fprintf(stderr, "  condget                   debug feature. if this core is wating for a condition value, ask what it is\n");
  //fprintf(stderr, "  condset                   debug feature. set condition value in shared mem for this core\n");
  fprintf(stderr, "\n");
}

int main(int argc, char** argv) {

   //vars for command line args
   program  = argv[0];
   
   int opt;
   char *value_end;
   const char* netaddress, *command;
   char     filename[FILENAME_LEN];
   char     bpstr[10];
   uint64_t uint64val = 0;
   
   // cpu access related
   int cpuId = 0;
   uint8_t firstCpu, lastCpu, cpuIdx, overrideFWcheck;
   uint32_t targetCpus;
   
   overrideFWcheck = 0;
   error    = 0;
   verbose  = 0;
   readonly = 1;
   
   // eb connection stuff
   eb_status_t ebstatus;
   eb_cycle_t  cycle;
   
   
   // eb read/write buffers for schedule data and firmware load
   uint8_t    bufWrite[BUF_SIZE];
   uint8_t    bufRead[BUF_SIZE];
   uint8_t*    pBufRead = &bufRead[0];
   uint8_t*    pBufWrite = &bufWrite[0];
   memset(pBufWrite, 0, BUF_SIZE);
   memset(pBufRead, 0, BUF_SIZE);
   
   t_ftmPage*  pNewPage = NULL;
   
   
   
   // start getopt 
   

   while ((opt = getopt(argc, argv, "c:ovh")) != -1) {
      switch (opt) {
         case 'o':
            overrideFWcheck = 1;
            break;
         case 'v':
            verbose = 1;
            break;
         case 'c':
            cpuId = strtol(optarg, &value_end, 0);
            if (*value_end || cpuId < -1 ||cpuId > 32) {
              fprintf(stderr, "%s: invalid cpu id -- '%s'\n", program, optarg);
              error = 1;
            }
            break;
         case 'h':
            help();
            return 0;
         case ':':
         case '?':
            error = 1;
            break;
         default:
            fprintf(stderr, "%s: bad getopt result\n", program);
            return 1;
      }
   }

   if (error) return 1;

   if (optind >= argc) {
   fprintf(stderr, "%s: expecting one non-optional argument: <etherbone-device>\n", program);
   fprintf(stderr, "\n");
   help();
   return 1;
   }
   
   // process command arguments
   
   netaddress = argv[optind];
   printf("\n");

   
   if (optind+1 < argc)  command = argv[++optind];
   else                 {command = "status"; cpuId = -1;}
   if (!strcasecmp(command, "loadfw")) overrideFWcheck = 1;  
   
   if ( (!strcasecmp(command, "put")) || (!strcasecmp(command, "loadfw")))
   {
      if (optind+1 < argc) {
         strncpy(filename, argv[optind+1], FILENAME_LEN);
         readonly = 0;
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <filename>\n", program);
         return 1;
      }
   }
   
    
   
   if ( (!strcasecmp(command, "preptime")) || (!strcasecmp(command, "duetime")) || (!strcasecmp(command, "trntime")) || (!strcasecmp(command, "maxmsg"))) { 
      if (optind+1 < argc) {
         long long unsigned int tmp = strtoll(argv[optind+1], NULL, 10);
         if(!strcasecmp(command, "maxmsg"))  uint64val = (uint64_t)tmp;
         else                                uint64val = (uint64_t)(tmp>>3);
      } else {
         if(!strcasecmp(command, "maxmsg")) fprintf(stderr, "%s: expecting one non-optional argument: <Message Quantity>\n", program);
         else fprintf(stderr, "%s: expecting one non-optional argument: <Time / ns>\n", program);
         return 1;
      }
   }
   
   if (!strcasecmp(command, "setbp")) {
      if (optind+1 < argc) {
         bpstr[9] = 0;
         strncpy(bpstr, argv[optind+1], 8);
      } else {
         fprintf(stderr, "%s: expecting one non-optional argument: <branchpoint name>\n", program);
         return 1;
      }
   }
   
//*****************************************************************************************************************//

  //printf("Connecting to FTM\n");
  validCpus = ftmOpen(netaddress, pAccess, overrideFWcheck);

  //op for one CPU or all?
  if(cpuId < 0) { firstCpu   = 0; 
    lastCpu    = pAccess->cpuQty-1;
  } else { firstCpu = (uint8_t)cpuId; 
    lastCpu  = (uint8_t)cpuId;
  }  
   
  if(cpuId < 0) {
    targetCpus = (1 << pAccess->cpuQty) -1;
  } else {
    targetCpus = 1 << firstCpu;
  }
  
  validTargetCpus = validCpus & targetCpus;


// DM prioQ Operations   
  if(!strcasecmp(command, "duetime")) {
    return ftmSetDuetime(uint64val);
  }
 
  if(!strcasecmp(command, "trntime")) {
    return ftmSetTrntime(uint64val);
  }
 
  if(!strcasecmp(command, "maxmsg")) {
    return ftmSetMaxMsgs(uint64val);
  }
 
//DM CPU Operations
  if (!strcasecmp(command, "loadfw")) {
    return ftmFwLoad(targetCpus, filename); // all selected, not just the ones with valid firmware !
  }      
        
  /* -------------------------------------------------------------------- */
  if (!strcasecmp(command, "status")) {
    char* bufString = (char*)malloc(DUMP_STR_LEN, 1); 
    ftmGetStatus(validTargetCpus, bufState);
    printf("%s", ftmShowStatus(bufState, bufString, verbose));
    free(pBufDump);
  }

  /* -------------------------------------------------------------------- */
  else if (!strcasecmp(command, "run")) {
    ftmCommand(validTargetCpus, CMD_START);
  }

  else if (!strcasecmp(command, "stop")) {
     ftmCommand(validTargetCpus, CMD_STOP_REQ);
  }

  else if (!strcasecmp(command, "idle")) {
   ftmCommand(validTargetCpus, CMD_IDLE);
  }
  else if (!strcasecmp(command, "fstop")) {
   ftmCommand(validTargetCpus, CMD_STOP_NOW) 
  } 

  else if (!strcasecmp(command, "swap")) {
   ftmCommand(validTargetCpus, CMD_COMMIT_PAGE);
  } 

  else if (!strcasecmp(command, "condump")) {
   ftmCommand(validTargetCpus, CMD_SHOW_ACT);
  } 
  
  else if (!strcasecmp(command, "clear")) {
   ftmClear(validTargetCpus, BUF_SIZE);
  }

  
  else if (!strcasecmp(command, "reset")) {
     ftmCpuRst(targetCpus);
  }

  else if (!strcasecmp(command, "preptime")) {
     ftmSetPreptime(validTargetCpus, uint64val);
  }
  
  else if(!strcasecmp(command, "put")) {
    if(!readonly) {
      return ftmPutFile(validTargetCpus, filename, pBufWrite, BUF_SIZE);   
    } else { fprintf(stderr, "No xml file specified\n"); return -1;}
  }
  
  else if(!strcasecmp(command, "dump")) {
    char* pBufDump = (char*)malloc(DUMP_STR_LEN, 1); 
    ftmDump(validTargetCpus, BUF_SIZE, ACTIVE, pBufDump, DUMP_STR_LEN);
    printf("%s\n", pBufDump);
    free(pBufDump);
    return 0;
  }   
  
  else if(!strcasecmp(command, "get")) {
    char* pBufDump = (char*)malloc(DUMP_STR_LEN, 1); 
    ftmDump(validTargetCpus, BUF_SIZE, INAACTIVE, pBufDump, DUMP_STR_LEN);
    printf("%s\n", pBufDump);
    free(pBufDump);
    return 0; 
  }

  else if (!strcasecmp(command, "setbp")) {
    return ftmSetBp(validTargetCpus, bpStr);
  }
     
  else  printf("Unknown command: %s\n", command);  


  return 0;
}


