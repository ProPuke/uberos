#pragma once

#include <lib/multiboot/multiboot.h>

// provide memory map
#define MULTIBOOT_FLAGS    (MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO | MULTIBOOT_VIDEO_MODE)
#define MULTIBOOT_CHECKSUM -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_FLAGS)
