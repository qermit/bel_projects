# This is included from ../Makefile, for the wrc build system.
# The Makefile in this directory is preserved from the upstream version

WRPC_SW_PATH ?= ../wrpc-sw

obj-y += $(WRPC_SW_PATH)/pp_printf/printf.o

ppprintf-$(CONFIG_PRINTF_FULL) += $(WRPC_SW_PATH)/pp_printf/vsprintf-full.o
ppprintf-$(CONFIG_PRINTF_MINI) += $(WRPC_SW_PATH)/pp_printf/vsprintf-mini.o
ppprintf-$(CONFIG_PRINTF_NONE) += $(WRPC_SW_PATH)/pp_printf/vsprintf-none.o
ppprintf-$(CONFIG_PRINTF_XINT) += $(WRPC_SW_PATH)/pp_printf/vsprintf-xint.o

ppprintf-y ?= $(WRPC_SW_PATH)/pp_printf/vsprintf-xint.o

obj-y += $(ppprintf-y)


