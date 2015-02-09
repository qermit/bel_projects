LD        := lm32-elf-ld
CC        := lm32-elf-gcc
DBGLVL    = 0 #Debug Outputs from LM32 to console, Level 0-3 
DBGTIME   = 0 #Debug Timestamps at LM32
DBGPRIOQ  = 1 #Debug Timestamps at Priority Queue
DBGPRIOQDST = 0x100900 #Dest Adr for PrioQueue Timstamps at target. Be VERY careful where you point that to!
TARGET    = ftm
MYPATH    = .
CHECKOUT  = ../../..
INCPATH   = $(CHECKOUT)/modules/lm32-include
W1        = $(CHECKOUT)/ip_cores/wrpc-sw
CFLAGS    = -std=gnu99 -I$(INCPATH) -mmultiply-enabled -mbarrel-shift-enabled -Os -I$(W1)/include -I$(W1)/pp_printf \
      -DDEBUGLEVEL=$(DBGLVL) -DDEBUGTIME=$(DBGTIME) -DDEBUGPRIOQ=$(DBGPRIOQ) -DDEBUGPRIOQDST=$(DBGPRIOQDST)

all: $(TARGET).bin $(TARGET).elf

$(TARGET).bin: $(TARGET).elf
	lm32-elf-objcopy -O binary $< $@

$(TARGET).elf:	$(INCPATH)/dbg.c $(INCPATH)/aux.c $(INCPATH)/ebm.c $(INCPATH)/irq.c \
		$(INCPATH)/mini_sdb.c $(MYPATH)/main.c $(INCPATH)/crt0.S $(INCPATH)/mprintf.c \
		$(W1)/dev/uart.c $(MYPATH)/ftm.c
		$(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld $^ 


clean:
	rm -f *.o *.elf *.bin
