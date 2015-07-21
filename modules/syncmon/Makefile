# Libraries and other stuff
# --------------------------------------------------------------------------------------------------
PREFIX  ?= /usr/local
STAGING ?= 
EB      ?= ../../ip_cores/etherbone-core/api
ECA     ?= ../../ip_cores/wr-cores/modules/wr_eca
TLU     ?= ../../ip_cores/wr-cores/modules/wr_tlu

# Get target architecture
# --------------------------------------------------------------------------------------------------
ARCH          := $(shell getconf LONG_BIT)
ARCH_FLAGS_32 := -DARCH_32_BIT
ARCH_FLAGS_64 := -DARCH_64_BIT
ARCH_FLAGS    := $(ARCH_FLAGS_$(ARCH))

# Compiler specific configuration
# --------------------------------------------------------------------------------------------------
EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(ECA) -I $(TLU) $(ARCH_FLAGS)
LIBS        ?= -L $(EB) -L $(ECA) -L $(TLU) -Wl,-rpath,$(PREFIX)/lib -letherbone -leca -ltlu

# Target(s)
# --------------------------------------------------------------------------------------------------
TARGETS := syncmon

all:	$(TARGETS)

clean:
	rm -f $(TARGETS)

install:
	mkdir -p $(STAGING)$(PREFIX)/bin
	cp $(TARGETS) $(STAGING)$(PREFIX)/bin

%:	%.c
	gcc $(CFLAGS) -o $@ $< $(LIBS)

%:	%.cpp
	g++ $(CFLAGS) -o $@ $< $(LIBS)
