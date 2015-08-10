#ifndef _ACCESS_H_
#define _ACCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>



#define SWAP_4(x) ( ((x) << 24) | \
         (((x) << 8) & 0x00ff0000) | \
         (((x) >> 8) & 0x0000ff00) | \
         ((x) >> 24) )

#define MAX_DEVICES 20
#define FWID_LEN 0x400
#define BOOTL_LEN 0x100
#define PACKET_SIZE  500
#define CMD_LM32_RST 0x2

#define ACTIVE    1
#define INACTIVE  2

#define CPU_MAX 8
#define THR_MAX 8



extern const char* program;






typedef struct {
   uint32_t ramAdr;
   uint32_t actOffs;
   uint32_t inaOffs;
   uint32_t sharedOffs;
   uint8_t  hasValidFW;
} t_core;

typedef struct {
   uint8_t cpuQty;
   uint8_t thrQty;
   t_core* pCores;
   uint32_t resetAdr;
   uint32_t clusterAdr;
   uint32_t sharedAdr;
   uint32_t prioQAdr;
   uint32_t ebmAdr;
} t_ftmAccess;



uint32_t ftm_shared_offs;
t_ftmAccess* p;
t_ftmAccess ftmAccess;


uint64_t cpus2thrs(uint32_t cpus);
uint32_t thrs2cpus(uint64_t thrs);

uint32_t ftmOpen(const char* netaddress, uint8_t overrideFWcheck); //returns bitField showing CPUs with valid DM firmware
void ftmClose(void);

//per DM
int ftmRst(void);
int ftmSetDuetime(uint64_t tdue);
int ftmSetTrntime(uint64_t ttrn);
int ftmSetMaxMsgs(uint64_t maxmsg);

//per CPU
int ftmCpuRst(uint32_t dstCpus);
int ftmFwLoad(uint32_t dstCpus, const char* filename);
int ftmSetPreptime(uint32_t dstCpus, uint64_t tprep);
int ftmGetStatus(uint32_t srcCpus, uint32_t* buff);
void ftmShowStatus(uint32_t* status, uint8_t verbose);

//per thread
int ftmThrRst(uint64_t dstBitField);

int ftmCommand(uint64_t dstThr, uint32_t command);
int ftmSignal(uint64_t dstThr, uint32_t offset, uint64_t value, uint64_t mask);
int ftmPutString(uint64_t dstThr, const char* sXml);
int ftmPutFile(uint64_t dstThr, const char* filename);
int ftmClear(uint64_t dstThr);
uint32_t ftmDump(uint64_t srcThr, uint32_t len, uint8_t actIna, char* stringBuf, uint32_t lenStringBuf);
int ftmSetBp(uint64_t dstThr, int32_t planIdx);




#endif
