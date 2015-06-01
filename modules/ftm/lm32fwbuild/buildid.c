#define BUILDID __attribute__((section(".buildid")))
const char BUILDID build_id_rom[] = "\
UserLM32\n\
Project     : dm\n\
Version     : 1.0.0\n\
Platform    : \n\
Build Date  : Fri May 29 14:32:46 CEST 2015\n\
Prepared by : mkreider Mathias Kreider <m.kreider@gsi.de>\n\
Prepared on : Starburst\n\
OS Version  : Ubuntu 13.04  Linux 3.8.0-30-generic x86_64\n\
GCC Version : lm32-elf-gcc (GCC) 4.5.3\n\
CFLAGS      : -I.  -mmultiply-enabled -mbarrel-shift-enabled -Os -DUSRCPUCLK=125000 -I/home/mkreider/hdlprojects/bel_projects/syn/../modules/lm32-include -I/home/mkreider/hdlprojects/bel_projects/syn/../ip_cores/wrpc-sw/include -I/home/mkreider/hdlprojects/bel_projects/syn/../ip_cores/wrpc-sw/sdb-lib -I/home/mkreider/hdlprojects/bel_projects/syn/../ip_cores/wrpc-sw/pp_printf -std=gnu99 -DCONFIG_WR_NODE -DCONFIG_PRINT_BUFSIZE=128 -DSDBFS_BIG_ENDIAN\n\n\
Build-ID ROM will contain:\n\n\
   d3535ca ftm-ctl: added newest required etherbone commit\n\
   422857d ftm-ctl: added legacy (mini-cs) compliance\n\
   0955944 eb-info: fixed severe output bug\n\
   4640db6 ftm-ctl: now supporting new gate & firmware\n\
   2cde2ce changed printf buffer to 128 bytes\n\
";

