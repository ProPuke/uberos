ENTRY(_start)

SECTIONS {
	. = 2M;
	__start = .;

	. = ALIGN(4K);
	__text_start = .;
	.text : {
		*(.multiboot)
		KEEP(*(.text.boot))
		KEEP(*(.init))
		*(.text .text.*)
	}
	. = ALIGN(4K);
	__text_end = .;


	. = ALIGN(4K);
	__rodata_start = .;
	.rodata : {
		*(.rodata .rodata.*)
	}
	. = ALIGN(4K);
	__rodata_end = .;

	. = ALIGN(4K);
	__data_start = .;
	.data : {
		*(.data .data.*)
	}
	. = ALIGN(4K);
	__data_end = .;

	. = ALIGN(4K);
	__bss_start = .;
	.bss : {
		*(.bss .bss.*)
		*(COMMON)
	}
	. = ALIGN(4K);
	__bss_end = .;

	__end = .;
}
