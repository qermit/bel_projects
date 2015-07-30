/*
 * WR PPS Generator, useful for reading the actual time
 * on the timing network
 *
 */
#include <wb_vendors.h>

#ifndef WR_PPS_GENERATOR_H_
#define WR_PPS_GENERATOR_H_

//device ID
#define WR_PPS_GEN_VENDOR       WB_CERN     //vendor ID
#define WR_PPS_GEN_PRODUCT      0xde0d8ced  //product ID
#define WR_PPS_GEN_VMAJOR      	1           //major revision
#define WR_PPS_GEN_VMINOR      	1           //minor revision

//register offsets
#define WR_PPS_GEN_CNTR_UTCLO   0x8         //UTC seconds low bytes
#define WR_PPS_GEN_CNTR_UTCHI   0xc         //UTC seconds high bytes
#define WR_PPS_GEN_CNTR_NSEC    0x4         //UTC nanoseconds
#define WR_PPS_GEN_ESCR         0x1c        //External Sync Control Register
#define WR_PPS_GEN_CR           0x0

//masks
#define WR_PPS_GEN_CR_MASK      0x1         //bit 1: PPS valid, bit 2: timestamp valid
#define WR_PPS_GEN_ESCR_MASK    0x6         //bit 1: PPS valid, bit 2: timestamp valid
#define WR_PPS_GEN_ESCR_MASKPPS 0x2         //PPS valid bit
#define WR_PPS_GEN_ESCR_MASKTS  0x4         //timestamp valid bit


#endif /* wr_pps_generator.h */
