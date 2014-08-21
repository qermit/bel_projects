#ifndef _DISP_LCD_H_
#define _DISP_LCD_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "font5x7.h"
#include "mini_sdb.h"

#define FB_SIZE 24
#define COL_MAX	6
#define ROW_MAX	3
#define CHR_WID 6
#define CHR_HEI 8
#define ERROR	-1

#define SDB_SERIAL_LCD_DISPLAY 0xb77a5045

unsigned long long framebuffer[FB_SIZE];
extern const char* program;
extern const char* netaddress;

int  disp_init(void);
void disp_fill(void);
void disp_write(void);
void lcd_disp_put_loc_c(char ascii, unsigned char row, unsigned char col);
void lcd_disp_put_c(char ascii);
void lcd_disp_put_s(const char* str);
void lcd_disp_put_line(const char *sPtr, unsigned char row);

#endif
