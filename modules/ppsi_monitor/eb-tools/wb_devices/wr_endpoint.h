/*
 * WR Endpoint, useful for reading info on the network interface
 * on the timing network
 * 
 */
#include <wb_vendors.h>

#ifndef WR_ENDPOINT_H_
#define WR_ENDPOINT_H_

//device ID
#define WR_ENDPOINT_VENDOR      WB_CERN     //vendor ID
#define WR_ENDPOINT_PRODUCT     0x650c2d4f  //device ID
#define WR_ENDPOINT_VMAJOR      1           //major revision
#define WR_ENDPOINT_VMINOR      1           //minor revision

//register offsets
#define WR_ENDPOINT_MACHI       0x24        //MAC high bytes
#define WR_ENDPOINT_MACLO       0x28        //MAC low bytes

//masks
#define WR_ENDPOINT_MACHI_MASK  0x0000ffff  //only two bytes are of interest

#endif /* wr_endpoint.h */
