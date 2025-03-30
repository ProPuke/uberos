/*  multiboot.h - Multiboot header file.  */
/*  Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MULTIBOOT1_HEADER
#define MULTIBOOT1_HEADER 1

/* How many bytes from the start of the file we search for the header.  */
#define MULTIBOOT1_SEARCH			8192
#define MULTIBOOT1_HEADER_ALIGN			4

/* The magic field should contain this.  */
#define MULTIBOOT1_HEADER_MAGIC			0x1BADB002

/* This should be in %eax.  */
#define MULTIBOOT1_BOOTLOADER_MAGIC		0x2BADB002

/* Alignment of multiboot modules.  */
#define MULTIBOOT1_MOD_ALIGN			0x00001000

/* Alignment of the multiboot info structure.  */
#define MULTIBOOT1_INFO_ALIGN			0x00000004

/* Flags set in the 'flags' member of the multiboot header.  */

/* Align all boot modules on i386 page (4KB) boundaries.  */
#define MULTIBOOT1_PAGE_ALIGN			0x00000001

/* Must pass memory information to OS.  */
#define MULTIBOOT1_MEMORY_INFO			0x00000002

/* Must pass video information to OS.  */
#define MULTIBOOT1_VIDEO_MODE			0x00000004

/* This flag indicates the use of the address fields in the header.  */
#define MULTIBOOT1_AOUT_KLUDGE			0x00010000

/* Flags to be set in the 'flags' member of the multiboot info structure.  */

/* is there basic lower/upper memory information? */
#define MULTIBOOT1_INFO_MEMORY			0x00000001
/* is there a boot device set? */
#define MULTIBOOT1_INFO_BOOTDEV			0x00000002
/* is the command-line defined? */
#define MULTIBOOT1_INFO_CMDLINE			0x00000004
/* are there modules to do something with? */
#define MULTIBOOT1_INFO_MODS			0x00000008

/* These next two are mutually exclusive */

/* is there a symbol table loaded? */
#define MULTIBOOT1_INFO_AOUT_SYMS		0x00000010
/* is there an ELF section header table? */
#define MULTIBOOT1_INFO_ELF_SHDR			0X00000020

/* is there a full memory map? */
#define MULTIBOOT1_INFO_MEM_MAP			0x00000040

/* Is there drive info?  */
#define MULTIBOOT1_INFO_DRIVE_INFO		0x00000080

/* Is there a config table?  */
#define MULTIBOOT1_INFO_CONFIG_TABLE		0x00000100

/* Is there a boot loader name?  */
#define MULTIBOOT1_INFO_BOOT_LOADER_NAME		0x00000200

/* Is there a APM table?  */
#define MULTIBOOT1_INFO_APM_TABLE		0x00000400

/* Is there video information?  */
#define MULTIBOOT1_INFO_VBE_INFO		        0x00000800
#define MULTIBOOT1_INFO_FRAMEBUFFER_INFO	        0x00001000

#ifndef ASM_FILE

typedef unsigned char		multiboot1_uint8_t;
typedef unsigned short		multiboot1_uint16_t;
typedef unsigned int		multiboot1_uint32_t;
typedef unsigned long long	multiboot1_uint64_t;

struct multiboot1_header
{
  /* Must be MULTIBOOT1_MAGIC - see above.  */
  multiboot1_uint32_t magic;

  /* Feature flags.  */
  multiboot1_uint32_t flags;

  /* The above fields plus this one must equal 0 mod 2^32. */
  multiboot1_uint32_t checksum;

  /* These are only valid if MULTIBOOT1_AOUT_KLUDGE is set.  */
  multiboot1_uint32_t header_addr;
  multiboot1_uint32_t load_addr;
  multiboot1_uint32_t load_end_addr;
  multiboot1_uint32_t bss_end_addr;
  multiboot1_uint32_t entry_addr;

  /* These are only valid if MULTIBOOT1_VIDEO_MODE is set.  */
  multiboot1_uint32_t mode_type;
  multiboot1_uint32_t width;
  multiboot1_uint32_t height;
  multiboot1_uint32_t depth;
};

/* The symbol table for a.out.  */
struct multiboot1_aout_symbol_table
{
  multiboot1_uint32_t tabsize;
  multiboot1_uint32_t strsize;
  multiboot1_uint32_t addr;
  multiboot1_uint32_t reserved;
};
typedef struct multiboot1_aout_symbol_table multiboot1_aout_symbol_table_t;

/* The section header table for ELF.  */
struct multiboot1_elf_section_header_table
{
  multiboot1_uint32_t num;
  multiboot1_uint32_t size;
  multiboot1_uint32_t addr;
  multiboot1_uint32_t shndx;
};
typedef struct multiboot1_elf_section_header_table multiboot1_elf_section_header_table_t;

struct multiboot1_info
{
  /* Multiboot info version number */
  multiboot1_uint32_t flags;

  /* Available memory from BIOS */
  multiboot1_uint32_t mem_lower;
  multiboot1_uint32_t mem_upper;

  /* "root" partition */
  multiboot1_uint32_t boot_device;

  /* Kernel command line */
  multiboot1_uint32_t cmdline;

  /* Boot-Module list */
  multiboot1_uint32_t mods_count;
  multiboot1_uint32_t mods_addr;

  union
  {
    multiboot1_aout_symbol_table_t aout_sym;
    multiboot1_elf_section_header_table_t elf_sec;
  } u;

  /* Memory Mapping buffer */
  multiboot1_uint32_t mmap_length;
  multiboot1_uint32_t mmap_addr;

  /* Drive Info buffer */
  multiboot1_uint32_t drives_length;
  multiboot1_uint32_t drives_addr;

  /* ROM configuration table */
  multiboot1_uint32_t config_table;

  /* Boot Loader Name */
  multiboot1_uint32_t boot_loader_name;

  /* APM table */
  multiboot1_uint32_t apm_table;

  /* Video */
  multiboot1_uint32_t vbe_control_info;
  multiboot1_uint32_t vbe_mode_info;
  multiboot1_uint16_t vbe_mode;
  multiboot1_uint16_t vbe_interface_seg;
  multiboot1_uint16_t vbe_interface_off;
  multiboot1_uint16_t vbe_interface_len;

  multiboot1_uint64_t framebuffer_addr;
  multiboot1_uint32_t framebuffer_pitch;
  multiboot1_uint32_t framebuffer_width;
  multiboot1_uint32_t framebuffer_height;
  multiboot1_uint8_t framebuffer_bpp;
#define MULTIBOOT1_FRAMEBUFFER_TYPE_INDEXED 0
#define MULTIBOOT1_FRAMEBUFFER_TYPE_RGB     1
#define MULTIBOOT1_FRAMEBUFFER_TYPE_EGA_TEXT	2
  multiboot1_uint8_t framebuffer_type;
  union
  {
    struct
    {
      multiboot1_uint32_t framebuffer_palette_addr;
      multiboot1_uint16_t framebuffer_palette_num_colors;
    };
    struct
    {
      multiboot1_uint8_t framebuffer_red_field_position;
      multiboot1_uint8_t framebuffer_red_mask_size;
      multiboot1_uint8_t framebuffer_green_field_position;
      multiboot1_uint8_t framebuffer_green_mask_size;
      multiboot1_uint8_t framebuffer_blue_field_position;
      multiboot1_uint8_t framebuffer_blue_mask_size;
    };
  };
};
typedef struct multiboot1_info multiboot1_info_t;

struct multiboot1_color
{
  multiboot1_uint8_t red;
  multiboot1_uint8_t green;
  multiboot1_uint8_t blue;
};

struct multiboot1_mmap_entry
{
  multiboot1_uint32_t size;
  multiboot1_uint64_t addr;
  multiboot1_uint64_t len;
#define MULTIBOOT1_MEMORY_AVAILABLE		1
#define MULTIBOOT1_MEMORY_RESERVED		2
#define MULTIBOOT1_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT1_MEMORY_NVS                    4
#define MULTIBOOT1_MEMORY_BADRAM                 5
  multiboot1_uint32_t type;
} __attribute__((packed));
typedef struct multiboot1_mmap_entry multiboot1_memory_map_t;

struct multiboot1_mod_list
{
  /* the memory used goes from bytes 'mod_start' to 'mod_end-1' inclusive */
  multiboot1_uint32_t mod_start;
  multiboot1_uint32_t mod_end;

  /* Module command line */
  multiboot1_uint32_t cmdline;

  /* padding to take it to 16 bytes (must be zero) */
  multiboot1_uint32_t pad;
};
typedef struct multiboot1_mod_list multiboot1_module_t;

/* APM BIOS info.  */
struct multiboot1_apm_info
{
  multiboot1_uint16_t version;
  multiboot1_uint16_t cseg;
  multiboot1_uint32_t offset;
  multiboot1_uint16_t cseg_16;
  multiboot1_uint16_t dseg;
  multiboot1_uint16_t flags;
  multiboot1_uint16_t cseg_len;
  multiboot1_uint16_t cseg_16_len;
  multiboot1_uint16_t dseg_len;
};

#endif /* ! ASM_FILE */

#endif /* ! MULTIBOOT1_HEADER */