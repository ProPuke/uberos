OUTPUT_FORMAT(pei-i386)

ENTRY(_start)

SECTIONS {
	. = 2M;
	__start = .;

	__text_start = .;
	.text : {
		*(.multiboot)
		KEEP(*(.text.boot))
		KEEP(*(.init))
		*(.text .text.*)
	}
	. = ALIGN(4K);
	__text_end = .;

	__rodata_start = .;
	.rodata : {
		*(.rodata .rodata.*)
	}
	. = ALIGN(4K);
	__rodata_end = .;

	__data_start = .;
	.data : {
		*(.data .data.*)
	}
	. = ALIGN(4K);
	__data_end = .;

	__bss_start = .;
	.bss : {
		*(.bss .bss.*)
		*(COMMON)
	}
	. = ALIGN(4K);
	__bss_end = .;

	__end = .;
}
