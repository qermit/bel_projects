LD        := lm32-elf-ld
CC        := lm32-elf-gcc
DBGLVL    = 0 #Debug Outputs from LM32 to console, Level 0-3 
DBGTIME   = 0 #Debug Timestamps at LM32
DBGPRIOQ  = 1 #Debug Timestamps at Priority Queue
DBGPRIOQDST = 0x100900 #Dest Adr for PrioQueue Timstamps at target. Be VERY careful where you point that to!
TARGET    = ftm
MYPATH    = .
CHECKOUT  = ../../..
INCPATH   := $(CHECKOUT)/modules/lm32-include
W1        := $(CHECKOUT)/ip_cores/wrpc-sw
CFLAGS    := -std=gnu99 -I$(INCPATH) -mmultiply-enabled -mbarrel-shift-enabled -Os -I$(W1)/include -I$(W1)/pp_printf \
      -DDEBUGLEVEL=$(DBGLVL) -DDEBUGTIME=$(DBGTIME) -DDEBUGPRIOQ=$(DBGPRIOQ) -DDEBUGPRIOQDST=$(DBGPRIOQDST)

CBR_DATE := `date +"%a %b %d %H:%M:%S %Z %Y"`
CBR_USR  := `git config user.name`
CBR_MAIL := `git config user.email`
CBR_HOST := `hostname`
CBR_GCC  := `lm32-elf-gcc --version | grep gcc`
CBR_FLGS := $(CFLAGS)
CBR_KRNL := `uname -mrs`
CBR_OS   := `lsb_release -d -s` 
CBR_PF   := ""
CBR_GIT1  := `git log HEAD~1 --oneline --decorate=no -n 1`
CBR_GIT2  := `git log HEAD~2 --oneline --decorate=no -n 1`
CBR_GIT3  := `git log HEAD~3 --oneline --decorate=no -n 1`
CBR_GIT4  := `git log HEAD~4 --oneline --decorate=no -n 1`
CBR_GIT5  := `git log HEAD~5 --oneline --decorate=no -n 1`

CBR := "const char build_id_rom[] = \""' \\'"\n"
CBR := $(CBR)"Project     : $(TARGET)"'\\n \\'"\n"
CBR := $(CBR)"Platform    : $(CBR_PF)"'\\n \\'"\n"
CBR := $(CBR)"Build Date  : $(CBR_DATE)"'\\n \\'"\n"
CBR := $(CBR)"Prepared by : $(USER) $(CBR_USR) <$(CBR_MAIL)>"'\\n \\'"\n"
CBR := $(CBR)"Prepared on : $(CBR_HOST)"'\\n \\'"\n"
CBR := $(CBR)"OS Version  : $(CBR_OS) $(CBR_KRNL)"'\\n \\'"\n"
CBR := $(CBR)"GCC Version : $(CBR_GCC)"'\\n \\'"\n"
CBR := $(CBR)"CFLAGS      : $(CBR_FLGS)"'\\n\\n \\'"\n"
CBR := $(CBR)"Build-ID ROM will contain:"'\\n\\n \\'"\n"
CBR := $(CBR)"   $(CBR_GIT1)"'\\n \\'"\n"
CBR := $(CBR)"   $(CBR_GIT2)"'\\n \\'"\n"
CBR := $(CBR)"   $(CBR_GIT3)"'\\n \\'"\n"
CBR := $(CBR)"   $(CBR_GIT4)"'\\n \\'"\n"
CBR := $(CBR)"   $(CBR_GIT5)"'\\n \\'"\n"
CBR := $(CBR)"\";\n"

$(shell echo $(CBR) > $(MYPATH)/buildid.c)


print-%  : ; @echo $* = $($*)

all:  buildid.o $(TARGET).bin $(TARGET).elf

buildid.o:	$(MYPATH)/buildid.c
	$(CC) -c $^ -o buildid.o

$(TARGET).bin: $(TARGET).elf
	lm32-elf-objcopy -O binary $< $@

$(TARGET).elf:	$(INCPATH)/dbg.c $(INCPATH)/aux.c $(INCPATH)/ebm.c $(INCPATH)/irq.c \
		$(INCPATH)/mini_sdb.c $(MYPATH)/main.c $(INCPATH)/crt0.S $(INCPATH)/mprintf.c \
		$(W1)/dev/uart.c $(MYPATH)/ftm.c
		$(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld $^ 


clean:
	rm -f *.o *.elf *.bin
