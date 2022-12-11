# tuts
	Building an Operating System for the Raspberry Pi
	https://jsandler18.github.io/

	Bare Metal Programming on Raspberry Pi 3
	https://github.com/bztsrc/raspi3-tutorial

	A tiny, modern kernel for Raspberry Pi 3
	https://github.com/fxlin/p1-kernel

	https://s-matyukevich.github.io/raspberry-pi-os/

	https://www.rpi4os.com/

	https://github.com/LdB-ECM/Raspberry-Pi

	https://github.com/valvers/arm-tutorial-rpi
	https://www.valvers.com/open-software/raspberry-pi/bare-metal-programming-in-c-part-1/

	https://github.com/rust-embedded/rust-raspberrypi-OS-tutorials

# kernel references:
	https://github.com/xinu-os/xinu/tree/master/system/arch/arm

# enable floating point:
	https://developer.arm.com/documentation/ddi0463/d/Programmers-Model/About-the-programmers-model/Enabling-VFP-support

# pi references
	mailbox docs
	https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface

	some code samples
	https://github.com/raspberrypi/firmware/tree/master/hardfp/opt/vc
	

# stack pointers

	`msr SPSel, #0` Use SP_EL0 for all exception levels
	`msr SPSel, #1` Use SP_ELx for each exception level

## stackpointer selection
	see https://developer.arm.com/documentation/ddi0488/d/programmers-model/armv8-architecture-concepts/stack-pointer-selection

	`ELxt` means "use SP_EL0" (thread)
	`Elxh` means "use SP_ELx" (handler)