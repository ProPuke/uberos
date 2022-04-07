ENTRY(_start)

SECTIONS {
	. = 0x80000; /* start at LOADER_ADDR */
	__start = .;

	__text_start = .;
	.text : {
		KEEP(*(.text.boot))
		*(.text .text.*)
	}
	. = ALIGN(4096);
	__text_end = .;
 
	__rodata_start = .;
	.rodata : {
		*(.rodata .rodata.*)

		. = ALIGN(16);
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
	}
	. = ALIGN(4096);
	__rodata_end = .;
 
	__data_start = .;
	.data : {
		*(.data .data.*)
	}
	. = ALIGN(4096);
	__data_end = .;
 
	__bss_start = .;
	.bss : {
		bss = .;
		*(.bss .bss.*)
		*(COMMON)
	}
	. = ALIGN(4096);
	__bss_end = .;
	
	__end = .;
}
