WRPC_SW_PATH ?= ../wrpc-sw

obj-y += \
	$(WRPC_SW_PATH)/lib/util.o \
	$(WRPC_SW_PATH)/lib/atoi.o \
	$(WRPC_SW_PATH)/lib/usleep.o

obj-$(CONFIG_WR_NODE) += $(WRPC_SW_PATH)/lib/net.o

obj-$(CONFIG_ETHERBONE) += \
	$(WRPC_SW_PATH)/lib/arp.o \
	$(WRPC_SW_PATH)/lib/icmp.o \
	$(WRPC_SW_PATH)/lib/ipv4.o \
	$(WRPC_SW_PATH)/lib/bootp.o
