#pragma once

#include <drivers/Hardware.hpp>

#include <common/Try.hpp>

struct PciDevice;

namespace driver::system {
	struct Pci final: Hardware {
		DRIVER_INSTANCE(Pci, 0x66d1d64e, "pci", "PCI Bus Controller", Hardware)

		enum struct Class: U8 {
			unclassified,
			massStorageController,
			networkController,
			displayController,
			multimediaController,
			memoryController,
			bridge,
			simpleCommunicationsController,
			baseSystemPeripheral,
			inputDeviceController,
			dockingStation,
			processor,
			serialBusController,
			wirelessController,
			intelligentController,
			satelliteCommunicationController,
			encryptionController,
			signalProcessingController,
			processingAccelerator,
			nonEssentialInstrumentation,
			coprocessor = 0x40
		};

		enum struct Subclass: U16 {
			unclassified,

			// massStorageController
			scsiBusController = 0x0100,
			ideController,
			floppyDiskController,
			ipiBusController,
			raidController,
			ataController,
			serialAtaController,
			serialAttachedScsiController,
			nonVolatileMemoryController,

			// bridge
			hostBridge = 0x0600,
			isaBridge,
			eisaBridge,
			mcaBridge,
			pciToPciBridge,
			pcmciaBridge,
			nubusBridge,
			cardbusBridge,
			racewayBridge,
			pciToPciBridgeTransparent,
			infinibandToPciBridge,

			// simpleCommunicationsController
			serialController = 0x0700,
			parallelController,
			multiportSerialController,
			modem,
			ieee488HalfGpibController,
			smartCardController,

			// baseSystemPeripheral
			interruptController = 0x0800,
			dmaController,
			timer,
			rtcController,
			pciHotPlugController,
			sdHostController,
			iommu,

			// inputDeviceController
			keyboardController = 0x0900,
			digitiserPenController,
			mouseController,
			scannerController,
			gameportController,
		};

		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		auto find_device_by_id(U32) -> PciDevice*;
		auto find_device_by_class(Class) -> PciDevice*;
		auto find_device_by_subclass(Subclass) -> PciDevice*;

		auto find_next_device_by_id(PciDevice *from, U32) -> PciDevice*;
		auto find_next_device_by_class(PciDevice *from, Class) -> PciDevice*;
		auto find_next_device_by_subclass(PciDevice *from, Subclass) -> PciDevice*;
	};
}

inline auto to_string(driver::system::Pci::Class _class) -> const char* {
	switch(_class){
		case driver::system::Pci::Class::unclassified: return "unclassified";
		case driver::system::Pci::Class::massStorageController: return "mass storage controller";
		case driver::system::Pci::Class::networkController: return "network controller";
		case driver::system::Pci::Class::displayController: return "display controller";
		case driver::system::Pci::Class::multimediaController: return "multimedia controller";
		case driver::system::Pci::Class::memoryController: return "memory controller";
		case driver::system::Pci::Class::bridge: return "bridge";
		case driver::system::Pci::Class::simpleCommunicationsController: return "simple communications controller";
		case driver::system::Pci::Class::baseSystemPeripheral: return "base system peripheral";
		case driver::system::Pci::Class::inputDeviceController: return "input device controller";
		case driver::system::Pci::Class::dockingStation: return "docking station";
		case driver::system::Pci::Class::processor: return "processor";
		case driver::system::Pci::Class::serialBusController: return "serial bus controller";
		case driver::system::Pci::Class::wirelessController: return "wireless controller";
		case driver::system::Pci::Class::intelligentController: return "intelligent controller";
		case driver::system::Pci::Class::satelliteCommunicationController: return "satellite communication controller";
		case driver::system::Pci::Class::encryptionController: return "encryption controller";
		case driver::system::Pci::Class::signalProcessingController: return "signal processing controller";
		case driver::system::Pci::Class::processingAccelerator: return "processing accelerator";
		case driver::system::Pci::Class::nonEssentialInstrumentation: return "non-essential instrumentation";
		case driver::system::Pci::Class::coprocessor: return "co-processor";
		default: return nullptr;
	}
}

inline auto to_string(driver::system::Pci::Subclass subclass) -> const char* {
	switch(subclass){
		case driver::system::Pci::Subclass::unclassified: return "unclassified";

		case driver::system::Pci::Subclass::scsiBusController: return "scsi bus controller";
		case driver::system::Pci::Subclass::ideController: return "ide controller";
		case driver::system::Pci::Subclass::floppyDiskController: return "floppy disk controller";
		case driver::system::Pci::Subclass::ipiBusController: return "ipi bus controller";
		case driver::system::Pci::Subclass::raidController: return "raid controller";
		case driver::system::Pci::Subclass::ataController: return "ata controller";
		case driver::system::Pci::Subclass::serialAtaController: return "serial ata controller";
		case driver::system::Pci::Subclass::serialAttachedScsiController: return "serial attached scsi controller";
		case driver::system::Pci::Subclass::nonVolatileMemoryController: return "non-volatile memory controller";

		case driver::system::Pci::Subclass::hostBridge: return "host bridge";
		case driver::system::Pci::Subclass::isaBridge: return "isa bridge";
		case driver::system::Pci::Subclass::eisaBridge: return "eisa bridge";
		case driver::system::Pci::Subclass::mcaBridge: return "mca bridge";
		case driver::system::Pci::Subclass::pciToPciBridge: return "pci-to-pci bridge";
		case driver::system::Pci::Subclass::pcmciaBridge: return "pcmcia bridge";
		case driver::system::Pci::Subclass::nubusBridge: return "nubus bridge";
		case driver::system::Pci::Subclass::cardbusBridge: return "cardbus bridge";
		case driver::system::Pci::Subclass::racewayBridge: return "raceway bridge";
		case driver::system::Pci::Subclass::pciToPciBridgeTransparent: return "transparent pci-to-pci bridge";
		case driver::system::Pci::Subclass::infinibandToPciBridge: return "infiniband-to-pci bridge";

		case driver::system::Pci::Subclass::serialController: return "serial controller";
		case driver::system::Pci::Subclass::parallelController: return "parallel controller";
		case driver::system::Pci::Subclass::multiportSerialController: return "multiport serial controller";
		case driver::system::Pci::Subclass::modem: return "modem";
		case driver::system::Pci::Subclass::ieee488HalfGpibController: return "ieee 488.1/2 (gpib) controller";
		case driver::system::Pci::Subclass::smartCardController: return "smart card controller";

		case driver::system::Pci::Subclass::interruptController: return "interrupt controller";
		case driver::system::Pci::Subclass::dmaController: return "dma controller";
		case driver::system::Pci::Subclass::timer: return "timer";
		case driver::system::Pci::Subclass::rtcController: return "rtc controller";
		case driver::system::Pci::Subclass::pciHotPlugController: return "pci hot-plug controller";
		case driver::system::Pci::Subclass::sdHostController: return "sd host controller";
		case driver::system::Pci::Subclass::iommu: return "iommu";

		case driver::system::Pci::Subclass::keyboardController: return "keyboard controller";
		case driver::system::Pci::Subclass::digitiserPenController: return "digitiser pen controller";
		case driver::system::Pci::Subclass::mouseController: return "mouse controller";
		case driver::system::Pci::Subclass::scannerController: return "scanner controller";
		case driver::system::Pci::Subclass::gameportController: return "gameport controller";

		default: return to_string((driver::system::Pci::Class)((U16)subclass>>8));
	}
}
