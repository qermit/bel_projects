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
  
  /* Compiler check */
#if TP == TARGET_PLATFORM_32
  #warning "Compiling for 32-bit target..."
#elif TP == TARGET_PLATFORM_64
  #warning "Compiling for 64-bit target..."
#else
  #error "Please define TP (32/64-bit)!"
#endif

  /* Etherbone */
  Socket socket;
  Device device;
  status_t status;
  
  /* TLU */
  std::vector<TLU> tlus;
  std::vector<std::vector<uint64_t> > queues;
  s_IOMeasurement a_sIOMeasurement[LEMO_TOTAL_IOS];

  /* Format and Measurement */
  uint32_t uQueueIterator = 0;
  uint32_t uQueuesTotal = 0;
  uint32_t uQueneItems = 0;
  uint64_t uTimeDiff = 0;
  uint32_t uIterator = 0;
  uint64_t uEventsTemp = 0;
  uint32_t uRefIO = 0;
  
  /* Configuration */
  char a_cDeviceName[0x100];
  char a_cDeviceHandle[0x100];
  char a_cReferenceIO[0x100];
  
  /* Logging */
  FILE *fp;
  char a_cFileNameBuffer[0x100];
  int iSysCallRes = 0;

  /* Clean results */
  for(uIterator = 0; uIterator < LEMO_TOTAL_IOS; uIterator++)
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
  
  /* Read configuration file */
  snprintf(a_cFileNameBuffer, sizeof(a_cFileNameBuffer), "%s", argv[1]);
  fp = fopen(a_cFileNameBuffer,"r");
  if (fp != NULL)
  {
    /* Get device name for logging and handle for Etherbone */
    iSysCallRes = fscanf(fp, "%s %s %s\n", a_cDeviceName, a_cDeviceHandle, a_cReferenceIO);
    if (iSysCallRes != 3) /* Expecting 3 arguments */
    {
      fprintf(stdout, "%s: Can't open configuration file (code: %d)! \n", argv[0], iSysCallRes);
      exit(1);
    }
    /* Get reference IO */
    uRefIO = atoi(a_cReferenceIO);
    fclose(fp);
  }
  else
  {
    fprintf(stderr, "%s: failed to open file %s\n", argv[0], argv[1]);
    exit(1);
  }
  
  /* Try to open a (etherbone-) socket */
  socket.open();
  if ((status = device.open(socket, a_cDeviceHandle)) != EB_OK) 
  {
    fprintf(stderr, "%s: failed to open %s: (status %s)\n", argv[0], a_cDeviceHandle, eb_status(status));
    exit(1);
  }
  else
  {
#if DEBUG_MODE
    fprintf(stdout, "%s: succeeded to open %s (status %s)\n", argv[0], a_cDeviceHandle, eb_status(status));
#endif
  }
  
  /* Find the TLU */
  TLU::probe(device, tlus);
  assert (tlus.size() == 1);
  TLU& tlu = tlus[0];
  
  /* Configure the TLU to record rising edge timestamps */
  tlu.hook(-1, false);
  tlu.set_enable(false); /* No interrupts */
  tlu.clear(-1);
  tlu.listen(-1, true, true, 8); /* Listen on all inputs */
  
  /* Reset variable */
  iSysCallRes = 0;
  
  /* Check TLU */
  while (true)
  {
  
    /* Check for system call error */
    if (iSysCallRes)
    { 
      fprintf(stdout, "%s: System call error (code: %d)!\n", argv[0], iSysCallRes);
      exit(1);
    }
    
    /* CTRL+C or dump time? */
    if(s_sigint || s_dump)
    {
      /* Create debug dump */
      for(uIterator = 0; uIterator < LEMO_TOTAL_IOS; uIterator++)
      {
        /* Did we capture any event here? */
        if(a_sIOMeasurement[uIterator].uTotalEvents)
        {
          /* Create or overwrite a log file */
          snprintf(a_cFileNameBuffer, sizeof(a_cFileNameBuffer), "log/%s_syncmon_dev_io%d.log", a_cDeviceName, uIterator);
          fp = fopen(a_cFileNameBuffer, "w");
          /* Print result to file */
          fprintf(fp, "Results for IO%d:\n", uIterator);
#if TP == TARGET_PLATFORM_64
          fprintf(fp, "  Events:               %lu\n", a_sIOMeasurement[uIterator].uTotalEvents);
          fprintf(fp, "  Latest Timestamp:     %lu\n", a_sIOMeasurement[uIterator].uLastTimestamp);
#elif TP == TARGET_PLATFORM_32
          fprintf(fp, "  Events:               %llu\n", a_sIOMeasurement[uIterator].uTotalEvents);
          fprintf(fp, "  Latest Timestamp:     %llu\n", a_sIOMeasurement[uIterator].uLastTimestamp);
#endif
          /* Only for non ref devices */
          if (uIterator != uRefIO)
          {
#if TP == TARGET_PLATFORM_64
            fprintf(fp, "  Latest Printed Event: %lu\n", a_sIOMeasurement[uIterator].uLatestPrintedEvent);
            fprintf(fp, "  Late Difference:      %lu\n", a_sIOMeasurement[uIterator].iLastDiff);
            fprintf(fp, "  Max. Past:            %ldns\n", a_sIOMeasurement[uIterator].iMaxPast);
            fprintf(fp, "  Min. Past:            %ldns\n", a_sIOMeasurement[uIterator].iMinPast);
            fprintf(fp, "  Max. Future:          %ldns\n", a_sIOMeasurement[uIterator].iMaxFuture);
            fprintf(fp, "  Min. Future:          %ldns\n", a_sIOMeasurement[uIterator].iMinFuture);
            /* Calculate statistics */
            fprintf(fp, "  Average:              %fns\n\n", (double)a_sIOMeasurement[uIterator].iDiffSum/(double)a_sIOMeasurement[uIterator].uTotalEvents);
#elif TP == TARGET_PLATFORM_32
            fprintf(fp, "  Latest Printed Event: %llu\n", a_sIOMeasurement[uIterator].uLatestPrintedEvent);
            fprintf(fp, "  Late Difference:      %llu\n", a_sIOMeasurement[uIterator].iLastDiff);
            fprintf(fp, "  Max. Past:            %lldns\n", a_sIOMeasurement[uIterator].iMaxPast);
            fprintf(fp, "  Min. Past:            %lldns\n", a_sIOMeasurement[uIterator].iMinPast);
            fprintf(fp, "  Max. Future:          %lldns\n", a_sIOMeasurement[uIterator].iMaxFuture);
            fprintf(fp, "  Min. Future:          %lldns\n", a_sIOMeasurement[uIterator].iMinFuture);
            /* Calculate statistics */
            fprintf(fp, "  Average:              %fns\n\n", (double)a_sIOMeasurement[uIterator].iDiffSum/(double)a_sIOMeasurement[uIterator].uTotalEvents);
#endif
          }
          /* Close file */
          fclose(fp);
          
          /* Dump event only once to file */
          if(a_sIOMeasurement[uIterator].uLatestPrintedEvent != a_sIOMeasurement[uIterator].uTotalEvents)
          {
            /* Concatenate data into plot log file */
            snprintf(a_cFileNameBuffer, sizeof(a_cFileNameBuffer), "log/%s_syncmon_dev_plot_io%d.log", a_cDeviceName, uIterator);
            fp = fopen(a_cFileNameBuffer, "a+");
            /* Print result to file */
#if TP == TARGET_PLATFORM_64
            fprintf(fp, "%lu %lu\n", a_sIOMeasurement[uIterator].uTotalEvents, a_sIOMeasurement[uIterator].uLastTimestamp);
#elif TP == TARGET_PLATFORM_32
            fprintf(fp, "%llu %llu\n", a_sIOMeasurement[uIterator].uTotalEvents, a_sIOMeasurement[uIterator].uLastTimestamp);
#endif
            /* Close file */
            fclose(fp);
            /* Increase printed counter */
            a_sIOMeasurement[uIterator].uLatestPrintedEvent++;
          }
        }
      }
      /* Reset signals and maybe quit */
      if (s_sigint != 0) { s_sigint = 0; return(0); }
      if (s_dump != 0)   { s_dump = 0; }
    }
    
    /* Sleep */
    usleep(250000);
    
    /* Read-out result */
    tlu.pop_all(queues);
    uQueuesTotal = queues.size();
     
    /* Check each queue now */
    for(uQueueIterator=0; uQueueIterator<uQueuesTotal; uQueueIterator++)
    {
      std::vector<uint64_t>& queue = queues[uQueueIterator];
      uQueneItems = queue.size(); /* Get the actual size */
      
      /* Inspect items with queue contains data */
      if(uQueneItems > a_sIOMeasurement[uRefIO].uTotalEvents)
      {
        /* Got new data, create log dump after printing */
        s_dump = 1;
      }
      
      /* Got something new? */
      if(uQueneItems > a_sIOMeasurement[uQueueIterator].uTotalEvents)
      {
        /* Make sure that we don't have more PPS pulse form any device under test than from our reference device */
        if(uQueneItems > a_sIOMeasurement[uRefIO].uTotalEvents && uQueueIterator != uRefIO)
        {
#if DEBUG_MODE
          fprintf(stdout, "%s: Bad measurement or bit jitter: %d %d\n", argv[0], uQueneItems, a_sIOMeasurement[uRefIO].uTotalEvents);
#endif
        }
        else
        {
          /* Calculate time difference */
          uTimeDiff = queue[uQueneItems-1]-a_sIOMeasurement[uRefIO].uLastTimestamp;

          /* Event seen? */
          if(uQueneItems > a_sIOMeasurement[uQueueIterator].uTotalEvents)
          {
            a_sIOMeasurement[uQueueIterator].iDiffSum += uTimeDiff;            
            a_sIOMeasurement[uQueueIterator].iLastDiff = uTimeDiff;       
          }
          a_sIOMeasurement[uQueueIterator].uTotalEvents++;
          a_sIOMeasurement[uQueueIterator].uLastTimestamp = queue[uQueneItems-1];
          
          if (uQueueIterator-uRefIO == 0)
          {
#if DEBUG_MODE
            fprintf(stdout, "%s: Checking reference device ...\n", argv[0]);
#endif
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
          }
        }
      }
    }
    
    /* Print results */
    iSysCallRes = system("clear");
    fprintf(stdout, "%s: TS#   Latest TS            Count     Offset ts%02d  MaxFuture  MinFuture  MaxPast  MinPast  Average\n", argv[0], uRefIO);
    fprintf(stdout, "%s: --------------------------------------------------------------------------------------------------------\n", argv[0]);
    
    /* Print each IO */
    for(uQueueIterator=0; uQueueIterator<uQueuesTotal; uQueueIterator++)
    {
      uEventsTemp = a_sIOMeasurement[uQueueIterator].uTotalEvents;
      if (uQueueIterator == uRefIO)
      {
#if TP == TARGET_PLATFORM_64
        fprintf(stdout, "%s: ts%02d: %019lu  %08lu\n", argv[0], uQueueIterator, a_sIOMeasurement[uQueueIterator].uLastTimestamp, uEventsTemp);
#elif TP == TARGET_PLATFORM_32
        fprintf(stdout, "%s: ts%02d: %019llu  %08llu\n", argv[0], uQueueIterator, a_sIOMeasurement[uQueueIterator].uLastTimestamp, uEventsTemp);
#endif

      }
      if (uEventsTemp && uQueueIterator > uRefIO)
      {
        /* Print old status */
#if TP == TARGET_PLATFORM_64
        fprintf(stdout, "%s: ts%02d: %019lu  %08lu", argv[0], uQueueIterator, a_sIOMeasurement[uQueueIterator].uLastTimestamp, uEventsTemp);
        fprintf(stdout, "  %+04ldns       %+04ldns     %+04ldns     %+04ldns   %+04ldns   %+fns\n", a_sIOMeasurement[uQueueIterator].iLastDiff, 
                                                                                                         a_sIOMeasurement[uQueueIterator].iMaxFuture,
                                                                                                         a_sIOMeasurement[uQueueIterator].iMinFuture,
                                                                                                         a_sIOMeasurement[uQueueIterator].iMaxPast,
                                                                                                         a_sIOMeasurement[uQueueIterator].iMinPast,
                                                                                                         (double)a_sIOMeasurement[uQueueIterator].iDiffSum/(double)a_sIOMeasurement[uQueueIterator].uTotalEvents);
#elif TP == TARGET_PLATFORM_32
        fprintf(stdout, "%s: ts%02d: %019llu  %08llu", argv[0], uQueueIterator, a_sIOMeasurement[uQueueIterator].uLastTimestamp, uEventsTemp);
        fprintf(stdout, "  %+04lldns       %+04lldns     %+04lldns     %+04lldns   %+04lldns   %+fns\n", a_sIOMeasurement[uQueueIterator].iLastDiff, 
                                                                                                         a_sIOMeasurement[uQueueIterator].iMaxFuture,
                                                                                                         a_sIOMeasurement[uQueueIterator].iMinFuture,
                                                                                                         a_sIOMeasurement[uQueueIterator].iMaxPast,
                                                                                                         a_sIOMeasurement[uQueueIterator].iMinPast,
                                                                                                         (double)a_sIOMeasurement[uQueueIterator].iDiffSum/(double)a_sIOMeasurement[uQueueIterator].uTotalEvents);
#endif
      }
    }
  }

  /* Should never get here */
  return (1);
  
}
