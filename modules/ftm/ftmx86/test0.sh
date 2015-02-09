#!/bin/bash
dev=$1
shared="0x100800"
ftmp="/home/mkreider/hdlprojects/new_bel_projects/modules/ftm"
fwp="$ftmp/ftmfw"
x86p="$ftmp/ftmx86"


./ftm-ctl $dev -c -1 loadfw $fwp/ftm.bin
./ftm-ctl $dev -c -1 clear
./ftm-ctl $dev -c 0 put $x86p/sourceLinac.xml
./ftm-ctl $dev -c 1 put $x86p/ring.xml
./ftm-ctl $dev -c 2 put $x86p/heartbeat.xml
eb-write  $dev $shared/4 0x4712
./ftm-ctl $dev -c -1 swap
./ftm-ctl $dev -c 2 run
./ftm-ctl $dev -c 1 run
./ftm-ctl $dev -c 0 run
./ftm-ctl $dev
