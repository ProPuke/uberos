#include <kernel/drivers/common/system/CpuScheduler.hpp>
#include <kernel/drivers/common/system/DesktopManager.hpp>
#include <kernel/drivers/common/system/DisplayManager.hpp>

#ifdef ARCH_UEFI
#include <kernel/drivers/uefi/console/UefiConsole.hpp>
#endif

#ifdef ARCH_X86
#include <kernel/drivers/x86/graphics/BochsVga.hpp>
#include <kernel/drivers/x86/graphics/Vbe.hpp>
#include <kernel/drivers/x86/input/Ps2Keyboard.hpp>
#include <kernel/drivers/x86/input/Ps2Mouse.hpp>
#include <kernel/drivers/x86/interrupt/Pic8259.hpp>
#include <kernel/drivers/x86/processor/X86.hpp>
#include <kernel/drivers/x86/system/Acpi.hpp>
#include <kernel/drivers/x86/system/Apic.hpp>
#include <kernel/drivers/x86/system/Gdt.hpp>
#include <kernel/drivers/x86/system/IbmBios.hpp>
#include <kernel/drivers/x86/system/Idt.hpp>
#include <kernel/drivers/x86/system/Pci.hpp>
#include <kernel/drivers/x86/system/Ps2.hpp>
#include <kernel/drivers/x86/system/Smbios.hpp>
#include <kernel/drivers/x86/textmode/VgaTextmode.hpp>
#include <kernel/drivers/x86/timer/Hpet.hpp>
#endif

#ifdef ARCH_HOSTED
#include <kernel/drivers/hosted/serial/Stdout.hpp>
#endif

#ifdef ARCH_RASPI
#include <kernel/drivers/raspi/graphics/Raspi_videocore_mailbox.hpp>
#include <kernel/drivers/raspi/interrupt/Arm_raspi_legacy.hpp>
#include <kernel/drivers/raspi/processor/Raspi_bcm2711.hpp>
#include <kernel/drivers/raspi/processor/Raspi_bcm2835.hpp>
#include <kernel/drivers/raspi/processor/Raspi_bcm2836.hpp>
#include <kernel/drivers/raspi/processor/Raspi_bcm2837.hpp>
#endif

#define DRIVER(PATH, STARTUP) PATH PATH::instance(DriverApi::Startup::STARTUP)

namespace driver {
	DRIVER(system   ::CpuScheduler  , onDemand);
	DRIVER(system   ::DesktopManager, onDemand);
	DRIVER(system   ::DisplayManager, onDemand);

	#ifdef ARCH_UEFI
	DRIVER(console  ::UefiConsole   , automatic);
	#endif

	#ifdef ARCH_X86
	DRIVER(graphics ::BochsVga   , onDemand);
	// DRIVER(graphics ::Vbe        , onDemand);
	DRIVER(input    ::Ps2Keyboard, automatic);
	DRIVER(input    ::Ps2Mouse   , onDemand);
	DRIVER(interrupt::Pic8259    , onDemand);
	DRIVER(processor::X86        , onDemand);
	DRIVER(system   ::Acpi       , onDemand);
	DRIVER(system   ::Apic       , automatic);
	DRIVER(system   ::Gdt        , onDemand);
	DRIVER(system   ::IbmBios    , automatic);
	DRIVER(system   ::Idt        , onDemand);
	DRIVER(system   ::Pci        , onDemand);
	DRIVER(system   ::Ps2        , onDemand);
	DRIVER(system   ::Smbios     , onDemand);
	DRIVER(textmode ::VgaTextmode, onDemand);
	DRIVER(timer    ::Hpet       , automatic);
	#endif

	#ifdef ARCH_HOSTED
	DRIVER(serial   ::Stdout     , automatic);
	#endif

	#ifdef ARCH_RASPI
	DRIVER(graphics ::Raspi_videocore_mailbox, onDemand);
	DRIVER(interrupt::Arm_raspi_legacy       , automatic);
	DRIVER(processor::Raspi_bcm2711          , automatic);
	DRIVER(processor::Raspi_bcm2835          , automatic);
	DRIVER(processor::Raspi_bcm2836          , automatic);
	DRIVER(processor::Raspi_bcm2837          , automatic);
	#endif
}
