ENTRY(_start)

SECTIONS
{
	/* Starts at LOADER_ADDR. */
	. = 0x8000;
	__start = .;
	__text_start = .;
	.text :
	{
		KEEP(*(.text.boot))
		*(.text)
	}
	. = ALIGN(4096); /* align to page size */
	__text_end = .;
 
	__rodata_start = .;
	.rodata :
	{
		*(.rodata)

		. = ALIGN(16);
		__init_array_start = .;
		KEEP (*(SORT(.init_array.*)))
		KEEP (*(.init_array))
		__init_array_end = .;
	}
	. = ALIGN(4096); /* align to page size */
	__rodata_end = .;
 
	__data_start = .;
	.data :
	{
		*(.data)
	}
	. = ALIGN(4096); /* align to page size */
	__data_end = .;
 
	__bss_start = .;
	.bss :
	{
		bss = .;
		*(.bss)
	}
	. = ALIGN(4096); /* align to page size */
	__bss_end = .;
	
	__end = .;
}