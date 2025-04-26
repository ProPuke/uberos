#include "Ide.hpp"

#include "ide/InfoBlock.hpp"

#include <drivers/x86/system/Pci.hpp>

#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/arch/x86/PciDevice.hpp>
#include <kernel/drivers.hpp>

#include <common/ListUnordered.hpp>
#include <common/String.hpp>

namespace driver::storage {
	namespace {
		const auto busCount = 2u;
		const auto driveCount = busCount*2u;

		const U16 legacyIoAta1 = 0x170; // (..0x177) secondary base
		const U16 legacyIoAta2 = 0x1f0; // (..0x1f7) primary base
		const U16 legacyIoAta3 = 0x376; // secondary control
		const U16 legacyIoAta4 = 0x3f6; // primary control

		void pause();

		enum struct Bus: U8 {
			primary,
			secondary,
			max = secondary
		};

		enum struct Drive: U8 {
			master,
			slave,
			max = slave
		};

		enum struct Register: U16 {
			data,             // readwrite
			features,         // write
			error = features, // read
			sectorCount,      // readwrite
			lbaLow,           // readwrite
			lbaMedium,        // readwrite
			lbaHigh,          // readwrite
			driveSelect,      // readwrite
			command,          // write
			status = command, // read
			
			// sectorCount,
			// lba3,
			// lba4,
			// lba5,

			control = 0x206,
			// altStatus = control
		};

		enum struct Command: U8 {
			readPio        = 0x20,
			readPioExt     = 0x24,
			readDma        = 0xc8,
			readDmaExt     = 0x25,
			writePio       = 0x30,
			writePioExt    = 0x34,
			writeDma       = 0xca,
			writeDmaExt    = 0x35,
			cacheFlush     = 0xe7,
			cacheFlushExt  = 0xea,
			packet         = 0xa0,
			identifyPacket = 0xa1,
			identifyDevice = 0xec,
		};

		enum struct Status: U8 {
			error             = 1<<0,
			index             = 1<<1,
			correctedData     = 1<<2,
			dataRequestReady  = 1<<3,
			driveSeekComplete = 1<<4,
			driveWriteFault   = 1<<5,
			driveReady        = 1<<6,
			busy              = 1<<7,
		};

		enum struct ScsiCommand: U8 {
			testUnitReady       = 0x00,
			requestSense        = 0x03,
			format              = 0x04,
			read6               = 0x08,
			write6              = 0x0a,
			inquiry             = 0x12,
			verify6             = 0x13,
			modeSelect6         = 0x15,
			reserve             = 0x16,
			release             = 0x17,
			modeSense6          = 0x1a,
			startStop           = 0x1b,
			receiveDiagnostic   = 0x1c,
			sendDiagnostic      = 0x1d,
			preventAllow        = 0x1e,
			readCapacity        = 0x25,
			read10              = 0x28,
			write10             = 0x2a,
			positionToElement   = 0x2b,
			verify10            = 0x2f,
			synchronizeCache    = 0x35,
			writeBuffer         = 0x3b,
			readBuffer          = 0x3c,
			changeDefinition    = 0x40,
			writeSame10         = 0x41,
			unmap               = 0x42,
			readSubChannel      = 0x42,
			readToc             = 0x43,
			playMsf             = 0x47,
			playAudioTrackIndex = 0x48,	// obsolete, spec missing
			pauseResume         = 0x4b,
			stopPlay            = 0x4e,
			modeSelect10        = 0x55,
			modeSense10         = 0x5a,
			variableLengthCdb   = 0x7f,
			read16              = 0x88,
			write16             = 0x8a,
			verify16            = 0x8f,
			writeSame16         = 0x93,
			serviceActionIn     = 0x9e,
			serviceActionOut    = 0x9f,
			moveMedium          = 0xa5,
			read12              = 0xa8,
			write12             = 0xaa,
			verify12            = 0xaf,
			readElementStatus   = 0xb8,
			scan                = 0xba,
			readCd              = 0xbe,
		};

		enum struct Error: U8 {
			addressMarkNotFound    = 1<<0,
			trackZeroNotFOund      = 1<<1,
			abortedCommand         = 1<<2,
			mediaChangeRequest     = 1<<3,
			idNotFound             = 1<<4,
			mediaChanged           = 1<<5,
			uncorrectableDataError = 1<<6,
			badBlockDetected       = 1<<7
		};

		struct BusMaster {
			arch::x86::IoPort ioPort;
			volatile void *address;

			auto read8(U8 offset) -> U8 {
				if(ioPort){
					return arch::x86::ioPort::read8(ioPort + offset);
				}else{
					return *((U8*)address+offset);
				}
			}

			void write8(U8 offset, U8 value) {
				if(ioPort){
					arch::x86::ioPort::write8(ioPort+offset, value);
				}else{
					*((U8*)address+offset) = value;
				}
			}

			auto read16(U8 offset) -> U16 {
				if(ioPort){
					return arch::x86::ioPort::read16(ioPort + offset);
				}else{
					return *((U16*)((U8*)address+offset));
				}
			}

			void write16(U8 offset, U16 value) {
				if(ioPort){
					arch::x86::ioPort::write16(ioPort+offset, value);
				}else{
					*((U16*)((U8*)address+offset)) = value;
				}
			}

			auto read32(U8 offset) -> U32 {
				if(ioPort){
					return arch::x86::ioPort::read32(ioPort + offset);
				}else{
					return *(U32*)((U8*)address+offset);
				}
			}

			void write32(U8 offset, U32 value) {
				if(ioPort){
					arch::x86::ioPort::write32(ioPort+offset, value);
				}else{
					*(U32*)((U8*)address+offset) = value;
				}
			}

			auto read_command(Bus bus) -> U8 {
				return read8(0x00+(bus==Bus::secondary?0x08:0x00));
			}

			auto write_command(U8 value) {
				write8(0x00, value);
			}

			auto read_status(Bus bus) -> U8 {
				return read8(0x02);
			}

			auto write_status(U8 value) {
				write8(0x02, value);
			}

			auto read_prdt(Bus bus) -> U32 {
				return read8(0x04);
			}

			auto write_prdt(U32 value) {
				write32(0x04, value);
			}
		};

		struct IdeChannel {
			bool isPresent;
			arch::x86::IoPort ioBase;
			arch::x86::IoPort ioControl;
			BusMaster busMaster;

			auto read8(Register reg) {
				return arch::x86::ioPort::read8(ioBase+(arch::x86::IoPort)reg);
			}
			void write8(Register reg, U8 value) {
				return arch::x86::ioPort::write8(ioBase+(arch::x86::IoPort)reg, value);
			}

			auto read16(Register reg) {
				return arch::x86::ioPort::read16(ioBase+(arch::x86::IoPort)reg);
			}
			void write16(Register reg, U16 value) {
				return arch::x86::ioPort::write16(ioBase+(arch::x86::IoPort)reg, value);
			}

			auto read32(Register reg) {
				return arch::x86::ioPort::read32(ioBase+(arch::x86::IoPort)reg);
			}
			void write32(Register reg, U32 value) {
				return arch::x86::ioPort::write32(ioBase+(arch::x86::IoPort)reg, value);
			}

			auto read_control() {
				return arch::x86::ioPort::read8(ioControl);
			}

			void select_drive(Drive drive) {
				//TODO:OPTIMISE: no need to select drive and pause if we're already on it
				write8(Register::driveSelect, 0xa0|((U8)drive<<4));
				pause();
			}

			void command(Command command) {
				write8(Register::command, (U8)command);
			}

			auto wait_for_status(U32 usecs = 400) -> Try<Status> {
				auto start = time::now();
				do{
					const auto status = read8(Register::status);
					if(status&(U8)Status::busy){
						asm("pause");
						if(time::now()-start>usecs) break;
						continue;
					}

					return (Status)status;
				}while(false);

				return Failure{"timeout waiting on status"};
			}

			auto wait_until_data_request_ready(U32 usecs = 1000) -> Try<Status> {
				auto start = time::now();
				do{
					const auto control = read_control();
					if(!(control&(U8)Status::dataRequestReady)){
						asm("pause");
						if(time::now()-start>usecs) break;
						continue;
					}

					return (Status)control;
				}while(false);

				return Failure{"timeout waiting for DQR"};
			}
		};

		IdeChannel ideChannel[(U8)Bus::max+1];

		void pause() {
			// arch::x86::ioPort::wait();
			auto now = time::now();
			while(time::now()<now+400) asm ("pause");
		}

		U32 nextId = 1;

		struct AtaDisk {
			/**/ AtaDisk(IdeChannel &ideChannel, Drive drive, U32 id, const char *name):
				ideChannel(ideChannel),
				drive(drive),
				id(id),
				deviceName(name)
			{}

			IdeChannel &ideChannel;
			Drive drive;
			U32 id;
			String8 deviceName;
			String8 model;
			String8 serialNumber;
			U64 size = 0;

			virtual auto eject() -> Try<bool> { return Failure{"not supported"}; }
			virtual auto is_present() -> Try<bool> { return false; }
			virtual auto is_removable() -> Try<bool> { return false; }
		};

		struct AtapiDisk: AtaDisk {
			typedef AtaDisk Super;

			bool isRemovable = false;

			/**/ AtapiDisk(IdeChannel &ideChannel, Drive drive, U32 id, const char *name):
				Super(ideChannel, drive, id, name)
			{}

			auto eject() -> Try<bool> override {
				auto status = TRY_RESULT(send_command(ScsiCommand::startStop, 0, 2, 0, 0, 0));

				if((U8)status&((U8)Status::error|(U8)Status::driveWriteFault)){
					return false;
				}

				if(!((U8)status&(U8)Status::dataRequestReady)){
					return false;
				}

				return true;
			}

			auto is_present() -> Try<bool> override {
				auto status = TRY_RESULT(send_command(ScsiCommand::testUnitReady, 0, 0, 0, 0, 0));

				if((U8)status&(U8)Status::error){
					auto sense = ideChannel.read8(Register::error);
					if(sense&(U8)Error::mediaChanged){
						return false;
					}else{
						return Failure{"Unknown error reading drive"};
					}
				}else{
					return true;
				}
			}

			auto is_removable() -> Try<bool> override {
				return isRemovable;
			}

			auto send_command(ScsiCommand command, U16 p1, U16 p2, U16 p3, U16 p4, U16 p5) -> Try<Status> {
				U16 data[5] = {p1, p2, p3, p4, p5};
				return send_command(command, data);
			}

			auto send_command(ScsiCommand command, U16 data[5]) -> Try<Status> {
				ideChannel.select_drive(drive);

				ideChannel.command(Command::packet);
				while(!(ideChannel.read_control()&(1<<3)));

				TRY(ideChannel.wait_until_data_request_ready());

				ideChannel.write16(Register::data, (U16)command);
				for(auto i=0;i<5;i++){
					ideChannel.write16(Register::data, data[i]);
				}

				return TRY_RESULT(ideChannel.wait_for_status());
			}
		};

		ListUnordered<AtaDisk*> disks;

		auto get_disk_by_id(U32 id) -> AtaDisk* {
			for(auto disk:disks){
				if(disk->id==id) return disk;
			}

			return nullptr;
		}

		Lock<LockType::flat> lock;
	}

	auto Ide::_on_start() -> Try<> {
		Lock_Guard guard(lock);

		auto pci = drivers::find_and_activate<driver::system::Pci>(this);
		if(!pci) return Failure{"PCI unavailable"};

		auto pciDevice = pci->find_device_by_subclass(system::Pci::Subclass::ideController);
		if(!pciDevice) return Failure{"device not present"};

		TRY(api.subscribe_pci(*pciDevice, {.busMastering=true, .interrupts=true}));

		struct ProgIf {
			bool primaryIsPciNativeMode:1;
			bool primarySupportsPciNativeSwitching:1;
			bool secondaryIsPciNativeMode:1;
			bool secondarySupportsPciNativeSwitching:1;
			U8:3;
			bool isBusMaster:1; // (supports DMA)
		};

		auto progIf = *(ProgIf*)(&pciDevice->progIf);

		// log.print_info("primary is native = ", progIf.primaryIsPciNativeMode);
		// log.print_info("primary supports switching = ", progIf.primarySupportsPciNativeSwitching);
		// log.print_info("secondary is native = ", progIf.secondaryIsPciNativeMode);
		// log.print_info("secondary supports switching = ", progIf.secondarySupportsPciNativeSwitching);
		// log.print_info("is bus master = ", progIf.isBusMaster);

		ideChannel[(U8)Bus::primary].isPresent = false;
		ideChannel[(U8)Bus::primary].busMaster.ioPort = 0;
		ideChannel[(U8)Bus::primary].busMaster.address = nullptr;
		ideChannel[(U8)Bus::secondary].isPresent = false;
		ideChannel[(U8)Bus::secondary].busMaster.ioPort = 0;
		ideChannel[(U8)Bus::secondary].busMaster.address = nullptr;

		if(!progIf.isBusMaster) return Failure{"bus mastering (DMA) is not supported for this ide controller"};

		{ // retrieve bus master
			if(pciDevice->bar[4].ioPort){
				ideChannel[(U8)Bus::primary].busMaster.ioPort = TRY_RESULT(api.subscribe_ioPort(pciDevice->bar[4].ioPort));
				ideChannel[(U8)Bus::secondary].busMaster.ioPort = TRY_RESULT(api.subscribe_ioPort(ideChannel[(U8)Bus::primary].busMaster.ioPort + 8));

			}else if(pciDevice->bar[4].memoryAddress){
				ideChannel[(U8)Bus::primary].busMaster.address = TRY_RESULT(api.subscribe_memory(pciDevice->bar[4].memoryAddress.as_native(), (size_t)pciDevice->bar[4].memorySize, {}));
				ideChannel[(U8)Bus::secondary].busMaster.address = (U8*)ideChannel[(U8)Bus::primary].busMaster.address + 8;

			}else{
				return Failure{"PCI device misconfigured (missing BAR 4)"};
			}
		}

		{ // retrieve base and control ports
			if(progIf.primaryIsPciNativeMode&&pciDevice->bar[0].ioPort&&pciDevice->bar[1].ioPort){
				ideChannel[(U8)Bus::primary].ioBase = TRY_RESULT(api.subscribe_ioPort(pciDevice->bar[0].ioPort));
				ideChannel[(U8)Bus::primary].ioControl = TRY_RESULT(api.subscribe_ioPort(pciDevice->bar[1].ioPort));
			}else{
				ideChannel[(U8)Bus::primary].ioBase = TRY_RESULT(api.subscribe_ioPort(legacyIoAta2));
				ideChannel[(U8)Bus::primary].ioControl = TRY_RESULT(api.subscribe_ioPort(legacyIoAta4));
			}

			if(progIf.secondaryIsPciNativeMode&&pciDevice->bar[2].ioPort&&pciDevice->bar[3].ioPort){
				ideChannel[(U8)Bus::secondary].ioBase = TRY_RESULT(api.subscribe_ioPort(pciDevice->bar[2].ioPort));
				ideChannel[(U8)Bus::secondary].ioControl = TRY_RESULT(api.subscribe_ioPort(pciDevice->bar[3].ioPort));
			}else{
				ideChannel[(U8)Bus::secondary].ioBase = TRY_RESULT(api.subscribe_ioPort(legacyIoAta1));
				ideChannel[(U8)Bus::secondary].ioControl = TRY_RESULT(api.subscribe_ioPort(legacyIoAta3));
			}
		}

		for(auto bus=Bus::primary; bus<=Bus::max; bus=(Bus)((U8)bus+1)){
			auto &channel = ideChannel[(U8)bus];

			// does the bus appear to be present?
			if(channel.read8(Register::command)==0xff) continue;

			// are the lba registers writable?
			channel.write8(Register::lbaLow, 0xab);
			channel.write8(Register::lbaMedium, 0xcd);
			channel.write8(Register::lbaHigh, 0xef);
			if(channel.read8(Register::lbaLow)!=0xab) continue;
			if(channel.read8(Register::lbaMedium)!=0xcd) continue;
			if(channel.read8(Register::lbaHigh)!=0xef) continue;

			// clear command register
			channel.write8(Register::control, 1);

			channel.isPresent = true;
			// log.print_debug("bus ", (U32)bus+1, " found");

			for(auto drive=Drive::master; drive<=Drive::max; drive=(Drive)((U8)drive+1)){
				channel.select_drive(drive);

				channel.write8(Register::lbaLow, 0x00);
				channel.write8(Register::lbaMedium, 0x00);
				channel.write8(Register::lbaHigh, 0x00);

				channel.command(Command::identifyDevice);
				pause();

				bool isAtapi = false;

				if(auto status = channel.wait_for_status()){
					if((U8)status.result&(U8)Status::error){
						// error, device is not ATA (so identify it..)

						auto signature = channel.read8(Register::lbaMedium)|(U16)channel.read8(Register::lbaHigh)<<8;

						if(signature==0xeb14){
							isAtapi = true;
		
						}else if(signature==0x0000){
							isAtapi = false;
		
						}else{
							log.print_warning("pata", (C8)('1'+(U8)bus), (C8)('a'+(U8)drive), " - Unrecognised device type: ", format::Hex16(signature));
							continue;
						}

						channel.command(Command::identifyPacket);
						pause();

					}else{
						if(!((U8)status.result&(U8)Status::dataRequestReady)) continue;
					}

				}else{
					continue; // timeout
				}

				if(auto status = channel.wait_for_status()){
					if((U8)status.result&((U8)Status::error|(U8)Status::driveWriteFault)) continue; // something went wrong

				}else{
					continue; // timeout
				}

				Infoblock infoBlock;
				for(auto i=0u;i<(sizeof(infoBlock)+1)/2;i++){
					((U16*)&infoBlock)[i] = channel.read16(Register::data);
				}

				// serialNumber[20];
				// firmwareRevision[8];
				// modelNumber[40];

				AtaDisk *disk;
				const char *protocol;

				if(isAtapi){
					auto atapiDisk = new AtapiDisk{channel, drive, nextId++, "pata1a"};
					disk = atapiDisk;
					disk->deviceName[4] += (U8)bus;
					disk->deviceName[5] += (U8)drive;
					atapiDisk->isRemovable = !!infoBlock.atapi.removableMediaDevice;
					protocol = "ATAPI";

				}else{
					disk = new AtaDisk{channel, drive, nextId++, "pata1a"};
					disk->deviceName[4] += (U8)bus;
					disk->deviceName[5] += (U8)drive;
					protocol = "ATA";
				}

				disk->model = infoBlock.get_modelNumber();
				disk->serialNumber = infoBlock.get_serialNumber();

				bool _48bitSectorCount;
				auto sectorCount = infoBlock.get_sector_count(_48bitSectorCount);

				disk->size = infoBlock.get_sector_size()*sectorCount;

				{
					auto section = log.section(disk->deviceName.get_data(), " (", protocol, ')');

					log.print_info("model: ", disk->model.get_data());
					log.print_info("serialNumber: ", disk->serialNumber.get_data());
					if(disk->size>0){
						log.print_info("size: ", disk->size/1024/1024, "MiB");
					}
					if(TRY_RESULT(disk->is_removable())){
						log.print_info("removable media");
						if(auto present = disk->is_present()){
							log.print_info(present.result?"media present":"media not present");
						}else{
							log.print_info("error querying media");
						}
					}
				}

				disks.push(disk);
			}
		}

		return {};
	}

	auto Ide::_on_stop() -> Try<> {
		Lock_Guard guard(lock);

		return {};
	}

	auto Ide::get_drive_count() -> U32 {
		Lock_Guard guard(lock);

		return disks.length;
	}

	auto Ide::get_drive_id(U32 index) -> U32 {
		Lock_Guard guard(lock);

		if(index>=disks.length) return 0;

		return disks[index]->id;
	}

	auto Ide::does_drive_exist(U32 id) -> bool {
		Lock_Guard guard(lock);
		
		auto disk = get_disk_by_id(id);
		return !!disk;
	}
	
	auto Ide::get_drive_device(U32 id) -> Try<const char*> {
		Lock_Guard guard(lock);
		
		auto disk = get_disk_by_id(id);
		if(!disk) return Failure{"drive not present"};
		
		return disk->deviceName.get_data();
	}
	
	auto Ide::get_drive_model(U32 id) -> Try<const char*> {
		Lock_Guard guard(lock);
		
		auto disk = get_disk_by_id(id);
		if(!disk) return Failure{"drive not present"};
		
		return disk->model.get_data();
	}
	
	auto Ide::get_drive_serialNumber(U32 id) -> Try<const char*> {
		Lock_Guard guard(lock);
		
		auto disk = get_disk_by_id(id);
		if(!disk) return Failure{"drive not present"};
		
		return disk->serialNumber.get_data();
	}
	
	auto Ide::get_drive_size(U32 id) -> Try<U64> {
		Lock_Guard guard(lock);

		auto disk = get_disk_by_id(id);
		if(!disk) return Failure{"drive not present"};

		return disk->size;
	}

	auto Ide::is_drive_present(U32 id) -> Try<bool> {
		Lock_Guard guard(lock);

		auto disk = get_disk_by_id(id);
		if(!disk) return Failure{"drive not present"};

		return disk->is_present();
	}

	auto Ide::is_drive_removable(U32 id) -> Try<bool> {
		Lock_Guard guard(lock);

		auto disk = get_disk_by_id(id);
		if(!disk) return Failure{"drive not present"};

		return disk->is_removable();
	}

	auto Ide::eject_drive(U32 id) -> Try<bool> {
		Lock_Guard guard(lock);

		auto disk = get_disk_by_id(id);
		if(!disk) return Failure{"drive not present"};

		return disk->eject();
	}
}
