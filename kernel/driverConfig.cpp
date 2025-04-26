#include <drivers/CpuScheduler.hpp>
#include <drivers/DesktopManager.hpp>
#include <drivers/DisplayManager.hpp>
#include <drivers/ThemeManager.hpp>

#ifdef ARCH_UEFI
#include <drivers/uefi/console/UefiConsole.hpp>
#endif

#ifdef ARCH_X86
#include <drivers/x86/clock/CmosRtc.hpp>
#include <drivers/x86/graphics/BochsVga.hpp>
#include <drivers/x86/graphics/MultibootFramebuffer.hpp>
#include <drivers/x86/graphics/Vbe.hpp>
#include <drivers/x86/input/Ps2Keyboard.hpp>
#include <drivers/x86/input/Ps2Mouse.hpp>
#include <drivers/x86/interrupt/Pic8259.hpp>
#include <drivers/x86/processor/X86.hpp>
#include <drivers/x86/system/Acpi.hpp>
#include <drivers/x86/system/Apic.hpp>
#include <drivers/x86/system/Gdt.hpp>
#include <drivers/x86/system/IbmBios.hpp>
#include <drivers/x86/system/Idt.hpp>
#include <drivers/x86/system/Pci.hpp>
#include <drivers/x86/system/Ps2.hpp>
#include <drivers/x86/system/Smbios.hpp>
#include <drivers/x86/textmode/VgaTextmode.hpp>
#include <drivers/x86/timer/Hpet.hpp>
#endif

#ifdef ARCH_HOSTED
#include <drivers/hosted/serial/Stdout.hpp>
#endif

#ifdef ARCH_RASPI
#include <drivers/raspi/graphics/Raspi_videocore_mailbox.hpp>
#include <drivers/raspi/interrupt/Arm_raspi_legacy.hpp>
#include <drivers/raspi/processor/Raspi_bcm2711.hpp>
#include <drivers/raspi/processor/Raspi_bcm2835.hpp>
#include <drivers/raspi/processor/Raspi_bcm2836.hpp>
#include <drivers/raspi/processor/Raspi_bcm2837.hpp>
#endif

#define DRIVER(PATH, STARTUP) PATH PATH::instance(DriverApi::Startup::STARTUP)

namespace driver {
	DRIVER(           CpuScheduler  , onDemand);
	DRIVER(           DesktopManager, onDemand);
	DRIVER(           DisplayManager, onDemand);
	DRIVER(           ThemeManager  , onDemand);

	#ifdef ARCH_UEFI
	DRIVER(console  ::UefiConsole   , automatic);
	#endif

	#ifdef ARCH_X86
	DRIVER(clock    ::CmosRtc             , onDemand);
	DRIVER(graphics ::MultibootFramebuffer, onDemand);
	DRIVER(graphics ::BochsVga            , onDemand);
	// DRIVER(graphics ::Vbe                 , onDemand);
	DRIVER(input    ::Ps2Keyboard         , automatic);
	DRIVER(input    ::Ps2Mouse            , onDemand);
	DRIVER(interrupt::Pic8259             , onDemand);
	DRIVER(processor::X86                 , onDemand);
	DRIVER(system   ::Acpi                , onDemand);
	DRIVER(system   ::Apic                , automatic);
	DRIVER(system   ::Gdt                 , onDemand);
	DRIVER(system   ::IbmBios             , automatic);
	DRIVER(system   ::Idt                 , onDemand);
	DRIVER(system   ::Pci                 , onDemand);
	DRIVER(system   ::Ps2                 , onDemand);
	DRIVER(system   ::Smbios              , onDemand);
	DRIVER(textmode ::VgaTextmode         , onDemand);
	DRIVER(timer    ::Hpet                , automatic);
	#endif

	#ifdef ARCH_HOSTED
	DRIVER(serial   ::Stdout              , automatic);
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
