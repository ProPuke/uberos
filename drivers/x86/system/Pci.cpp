#include "Pci.hpp"

#include <drivers/x86/system/Smbios.hpp>

#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/assert.hpp>
#include <kernel/drivers.hpp>

#include <common/format.hpp>
#include <common/PodArray.hpp>

namespace driver::system {
	namespace {
		const U16 ioConfig = 0x0cf8;
		const U16 ioData = 0x0cfc;

		auto readConfig8(U8 bus, U8 slot, U8 function, UPtr offset) -> U8 {
			assert((offset&3) == 0); // must be 4-byte aligned
			arch::x86::ioPort::write32(ioConfig, (U32)(0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | offset));
			return arch::x86::ioPort::read8(ioData);
		}
		auto readConfig16(U8 bus, U8 slot, U8 function, UPtr offset) -> U16 {
			assert((offset&3) == 0); // must be 4-byte aligned
			arch::x86::ioPort::write32(ioConfig, (U32)(0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | offset));
			return arch::x86::ioPort::read16(ioData);
		}
		auto readConfig32(U8 bus, U8 slot, U8 function, UPtr offset) -> U32 {
			assert((offset&3) == 0); // must be 4-byte aligned
			arch::x86::ioPort::write32(ioConfig, (U32)(0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | offset));
			return arch::x86::ioPort::read32(ioData);
		}
		void writeConfig8(U8 bus, U8 slot, U8 function, UPtr offset, U8 value) {
			assert((offset&3) == 0); // must be 4-byte aligned
			arch::x86::ioPort::write32(ioConfig, (U32)(0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | offset));
			arch::x86::ioPort::write8(ioData, value);
		}
		void writeConfig16(U8 bus, U8 slot, U8 function, UPtr offset, U16 value) {
			assert((offset&3) == 0); // must be 4-byte aligned
			arch::x86::ioPort::write32(ioConfig, (U32)(0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | offset));
			arch::x86::ioPort::write16(ioData, value);
		}
		void writeConfig32(U8 bus, U8 slot, U8 function, UPtr offset, U32 value) {
			assert((offset&3) == 0); // must be 4-byte aligned
			arch::x86::ioPort::write32(ioConfig, (U32)(0x80000000 | (bus << 16) | (slot << 11) | (function << 8) | offset));
			arch::x86::ioPort::write32(ioData, value);
		}

		PodArray<PciDevice> devices;
		Bitmask256 scannedBusses;

		auto classCode_to_string(U8 _class) -> const char* {
			switch(_class){
				case 0x00: return "unclassified";
				case 0x01: return "mass storage controller";
				case 0x02: return "network controller";
				case 0x03: return "display controller";
				case 0x04: return "multimedia controller";
				case 0x05: return "memory controller";
				case 0x06: return "bridge";
				case 0x07: return "simple communications controller";
				case 0x08: return "base system peripheral";
				case 0x09: return "input device controller";
				case 0x0a: return "docking station";
				case 0x0b: return "processor";
				case 0x0c: return "serial bus controller";
				case 0x0d: return "wireless controller";
				case 0x0e: return "intelligent controller";
				case 0x0f: return "satellite communication controller";
				case 0x10: return "encryption controller";
				case 0x11: return "signal processing controller";
				case 0x12: return "processing accelerator";
				case 0x13: return "non-essential instrumentation";
				case 0x14 ... 0x3f: return nullptr; // reserved
				case 0x40: return "co-processor";
				case 0x41 ... 0xfe: return nullptr; // reserved
				case 0xff: return nullptr; // vendor specific
			}
		}

		auto classCode_to_string(U8 _class, U8 device) -> const char* {
			switch(_class){
				case 0x00: return "unclassified";
				case 0x01: switch(device) {
					case 0x00: return "scsi bus controller";
					case 0x01: return "ide controller";
					case 0x02: return "floppy disk controller";
					case 0x03: return "ipi bus controller";
					case 0x04: return "raid controller";
					case 0x05: return "ata controller";
					case 0x06: return "serial ata controller";
					case 0x07: return "serial attached scsi controller";
					case 0x08: return "non-volatile memory controller";
					default: return "mass storage controller"; // unknown
				}
				case 0x02: return "network controller";
				case 0x03: return "display controller";
				case 0x04: return "multimedia controller";
				case 0x05: return "memory controller";
				case 0x06: switch(device) {
					case 0x00: return "host bridge";
					case 0x01: return "isa bridge";
					case 0x02: return "eisa bridge";
					case 0x03: return "mca bridge";
					case 0x04: return "pci-to-pci bridge";
					case 0x05: return "pcmcia bridge";
					case 0x06: return "nubus bridge";
					case 0x07: return "cardbus bridge";
					case 0x08: return "raceway bridge";
					case 0x09: return "pci-to-pci bridge";
					case 0x0a: return "infiniband-to-pci bridge";
					default: return "bridge"; // unknown
				}
				case 0x07: switch(device) {
					case 0x00: return "serial controller";
					case 0x01: return "parallel controller";
					case 0x02: return "multiport serial controller";
					case 0x03: return "modem";
					case 0x04: return "ieee 488.1/2 (gpib) controller";
					case 0x05: return "smart card controller";
					default: return "simple communications controller"; // unknown
				}
				case 0x08: switch(device) {
					case 0x00: return "interrupt controller";
					case 0x01: return "dma controller";
					case 0x02: return "timer";
					case 0x03: return "rtc controller";
					case 0x04: return "pci hot-plug controller";
					case 0x05: return "sd host controller";
					case 0x06: return "iommu";
					default: return "base system peripheral"; // unknown
				}
				case 0x09: switch(device) {
					case 0x00: return "keyboard controller";
					case 0x01: return "digitiser pen controller";
					case 0x02: return "mouse controller";
					case 0x03: return "scanner controller";
					case 0x04: return "gameport controller";
					default: return "input device controller";
				}
				case 0x0a: return "docking station";
				case 0x0b: return "processor";
				case 0x0c: return "serial bus controller";
				case 0x0d: return "wireless controller";
				case 0x0e: return "intelligent controller";
				case 0x0f: return "satellite communication controller";
				case 0x10: return "encryption controller";
				case 0x11: return "signal processing controller";
				case 0x12: return "processing accelerator";
				case 0x13: return "non-essential instrumentation";
				case 0x14 ... 0x3f: return nullptr; // reserved
				case 0x40: return "co-processor";
				case 0x41 ... 0xfe: return nullptr; // reserved
				case 0xff: return nullptr; // vendor specific
			}
		}

		void scan_function(U8 bus, U8 device, U8 function);

		void scan_bus(unsigned bus) {
			scannedBusses.set(bus, true);

			for(auto device=0; device<32; device++){
				auto id = readConfig32(bus, device, 0, 0x00);
				U16 vendorId = id&0xffff;
				// U16 deviceId = id>>16;

				if(vendorId == 0xffff) continue;

				auto headerType = readConfig32(bus, device, 0, 0x0c)>>16 & 0xff;
				auto isMultiHost = headerType & 0x80;

				if(isMultiHost) {
					for(auto function=0;function<8;function++){
						scan_function(bus, device, function);
					}
				} else {
					scan_function(bus, device, 0);
				}
			}
		}

		void scan_function(U8 bus, U8 device, U8 function) {
			auto id = readConfig32(bus, device, function, 0x00);
			U16 vendorId = id&0xffff;
			// U16 deviceId = id>>16;

			if(vendorId == 0xffff) return;

			auto _class = readConfig32(bus, device, function, 0x08);
			// auto interruptInformation = readConfig32(bus, device, function, 0x3C);

			U8 classCode = (_class >> 24) & 0xff;
			U8 subclassCode = (_class >> 16) & 0xff;
			// U8 progIF = (_class >> 8) & 0xff;

			if(classCode==0x06&&subclassCode==0x04) { // pci-to-pci bridge
				U8 secondaryBus = readConfig32(bus, device, 0, 0x18)>>8 & 0xff;
				if(!scannedBusses.get(secondaryBus)){
					scan_bus(secondaryBus);
				}
			}

			// auto interruptPin = (interruptInformation >> 8) & 0xff;
			// auto interruptLine = (interruptInformation >> 0) & 0xff;

			// auto deviceID = readConfig32(bus, device, function, 0x00);
			// auto subsystemID = readConfig32(bus, device, function, 0x2C);

			auto classCodeString = classCode_to_string(classCode, subclassCode);

			if(classCodeString){
				Pci::instance.log.print_info("detected ", format::Hex8{bus, false}, ':', format::Hex8{device, false}, '.', function, ' ', classCodeString, " (", format::Hex32{_class}, ") ", format::Hex32{id});
			}else{
				Pci::instance.log.print_info("detected ", format::Hex8{bus, false}, ':', format::Hex8{device, false}, '.', function, " unknown device (", format::Hex32{_class}, ") ", format::Hex32{id});
			}

			auto &instance = devices.push_back((PciDevice){
				.id = id,
				._class = _class,
				.bus = bus,
				.device = device,
				.function = function
			});

			//TODO: set initial pci device options?

			for(auto i=0;i<6;i++){
				auto &bar = instance.bar[i];

				// disable these initially (also these need to be off when querying size, as some can respond to that)
				instance.enable_memory_space(false);
				instance.enable_io_space(false);

				const auto address = instance.readConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4);
				const auto isIoPort = address&0b1;

				if(isIoPort){
					bar.memoryAddress.address = 0;
					bar.memorySize = 0;
					bar.ioAddress = address & 0xfffffffc;

				}else{
					bar.ioAddress = 0;
					const auto addressType = (address&0b110)>>1;

					switch(addressType){
						case 0b00: // a 32bit address
						case 0b11: // unknown? also treat as 32bit for now?
							bar.size = PciDevice::Bar::BarSize::_32bit;
							bar.memoryAddress.address = address & 0xfffffff0;

							instance.writeConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4, 0xffffffff);
							bar.memorySize = ~(instance.readConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4) & 0xfffffff0 | 0xffffffff00000000) + 1;
							instance.writeConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4, address);
						break;
						case 0b10: { // a 64bit address
							const auto address2 = instance.readConfig32((UPtr)PciDevice::RegisterOffset::bar0+(i+1)*4);

							bar.size = PciDevice::Bar::BarSize::_64bit;
							bar.memoryAddress.address = address & 0xfffffff0 | (U64)address2<<32;

							instance.writeConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4, 0xffffffff);
							instance.writeConfig32((UPtr)PciDevice::RegisterOffset::bar0+(i+1)*4, 0xffffffff);
							bar.memorySize = ~(instance.readConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4) & 0xfffffff0 | (U64)address2<<32) + 1;
							instance.writeConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4, address);
							instance.writeConfig32((UPtr)PciDevice::RegisterOffset::bar0+(i+1)*4, address2);
						} break;
						case 0b01: // reserved/16bit address?
							bar.size = PciDevice::Bar::BarSize::_16bit;
							bar.memoryAddress.address = address & 0xfff0;

							instance.writeConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4, 0xffffffff);
							bar.memorySize = ~(instance.readConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4) & 0xfff0 | 0xffffffffffff0000) + 1;
							instance.writeConfig32((UPtr)PciDevice::RegisterOffset::bar0+i*4, address);
						break;
					}

					if(bar.memorySize>0 && bar.memoryAddress){
						switch(bar.size){
							case PciDevice::Bar::BarSize::_16bit:
								Pci::instance.log.print_info("  BAR ", i, ": ", format::Hex16{(U16)bar.memoryAddress.address}, " - ", format::Hex16{(U16)(bar.memoryAddress.address + bar.memorySize - 1)});
							break;
							case PciDevice::Bar::BarSize::_32bit:
								Pci::instance.log.print_info("  BAR ", i, ": ", format::Hex32{(U32)bar.memoryAddress.address}, " - ", format::Hex32{(U32)(bar.memoryAddress.address + bar.memorySize - 1)});
							break;
							case PciDevice::Bar::BarSize::_64bit:
								Pci::instance.log.print_info("  BAR ", i, ": ", format::Hex64{(U64)bar.memoryAddress.address}, " - ", format::Hex64{(U64)(bar.memoryAddress.address + bar.memorySize - 1)});
							break;
						}
					}
				}

			}
		}
	}

	auto Pci::_on_start() -> Try<> {
		auto smbios = drivers::find_and_activate<system::Smbios>(this);
		if(smbios){
			if(smbios->is_pci_supported()==Maybe::no) return {"PCI not supported"};
		}

		TRY(api.subscribe_ioPort(ioConfig));
		TRY(api.subscribe_ioPort(ioData));

		devices.clear();
		scannedBusses.clear();

		auto baseHeaderType = readConfig32(0, 0, 0, 0x0C);
		auto isMultiHost = baseHeaderType & 0x80;

		if(isMultiHost) {
			for(auto function=0;function<8;function++){
				auto id = readConfig32(0, 0, function, 0x00);
				U16 vendorId = id&0xffff;
				// U16 deviceId = id>>16;

				if(vendorId == 0xffff) continue;

				if(!scannedBusses.get(function)){
					scan_bus(function);
				}
			}

		} else {
			scan_bus(0);
		}

		return {};
	}

	auto Pci::_on_stop() -> Try<> {
		return {};
	}

	auto Pci::find_device_by_id(U32 id) -> PciDevice* {
		for(auto &device:devices) {
			if(device.id==id) return &device;
		}

		return nullptr;
	}
}

auto PciDevice::readConfig8(UPtr offset) -> U8 {
	return driver::system::readConfig8(bus, device, function, offset);
}
auto PciDevice::readConfig16(UPtr offset) -> U16 {
	return driver::system::readConfig16(bus, device, function, offset);
}
auto PciDevice::readConfig32(UPtr offset) -> U32 {
	return driver::system::readConfig32(bus, device, function, offset);
}
void PciDevice::writeConfig8(UPtr offset, U8 value) {
	return driver::system::writeConfig8(bus, device, function, offset, value);
}
void PciDevice::writeConfig16(UPtr offset, U16 value) {
	return driver::system::writeConfig16(bus, device, function, offset, value);
}
void PciDevice::writeConfig32(UPtr offset, U32 value) {
	return driver::system::writeConfig32(bus, device, function, offset, value);
}
