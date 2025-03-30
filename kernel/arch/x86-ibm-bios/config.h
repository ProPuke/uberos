#pragma once

#include <lib/multiboot/multiboot1.h>
#include <lib/multiboot/multiboot2.h>

// provide memory map
#define MULTIBOOT1_FLAGS    (MULTIBOOT1_PAGE_ALIGN | MULTIBOOT1_MEMORY_INFO | MULTIBOOT1_VIDEO_MODE)
#define MULTIBOOT1_CHECKSUM -(MULTIBOOT1_HEADER_MAGIC + MULTIBOOT1_FLAGS)
