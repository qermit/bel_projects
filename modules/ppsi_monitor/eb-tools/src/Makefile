PWD     := $(shell pwd)
PREFIX  ?= /usr/local
LIBS	  += -letherbone
LIBS    += -Wl,-rpath,$(PREFIX)/lib
CFLAGS  += -I../wb_devices -I./
CFLAGS  += -g -Wall
OBJ 		= wr-mon.o
TOOLS	   = wr-datum wr-track wr-mon wr-mac

all: $(TOOLS)

%: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS)
	
#$(CC) $(CFLAGS) -o $@ -c $^

#wr-mon: $(OBJ)
#	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f *.o wr-datum wr-track wr-mac wr-mon

.PHONY: all clean
