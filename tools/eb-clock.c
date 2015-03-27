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

static void clk(int halfper, int *hperr, int *maskr)
{
  int pp, p, i;

  p = halfper;
  if (p < 1) {
    fprintf(stderr, "Error: p cannot be less than 1\n");
    exit(1);
  }

  // p is the length of HALF the period
  pp = p;
  while (pp < 8) pp += p;

  int b = 0;
  for (i = 15; i >= 0; --i) {
    // offset = 7-i
    b |= ((7-i)%p == 0) << i;
  }

  *hperr = p;
  *maskr = b;
}

int main(int argc, char** argv) {
  int opt, error, c;
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

  if (optind + 1 != argc) {
    fprintf(stderr, "%s: expecting one non-optional arguments: <proto/host/port> <num_cycles>\n", program);
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

  int p, m;

  int hp1, hp2, hp3;

  printf("hp1 = ");
  scanf("%d", &hp1);
  printf("hp2 = ");
  scanf("%d", &hp2);
  printf("hp3 = ");
  scanf("%d", &hp3);

  /*-------------------------------------------------------------------------*/
  clk(hp1, &p, &m);

  if ((status = eb_device_write(device, selr, EB_DATA32, 0, 0, 0)) != EB_OK)
    die("eb_device_write(selr)", status);

  if ((status = eb_device_write(device, hperr, EB_DATA32, p, 0, 0)) != EB_OK)
    die("eb_device_write(hperr)", status);

  if ((status = eb_device_write(device, maskr, EB_DATA32, m, 0, 0)) != EB_OK)
    die("eb_device_write(maskr)", status);

  /*-------------------------------------------------------------------------*/
  clk(hp2, &p, &m);

  if ((status = eb_device_write(device, selr, EB_DATA32, 1, 0, 0)) != EB_OK)
    die("eb_device_write(selr)", status);

  if ((status = eb_device_write(device, hperr, EB_DATA32, p, 0, 0)) != EB_OK)
    die("eb_device_write(hperr)", status);

  if ((status = eb_device_write(device, maskr, EB_DATA32, m, 0, 0)) != EB_OK)
    die("eb_device_write(maskr)", status);

  /*-------------------------------------------------------------------------*/
  clk(hp3, &p, &m);

  if ((status = eb_device_write(device, selr, EB_DATA32, 2, 0, 0)) != EB_OK)
    die("eb_device_write(selr)", status);

  if ((status = eb_device_write(device, hperr, EB_DATA32, p, 0, 0)) != EB_OK)
    die("eb_device_write(hperr)", status);

  if ((status = eb_device_write(device, maskr, EB_DATA32, m, 0, 0)) != EB_OK)
    die("eb_device_write(maskr)", status);

  return 0;
}

