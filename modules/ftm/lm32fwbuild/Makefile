TARGET		= dm
RAM_SIZE	= 65536
USRCPUCLK       = 125000
SHARED_SIZE     = 8K

MYPATH    = .
CFLAGS    = -I$(MYPATH) 

include /home/mkreider/hdlprojects/bel_projects/syn/build.mk

$(TARGET).elf: $(MYPATH)/main.c $(INCPATH)/ebm.c 

clean::
	rm -f $(MYPATH)/*.o


