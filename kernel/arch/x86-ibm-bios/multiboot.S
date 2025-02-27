#define ASM_FILE
#include <kernel/arch/x86-ibm-bios/config.h>

.code32

.section .multiboot, "a"
	.align 4
	.long MULTIBOOT_HEADER_MAGIC
	.long MULTIBOOT_FLAGS
	.long MULTIBOOT_CHECKSUM

// multiboot memory info
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
