#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include <etherbone.h>
#include <eca.h>
#include <cassert>
#include <cstdio>

using namespace GSI_ECA;

struct MyHandler : public Handler {
  ActionQueue* aq;
  ECA* eca;
  
  status_t write(address_t address, width_t width, data_t data) {
    ActionEntry entry;
    aq->refresh();
    aq->pop(entry);
    
    printf("Action: 0x%"PRIx64" 0x%"PRIx64" 0x%"PRIx32" 0x%"PRIx32" %s\n",
      entry.event, entry.param, entry.tag, entry.tef,
      eca->date(entry.time).c_str());
    fflush(stdout);
    return EB_OK;
  }
  status_t read(address_t address, width_t width, data_t* data) {
    return EB_OK;
  }
};

struct sdb_device mydevice = {
  0, 0, 0, SDB_WISHBONE_WIDTH,
  { 0, 0, 
  { 0x651, 0xdeadbeef, 1, 0x20140516, "Debug device      ", sdb_record_device 
}}};

int main(int argc, const char** argv) {
  Socket socket;
  Device device;
  MyHandler handler;
  eb_data_t irq_start_high, irq_start_low, irq_end_high, irq_end_low;
    
  /* Setup connection to FPGA via PCIe */
  socket.open();
  socket.passive("dev/wbs0");
  device.open(socket, "dev/wbm0");
  
  /* Find the ECA */
  std::vector<ECA> ecas;
  ECA::probe(device, ecas);
  assert (ecas.size() == 1);
  ECA& eca = ecas[0];
  
  assert (eca.channels.size() >= 2);
  assert (!eca.channels[1].queue.empty());
  handler.eca = &eca;
  handler.aq = &eca.channels[1].queue.front();
  
  /* Stop the ECA for now */
  eca.disable(true);
  eca.channels[1].drain(true);
  
  /* Program a catch all to channel 1 rule */
  Table table;
  table.add(TableEntry(0, 0, 0xdeadbeef, 1, 0));
  eca.store(table);
  eca.flipTables();
  
  /* Flush any crap pending in the AQ */
  ActionEntry ae;
  while (1) {
    handler.aq->refresh();
    if (!handler.aq->queued_actions) break;
    handler.aq->pop(ae);
  }
  
  /* Get the interrupt range from device */
  device.get_irq_range(&irq_start_high, &irq_start_low, &irq_end_high, &irq_end_low);
  
  /* Get interrupt range and hook arrival interrupts via PCIe */
  mydevice.sdb_component.addr_first = irq_start_low;
  mydevice.sdb_component.addr_last = irq_end_low;
  socket.attach(&mydevice, &handler);
  
  /* Print address range and attach to device */
  printf("Config: Listening to interrupt range 0x%"PRIx64"-0x%"PRIx64"\n", mydevice.sdb_component.addr_first, mydevice.sdb_component.addr_last);
  
  /* Enable interrupt delivery to us */
  handler.aq->hook_arrival(true, mydevice.sdb_component.addr_first);
  eca.channels[1].freeze(false);
  eca.channels[1].drain(false);
  eca.interrupt(true);
  eca.disable(false);
  
  /* Wait forever, pumping out the actions as they arrive */
  while (true) socket.run();
  
  return 0;
}
