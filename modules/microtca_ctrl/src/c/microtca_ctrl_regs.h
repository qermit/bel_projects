/** @file        microtca_ctrl_regs.h
  * DesignUnit   microtca_ctrl
  * @author      A. Hahn <a.hahn@gsi.de>
  * @date        19/11/2015
  * @version     0.0.1
  * @copyright   2015 GSI Helmholtz Centre for Heavy Ion Research GmbH
  *
  * @brief       Register map for Wishbone interface of VHDL entity <microtca_ctrl_auto>
  */

#ifndef _MICROTCA_CTRL_H_
#define _MICROTCA_CTRL_H_

   #define SLAVE_HEX_SWITCH_GET        0x04  //ro,  4 b, Shows hex switch inputs
   #define SLAVE_PUSH_BUTTON_GET       0x08  //ro,  1 b, Shows status of the push button
   #define SLAVE_HEX_SWITCH_CPLD_GET   0x0c  //ro,  4 b, Shows hex switch inputs (CPLD)
   #define SLAVE_PUSH_BUTTON_CPLD_GET  0x10  //ro,  1 b, Shows status of the push button (CPLD)
   #define SLAVE_CLOCK_CONTROL_OE_RW   0x14  //rw,  1 b, External input clock output enable
   #define SLAVE_LOGIC_CONTROL_OE_RW   0x18  //rw, 17 b, External logic analyzer output enable
   #define SLAVE_LOGIC_OUTPUT_RW       0x1c  //rw, 17 b, External logic analyzer output (write)
   #define SLAVE_LOGIC_INPUT_GET       0x20  //ro, 17 b, External logic analyzer input (read)
   #define SLAVE_BACKPLANE_CONF0_RW    0x24  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF1_RW    0x28  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF2_RW    0x2c  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF3_RW    0x30  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF4_RW    0x34  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF5_RW    0x38  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF6_RW    0x3c  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF7_RW    0x40  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF8_RW    0x44  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF9_RW    0x48  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF10_RW   0x4c  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF11_RW   0x50  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF12_RW   0x54  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF13_RW   0x58  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF14_RW   0x5c  //rw, 32 b, Backplane
   #define SLAVE_BACKPLANE_CONF15_RW   0x60  //rw, 32 b, Backplane

#endif
