MAKEFLAGS := --jobs=$(shell nproc)

OUTPUTDIR := ../build
KERNEL_SRCS    := $(shell find . -type d -name 'arch' -prune -or -name "*.S" -print -or -name "*.cpp" -print -or -name "*.c" -print)
KERNEL_HEADERS := $(shell find . -type d -name 'arch' -prune -or -name "*.hpp" -print -or -name "*.inl" -print)
COMMON_SRCS    := $(shell find ../common -name "*.S" -or -name "*.cpp" -or -name "*.c")
COMMON_HEADERS := $(shell find ../common -name "*.hpp" -or -name "*.inl")

ifndef RASPI_MODEL
	RASPI_MODEL := 2
endif

ifndef RASPI_UART
	RASPI_UART := 1
endif

CFLAGS   := -g -fpic -nostartfiles -ffreestanding -nostdlib -Werror -Wall -O2
CXXFLAGS := -g -fpic -nostartfiles -ffreestanding -nostdlib -Werror -Wall -O2 -fno-rtti -std=c++17 -Wno-invalid-offsetof -Wno-parentheses -fno-exceptions
LFLAGS := -O2 -nostdlib -nostartfiles

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

	CC  := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-gcc
	CXX := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-g++
	LD  := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-g++
	GDB := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-gdb
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

	CC  := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-gcc
	CXX := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-g++
	LD  := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-g++
	GDB := ../tools/gcc-arm-none-eabi/bin/arm-none-eabi-gdb
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

	CC  := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-gcc
	CXX := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-g++
	LD  := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-gcc
	GDB := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-gdb
	CPY := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-objcopy
endif

ifeq ($(RASPI_MODEL),4)
	OUTPUTDIR := $(OUTPUTDIR)/raspi4
	# GCC_CPU := cortex-a72
	CLANG_CPU := cortex-a72+nosimd
	# CLANG_TARGET := aarch64-elf
	GCC_ARCH := armv8-a
	DIRECTIVES := -D ARCH_ARM -D ARCH_ARM64 -D ARCH_RASPI -D ARCH_RASPI4
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

	CC  := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-gcc
	CXX := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-g++
	LD  := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-gcc
	GDB := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-gdb
	CPY := ../tools/gcc-arm-aarch64-none-elf/bin/aarch64-none-elf-objcopy
endif

DIRECTIVES := $(DIRECTIVES) -D MEMORY_CHECKS
DIRECTIVES := $(DIRECTIVES) -D ARCH_RASPI_UART$(RASPI_UART)

CFLAGS   := $(CFLAGS) $(DIRECTIVES)
CXXFLAGS := $(CXXFLAGS) $(DIRECTIVES)

CC  := $(CC)  -I ../ $(CFLAGS)
CXX := $(CXX) -I ../ $(CXXFLAGS)

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

all: kernel

test: kernel $(OUTPUTDIR)/$(KERNEL_FILENAME)
	@echo
	$(QEMU) -no-reboot -M $(QEMU_MODEL) -serial stdio -kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) -usb -device usb-kbd -device usb-mouse

deploy: kernel $(OUTPUTDIR)/$(KERNEL_FILENAME)
	@echo
	@cp $(OUTPUTDIR)/boot/$(KERNEL_FILENAME) /media/propuke/boot/
	@umount /media/propuke/boot
	@echo "boot unmounted"

debug: kernel $(OUTPUTDIR)/$(KERNEL_FILENAME)
	@echo
	$(QEMU) -s -S -no-reboot -M $(QEMU_MODEL) -serial stdio -kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) -usb -device usb-kbd -device usb-mouse &
	$(GDB) $(OUTPUTDIR)/$(KERNEL_FILENAME) --eval-command="target remote localhost:1234"

remote-debug: kernel $(OUTPUTDIR)/$(KERNEL_FILENAME)
	@echo
# $(QEMU) -s -S -no-reboot -M $(QEMU_MODEL) -serial stdio -kernel $(OUTPUTDIR)/$(KERNEL_FILENAME) -usb -device usb-kbd -device usb-mouse
	$(QEMU) -s -S -no-reboot -M $(QEMU_MODEL) -serial stdio -kernel $(OUTPUTDIR)/$(KERNEL_FILENAME)


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
	@$(CXX) -Wa,-adhln -c $< -o $@ > $<.output

ifeq ($(RASPI_MODEL),1)
$(OUTPUTDIR)/$(KERNEL_FILENAME): $(KERNEL_OBJS) $(COMMON_OBJS) $(OUTPUTDIR) linker.ld Makefile
	@echo "LD   " $@
	@$(LD) -T linker.ld -o $@ $(LFLAGS) $(KERNEL_OBJS) $(COMMON_OBJS) -lgcc
endif

ifeq ($(RASPI_MODEL),2)
$(OUTPUTDIR)/$(KERNEL_FILENAME): $(KERNEL_OBJS) $(COMMON_OBJS) $(OUTPUTDIR) linker.ld Makefile
	@echo "LD   " $@
	@$(LD) -T linker.ld -o $@ $(LFLAGS) $(KERNEL_OBJS) $(COMMON_OBJS) -lgcc
endif

ifeq ($(RASPI_MODEL),3)
$(OUTPUTDIR)/boot:
	@echo "MKDIR" $@
	@mkdir -p $@
	@echo "CP   " "../tools/firmware"
	@cp -r ../tools/firmware/* $(OUTPUTDIR)/boot
	@rm $(OUTPUTDIR)/boot/*.linux
	@rm $(OUTPUTDIR)/boot/kernel*.img
	@rm $(OUTPUTDIR)/boot/README.*

$(OUTPUTDIR)/$(KERNEL_FILENAME): $(KERNEL_OBJS) $(COMMON_OBJS) $(OUTPUTDIR) $(OUTPUTDIR)/boot linker.ld Makefile
	@echo "LD   " $@
	@$(LD) -T linker.ld -o $@ $(LFLAGS) $(KERNEL_OBJS) $(COMMON_OBJS) -lgcc
	@echo "CPY  " $(OUTPUTDIR)/boot/$(KERNEL_FILENAME)
	@$(CPY) -O binary $@ $(OUTPUTDIR)/boot/$(KERNEL_FILENAME)

endif

ifeq ($(RASPI_MODEL),4)
$(OUTPUTDIR)/boot:
	@echo "MKDIR" $@
	@mkdir -p $@
	@echo "CP   " "../tools/firmware"
	@cp -r ../tools/firmware/* $(OUTPUTDIR)/boot
	@rm $(OUTPUTDIR)/boot/*.linux
	@rm $(OUTPUTDIR)/boot/kernel*.img
	@rm $(OUTPUTDIR)/boot/README.*

$(OUTPUTDIR)/$(KERNEL_FILENAME): $(KERNEL_OBJS) $(COMMON_OBJS) $(OUTPUTDIR) $(OUTPUTDIR)/boot linker.ld Makefile
	@echo "LD   " $@
	@$(LD) -T linker.ld -o $@ $(LFLAGS) $(KERNEL_OBJS) $(COMMON_OBJS) -lgcc
	@echo "CPY  " $(OUTPUTDIR)/boot/$(KERNEL_FILENAME)
	@$(CPY) -O binary $@ $(OUTPUTDIR)/boot/$(KERNEL_FILENAME)

endif

# $(OUTPUTDIR)/$(KERNEL_FILENAME): $(KERNEL_OBJS) $(COMMON_OBJS) $(OUTPUTDIR) linker.ld Makefile
# 	@echo "LD   " $@
# 	@$(LD) -T linker.ld -o $@ $(LFLAGS) $(KERNEL_OBJS) $(COMMON_OBJS) -lgcc

.PHONY: clean
clean:
	@$(RM) -rf ../build
	@find . -type f -name '*.o' -delete
	@find ../common -type f -name '*.o' -delete

.PHONY: kernel
kernel: $(OUTPUTDIR)/$(KERNEL_FILENAME)
