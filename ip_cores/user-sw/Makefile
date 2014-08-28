# ====================================================================================================
# Makefile for user core
# ====================================================================================================
# Taken and modified from ../wrpc-sw/

# ====================================================================================================
# Tool chain
# ====================================================================================================
CROSS_COMPILE ?= lm32-elf-
CC             = $(CROSS_COMPILE)gcc
LD             = $(CROSS_COMPILE)ld
OBJDUMP        = $(CROSS_COMPILE)objdump
OBJCOPY        = $(CROSS_COMPILE)objcopy
SIZE           = $(CROSS_COMPILE)size
export CROSS_COMPILE

# ====================================================================================================
# Includes
# ====================================================================================================
-include $(CURDIR)/.config

# ====================================================================================================
# Paths for sources (wrpc core, lm32 utilities, ...)
# ====================================================================================================
WRPC_SW_PATH        = ../wrpc-sw
USER_LOCATION       = ../../../
USER_LM32_INC_PATH  = $(USER_LOCATION)/bel_projects/modules/lm32-include
USER_LM32_STUB_PATH = $(USER_LOCATION)/bel_projects/modules/lm32_stub

# ====================================================================================================
# Variables
# ====================================================================================================
TOOLS                  = $(WRPC_SW_PATH)/tools
AUTOCONF               = $(CURDIR)/include/generated/autoconf.h
CFLAGS_PLATFORM        = -mmultiply-enabled -mbarrel-shift-enabled
LDFLAGS_PLATFORM       = -mmultiply-enabled -mbarrel-shift-enabled \
                         -nostdlib -T $(LDS-y)
CFLAGS                 = $(CFLAGS_PLATFORM) $(cflags-y) -Wall \
                         -ffunction-sections -fdata-sections -Os \
                         -include include/trace.h -ggdb \
                         -I$(USER_LM32_INC_PATH) \
                         -DCONFIG_USER_UART -DBASE_ONEWIRE=pOneWire -DCONFIG_USER_ONE_WIRE \
                         -I$(WRPC_SW_PATH)/softpll
LDFLAGS                = $(LDFLAGS_PLATFORM) \
                         -Wl,--gc-sections -Os -lgcc -lc
obj-$(CONFIG_WR_NODE) += main.o
cflags-y               = -ffreestanding -include $(AUTOCONF) -Iinclude/std -Iinclude \
                         -I. -Iipc
cflags-y              += -I$(WRPC_SW_PATH)/pp_printf
REVISION               = $(shell git describe --dirty --always)

# ====================================================================================================
# Platform dependent sources
# ====================================================================================================
obj-y += arch/lm32/crt0.o $(WRPC_SW_PATH)/arch/lm32/irq.o

# ====================================================================================================
# Linker files
# ====================================================================================================
LDS-$(CONFIG_WR_NODE) = $(WRPC_SW_PATH)/arch/lm32/ram.ld

# ====================================================================================================
# Variables from other makefiles
# ====================================================================================================
include shell/shell.mk
include lib/lib.mk
include pp_printf/printf.mk
include dev/dev.mk

# ====================================================================================================
# Misc
# ====================================================================================================
obj-$(CONFIG_WR_NODE)    += $(WRPC_SW_PATH)/check-error.o
obj-$(CONFIG_WR_NODE)    += sdb-lib/libsdbfs.a
cflags-$(CONFIG_WR_NODE) += -Isdb-lib
OBJS                      = $(obj-y)
OUTPUT-$(CONFIG_WR_NODE)  = user_main
OUTPUT                   := $(OUTPUT-y)

# ====================================================================================================
# Targets
# ====================================================================================================
# special targets
.PRECIOUS: %.elf %.bin
.PHONY: all tools clean

# our linker script is preprocessed, so have a rule here
%.ld: %.ld.S $(AUTOCONF) .config
	$(CC) -include $(AUTOCONF) -E -P $*.ld.S -o $@

all: tools $(OUTPUT).ram $(OUTPUT).vhd $(OUTPUT).mif

sdb-lib/libsdbfs.a:
	$(MAKE) -C sdb-lib

$(OUTPUT).elf: $(LDS-y) $(AUTOCONF) $(OUTPUT).o config.o
	$(CC) $(CFLAGS) -DGIT_REVISION=\"$(REVISION)\" -c $(WRPC_SW_PATH)/revision.c
	${CC} -o $@ revision.o config.o $(OUTPUT).o $(LDFLAGS)
	${OBJDUMP} -d $(OUTPUT).elf > $(OUTPUT)_disasm.S
	$(SIZE) $@

$(OUTPUT).o: $(OBJS)
#	$(LD) --gc-sections -e _start -r $(OBJS) -T bigobj.lds -o $@
	$(LD) -e _start -r $(OBJS) -T bigobj.lds -o $@

config.o: .config
	sed '1,3d' .config > .config.bin
	dd bs=1 count=1 if=/dev/zero 2> /dev/null >> .config.bin
	$(OBJCOPY) -I binary -O elf32-lm32 -B lm32 \
		--rename-section .data=.data.config  .config.bin $@
	rm -f .config.bin

%.bin: %.elf
	${OBJCOPY} -O binary $^ $@

%.ram: tools %.bin
	$(TOOLS)/genraminit $*.bin 0 > $@

%.vhd: tools %.bin
	$(TOOLS)/genramvhd -s `. ./.config; echo $$CONFIG_RAMSIZE` $*.bin > $@

%.mif: tools %.bin
	$(TOOLS)/genrammif $*.bin `. ./.config; echo $$CONFIG_RAMSIZE` > $@

$(AUTOCONF): silentoldconfig

clean:
	rm -f $(OBJS) $(OUTPUT).elf $(OUTPUT).bin $(OUTPUT).ram $(LDS)
	$(MAKE) -C sdb-lib clean

%.o:		%.c
	${CC} $(CFLAGS) $(PTPD_CFLAGS) $(LIB_DIR) -c $*.c -o $@

tools:
	$(MAKE) -C $(TOOLS)

# following targets from Makefile.kconfig
silentoldconfig:
	@mkdir -p include/config
	$(MAKE) -f Makefile.kconfig $@

scripts_basic config:
	$(MAKE) -f Makefile.kconfig $@

%config:
	$(MAKE) -f Makefile.kconfig $@

defconfig:
	$(MAKE) -f Makefile.kconfig spec_defconfig

.config: silentoldconfig

# This forces more compilations than needed, but it's useful
# (we depend on .config and not on include/generated/autoconf.h
# because the latter is touched by silentoldconfig at each build)
$(obj-y): .config $(wildcard include/*.h)

test: clean all
	lm32-elf-readelf -hS $(OUTPUT).elf
	lm32-ctl load $(OUTPUT).elf
