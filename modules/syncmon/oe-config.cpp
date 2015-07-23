/* Synopsis */
/* ==================================================================================================== */
/* @file syncmon.cpp
 * @brief Simple monitor for timing nodes
 *
 * Copyright (C) 2014 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 * @author A. Hahn <a.hahn@gsi.de>
 *
 * @bug No know bugs.
 * 
 * TBD:
 * - LONG LONG runtime? Integer overflow(s)?
 * - Fix "format ‘%lld’ expects argument" warnings for raspberry pi and std linux/x84_64
 *
 * *****************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 * *****************************************************************************
 */

/* Includes */
/* ==================================================================================================== */
#include <etherbone.h>
#include <tlu.h>
#include <eca.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <signal.h>

/* Namespaces */
/* ==================================================================================================== */
using namespace GSI_ECA;
using namespace GSI_TLU;

/* Defines */
/* ==================================================================================================== */
#define LEMO_TOTAL_IOS            16
#define LEMO_OE_SETUP             0
#define TLU_ID                    0x4d78adfdU
#define TLU_VENDOR                0x651
#define DEBUG_MODE                0
#define TARGET_PLATFORM_UNKNOWN   0
#define TARGET_PLATFORM_32        1
#define TARGET_PLATFORM_64        2
#ifdef ARCH_32_BIT
  #define TP                      TARGET_PLATFORM_32
#else
  #ifdef ARCH_64_BIT
    #define TP                    TARGET_PLATFORM_64
  #else
    #define TP                    TARGET_PLATFORM_UNKNOWN
  #endif
#endif

/* Structures */
/* ==================================================================================================== */
typedef struct
{
  uint64_t uTotalEvents;
  uint64_t uLastTimestamp;
  uint64_t uLatestPrintedEvent;
  int64_t iLastDiff;
  int64_t iMaxPast;
  int64_t iMinPast;
  int64_t iMaxFuture;
  int64_t iMinFuture;
  int64_t iDiffSum;
} s_IOMeasurement;

/* Global */
/* ==================================================================================================== */
volatile sig_atomic_t s_sigint = 0;
volatile sig_atomic_t s_dump = 0;

/* Function vSignalHandler */
/* ==================================================================================================== */
void vSignalHandler(int sig)
{
  s_sigint = 1;
}

/* Function main */
/* ==================================================================================================== */
int main (int argc, const char** argv)
{
  
  /* Etherbone */
  Socket socket;
  Device device;
  status_t status;
  
  /* Try to open a (etherbone-) socket */
  socket.open();
  if ((status = device.open(socket, argv[1])) != EB_OK) 
  {
    fprintf(stderr, "%s: failed to open %s: (status %s)\n", argv[0], argv[1], eb_status(status));
    exit(1);
  }
  else
  {
    fprintf(stdout, "%s: succeeded to open %s (status %s)\n", argv[0], argv[1], eb_status(status));
  }
  
  /* Find the IO reconfig to enable/disable outputs to specific IOs */
  std::vector<sdb_device> devs;
  device.sdb_find_by_identity(TLU_VENDOR, TLU_ID, devs);
  assert (devs.size() == 1);
  address_t ioconf = devs[0].sdb_component.addr_first;
  device.write(ioconf, EB_DATA32, 0xffffffff);
  ioconf = devs[0].sdb_component.addr_first + 4;
  device.write(ioconf, EB_DATA32, 0xffffffff);
  
  /* Done */
  return (0);
  
}
