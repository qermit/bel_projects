/*
 * GSI time stamp latch unit 
 * ... (more info bla bla)
 * 
 */
#include <wb_vendors.h>

#ifndef GSI_TM_LATCH_H_
#define GSI_TM_LATCH_H_

//device ID
#define GSI_TM_LATCH_VENDOR          WB_GSI      //vendor ID
#define GSI_TM_LATCH_PRODUCT         0x10051981  //product ID
#define GSI_TM_LATCH_VMAJOR          1           //major revision
#define GSI_TM_LATCH_VMINOR          1           //minor revision

//clock
#define GSI_TM_LATCH_CLOCK           8   //clock period [ns]

//register offsets
#define GSI_TM_LATCH_FIFO_READY      0x000       //n..0 channel(n) timestamp(s) ready   (ro)
#define GSI_TM_LATCH_FIFO_CLEAR      0x004       //n..0 channel(n) FIFO clear           (wo)
#define GSI_TM_LATCH_TRIG_ARMSTAT    0x008       //n..0 channel(n) trigger armed status (ro)
#define GSI_TM_LATCH_TRIG_ARMSET     0x00c       //n..0 channel(n) trigger set armed    (wo)
#define GSI_TM_LATCH_TRIG_ARMCLR     0x010       //n..0 channel(n) trigger clr armed    (wo)
#define GSI_TM_LATCH_TRIG_EDGESTAT   0x014 	 //n..0 channel(n) trigger edge status  (ro)
#define GSI_TM_LATCH_TRIG_EDGEPOS    0x018 	 //n..0 channel(n) trigger edge set pos (wo)
#define GSI_TM_LATCH_TRIG_EDGENEG    0x01C 	 //n..0 channel(n) trigger edge set neg (wo)
//one must read ATSHI prior to reading ATSLO
#define GSI_TM_LATCH_ATSHI           0x100 	 //actual time stamp HIGH words in cycles (ro)
#define GSI_TM_LATCH_ATSLO           0x104 	 //actual time stamp LOW words in cycles  (ro)
#define GSI_TM_LATCH_FIFO_OFFSET     0x400 	 //address of FIFO 0 
#define GSI_TM_LATCH_FIFO_INCR       0x020 	 //address increment between FIFO address
//for obtaining the correct offsets for the	  following register, use FIFO_OFFSET/INCREMENT
//example for FIFO 1 and fill countp: offset	  = FIFO_OFFSET + 1 * FIFO_INCREMENT + FIFO_CNT = 0x424
//remark: the number of FIFOs and their size are parameters defined in the VHDL top file
//background: FIFOs are implmented as ring buffers with two pointers
#define GSI_TM_LATCH_FIFO_POP        0x000 	 //pop                                  (wo)
//pop just adjusts the pointer to the FIFO, it does not re-write a default value
#define GSI_TM_LATCH_FIFO_CNT        0x004 	 //fill count                           (ro)
#define GSI_TM_LATCH_FIFO_FTSHI      0x008 	 //timestamp HIGH words in cycles       (ro)
#define GSI_TM_LATCH_FIFO_FTSLO      0x00c 	 //timestamp LOW words in cycles        (ro)
#define GSI_TM_LATCH_FIFO_FTSSUB     0x010 	 //timestamp sub-cycle                  (ro)

//masks

#endif /* gsi_tm_latch.h */
