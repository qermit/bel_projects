#ifndef _XWB_SERDES_CLK_GEN_H_
#define _XWB_SERDES_CLK_GEN_H_

#define XWB_SERDES_CLK_GEN_WBS_VENDOR_ID 0x0000000000000651ull
#define XWB_SERDES_CLK_GEN_WBS_DEVICE_ID 0xc3961dd6

//| Address Map ------------------------- wbs -----------------------------------------------|
#define WBS_selr_RW     0x0   // rw _0xffffffff , Selects which channel to control.
#define WBS_hperr_RW    0x4   // rw _0xffffffff , Half-period register
#define WBS_maskr_RW    0x8   // rw _0xffffffff , Bit flip mask register

#endif
