#define ASM_FILE
#include <kernel/arch/x86-ibm-bios/config.h>

.code32

.align 4
.section .multiboot, "a"
	.long MULTIBOOT1_HEADER_MAGIC
	.long MULTIBOOT1_FLAGS
	.long MULTIBOOT1_CHECKSUM

	// multiboot aout kludge
	.long 0x00000000 // header_addr
	.long 0x00000000 // load_addr
	.long 0x00000000 // load_end_addr
	.long 0x00000000 // bss_end_addr
	.long 0x00000000 // entry_addr

	// multiboot video info
	.long 0x00000000 // mode_type
	.long 1920       // width
	.long 1080       // height
	.long 32         // depth

.align 8
multiboot2_start:
	.long MULTIBOOT2_HEADER_MAGIC
	.long MULTIBOOT2_ARCHITECTURE_I386
	.long multiboot2_end - multiboot2_start
	.long -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT2_ARCHITECTURE_I386 + (multiboot2_end - multiboot2_start))

	.align 8
	multiboot2_entry_address_tag_start:        
		.short MULTIBOOT2_HEADER_TAG_ENTRY_ADDRESS
		.short MULTIBOOT2_HEADER_TAG_OPTIONAL
		.long multiboot2_entry_address_tag_end - multiboot2_entry_address_tag_start
		.long _start
	multiboot2_entry_address_tag_end:

	.align 8
	multiboot2_framebuffer_tag_start:  
		.short MULTIBOOT2_HEADER_TAG_FRAMEBUFFER
		.short MULTIBOOT2_HEADER_TAG_OPTIONAL
		.long multiboot2_framebuffer_tag_end - multiboot2_framebuffer_tag_start
		.long 1920
		.long 1080
		.long 32
	multiboot2_framebuffer_tag_end:

	.align 8
	.short MULTIBOOT2_HEADER_TAG_END
	.short 0
	.long 8
multiboot2_end:
