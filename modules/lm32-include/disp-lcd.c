#include "disp-lcd.h"

unsigned char cursorCol = 1;
unsigned char cursorRow = 1;
volatile uint32_t *p_uSerialDisplay;

int disp_init(void)
{
  p_uSerialDisplay = (uint32_t*) find_device(SDB_SERIAL_LCD_DISPLAY);
  if (p_uSerialDisplay==NULL)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

void disp_write(void)
{

  unsigned int* writeout;
  int i;
  //eb_status_t status;
  //eb_format_t format;
  //eb_cycle_t cycle;

  //format = EB_ADDR32|EB_DATA32;

  writeout = (unsigned int*)framebuffer;	

  //wrap frame buffer in EB packet
  for(i=0;i<FB_SIZE*2;i+=2) 
  {
    *(p_uSerialDisplay+(i+1)) = writeout[i+1];
    *(p_uSerialDisplay+(i)) = writeout[i];
  }
  return;
}




static unsigned char* render_char(char ascii, unsigned char* pDotMatrix)
{
	unsigned char i, j, tmpRow;
	const unsigned char* pBmp = &font5x7[(ascii-0x20)*(CHR_WID-1)];

	//this is written 5x7, turn it into 8x8
	//no time or desire to compile my own table
	for(i=0;i<CHR_HEI-1;i++)
	{
		tmpRow = 0;		
		for(j=0;j<CHR_WID-1;j++) tmpRow |= ((pBmp[j]>>i)&0x01)<<(CHR_WID-2-j);
		pDotMatrix[i] = tmpRow;
	}	 
	pDotMatrix[CHR_HEI-1] = 0x00;

	return pDotMatrix;

				
}


void lcd_disp_put_loc_c(char ascii, unsigned char row, unsigned char col)
{
  unsigned char i, x, y;
  unsigned char bitmap[CHR_HEI];
  unsigned long long line, mask;
  const unsigned long long cMask = ((unsigned long long)(2^CHR_WID)-1)<<(63-CHR_WID); //make a mask CHR_WID bits wide and left align it in a 64bit word
  
  render_char(ascii, bitmap);
  
  if(col == 0) x = 0; 
  else x = (col <= COL_MAX) ? (col-1)*CHR_WID :  (COL_MAX-1)*CHR_WID;	
  
  if(row == 0) y = 0; 
  else y = (row <= ROW_MAX) ? (row-1)*CHR_HEI : (ROW_MAX-1)*CHR_HEI;
	
  //1x 64 bit word per line to cover 36x24pix. x is bitshift, y is word 
  mask = ~(cMask >> x);
  
  for(i=0;i<CHR_HEI;i++)
    {
      framebuffer[y+i] &= mask;	//clear the part in the framebuffer row
      line = ((unsigned long long)bitmap[i])<<(59-x);
      framebuffer[y+i] |= line;	//OR it with the bitmap row
    }
  
  
  disp_write();	 
}


void lcd_disp_put_c(char ascii)
{
  unsigned char i;	
  
  if(ascii == 0x0c)
    {
      cursorCol = 1;
      cursorRow = 1;
      for(i=0;i<FB_SIZE;i++) framebuffer[i] = 0;	
      disp_write();			
    }
  else if(ascii == '\n')
    {
      
      cursorCol = 1;
      if(cursorRow < 3) cursorRow++;
      else 							cursorRow=1;
      for(i=(cursorRow-1)*CHR_HEI;i<(cursorRow*CHR_HEI);i++) framebuffer[i] = 0;
      disp_write(); 
    }	
  else
    {	
      
      lcd_disp_put_loc_c(ascii, cursorRow, cursorCol);
      if(cursorCol < 6) cursorCol++;
      else 
	{
	  cursorCol = 1;
	  if(cursorRow < 3) cursorRow++;
	  else 							cursorRow=1;
	  for(i=(cursorRow-1)*CHR_HEI;i<(cursorRow*CHR_HEI);i++) framebuffer[i] = 0;  
	}

    }
}

void lcd_disp_put_s(const char *sPtr)
{

  while(*sPtr != '\0') lcd_disp_put_c(*sPtr++);
}

/*
void lcd_disp_put_line(const char *sPtr, unsigned char row)
{
  char col, outp, pad;
  pad = 0;
  
  for(col=0; col<COL_MAX; col++)
    {
      if(*(sPtr+col) == '\0') pad = 1;
      
      if(pad) outp = ' ';
      else 	outp = *(sPtr+col);	
      
      lcd_disp_put_loc_c(device, outp, row, col+1);
    }
  
  cursorCol = 1;
  if(row < 3) cursorRow = row+1;
  else 				cursorRow=1;
}
*/
