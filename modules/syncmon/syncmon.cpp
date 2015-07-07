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
 * - Check int/uint conversation
 * - Check failure cases (ts0 lost, ...)
 * - LONG LONG runtime? Integer overflow(s)?
 * - Fix "format ‘%lld’ expects argument" warnings for raspberry pi and std linux/x84_64
 * - Std. Deviation
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
#define DEBUG_MODE                0

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
    a_sIOMeasurement[uIterator].uLatestPrintedEvent = 0;
    a_sIOMeasurement[uIterator].uLastTimestamp = 0;
    a_sIOMeasurement[uIterator].iLastDiff = 0;
    a_sIOMeasurement[uIterator].iMaxPast = 0;
    a_sIOMeasurement[uIterator].iMinPast = 0;
    a_sIOMeasurement[uIterator].iMaxFuture = 0;
    a_sIOMeasurement[uIterator].iMinFuture = 0;
    a_sIOMeasurement[uIterator].iDiffSum = 0;
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
  
    /* Check for system call error */
    if (iSysCallRes)
    { 
      fprintf(stdout, "%s: System call error!\n", argv[0]);
      exit(1);
    }
    
    /* CTRL+C or dump time? */
    if(s_sigint || s_dump)
    {
      /* Create debug dump */
      for(uIterator = 0; uIterator < EXPLODER5_IOS; uIterator++)
      {
        /* Did we capture any event here? */
        if(a_sIOMeasurement[uIterator].uTotalEvents)
        {
          /* Create or overwrite a log file */
          snprintf(a_cFileNameBuffer, sizeof(a_cFileNameBuffer), "log/syncmon_dev_io%d.log", uIterator-EXPLODER5_LEMO_OFFSET);
          fp = fopen(a_cFileNameBuffer, "w");
          /* Print result to file */
          fprintf(fp, "Results for IO%d (device %d):\n", uIterator, uIterator-EXPLODER5_LEMO_OFFSET);
          fprintf(fp, "  Events:               %llu\n", a_sIOMeasurement[uIterator].uTotalEvents);
          fprintf(fp, "  Latest Timestamp:     %llu\n", a_sIOMeasurement[uIterator].uLastTimestamp);
          fprintf(fp, "  Latest Printed Event: %llu\n", a_sIOMeasurement[uIterator].uLatestPrintedEvent);
          fprintf(fp, "  Late Difference:      %llu\n", a_sIOMeasurement[uIterator].iLastDiff);
          fprintf(fp, "  Max. Past:            %lldns\n", a_sIOMeasurement[uIterator].iMaxPast);
          fprintf(fp, "  Min. Past:            %lldns\n", a_sIOMeasurement[uIterator].iMinPast);
          fprintf(fp, "  Max. Future:          %lldns\n", a_sIOMeasurement[uIterator].iMaxFuture);
          fprintf(fp, "  Min. Future:          %lldns\n", a_sIOMeasurement[uIterator].iMinFuture);
          /* Calculate statistics */
          fprintf(fp, "  Average:              %fns\n\n", (double)a_sIOMeasurement[uIterator].iDiffSum/(double)a_sIOMeasurement[uIterator].uTotalEvents);
          /* Close file */
          fclose(fp);
          
          if(a_sIOMeasurement[uIterator].uLatestPrintedEvent != a_sIOMeasurement[uIterator].uTotalEvents)
          {
            /* Concatenate data into plot log file */
            snprintf(a_cFileNameBuffer, sizeof(a_cFileNameBuffer), "log/syncmon_dev_plot_io%d.log", uIterator-EXPLODER5_LEMO_OFFSET);
            fp = fopen(a_cFileNameBuffer, "a+");
            /* Print result to file */
            fprintf(fp, "%llu %llu\n", a_sIOMeasurement[uIterator].uTotalEvents, a_sIOMeasurement[uIterator].uLastTimestamp);
            /* Close file */
            fclose(fp);
            /* Increase printed counter */
            a_sIOMeasurement[uIterator].uLatestPrintedEvent++;
          }
        }
      }
      /* Reset signals and maybe quit */
      if (s_sigint != 0) { s_sigint = 0; exit(0); }
      if (s_dump != 0)   { s_dump = 0; }
    }

    /* Sleep */
    usleep(100000);

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
        fprintf(stdout, "%s: Latest TS                 Count     Offset ts0  MaxFuture  MinFuture  MaxPast  MinPast  Average\n", argv[0]);
        fprintf(stdout, "%s: ----------------------------------------------------------------------------------------------------\n", argv[0]);
        /* Got new data, create log dump after printing */
        s_dump = 1;
        /* Make sure we all popped everything (even delay edges) */
        tlu.pop_all(queues);
      }
      
      /* Got something new? */
      if(uQueneItems > a_sIOMeasurement[uQueueIterator].uTotalEvents)
      {
        /* Make sure that we don't have more PPS pulse form any device under test than from our reference device */
        if(uQueneItems > a_sIOMeasurement[EXPLODER5_LEMO_OFFSET].uTotalEvents && uQueueIterator != EXPLODER5_LEMO_OFFSET)
        {
#if DEBUG_MODE
          fprintf(stdout, "%s: Bad measurement: %d % d\n", argv[0], uQueneItems, a_sIOMeasurement[EXPLODER5_LEMO_OFFSET].uTotalEvents);
#endif
        }
        else
        {
          /* Calculate time difference */
          uTimeDiff = queue[uQueneItems-1]-a_sIOMeasurement[EXPLODER5_LEMO_OFFSET].uLastTimestamp;

          /* Event seen? */
          if(uQueneItems > a_sIOMeasurement[uQueueIterator].uTotalEvents)
          {
            a_sIOMeasurement[uQueueIterator].iDiffSum += uTimeDiff;            
            a_sIOMeasurement[uQueueIterator].iLastDiff = uTimeDiff;       
          }
        
          a_sIOMeasurement[uQueueIterator].uTotalEvents = uQueneItems;
          a_sIOMeasurement[uQueueIterator].uLastTimestamp = queue[uQueneItems-1];
          
          fprintf(stdout, "%s: ts%d: %019llu  %08d", argv[0], uQueueIterator-EXPLODER5_LEMO_OFFSET, queue[uQueneItems-1], uQueneItems);
          
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
              if((int64_t)uTimeDiff<(int64_t)a_sIOMeasurement[uQueueIterator].iMaxFuture) { a_sIOMeasurement[uQueueIterator].iMaxFuture = uTimeDiff; }
              if((int64_t)uTimeDiff>(int64_t)a_sIOMeasurement[uQueueIterator].iMinFuture) { a_sIOMeasurement[uQueueIterator].iMinFuture = uTimeDiff; }
              /* This is the first time we see an event, set time marks */
              if(uQueneItems==1)
              {
                a_sIOMeasurement[uQueueIterator].iMaxFuture = uTimeDiff;
                a_sIOMeasurement[uQueueIterator].iMinFuture = uTimeDiff;
              }
            }
            else if ((int64_t)uTimeDiff > 0)
            {
              /* Update differences */
              if((int64_t)uTimeDiff>(int64_t)a_sIOMeasurement[uQueueIterator].iMaxPast) { a_sIOMeasurement[uQueueIterator].iMaxPast = uTimeDiff; }
              if((int64_t)uTimeDiff<(int64_t)a_sIOMeasurement[uQueueIterator].iMinPast) { a_sIOMeasurement[uQueueIterator].iMinPast = uTimeDiff; }
              /* This is the first time we see an event, set time marks */
              if(uQueneItems==1)
              {
                a_sIOMeasurement[uQueueIterator].iMaxPast = uTimeDiff;
                a_sIOMeasurement[uQueueIterator].iMinPast = uTimeDiff;
              }
            }
            else
            {
              /* Set minimal values to zero, because there is no difference */
              a_sIOMeasurement[uQueueIterator].iMinFuture = 0;
              a_sIOMeasurement[uQueueIterator].iMinPast = 0;
            }
            
            fprintf(stdout, "  %+04lldns      %+04lldns     %+04lldns     %+04lldns   %+04lldns   %+fns\n", uTimeDiff, 
                                                                                                    a_sIOMeasurement[uQueueIterator].iMaxFuture,
                                                                                                    a_sIOMeasurement[uQueueIterator].iMinFuture,
                                                                                                    a_sIOMeasurement[uQueueIterator].iMaxPast,
                                                                                                    a_sIOMeasurement[uQueueIterator].iMinPast,
                                                                                                    (double)a_sIOMeasurement[uQueueIterator].iDiffSum/(double)a_sIOMeasurement[uQueueIterator].uTotalEvents);
          }
        }
      }
    }
  }

  /* Should never get here */
  return 0;
  
}
