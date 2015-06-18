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
#define EXPLODER5_IOS 16
#define EXPLODER5_LEMO_OFFSET 4
#define LEMO_OE_OFFSET_REG 4
#define EVENTS 2000
#define MAX_JITTER_NS 2
#define EXPECTED_PULSE_DIFF_NS   100000000
#define MAX_ALLOWED_JITTER_NS 200

volatile sig_atomic_t flag = 0;
void vSignalHandler(int sig)
{
  flag = 1;
}

typedef struct
{
  uint64_t uTotalEvents;
  uint64_t uLastTimestamp;
  uint64_t uMaxPast;
  uint64_t uMinPast;
  uint64_t uMaxFuture;
  uint64_t uMinFuture;
  uint64_t uAverage;
  uint64_t uStdDeviation;
} s_IOMeasurement;

int main (int argc, const char** argv)
{

  /* Helpers */
  Socket socket;
  Device device;
  status_t status;
  std::vector<TLU> tlus;
  std::vector<std::vector<uint64_t> > queues;

  s_IOMeasurement a_sIOMeasurement[EXPLODER5_IOS];
  int test = 0;


  //uint32_t a_uEdges[EVENTS];
  //uint64_t a_uFrequency[EVENTS];
  
  Table table;
  uint32_t uQueueInterator = 0;
  uint32_t uQueueItemIterator = 0;
  uint32_t uQueuesTotal = 0;
  uint32_t uQueneItems = 0;
  uint64_t uTimeDiff = 0;


  for(test = 0; test < 16; test++)
  {
    a_sIOMeasurement[test].uTotalEvents = 0;
    a_sIOMeasurement[test].uLastTimestamp = 0;
    a_sIOMeasurement[test].uMaxPast = 0;
    a_sIOMeasurement[test].uMinPast = 0;
    a_sIOMeasurement[test].uMaxFuture = 0;
    a_sIOMeasurement[test].uMinFuture = 0;
    a_sIOMeasurement[test].uAverage = 0;
    a_sIOMeasurement[test].uStdDeviation = 0;
  }


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
  device.sdb_find_by_identity(0x651, 0x4d78adfdU, devs);
  assert (devs.size() == 1);
  address_t ioconf = devs[0].sdb_component.addr_first + LEMO_OE_OFFSET_REG;
  device.write(ioconf, EB_DATA32, 0x00);
  
  /* Check TLU */
  while (test!=1)
  {

    if(flag)
    {
      printf("\nSignal caught!\n");
      printf("default action is termination!\n");
      flag = 0;
      exit(0);
    }
    
    /* TODO: Poll device */
    usleep(500000);

    /* Read-out result */
    tlu.pop_all(queues);
    uQueuesTotal = queues.size();
    
    fprintf(stdout, "\n\n%s: Latest TS                            Count          Offset ts0          MaxFuture  MinFuture  MaxPast  MinPast\n", argv[0]);
    fprintf(stdout, "%s: ------------------------------------------------------------------------------------------------------------------\n", argv[0]);

    /* Check each queue now */
    for(uQueueInterator=0; uQueueInterator<uQueuesTotal; uQueueInterator++)
    {
      
       
      std::vector<uint64_t>& queue = queues[uQueueInterator];
      uQueneItems = queue.size(); /* Get the actual size */
       
       
      if(uQueneItems!=0)
      {
        //fprintf(stdout, "%s: queue %d has a size of %d ...\n", argv[0], uQueueInterator, uQueneItems);
      }
      /* Inspect items with queue contains data */
      if(uQueneItems)
      {
        
          // TODO: Type fix needed: interator
          a_sIOMeasurement[uQueueInterator].uTotalEvents = uQueneItems;
          a_sIOMeasurement[uQueueInterator].uLastTimestamp = queue[uQueneItems-1];
          
          uTimeDiff = queue[uQueneItems-1]-a_sIOMeasurement[EXPLODER5_LEMO_OFFSET].uLastTimestamp;
          
          /* Ignore broken LEMO */
          if(!(uQueueInterator-EXPLODER5_LEMO_OFFSET == 5))
          {
            fprintf(stdout, "%s: ts%d: %ld                      %d", argv[0], uQueueInterator-EXPLODER5_LEMO_OFFSET, queue[uQueneItems-1], uQueneItems);
          }
          
          if (uQueueInterator-EXPLODER5_LEMO_OFFSET == 0)
          {
            fprintf(stdout, "\n");
          }
          else
          {
            /* Is this a pulse after or before ts0? */
            if ((int64_t)uTimeDiff < 0)
            {
              /* Update differences */
              if((int64_t)uTimeDiff<(int64_t)a_sIOMeasurement[uQueueInterator].uMaxFuture) { a_sIOMeasurement[uQueueInterator].uMaxFuture = uTimeDiff; }
              if((int64_t)uTimeDiff>(int64_t)a_sIOMeasurement[uQueueInterator].uMinFuture) { a_sIOMeasurement[uQueueInterator].uMinFuture = uTimeDiff; }
              /* This is the first time we see an event, set time marks */
              if(uQueneItems==1)
              {
                a_sIOMeasurement[uQueueInterator].uMaxFuture = uTimeDiff;
                a_sIOMeasurement[uQueueInterator].uMinFuture = uTimeDiff;
              }
            }
            else if ((int64_t)uTimeDiff > 0)
            {
              /* Update differences */
              if((int64_t)uTimeDiff>(int64_t)a_sIOMeasurement[uQueueInterator].uMaxPast) { a_sIOMeasurement[uQueueInterator].uMaxPast = uTimeDiff; }
              if((int64_t)uTimeDiff<(int64_t)a_sIOMeasurement[uQueueInterator].uMinPast) { a_sIOMeasurement[uQueueInterator].uMinPast = uTimeDiff; }
              /* This is the first time we see an event, set time marks */
              if(uQueneItems==1)
              {
                a_sIOMeasurement[uQueueInterator].uMaxPast = uTimeDiff;
                a_sIOMeasurement[uQueueInterator].uMinPast = uTimeDiff;
              }
            }
            else
            {
              /* Set minimal values to zero, because there is no difference */
              a_sIOMeasurement[uQueueInterator].uMinFuture = 0;
              a_sIOMeasurement[uQueueInterator].uMinPast = 0;
            }
            



            /* TODO: REMOVE THIS LATER */
            /* Ignore broken LEMO */
            if(!(uQueueInterator-EXPLODER5_LEMO_OFFSET == 5))
            {
              fprintf(stdout, "          %ldns               %ldns     %ldns     %ldns     %ldns\n", uTimeDiff, 
                                                                                a_sIOMeasurement[uQueueInterator].uMaxFuture,
                                                                                a_sIOMeasurement[uQueueInterator].uMinFuture,
                                                                                a_sIOMeasurement[uQueueInterator].uMaxPast,
                                                                                a_sIOMeasurement[uQueueInterator].uMinPast);
            }
            
          }
        
      }
    }
  }

  fprintf(stdout, "%s: done!\n", argv[0]);
  
  return 0;
  
}
