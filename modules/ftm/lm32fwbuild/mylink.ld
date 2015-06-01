OUTPUT_FORMAT("elf32-lm32")
ENTRY(_start)
SECTIONS
{
 .output : { 
   *(.text   .text.*)
   *(.rodata .rodata.*)
   *(.data   .data.*)
   *(.bss    .bss.*)
  }
  /DISCARD/ : {
   *(.got     .got.*)
   *(.got.plt .got.plt.*)
   *(.comment .comment.*)
  }
}
