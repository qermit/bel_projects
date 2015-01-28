# User LM32 firmware build script
################################################################
# only change stuff below if you really know what you're doing #
################################################################
RAMSIZE    ?= 64K
STACKSIZE  ?= 8K
SHAREDSIZE ?= 8K

LD        := lm32-elf-ld
CC        := lm32-elf-gcc
INCPATH   := $(CHECKOUT)/modules/lm32-include
W1        := $(CHECKOUT)/ip_cores/wrpc-sw
CFLAGS    := -std=gnu99 -I$(INCPATH) -mmultiply-enabled -mbarrel-shift-enabled -Os -I$(W1)/include -I$(W1)/pp_printf $(MYFLAGS)

SRC       = $(INCPATH)/dbg.c $(INCPATH)/aux.c $(INCPATH)/ebm.c $(INCPATH)/irq.c \
                $(INCPATH)/mini_sdb.c $(INCPATH)/crt0.S $(INCPATH)/mprintf.c \
                $(W1)/dev/uart.c $(MYSRC)

IDSIZE := 4K

LDS := "OUTPUT_FORMAT(\"elf32-lm32\")\n"
LDS := $(LDS)"ENTRY(_start)\nGROUP(-lgcc -lc)\nMEMORY\n{\n    RAM :\n"
LDS := $(LDS)" ORIGIN = 0x00000000,\n"
LDS := $(LDS)" LENGTH = $(RAMSIZE) - $(STACKSIZE) - $(SHAREDSIZE) - $(IDSIZE)\n    STACK :\n"
LDS := $(LDS)" ORIGIN = $(RAMSIZE) - $(STACKSIZE),\n LENGTH = $(STACKSIZE)\n    SHARED :\n"
LDS := $(LDS)" ORIGIN = $(RAMSIZE) - $(STACKSIZE) - $(SHAREDSIZE),\n LENGTH = $(SHAREDSIZE)\n  BUILDID (r) :\n"
LDS := $(LDS)" ORIGIN = $(RAMSIZE) - $(STACKSIZE) - $(SHAREDSIZE) - $(IDSIZE),\n LENGTH = $(IDSIZE)\n\n}"
LDS := $(LDS)"SECTIONS\n{\n .boot   : { *(.boot) } > RAM\n"
LDS := $(LDS)" .text   : { *(.text .text.*) } > RAM =0\n"
LDS := $(LDS)" .rodata : { *(EXCLUDE_FILE (buildid.o) .rodata*) } > RAM\n"
LDS := $(LDS)" .data   : { *(EXCLUDE_FILE (buildid.o) .data*)\n"
LDS := $(LDS)"  _gp = ALIGN(16) + 0x7ff0;\n } > RAM\n"
LDS := $(LDS)" .bss : {\n  _fbss = .;\n  *(.bss .bss.*)\n  *(COMMON)\n  _ebss = .;\n } > RAM\n"
LDS := $(LDS)" .build_id : { buildid.o(.data* .rodata*) } > BUILDID\n}\n"
LDS := $(LDS)"PROVIDE(_startshared = ORIGIN(SHARED));\n"
LDS := $(LDS)"PROVIDE(_endshared   = ORIGIN(SHARED) + LENGTH(SHARED) - 4);\n"
LDS := $(LDS)"PROVIDE(_buildid     = ORIGIN(BUILDID));\n"
LDS := $(LDS)"PROVIDE(_endram      = ORIGIN(RAM) + LENGTH(RAM) - 4);\n"
LDS := $(LDS)"PROVIDE(_fstack      = ORIGIN(STACK) + LENGTH(STACK) - 4);\n"
LDS := $(LDS)"PROVIDE(mprintf      = pp_printf);\n"



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

$(shell echo $(CBR) > $(MYPATH)/buildid.c; echo $(LDS) > $(MYPATH)/linker.ld)


print-%  : ; @echo $* = $($*)

all:  buildid.o $(TARGET).bin $(TARGET).elf

buildid.o:	$(MYPATH)/buildid.c
	$(CC) -c $^ -o buildid.o

$(TARGET).bin: $(TARGET).elf
	lm32-elf-objcopy -O binary $< $@

$(TARGET).elf:	$(SRC)
		$(CC) $(CFLAGS) -o $@ -nostdlib -T linker.ld $^ 


clean:
	rm -f *.o *.elf *.bin buildid.c linker.ld
