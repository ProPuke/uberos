MAKEFLAGS := --jobs=$(shell nproc)

OUTPUTDIR := ../build
KERNEL_SRCS    := $(shell find . -type d -name 'arch' -prune -or -name "*.S" -print -or -name "*.cpp" -print -or -name "*.c" -print)
KERNEL_HEADERS := $(shell find . -type d -name 'arch' -prune -or -name "*.hpp" -print -or -name "*.inl" -print)
COMMON_SRCS    := $(shell find ../common -name "*.S" -or -name "*.cpp" -or -name "*.c")
COMMON_HEADERS := $(shell find ../common -name "*.hpp" -or -name "*.inl")

DEBUG ?= 1

RASPI_MODEL ?= 4
RASPI_UART  ?= 0

CFLAGS   := -fpic -nostartfiles -ffreestanding -nostdlib -Werror -Wall
CXXFLAGS := -fpic -nostartfiles -ffreestanding -nostdlib -Werror -Wall -fno-rtti -std=c++17 -Wno-invalid-offsetof -Wno-parentheses -fno-exceptions
LFLAGS   := -nostdlib -nostartfiles

ifeq ($(DEBUG),1)
	CFLAGS   := $(CFLAGS) -g -DDEBUG
	CXXFLAGS := $(CXXFLAGS) -g -DDEBUG
else
	CFLAGS   := $(CFLAGS) -O2
	CXXFLAGS := $(CXXFLAGS) -O2
	LFLAGS   := $(LFLAGS) -O2
endif

ifeq ($(RASPI_MODEL),1)
	OUTPUTDIR := $(OUTPUTDIR)/raspi1
	GCC_CPU := arm1176jzf-s
	DIRECTIVES := -D ARCH_ARM -D ARCH_ARM32 -D ARCH_RASPI -D ARCH_RASPI1
	QEMU := qemu-system-arm
	QEMU_MODEL := raspi1ap
	# QEMU_DTB := ../tools/firmware/bcm2708-rpi-b.dtb
	KERNEL_FILENAME := kernel.img

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(wildcard arch/arm/*.S) $(wildcard arch/arm/*.cpp) $(wildcard arch/arm/*.c)
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(wildcard arch/arm/*.hpp) $(wildcard arch/arm/*.inl)

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(shell find arch/arm32 -name "*.S" -or -name "*.cpp" -or -name "*.c")
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(shell find arch/arm32 -name "*.hpp" -or -name "*.inl")

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(wildcard arch/raspi/*.S) $(wildcard arch/raspi/*.cpp) $(wildcard arch/raspi/*.c)
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(wildcard arch/raspi/*.hpp) $(wildcard arch/raspi/*.inl)

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(shell find arch/raspi/armv7 -name "*.S" -or -name "*.cpp" -or -name "*.c")
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(shell find arch/raspi/armv7 -name "*.hpp" -or -name "*.inl")

	TOOLCHAIN_PREFIX := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-
endif

ifeq ($(RASPI_MODEL),2)
	OUTPUTDIR := $(OUTPUTDIR)/raspi2
	# GCC_CPU := cortex-a7
	# GCC_CPU := armv7e-m
	GCC_CPU := generic-armv7-a
	DIRECTIVES := -D ARCH_ARM -D ARCH_ARM32 -D ARCH_RASPI -D ARCH_RASPI2
	QEMU := qemu-system-arm
	QEMU_MODEL := raspi2b
	# QEMU_DTB := ../tools/firmware/bcm2709-rpi-2-b.dtb
	KERNEL_FILENAME := kernel7.img

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(wildcard arch/arm/*.S) $(wildcard arch/arm/*.cpp) $(wildcard arch/arm/*.c)
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(wildcard arch/arm/*.hpp) $(wildcard arch/arm/*.inl)

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(shell find arch/arm32 -name "*.S" -or -name "*.cpp" -or -name "*.c")
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(shell find arch/arm32 -name "*.hpp" -or -name "*.inl")

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(wildcard arch/raspi/*.S) $(wildcard arch/raspi/*.cpp) $(wildcard arch/raspi/*.c)
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(wildcard arch/raspi/*.hpp) $(wildcard arch/raspi/*.inl)

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(shell find arch/raspi/armv7 -name "*.S" -or -name "*.cpp" -or -name "*.c")
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(shell find arch/raspi/armv7 -name "*.hpp" -or -name "*.inl")

	TOOLCHAIN_PREFIX := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-
endif

ifeq ($(RASPI_MODEL),3)
	OUTPUTDIR := $(OUTPUTDIR)/raspi3
	# GCC_CPU := cortex-a53
	CLANG_CPU := cortex-a53+nosimd
	# CLANG_TARGET := aarch64-elf
	GCC_ARCH := armv8-a
	DIRECTIVES := -D ARCH_ARM -D ARCH_ARM64 -D ARCH_RASPI -D ARCH_RASPI3
	QEMU := qemu-system-aarch64
	QEMU_MODEL := raspi3b
	# QEMU_DTB := ../tools/firmware/bcm2709-rpi-2-b.dtb
	KERNEL_FILENAME := kernel8.img

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(wildcard arch/arm/*.S) $(wildcard arch/arm/*.cpp) $(wildcard arch/arm/*.c)
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(wildcard arch/arm/*.hpp) $(wildcard arch/arm/*.inl)

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(shell find arch/arm64 -name "*.S" -or -name "*.cpp" -or -name "*.c")
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(shell find arch/arm64 -name "*.hpp" -or -name "*.inl")

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(wildcard arch/raspi/*.S) $(wildcard arch/raspi/*.cpp) $(wildcard arch/raspi/*.c)
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(wildcard arch/raspi/*.hpp) $(wildcard arch/raspi/*.inl)

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(shell find arch/raspi/armv8 -name "*.S" -or -name "*.cpp" -or -name "*.c")
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(shell find arch/raspi/armv8 -name "*.hpp" -or -name "*.inl")

	CFLAGS   := $(CFLAGS) -mstrict-align
	CXXFLAGS := $(CXXFLAGS) -mstrict-align
	LFLAGS   := $(LFLAGS) -mstrict-align

	TOOLCHAIN_PREFIX := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-
endif

ifeq ($(RASPI_MODEL),4)
	OUTPUTDIR := $(OUTPUTDIR)/raspi4
	# GCC_CPU := cortex-a72
	CLANG_CPU := cortex-a72+nosimd
	# CLANG_TARGET := aarch64-elf
	GCC_ARCH := armv8-a
	DIRECTIVES := -D ARCH_ARM -D ARCH_ARM64 -D ARCH_RASPI -D ARCH_RASPI4
	# QEMU := qemu-system-aarch64
	QEMU := ~/libs/qemu-patch-raspberry4/build/qemu-system-aarch64
	QEMU_MODEL := raspi4b2g
	# QEMU_DTB := ../tools/firmware/bcm2709-rpi-2-b.dtb
	KERNEL_FILENAME := kernel8.img

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(wildcard arch/arm/*.S) $(wildcard arch/arm/*.cpp) $(wildcard arch/arm/*.c)
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(wildcard arch/arm/*.hpp) $(wildcard arch/arm/*.inl)

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(shell find arch/arm64 -name "*.S" -or -name "*.cpp" -or -name "*.c")
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(shell find arch/arm64 -name "*.hpp" -or -name "*.inl")

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(wildcard arch/raspi/*.S) $(wildcard arch/raspi/*.cpp) $(wildcard arch/raspi/*.c)
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(wildcard arch/raspi/*.hpp) $(wildcard arch/raspi/*.inl)

	KERNEL_SRCS    := $(KERNEL_SRCS)    $(shell find arch/raspi/armv8 -name "*.S" -or -name "*.cpp" -or -name "*.c")
	KERNEL_HEADERS := $(KERNEL_HEADERS) $(shell find arch/raspi/armv8 -name "*.hpp" -or -name "*.inl")

	CFLAGS   := $(CFLAGS) -mstrict-align
	CXXFLAGS := $(CXXFLAGS) -mstrict-align
	LFLAGS   := $(LFLAGS) -mstrict-align

	TOOLCHAIN_PREFIX := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-
endif

# DIRECTIVES := $(DIRECTIVES) -D MEMORY_CHECKS
DIRECTIVES := $(DIRECTIVES) -D ARCH_RASPI_UART$(RASPI_UART)

CFLAGS   := $(CFLAGS) $(DIRECTIVES)
CXXFLAGS := $(CXXFLAGS) $(DIRECTIVES)

CC  := $(TOOLCHAIN_PREFIX)gcc -I ../ $(CFLAGS)
CXX := $(TOOLCHAIN_PREFIX)g++ -I ../ $(CXXFLAGS)
LD  := $(TOOLCHAIN_PREFIX)gcc
GDB := $(TOOLCHAIN_PREFIX)gdb
CPY := $(TOOLCHAIN_PREFIX)objcopy

ifdef GCC_CPU
	CC  := $(CC)  -mcpu=$(GCC_CPU)
	CXX := $(CXX) -mcpu=$(GCC_CPU)
	LD  := $(LD)  -mcpu=$(GCC_CPU)
endif
ifdef GCC_ARCH
	CC  := $(CC)  -march=$(GCC_ARCH)
	CXX := $(CXX) -march=$(GCC_ARCH)
	LD  := $(LD)  -march=$(GCC_ARCH)
endif
ifdef CLANG_TARGET
	CC  := $(CC)  --target=$(CLANG_TARGET) -mcpu=$(CLANG_CPU)
	CXX := $(CXX) --target=$(CLANG_TARGET) -mcpu=$(CLANG_CPU)
	LD  := $(LD)  --target=$(CLANG_TARGET) -mcpu=$(CLANG_CPU)
endif

KERNEL_OBJS := $(KERNEL_SRCS:%=%.o)
COMMON_OBJS := $(COMMON_SRCS:%=%.o)
MSDF_ATLAS_GEN := ../tools/msdf-atlas-gen/bin/msdf-atlas-gen

.PHONY: all
all: kernel

.PHONY: test
test: kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) $(OUTPUTDIR)/boot/config.txt
	@echo
	$(QEMU) -no-reboot -M $(QEMU_MODEL) -serial stdio -kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) -usb -device usb-kbd -device usb-mouse

.PHONY: deploy
deploy: kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) $(OUTPUTDIR)/boot/config.txt
	@echo
	@echo "Deploy..."
	@echo "CPY  boot/" $(KERNEL_FILENAME)
	@echo "CPY  boot/" config.txt
	@cp $(OUTPUTDIR)/boot/$(KERNEL_FILENAME) /media/propuke/boot/
	@cp $(OUTPUTDIR)/boot/config.txt /media/propuke/boot/
	@umount /media/propuke/boot
	@echo "Unmount"

.PHONY: debug
debug: kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) $(OUTPUTDIR)/boot/config.txt
	@echo
	$(QEMU) -s -S -no-reboot -M $(QEMU_MODEL) -serial stdio -kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) -usb -device usb-kbd -device usb-mouse &
	$(GDB) $(OUTPUTDIR)/$(KERNEL_FILENAME) --eval-command="target remote localhost:1234"

.PHONY: remote-debug
remote-debug: kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) $(OUTPUTDIR)/boot/config.txt
	@echo
	$(QEMU) -s -S -no-reboot -M $(QEMU_MODEL) -serial stdio -kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) -usb -device usb-kbd -device usb-mouse


$(OUTPUTDIR):
	@echo "MKDIR" $@
	@mkdir -p $@

.PHONY: fonts
fonts:
	@$(MSDF_ATLAS_GEN) -size 32 -font fonts/Open_Sans/OpenSans-Regular.ttf -format text -imageout graphics2d/font/OpenSans.txt -json graphics2d/font/OpenSans.json
	@$(MSDF_ATLAS_GEN) -size 32 -font fonts/Roboto_Mono/RobotoMono-VariableFont_wght.ttf -format text -imageout graphics2d/font/Roboto_Mono.txt -json graphics2d/font/Roboto_Mono.json
	@cd graphics2d/font; ts-node build

%.S.o: %.S Makefile $(KERNEL_HEADERS) $(COMMON_HEADERS)
	@echo "C    " $<
	@$(CC) -c $< -o $@

%.c.o: %.c Makefile $(KERNEL_HEADERS) $(COMMON_HEADERS)
	@echo "C    " $<
	@$(CC) -c $< -o $@

%.cpp.o: %.cpp Makefile $(KERNEL_HEADERS) $(COMMON_HEADERS)
	@echo "CPP  " $<
	@$(CXX) -c $< -o $@
# @$(CXX) -Wa,-adhln -c $< -o $@ > $<.output

$(OUTPUTDIR)/boot:
	@echo "MKDIR" $@
	@mkdir -p $@
	@echo "CP   " "../tools/firmware"
	@cp -r ../tools/firmware/* $(OUTPUTDIR)/boot
	@echo "CP   " "../assets/boot"
	@cp -r ../assets/boot/* $(OUTPUTDIR)/boot
	@rm $(OUTPUTDIR)/boot/*.linux
	@rm $(OUTPUTDIR)/boot/kernel*.img
	@rm $(OUTPUTDIR)/boot/README.*

$(OUTPUTDIR)/$(KERNEL_FILENAME): $(KERNEL_OBJS) $(COMMON_OBJS) linker.ld Makefile | $(OUTPUTDIR) $(OUTPUTDIR)/boot
	@echo "LD   " $@
	@$(LD) -T linker.ld -o $@ $(LFLAGS) $(KERNEL_OBJS) $(COMMON_OBJS) -lgcc
	@echo "DUMP " $@.lst
	$(TOOLCHAIN_PREFIX)objdump --disassemble --demangle --source $@ > $@.lst
	@echo "CPY  " $(OUTPUTDIR)/boot/$(KERNEL_FILENAME)
	@$(CPY) -O binary $@ $(OUTPUTDIR)/boot/$(KERNEL_FILENAME)

$(OUTPUTDIR)/boot/config.txt: ../assets/boot/config.txt | $(OUTPUTDIR)/boot
	@echo "CP   " "../assets/boot/config.txt"
	@cp ../assets/boot/config.txt $(OUTPUTDIR)/boot/config.txt

# $(OUTPUTDIR)/$(KERNEL_FILENAME): $(KERNEL_OBJS) $(COMMON_OBJS) $(OUTPUTDIR) linker.ld Makefile
# 	@echo "LD   " $@
# 	@$(LD) -T linker.ld -o $@ $(LFLAGS) $(KERNEL_OBJS) $(COMMON_OBJS) -lgcc

.PHONY: clean
clean:
	@$(RM) -rf ../build
	@find . -type f -name '*.o' -delete
	@find . -type f -name '*.cpp.output' -delete
	@find ../common -type f -name '*.o' -delete
	@find ../common -type f -name '*.cpp.output' -delete

.PHONY: kernel
kernel: $(OUTPUTDIR)/$(KERNEL_FILENAME)
