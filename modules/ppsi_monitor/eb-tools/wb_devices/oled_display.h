/*
 * OLED display, used on SCUs. Useful for display of info
 * 
 */
#include <wb_vendors.h>

#ifndef OLED_DISPLAY_H_
#define OLED_DISPLAY_H_

//device ID
#define OLED_DISPLAY_VENDOR       WB_GSI      //vendor ID
#define OLED_DISPLAY_PRODUCT      0x93a6f3c4  //product ID
#define OLED_DISPLAY_VMAJOR       1           //major revision
#define OLED_DISPLAY_VMINOR       1           //minor revision

//register offsets
#define OLED_DISPLAY_CTRL         0x00000     //Mode Data Bits(1-0)
#define OLED_DISPLAY_UART         0x10000     // UART Written ascii code is put directly on screen
#define OLED_DISPLAY_CHAR         0x20000     // Char Written ascii code is put on supplied location
											  // -- Address Bits(8-6) Char location row
											  // -- Address Bits(5-2) Char location col        
#define OLED_DISPLAY_RAW          0x30000     // Raw Raw read/write to display memory. Organisation is column based
											  // -- 8px per column,
											  // -- Address Bits(12-2) Disp RAM address        
//control parameters
#define OLED_DISPLAY_CTRL_IDLE    0x00        //idle mode, 
#define OLED_DISPLAY_CTRL_UART    0x01        //uart mode
#define OLED_DISPLAY_CTRL_CHAR    0x10        //char mode
#define OLED_DISPLAY_CTRL_RAW     0x11        //raw mode
#define OLED_DISPLAY_CTRL_RESET   0x00004     //resets FSMs and display controller
#define OLED_DISPLAY_UART_CLEAR   0x0c        //clears display in UART mode
#define OLED_DISPLAY_CHAR_CLEAR   OLED_DISPLAY_UART_CLEAR 

//masks

#endif /* old_display.h */
