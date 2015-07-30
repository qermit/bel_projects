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


  printf("%s",mac);

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
