#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <etherbone.h>

#define PSTATS_CNT_PP 20   // counter per port
#define PSTATS_CNT_PW 2    // counter per word
#define PSTATS_ADR_PP ((PSTATS_CNT_PP+PSTATS_CNT_PW-1)/PSTATS_CNT_PW)
#define PSTATS_MSB_MSK 0xffff0000
#define PSTATS_NPORTS 1

#define WR_CERN_GEN_VENDOR    0xce42
#define WR_GSI_GEN_VENDOR     0x0651
#define WR_PSTATS_GEN_PRODUCT 0x6a0c4d4d
#define WR_EP_GEN_PRODUCT     0x650c2d4f
#define WR_PPS_GEN_PRODUCT    0xde0d8ced
#define WR_IP_CFG_PRODUCT     0x68202b22

#define WR_PSTATS_GEN_VMAJOR    1           //major revision
#define WR_PSTATS_GEN_VMINOR    1           //minor revision

#define CR           0x0
#define L1_CNT_VAL   0x4
#define L2_CNT_VAL   0x8
#define DEBUG        0xc
#define EIC_IDR      0x20
#define EIC_IER      0x24
#define EIC_IMR      0x28
#define EIC_ISR      0x2C

#define WBGEN2_GEN_MASK(offset, size) (((1<<(size))-1) << (offset))
#define PSTATS_CR_RD_EN                       WBGEN2_GEN_MASK(0, 1)
#define PSTATS_CR_RD_IRQ                      WBGEN2_GEN_MASK(1, 1)

#define WR_ENDPOINT_MACHI       0x24        //MAC high bytes
#define WR_ENDPOINT_MACLO       0x28        //MAC low bytes

#define WR_PPS_GEN_CNTR_UTCLO   0x8         //UTC seconds low bytes
#define WR_PPS_GEN_CNTR_UTCHI   0xc         //UTC seconds high bytes
#define WR_PPS_GEN_CNTR_NSEC    0x4         //UTC nanoseconds
#define WR_PPS_GEN_ESCR         0x1c        //External Sync Control Register

#define EB_IPV4                 0x18

static const char* program;

static void help(void) {

  fprintf(stderr, "Usage: %s [OPTION] <proto/host/post> \n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "   -k keep running the command\n");
  fprintf(stderr, "   -h displays this help\n");
  fprintf(stderr, "    \n");
  fprintf(stderr, "Report bugs to <c.prados@gsi.de>\n");
}

static int hwcnt_to_sw(uint32_t l1_val, uint32_t l2_val, int n)
{
  uint32_t lsb, msb;

  lsb = ((l1_val & (0xffff<<16*n))>>16*n)&0xffff;
  msb = ((l2_val & (0xffff<<16*n))>>16*n)&0xffff;

  return (msb<<8) | lsb;
}

int main(int argc, char** argv) {
  eb_status_t status;
  eb_device_t device;
  eb_socket_t socket;
  struct sdb_device sdbDevice;
  eb_address_t wrPSTATS;
  eb_address_t wrEP;
  eb_address_t wrPPS;
  eb_address_t wrIP;
  eb_cycle_t cycle;
  int nDevices;
  uint32_t opt,error;
  uint8_t  ip[4];
  uint32_t adr;
  uint32_t ov;
  uint32_t ov_tx;
  uint32_t ov_rx;
  eb_data_t val[2];
  uint8_t keep_run = 0;
  uint8_t keep = 1;
  uint8_t net_val;
  char mac[20];
  eb_data_t data, data2;
  const char* devName;
  unsigned int ptr;
  int i;
  char time[60];
  time_t secs;
  const struct tm* tm;
  char info[][20] = {{"Tx Underun"}, {"Rx Overrun"}, {"Rx Invalid Code"}, {"Rx Sync Lost"}, {"Rx Pause"}, // 1,2,3,4,5
                     {"Rx P-Filter Drop"}, {"Rx PCS Error"}, {"Rx Giant"}, {"Rx Runt"}, {"Rx CRC Error"}, // 6,7,8,9,10
                     {"Rpcl_0"}, {"Rpcl_1"}, {"Rpcl_2"}, {"Rpcl_3"}, {"Rpcl_4"}, // 11,12,13,14,15
                     {"Rpcl_5"}, {"Rpcl_6"}, {"Rpcl_7"}, {"Tx"}, {"Rx"}};// 16,17,18,19,20
  uint32_t info_val[20];

  program = argv[0];
  error = 0;

  while((opt = getopt(argc, argv, "k:h")) != -1) {
    switch (opt) {
       case 'k':
        keep_run = 1;
        break;
       case 'h':
        help();
        return 1;
        break;
       default:
        fprintf(stderr, "%s: bad getopt result\n", program);
        error = 1;
    }
  }

  if (error) return 1;

  devName = argv[argc-1];

  if (optind-1 >= argc) {
    fprintf(stderr, "Syntax: %s <protocol/host/port>\n", argv[argc-optind]);
    exit(1);
  }

  /* Setting EB */
  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_ADDR32|EB_DATAX, &socket)) != EB_OK) {
    fprintf(stderr, "%s: failed to open eb socket: %s\n", program, eb_status(status));
    exit(1);
  }

 if ((status = eb_device_open(socket, devName, EB_ADDR32|EB_DATAX, 3, &device)) != EB_OK) {
    fprintf(stderr, "%s: failed to open eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  nDevices = 1;

  /* WR PSTATS */
  if ((status = eb_sdb_find_by_identity(device, WR_CERN_GEN_VENDOR, WR_PSTATS_GEN_PRODUCT, &sdbDevice, &nDevices)) != EB_OK) {
    fprintf(stderr, "%s: failed to find eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  if (nDevices != 1) {
    fprintf(stderr, "%s: failed to find eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  if (WR_PSTATS_GEN_VMAJOR != sdbDevice.abi_ver_major) {
    fprintf(stderr, "%s: major version conflicting -interface changed %s\n", program, eb_status(status));
    exit(1);
  }

  if (WR_PSTATS_GEN_VMINOR > sdbDevice.abi_ver_minor) {
    fprintf(stderr, "%s: minor version conflicting -interface changed %s\n", program, eb_status(status));
    exit(1);
  }

  wrPSTATS = sdbDevice.sdb_component.addr_first;

  nDevices = 1;

  /* WR END POINT */
  if ((status = eb_sdb_find_by_identity(device, WR_CERN_GEN_VENDOR, WR_EP_GEN_PRODUCT, &sdbDevice, &nDevices)) != EB_OK) {
    fprintf(stderr, "%s: failed to find eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  if (nDevices != 1) {
    fprintf(stderr, "%s: failed to find eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  if (WR_PSTATS_GEN_VMAJOR != sdbDevice.abi_ver_major) {
    fprintf(stderr, "%s: major version conflicting -interface changed %s\n", program, eb_status(status));
    exit(1);
  }

  if (WR_PSTATS_GEN_VMINOR > sdbDevice.abi_ver_minor) {
    fprintf(stderr, "%s: minor version conflicting -interface changed %s\n", program, eb_status(status));
    exit(1);
  }

  wrEP = sdbDevice.sdb_component.addr_first;

  nDevices = 1;

  /* WR PPS */
  if ((status = eb_sdb_find_by_identity(device, WR_CERN_GEN_VENDOR, WR_PPS_GEN_PRODUCT, &sdbDevice, &nDevices)) != EB_OK) {
    fprintf(stderr, "%s: failed to find eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  if (nDevices != 1) {
    fprintf(stderr, "%s: failed to find eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  if (WR_PSTATS_GEN_VMAJOR != sdbDevice.abi_ver_major) {
    fprintf(stderr, "%s: major version conflicting -interface changed %s\n", program, eb_status(status));
    exit(1);
  }

  if (WR_PSTATS_GEN_VMINOR > sdbDevice.abi_ver_minor) {
    fprintf(stderr, "%s: minor version conflicting -interface changed %s\n", program, eb_status(status));
    exit(1);
  }

  wrPPS = sdbDevice.sdb_component.addr_first;

  nDevices = 1;

  /* WR EB CFG */
  if ((status = eb_sdb_find_by_identity(device, WR_GSI_GEN_VENDOR, WR_IP_CFG_PRODUCT, &sdbDevice, &nDevices)) != EB_OK) {
    fprintf(stderr, "%s: failed to find eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  if (nDevices != 1) {
    fprintf(stderr, "%s: failed to find eb device: %s\n", program, eb_status(status));
    exit(1);
  }

  if (WR_PSTATS_GEN_VMAJOR != sdbDevice.abi_ver_major) {
    fprintf(stderr, "%s: major version conflicting -interface changed %s\n", program, eb_status(status));
    exit(1);
  }

  if (WR_PSTATS_GEN_VMINOR > sdbDevice.abi_ver_minor) {
    fprintf(stderr, "%s: minor version conflicting -interface changed %s\n", program, eb_status(status));
    exit(1);
  }

  wrIP = sdbDevice.sdb_component.addr_first;

  /* getting IP */
  if ((status = eb_device_read(device, wrIP + EB_IPV4, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK) {
    fprintf(stderr, "%s: Falied to IP reg %s\n", program, eb_status(status));
    exit(1);
  }

  memcpy(&ip, &data, 4);
  /* getting MAC */
  if ((status = eb_cycle_open(device, 0, eb_block, &cycle)) != EB_OK) {
    fprintf(stderr, "%s: Falied to open read cycle %s\n", program, eb_status(status));
    exit(1);
  }

  eb_cycle_read(cycle, wrEP + WR_ENDPOINT_MACHI, EB_BIG_ENDIAN|EB_DATA32, &data);
  eb_cycle_read(cycle, wrEP + WR_ENDPOINT_MACLO, EB_BIG_ENDIAN|EB_DATA32, &data2);

  if ((status = eb_cycle_close(cycle)) != EB_OK) {
    fprintf(stderr, "%s: Falied to close read cycle %s\n", program, eb_status(status));
    exit(1);
  }

  snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
    (int)((data  >>  8) & 0xff), (int)((data  >>  0) & 0xff),
    (int)((data2 >> 24) & 0xff), (int)((data2 >> 16) & 0xff),
    (int)((data2 >>  8) & 0xff), (int)((data2 >>  0) & 0xff));

  /* Disable "IRQ" PSTATS interrupt */
  if ((status = eb_device_write(device, wrPSTATS + EIC_IDR, EB_BIG_ENDIAN|EB_DATA32, 0x1, 0, eb_block)) != EB_OK) {
    fprintf(stderr, "%s: Failed to disable PSTATS IRQ %s\n", program, eb_status(status));
    exit(1);
  }

  /* Enable "IRQ" PSTATS interrupt */
  if ((status = eb_device_write(device, wrPSTATS + EIC_IER, EB_BIG_ENDIAN|EB_DATA32, 0x1, 0, eb_block)) != EB_OK) {
    fprintf(stderr, "%s: Failed to enable PSTATS IRQ %s\n", program, eb_status(status));
    exit(1);
  }

  while(keep)
  {
    if (keep_run) system("clear");
    else keep = 0;

    net_val = 0;
    for( adr = 0; adr < PSTATS_ADR_PP; ++adr) {

      val[0] = (adr<<16 | 1<<8 | PSTATS_CR_RD_EN);

      //printf("------ Addres ------- %d \n",adr);
      //printf("ADDR %d \n",val[0]);
      /* Set port and counter that is going to be read */
      if ((status = eb_device_write(device, wrPSTATS + CR, EB_BIG_ENDIAN|EB_DATA32, val[0], 0, eb_block)) != EB_OK) {
        fprintf(stderr, "%s: Failed to set Port and Counter to be read %s\n", program, eb_status(status));
        exit(1);
      }

      /* Read port and counter that is going to be read */
      if ((status = eb_device_read(device, wrPSTATS + L1_CNT_VAL, EB_BIG_ENDIAN|EB_DATA32, &val[0], 0, eb_block)) != EB_OK) {
        fprintf(stderr, "%s: Failed to read Port and L1 Counter to be read %s\n", program, eb_status(status));
        exit(1);
      }

      if ((status = eb_device_read(device, wrPSTATS + L2_CNT_VAL, EB_BIG_ENDIAN|EB_DATA32, &val[1], 0, eb_block)) != EB_OK) {
        fprintf(stderr, "%s: Failed to read Port and L2 Counter to be read %s\n", program, eb_status(status));
        exit(1);
      }

      //printf("VAL[0] %x VAL[1] %x\n",val[0],val[1]);

      ptr = 0;
      for (i = 0; i<= PSTATS_CNT_PW-1 ; i++) {
        if(PSTATS_CNT_PW*adr+1 >= PSTATS_CNT_PP)
          break;
        ptr = 0x0;
        ptr &= PSTATS_MSB_MSK;
        ptr |= hwcnt_to_sw(val[0], val[1], i);
        //printf("ptr %d \n",ptr);
        info_val[net_val] = ptr;
        net_val++;
      }
    }

  /* HACK for getting the overflow of tx and rx, we don't get HW interrupts... */
  //val[1] = 459009; for 4 words per counter
  val[1] = 983297;
  if ((status = eb_device_write(device, wrPSTATS + CR, EB_BIG_ENDIAN|EB_DATA32, val[1], 0, eb_block)) != EB_OK) {
    fprintf(stderr, "%s: Failed to set Port and Counter to be read %s\n", program, eb_status(status));
    exit(1);
  }

  if ((status = eb_device_read(device, wrPSTATS + L2_CNT_VAL, EB_BIG_ENDIAN|EB_DATA32, &val[1], 0, eb_block)) != EB_OK) {
    fprintf(stderr, "%s: Failed to read Port and L2 Counter to be read %s\n", program, eb_status(status));
    exit(1);
  }
  ov = val[1];
  ov_rx = (uint32_t)((ov&0xffff0000)>>16);
  ov_tx = (uint32_t)((ov&0x0000ffff));
  //printf("OV %x ov_rx %d ov_tx %d \n",ov,ov_rx,ov_tx);
  //printf("tx %d rx %d \n",info_val[19],(uint32_t)info_val[18]);
  info_val[19] = (uint32_t)(info_val[19] + (0xffff*ov_rx));
  info_val[18] = (uint32_t)(info_val[18] + (0xffff*ov_tx));

  /* HACK for getting the overflow of tx and rx */

  /* getting time of day */
  if ((status = eb_device_read(device, wrPPS + WR_PPS_GEN_CNTR_UTCLO, EB_BIG_ENDIAN|EB_DATA32, &data, 0, eb_block)) != EB_OK) {
    fprintf(stderr, "%s: Falied to read time from PPS %s\n", program, eb_status(status));
    exit(1);
  }

  secs = data;

  tm = localtime(&secs);
  strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S %Z", tm);

    //printf("GSI-Timing Receiver %s \n",time);
    //printf("MAC %s \t IP %d.%d.%d.%d\n",mac, ip[3],ip[2],ip[1],ip[0]);
    printf("%s: %d  \t\t %s: %d \n", info[19],info_val[19], info[18],info_val[18]);

    printf("%s:\x07 %d  \t %s:\x07 %d \t %s:\x07 %d \t\n", info[9],info_val[9], info[8],info_val[8], info[6], info_val[6]);
    printf("%s:\x07 %d  \t\t %s:\x07 %d \t %s:\x07 %d\n", info[0],info_val[0], info[1],info_val[1], info[7], info_val[7]);
    printf("%s:\x07 %d  \t %s:\x07 %d \t %s:\x07 %d\n", info[3],info_val[3], info[4],info_val[4], info[5], info_val[5]);
    printf("%s:\x07 %d \n", info[2], info_val[2]);
    printf("Rpcl:\x07 %d %d %d %d %d %d %d %d\n", info_val[10], info_val[11], info_val[12], info_val[13],
                                              info_val[14], info_val[15], info_val[16], info_val[17]);
    if (keep) sleep(1);
  }

  /* close handler cleanly */
  if ((status = eb_device_close(device)) != EB_OK) {
    fprintf(stderr, "%s: Failed to close eb device %s\n", program, eb_status(status));
    exit(1);
  }


  if ((status = eb_socket_close(socket)) != EB_OK) {
    fprintf(stderr, "%s: Failed to eb socket device %s\n", program, eb_status(status));
    exit(1);
  }

  return 0;
}
