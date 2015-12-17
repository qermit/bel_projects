/*
 * GSI General Purpose IO, on-board I/O for the SCU
 */
#include <wb_vendors.h>

#ifndef GSI_GPIO_32_H_
#define GSI_GPIO_32_H_

//device ID
#define GSI_GPIO_32_VENDOR       	 WB_GSI      //vendor ID
#define GSI_GPIO_32_PRODUCT      	 0x35aa6b95  //product ID
#define GSI_GPIO_32_VMAJOR      	 1           //major revision
#define GSI_GPIO_32_VMINOR      	 0           //minor revision

//register offsets
#define GSI_GPIO_32_DIR          	 0x4         //direction of LEMO connectors, input or output
#define GSI_GPIO_32_MUX          	 0x8         //MUX for assigning signals to I/O

//background: the binary number written to GSI_GPIO_32_MUX has the form aabbccdd, where
//ULED2 is aa: 00=link, 01=eca3, 10=gpio3
//ULED1 is bb: 00=pps,  01=eca2, 10=gpio2
//LEMO2 is cc: 00=pps,  01=eca1, 10=gpio1
//LEMO1 is dd: 00=0,    01=eca0, 10=gpio0
//
//control parameters for MUX. The MUX must be configured with a parameter=ULED2+ULED1+LEMO2+LEMO1
#define GSI_GPIO_MUX_ULED2_LINK      0x00        //connect ULED2 to status of White Rabbit link up/down
#define GSI_GPIO_MUX_ULED1_PPS       0x00        //connect ULED1 to PPS pulse
#define GSI_GPIO_MUX_LEMO2_PPS       0x00        //connect LEMO2 to PPS pulse
#define GSI_GPIO_MUX_LEMO1_0         0x00        //connect LEMO1 to logic LOW
#define GSI_GPIO_MUX_ULED2_ECA3      0x40        //connect ULED2 to ECA unit bit 3
#define GSI_GPIO_MUX_ULED1_ECA2      0x10        //connect ULED1 to ECA unit bit 2
#define GSI_GPIO_MUX_LEMO2_ECA1      0x04        //connect LEMO2 to ECA unit bit 1
#define GSI_GPIO_MUX_LEMO1_ECA0      0x01        //connect LEMO1 to ECA unit bit 0
#define GSI_GPIO_MUX_ULED2_GPIO3     0x80        //connect ULED2 to GPIO bit 3
#define GSI_GPIO_MUX_ULED1_GPIO2     0x20        //connect ULED1 to GPIO bit 2
#define GSI_GPIO_MUX_LEMO2_GPIO1     0x08        //connect LEMO2 to GPIO bit 1
#define GSI_GPIO_MUX_LEMO1_GPIO0     0x02        //connect LEMO1 to GPIO bit 0

//background: the binary number written to GSI_GPIO_32_DIR has the format ab, where
//LEMO2 is a: 0=output, 1=input
//LEMO1 is b: 0=output, 1=input
//
//if configured as input, LEMO1..2 are connected to the TLU trigger0..1
//
//control parameter for DIR. The DIR must be configured with a parameter=LEMO2+LEMO1
#define GSI_GPIO_DIR_LEMO2_OUT       0x0         //LEMO2 is output
#define GSI_GPIO_DIR_LEMO2_IN        0x2         //LEMO2 is input
#define GSI_GPIO_DIR_LEMO1_OUT       0x0         //LEMO1 is output
#define GSI_GPIO_DIR_LEMO1_IN        0x1         //LEMO1 is input

//control parameter for lm32 of new gateware (> September 2013)
#define GSI_GPIO_RESET               0xc         //resets FPGA, WRCORE, USERLM32

//masks
#define GSI_GPIO_RESET_USRLM32       0x8         //resets user lm32 - keep HIGH during programming

#endif /* gsi_gpio_32.h */
