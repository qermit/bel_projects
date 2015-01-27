OUTPUT_FORMAT("elf32-lm32")
ENTRY(_start)
GROUP(-lgcc -lc)
MEMORY
{
    RAM :
 ORIGIN = 0x00000000,
 LENGTH = 65536 - 8192 - 8192 - 4096
    STACK :
 ORIGIN = 65536 - 8192,
 LENGTH = 8192
    SHARED :
 ORIGIN = 65536 - 8192 - 8192,
 LENGTH = 8192
  BUILDID (r) :
 ORIGIN = 65536 - 8192 - 8192 - 4096,
 LENGTH = 4096
}

SECTIONS
{
 .boot : { *(.boot) } > RAM
 .text : { *(.text .text.*) } > RAM =0
 .rodata : { *(EXCLUDE_FILE (buildid.o) .rodata*) } > RAM
 .data : { *(EXCLUDE_FILE (buildid.o) .data*)
  _gp = ALIGN(16) + 0x7ff0;
 } > RAM
 .bss : {
  _fbss = .;
  *(.bss .bss.*)
  *(COMMON)
  _ebss = .;
 } > RAM
 .shared_mem : { *(.SHARED .SHARED*) } > SHARED
 .build_id : { buildid.o(.data* .rodata*) } > BUILDID

 PROVIDE(_startshared = ORIGIN(SHARED));
 PROVIDE(_endshared = ORIGIN(SHARED) + LENGTH(SHARED) - 4);
 PROVIDE(_buildid = ORIGIN(BUILDID));

 PROVIDE(_endram = ORIGIN(RAM) + LENGTH(RAM) - 4);
 PROVIDE(_fstack = ORIGIN(STACK) + LENGTH(STACK) - 4);
}
PROVIDE(mprintf = pp_printf);


