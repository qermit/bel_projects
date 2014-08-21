WRPC_SW_PATH ?= ../wrpc-sw

obj-$(CONFIG_WR_NODE) += \
	$(WRPC_SW_PATH)/dev/endpoint.o \
	$(WRPC_SW_PATH)/dev/ep_pfilter.o \
	$(WRPC_SW_PATH)/dev/i2c.o \
	$(WRPC_SW_PATH)/dev/minic.o \
	$(WRPC_SW_PATH)/dev/pps_gen.o \
	$(WRPC_SW_PATH)/dev/syscon.o \
	$(WRPC_SW_PATH)/dev/sdb.o \
	../../modules/lm32-include/mini_sdb.o \
	../../modules/lm32-include/disp-lcd.o

obj-$(CONFIG_WR_SWITCH) += \
	$(WRPC_SW_PATH)/dev/timer-wrs.o \
	 $(WRPC_SW_PATH)/dev/ad9516.o

obj-$(CONFIG_LEGACY_EEPROM) += $(WRPC_SW_PATH)/dev/eeprom.o

obj-$(CONFIG_SDB_EEPROM) += $(WRPC_SW_PATH)/dev/sdb-eeprom.o

obj-$(CONFIG_W1) += \
	$(WRPC_SW_PATH)/dev/w1.o \
	$(WRPC_SW_PATH)/dev/w1-hw.o

obj-$(CONFIG_W1) += \
	$(WRPC_SW_PATH)/dev/w1-temp.o \
	$(WRPC_SW_PATH)/dev/w1-eeprom.o

obj-$(CONFIG_UART) += $(WRPC_SW_PATH)/dev/uart.o

obj-$(CONFIG_UART_SW) += $(WRPC_SW_PATH)/dev/uart-sw.o
