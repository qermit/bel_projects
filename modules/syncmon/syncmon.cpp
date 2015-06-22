/* Synopsis */
/* ==================================================================================================== */
/*
 * @file syncmon.cpp
 * @brief Simple monitor for timing nodes
 *
 * Copyright (C) 2014 GSI Helmholtz Centre for Heavy Ion Research GmbH 
 *
 * @author A. Hahn <a.hahn@gsi.de>
 *
 * @bug No know bugs.
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
#define EXPLODER5_IOS             16
#define EXPLODER5_LEMO_OFFSET     4
#define EXPLODER5_OE_SETUP        0x0000
#define LEMO_OE_OFFSET_REG        4
#define MAX_JITTER_NS             2
#define MAX_ALLOWED_JITTER_NS     200
#define EXPECTED_PULSE_DIFF_NS    100000000
#define TLU_ID                    0x4d78adfdU
#define TLU_VENDOR                0x651

/* Structures */
/* ==================================================================================================== */
typedef struct
{
  uint64_t uTotalEvents;
  uint64_t uLastTimestamp;
  uint64_t uMaxPast;
  uint64_t uMinPast;
  uint64_t uMaxFuture;
  uint64_t uMinFuture;
  double uAverage;
  double uStdDeviation;
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
  
  /* TLU */
  std::vector<TLU> tlus;
  std::vector<std::vector<uint64_t> > queues;
  s_IOMeasurement a_sIOMeasurement[EXPLODER5_IOS];

  /* Format and Measurement */
  uint32_t uQueueIterator = 0;
  uint32_t uQueuesTotal = 0;
  uint32_t uQueneItems = 0;
  uint64_t uTimeDiff = 0;
  uint32_t uIterator = 0;
  
  /* Logging */
  FILE *fp;
  char a_cFileNameBuffer[0x100];
  int iSysCallRes = 0;

  /* Clean results */
  for(uIterator = 0; uIterator < EXPLODER5_IOS; uIterator++)
  {
    a_sIOMeasurement[uIterator].uTotalEvents = 0;
    a_sIOMeasurement[uIterator].uLastTimestamp = 0;
    a_sIOMeasurement[uIterator].uMaxPast = 0;
    a_sIOMeasurement[uIterator].uMinPast = 0;
    a_sIOMeasurement[uIterator].uMaxFuture = 0;
    a_sIOMeasurement[uIterator].uMinFuture = 0;
    a_sIOMeasurement[uIterator].uAverage = 0;
    a_sIOMeasurement[uIterator].uStdDeviation = 0;
  }

  /* Setup signal handler */
  signal(SIGINT, vSignalHandler); 

  /* Try to open a (etherbone-) socket */
  socket.open();
  if ((status = device.open(socket, argv[1])) != EB_OK) 
  {
    fprintf(stderr, "%s: failed to open %s: (status %s)\n", argv[0], argv[1], eb_status(status));
    return 1;
  }
  else
  {
    fprintf(stdout, "%s: succeeded to open %s (status %s)\n", argv[0], argv[1], eb_status(status));
  }
  
  /* Find the TLU */
  TLU::probe(device, tlus);
  assert (tlus.size() == 1);
  TLU& tlu = tlus[0];
  
  /* Configure the TLU to record rising edge timestamps */
  tlu.hook(-1, false);
  tlu.set_enable(false); // no interrupts, please
  tlu.clear(-1);
  tlu.listen(-1, true, true, 8); /* Listen on all inputs */
  
  /* Find the IO reconfig to enable/disable outputs to specific IOs */
  std::vector<sdb_device> devs;
  device.sdb_find_by_identity(TLU_VENDOR, TLU_ID, devs);
  assert (devs.size() == 1);
  address_t ioconf = devs[0].sdb_component.addr_first + LEMO_OE_OFFSET_REG;
  device.write(ioconf, EB_DATA32, EXPLODER5_OE_SETUP);
  
  /* Check TLU */
  while (true)
  {
    if(s_sigint || s_dump)
    {
      /* Create debug dump */
      for(uIterator = 0; uIterator < EXPLODER5_IOS; uIterator++)
      {
        /* Did we capture any event here? */
        if(a_sIOMeasurement[uIterator].uTotalEvents)
        {
          /* Don't create log file for ts0 IO */
          if(uIterator!=EXPLODER5_LEMO_OFFSET)
          {
            /* Create or overwrite a log file */
            snprintf(a_cFileNameBuffer, sizeof(a_cFileNameBuffer), "log/syncmon_dev_io%d.log", uIterator-EXPLODER5_LEMO_OFFSET);
            fp = fopen(a_cFileNameBuffer, "w");
            /* Print result to file */
            fprintf(fp, "Results for IO%d (device %d):\n", uIterator, uIterator-EXPLODER5_LEMO_OFFSET);
            fprintf(fp, "  Events:           %ld\n", a_sIOMeasurement[uIterator].uTotalEvents);
            fprintf(fp, "  Latest Timestamp: %ld\n", a_sIOMeasurement[uIterator].uLastTimestamp);
            fprintf(fp, "  Max. Past:        %ldns\n", a_sIOMeasurement[uIterator].uMaxPast);
            fprintf(fp, "  Min. Past:        %ldns\n", a_sIOMeasurement[uIterator].uMinPast);
            fprintf(fp, "  Max. Future:      %ldns\n", a_sIOMeasurement[uIterator].uMaxFuture);
            fprintf(fp, "  Min. Future:      %ldns\n", a_sIOMeasurement[uIterator].uMinFuture);
            /* Calculate statistics */
            a_sIOMeasurement[uIterator].uAverage /= a_sIOMeasurement[uIterator].uTotalEvents;
            fprintf(fp, "  Average:          %fns\n\n", a_sIOMeasurement[uIterator].uAverage);
            /* TODO: StdDeviation */
            /* Close file */
            fclose(fp);
          }
        }
      }
      /* Reset signals and maybe quit */
      if (s_sigint != 0) { s_sigint = 0; exit(0); }
      if (s_dump != 0)   { s_dump = 0; }
    }

    /* TODO: Poll device */
    usleep(500000);

    /* Read-out result */
    tlu.pop_all(queues);
    uQueuesTotal = queues.size();

    /* Check each queue now */
    for(uQueueIterator=0; uQueueIterator<uQueuesTotal; uQueueIterator++)
    {
      
      std::vector<uint64_t>& queue = queues[uQueueIterator];
      uQueneItems = queue.size(); /* Get the actual size */
      
      /* Inspect items with queue contains data */
      if(uQueneItems > a_sIOMeasurement[EXPLODER5_LEMO_OFFSET].uTotalEvents)
      {
        iSysCallRes = system("clear");
        fprintf(stdout, "%s: Latest TS                 Count     Offset ts0  MaxFuture  MinFuture  MaxPast  MinPast\n", argv[0]);
        fprintf(stdout, "%s: --------------------------------------------------------------------------------------\n", argv[0]);
        /* Got new data, create log dump after printing */
        s_dump = 1;
      }
      
      if(uQueneItems > a_sIOMeasurement[uQueueIterator].uTotalEvents)
      {
          
          /* Calculate time difference */
          uTimeDiff = queue[uQueneItems-1]-a_sIOMeasurement[EXPLODER5_LEMO_OFFSET].uLastTimestamp;
        
          /* Event seen */
          if(uQueneItems > a_sIOMeasurement[uQueueIterator].uTotalEvents)
          {
            a_sIOMeasurement[uQueueIterator].uAverage += (int64_t)uTimeDiff;
          }
        
          a_sIOMeasurement[uQueueIterator].uTotalEvents = uQueneItems;
          a_sIOMeasurement[uQueueIterator].uLastTimestamp = queue[uQueneItems-1];
          
          fprintf(stdout, "%s: ts%d: %019ld  %08d", argv[0], uQueueIterator-EXPLODER5_LEMO_OFFSET, queue[uQueneItems-1], uQueneItems);
          
          if (uQueueIterator-EXPLODER5_LEMO_OFFSET == 0)
          {
            fprintf(stdout, "\n");
          }
          else
          {
            /* Is this a pulse after or before ts0? */
            if ((int64_t)uTimeDiff < 0)
            {
              /* Update differences */
              if((int64_t)uTimeDiff<(int64_t)a_sIOMeasurement[uQueueIterator].uMaxFuture) { a_sIOMeasurement[uQueueIterator].uMaxFuture = uTimeDiff; }
              if((int64_t)uTimeDiff>(int64_t)a_sIOMeasurement[uQueueIterator].uMinFuture) { a_sIOMeasurement[uQueueIterator].uMinFuture = uTimeDiff; }
              /* This is the first time we see an event, set time marks */
              if(uQueneItems==1)
              {
                a_sIOMeasurement[uQueueIterator].uMaxFuture = uTimeDiff;
                a_sIOMeasurement[uQueueIterator].uMinFuture = uTimeDiff;
              }
            }
            else if ((int64_t)uTimeDiff > 0)
            {
              /* Update differences */
              if((int64_t)uTimeDiff>(int64_t)a_sIOMeasurement[uQueueIterator].uMaxPast) { a_sIOMeasurement[uQueueIterator].uMaxPast = uTimeDiff; }
              if((int64_t)uTimeDiff<(int64_t)a_sIOMeasurement[uQueueIterator].uMinPast) { a_sIOMeasurement[uQueueIterator].uMinPast = uTimeDiff; }
              /* This is the first time we see an event, set time marks */
              if(uQueneItems==1)
              {
                a_sIOMeasurement[uQueueIterator].uMaxPast = uTimeDiff;
                a_sIOMeasurement[uQueueIterator].uMinPast = uTimeDiff;
              }
            }
            else
            {
              /* Set minimal values to zero, because there is no difference */
              a_sIOMeasurement[uQueueIterator].uMinFuture = 0;
              a_sIOMeasurement[uQueueIterator].uMinPast = 0;
            }
            
            fprintf(stdout, "  %+04ldns      %+04ldns     %+04ldns     %+04ldns   %+04ldns\n", uTimeDiff, 
                                                                              a_sIOMeasurement[uQueueIterator].uMaxFuture,
                                                                              a_sIOMeasurement[uQueueIterator].uMinFuture,
                                                                              a_sIOMeasurement[uQueueIterator].uMaxPast,
                                                                              a_sIOMeasurement[uQueueIterator].uMinPast);
          }
      }
    }
  }

  /* Should never get here */
  return iSysCallRes;
  
}
