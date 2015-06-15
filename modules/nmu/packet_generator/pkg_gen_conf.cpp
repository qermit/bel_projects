#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS
//PRIX32: these macros are defined for C program. They are defined for C++ only when  __STDC_FORMAT_MACROS is defined before <inttypes.h> is included.
#define __STDC_CONSTANT_MACROS
//UINT32_C:  these macros are defined for C program. They are defined for C++ only when  __STDC_CONSTRANT_MACROS is defined before <stdint.h> is included.
#include <etherbone.h>
#include <tlu.h>
#include <eca.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <unistd.h> // sleep
#include <vector>
#include <iostream>
#include <math.h>

#define s_rate_max 0x14;
#define s_load_max 0x4;

unsigned long int bandwidth;
eb_data_t payload_length;
eb_data_t rate;
eb_data_t r_rate_max;
int pkg_length;
char * value_end;

using namespace GSI_ECA;
using namespace GSI_TLU;



int main(int argc, const char** argv) {
  Socket socket;
  Device device;
  status_t status;

  if (argc != 4) {
    fprintf(stderr, "%s: expecting argument <device>\n", argv[0]);
    return 1;
  }

  socket.open();
  if ((status = device.open(socket, argv[1])) != EB_OK) {
    fprintf(stderr, "%s: failed to open %s: %s\n", argv[0], argv[1], eb_status(status));
    return 1;
  }

  /*Find the FEC packet generator*/
  std::vector<sdb_device> devs;
  device.sdb_find_by_identity(0x651, 0x53bee0e2, devs);
  assert (devs.size() == 1);
  address_t pkg_gen = devs[0].sdb_component.addr_first;
  address_t rate_addr = pkg_gen + s_rate_max;
  address_t length_addr = pkg_gen + s_load_max;


  bandwidth = strtoull(argv[2], &value_end, 10);
  payload_length = strtoull (argv[3], &value_end, 10);

 // printf("~~~~~~~~~~~~%x------------------", (payload_length/2)<<16);
  printf("----------------------------\n bandwidth = %d bps\n payload_length = %d Bytes\n payload_length = 0x%x Bytes\n",bandwidth, payload_length, payload_length);

  //set the payload length Range 46-1500 byte
  if(status = device.write(length_addr, EB_BIG_ENDIAN|EB_DATA32,((payload_length/2)<<16))!= EB_OK)
    printf("Payload length error.\n");

  // payload + header length (bit)
  pkg_length = (payload_length + 14)*8;
  // Number of the packet
  rate = bandwidth/pkg_length;
  // clk ticks of one packet
  r_rate_max = 62500000/rate;

  //set the rate for the packet generator
  if(status = device.write(rate_addr, EB_BIG_ENDIAN|EB_DATA32, r_rate_max)!= EB_OK)
  printf("Rate setting error. \n");

  printf(" Packet length = %d Bytes\n Packet number per second = %d\n ", pkg_length/8, rate);
  return 0;
}
