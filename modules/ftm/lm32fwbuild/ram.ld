OUTPUT_FORMAT("elf32-lm32")
 SECTIONS
{
 .boot			: { _fboot    = .; *(.boot); 			_eboot      = .; }
 .buildid 0x100	: { _fbuildid = .; *(.buildid .buildid.*)   	_ebuildid   = .; }
 .shared  0x100 + 0x400	: { _fshared  = .; PROVIDE(_startshared = .);*(.shared .shared.*) 	_eshared    = .; }
 .text    0x100 + 0x400 + 8K	: { _ftext    = .; *(.text .text.*) 		_etext      = .; }
 .rodata			: { _frodata  = .; *(.rodata .rodata.*)     	_erodata    = .; }
 .data			: { _fdata    = .; *(.data .data.*)         	_edata      = .; }
 .bss			: { _fbss     = .; *(.bss .bss.*)           	_ebss       = .; }
}

