#define _POSIX_C_SOURCE 200112L /* strtoull */

#include <unistd.h> /* getopt */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <etherbone.h>

#define GSI_ID	0x651
#define SERDES_CLK_GEN_ID	0x5f3eaf43

const char *program;

static void help(void) {
  fprintf(stderr, "Usage: %s [OPTION] <proto/host/port>\n", program);
  fprintf(stderr, "\n");
  fprintf(stderr, "  -h             display this help and exit\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "Report bugs to <w.terpstra@gsi.de>\n");
}

static void die(const char* msg, eb_status_t status) {
  fprintf(stderr, "%s: %s: %s\n", program, msg, eb_status(status));
  exit(1);
}

int main(int argc, char** argv) {
  int opt, error, c, i;
  struct sdb_device sdb;
  eb_status_t status;
  eb_socket_t socket;
  eb_device_t device;

  /* Default arguments */
  program = argv[0];
  error = 0;

  /* Process the command-line arguments */
  error = 0;
  while ((opt = getopt(argc, argv, "h")) != -1) {
    switch (opt) {
    case 'h':
      help();
      return 0;
    case ':':
    case '?':
      error = 1;
      break;
    default:
      fprintf(stderr, "%s: bad getopt result\n", program);
      error = 1;
    }
  }

  if (error) return 1;

  if (optind + 2 != argc) {
    fprintf(stderr, "%s: expecting two non-optional arguments: <proto/host/port> <num_cycles>\n", program);
    return 1;
  }

  if ((status = eb_socket_open(EB_ABI_CODE, 0, EB_DATAX|EB_ADDRX, &socket)) != EB_OK)
    die("eb_socket_open", status);

  if ((status = eb_device_open(socket, argv[optind], EB_DATAX|EB_ADDRX, 3, &device)) != EB_OK)
    die(argv[optind], status);

  c = 1;
  if ((status = eb_sdb_find_by_identity(device, GSI_ID, SERDES_CLK_GEN_ID, &sdb, &c)) != EB_OK)
    die("eb_sdb_find_by_identity", status);
  if (c != 1) {
    fprintf(stderr, "Found %d clk gen identifiers on that device\n", c);
    exit(1);
  }

  /* Clocking paraphernaelia */
  int first, selr, hperr, maskr;
  first = sdb.sdb_component.addr_first;
  selr = first;
  hperr = first + 4;
  maskr = first + 8;

  int pp = 0;
  int p = 0;
  p = atol(argv[optind+1]);
  printf("%d\n", p);
  if (p < 1) {
    fprintf(stderr, "Error: p cannot be less than 1\n");
    return 1;
  }

  // p is the length of HALF the period
  pp = p;
  while (pp < 8) pp += p;
  printf("pp = %d\n", pp);

  int b = 0;
  for (i = 15; i >= 0; --i) {
    // offset = 7-i
    printf("%d", (7-i)%p == 0);
    b |= ((7-i)%p == 0) << i;
  }
  printf("\n%x\n", b);

  eb_data_t d;

  if ((status = eb_device_write(device, selr, EB_DATA32, 0, 0, 0)) != EB_OK)
    die("eb_device_write(selr)", status);

  if ((status = eb_device_write(device, hperr, EB_DATA32, pp, 0, 0)) != EB_OK)
    die("eb_device_write(hperr)", status);

  if ((status = eb_device_write(device, maskr, EB_DATA32, b, 0, 0)) != EB_OK)
    die("eb_device_write(maskr)", status);

  if ((status = eb_device_read(device, selr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(selr)", status);
  printf("selr=%d\n", (int)d);

  if ((status = eb_device_read(device, hperr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(hperr)", status);
  printf("hperr=%d\n", (int)d);

  if ((status = eb_device_read(device, maskr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(maskr)", status);
  printf("maskr=%d\n", (int)d);

  if ((status = eb_device_write(device, selr, EB_DATA32, 1, 0, 0)) != EB_OK)
    die("eb_device_write(selr)", status);

  if ((status = eb_device_read(device, selr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(selr)", status);
  printf("selr=%d\n", (int)d);

  if ((status = eb_device_read(device, hperr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(hperr)", status);
  printf("hperr=%d\n", (int)d);

  if ((status = eb_device_read(device, maskr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(maskr)", status);
  printf("maskr=%d\n", (int)d);

  if ((status = eb_device_write(device, selr, EB_DATA32, 2, 0, 0)) != EB_OK)
    die("eb_device_write(selr)", status);

  if ((status = eb_device_read(device, selr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(selr)", status);
  printf("selr=%d\n", (int)d);

  if ((status = eb_device_read(device, hperr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(hperr)", status);
  printf("hperr=%d\n", (int)d);

  if ((status = eb_device_read(device, maskr, EB_DATA32, &d, 0, 0)) != EB_OK)
    die("eb_device_read(maskr)", status);
  printf("maskr=%d\n", (int)d);

  return 0;
}

