PREFIX  ?= /usr/local
STAGING ?= 
EB      ?= ../../ip_cores/etherbone-core/api
ECA	?= ../../ip_cores/wr-cores/modules/wr_eca
TLU	?= ../../ip_cores/wr-cores/modules/wr_tlu
TARGETS := syncmon

EXTRA_FLAGS ?=
CFLAGS      ?= $(EXTRA_FLAGS) -Wall -O2 -I $(EB) -I $(ECA) -I $(TLU)
LIBS        ?= -L $(EB) -L $(ECA) -L $(TLU) -Wl,-rpath,$(PREFIX)/lib -letherbone -leca -ltlu

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
