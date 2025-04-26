#include "Smbios.hpp"

namespace driver::system {
	namespace {
		struct __attribute__((packed)) EntryPointVersion {
			U8 majorVersion;
			U8 minorVersion;
		};

		struct __attribute__((packed)) EntryPointStructure {
			U8 checksum;
			U8 length;
			EntryPointVersion version;
		};

		struct __attribute__((packed)) EntryPointStructure21: EntryPointStructure {
			// C8 anchorString[4]; // comes before
			U16 structureMaxSize;
			U8 revision;
			U8 formattedArea[5];
			U8 intermediateAnchorString[5];
			U8 intermediateChecksum;
			U16 structTableLength;
			U32 structTableAddress;
			U16 structureCount;
			U8 bcdRevision;
		};

		struct __attribute__((packed)) EntryPointStructure30: EntryPointStructure {
			// C8 anchorString[5]; // comes before
			U8 docrevl;
			U8 revision;
			U8 _reserved;
			U32 structTableMaxSize;
			U64 structTableAddress;
		};

		enum struct StructureTableType: U8 {
			firmwareInfo = 0,
			systemInfo = 1,
			systemEnclosure = 3,
			processorInformation = 4,
			cacheInformation = 7,
			systemSlots = 9,
			physicalMemoryArea = 16,
			memoryDevice = 17,
			memoryArrayMappedDevice = 19,
			systemBootInformation = 32,
			inactive = 126,
			endOfTable = 127
		};

		typedef U8 String;

		struct __attribute__((packed)) StructureTable {
			StructureTableType type;
			U8 length;
			U16 handle;

			auto get_string(String i) -> const char* {
				for(auto addr=(C8*)this+length;i--;){
					if(i==0) return addr;

					addr+=strlen(addr);
					addr++;
				}

				return "";
			}
		};

		struct __attribute__((packed)) FirmwareInformation: StructureTable {
			// 2.0+
			String vendor;
			String firmwareVersion;
			U16 biosStartingAddress;
			String firmwareReleaseDate;
			U8 firmwareRomSize; // size = (firmwareRomSize+1) * 64k

			union {
				struct __attribute__((packed)) {
					bool _reserved0:1;
					bool _reserved1:1;
					bool _unknown2:1;
					bool characteristicsNotSupported:1;
					bool isaSupported:1;
					bool mcaSupported:1;
					bool eisaSupported:1;
					bool pciSupported:1;
					bool pcmciaSupported:1;
					bool plugAndPlaySupported:1;
					bool apmSupported:1;
					bool firmwareUpgradable:1; // flash
					bool firmwareShadowingAllowed:1;
					bool vlVesaSupported:1;
					bool escdSupport:1;
					bool cdBootSupported:1;
					bool selectableBootSupported:1;
					bool firmwareRomSocketed:1; //e.g PLCC or SOP socket
					bool pcmciaBootSupported:1;
					bool eddSupported:1;
					bool nec9800FloppySupported:1; // japanese 1.2MB 3.5" 1KB/sector 360 RPM disk
					bool toshibaFloppySupported:1; // japanese 1.2MB 3.5" 360 RPM disk
					bool _525_360kFloppySupported:1; // 5.25" 260KB floppy
					bool _525_120mFloppySupported:1; // 5.25" 1.2MB floppy
					bool _350_720kFloppySupported:1; // 3.5" 720KB floppy
					bool _350_288mFloppySupported:1; // 3.5" 2.88MB floppy
					bool printscreenServiceSupported:1;
					bool _8042KeyboardServicesSupported:1;
					bool serialServicesSupported:1;
					bool printerServicesSupported:1;
					bool cgaMonoVideoSupported:1;
					bool necPc98:1;
					U16 _reserved32:16; // for platform firmware vendor
					U16 _reserved48:16; // for system vendor
				};
				U64 data;
			} firmwareCharacteristics;

			// // 2.4+
			// U8 firmwareCharacteristicsExtensionBytes[...];
		};

		enum struct WakeupType: U8 {
			_reserved0,
			other,
			unknown,
			apmTimer,
			modemRing,
			lanRemote,
			powerSwitch,
			pcmPme,
			acPowerRestored,
			max = acPowerRestored
		};

		const char *wakeupTypeString[(U8)WakeupType::max+1] {
			"unknown (0)",
			"other",
			"unknown",
			"APM timer",
			"modem ring",
			"LAN remote",
			"power switch",
			"PCI PME#",
			"AC power restored"
		};

		auto to_string(WakeupType type) -> const char * {
			switch(type){
				case WakeupType::_reserved0 ... WakeupType::max:
					return wakeupTypeString[(U8)type];
				default:
					return "unknown";
			}
		}

		struct __attribute__((packed)) SystemInformation: StructureTable {
			String manufacturer;
			String productName;
			String version;
			String serialNumber;
			U8 uuid[16];
			WakeupType wakeupType;
			String skuNumber;
			String family;
		};

		enum struct ChassisType: U8 {
			unknown0,
			other,
			unknown,
			desktop,
			lowProfileDesktop,
			pizzaBox,
			miniTower,
			tower,
			portable,
			laptop,
			notebook,

			handheld,
			dockingStation,
			allInOne,
			subNotebook,
			spacesaving,
			lunchbox,
			mainServer,
			expansion,
			subchassis,
			busExpansion,
			peripheral,
			raid,

			rackMount,
			sealedCasePc,
			multiSystem,
			compactPci,
			advancedTga,
			blade,
			bladeEnclosure,
			tablet,
			convertible,
			detachable,

			iotGateway,
			embeddedPc,
			miniPc,
			stickPc,
			max = stickPc
		};

		const char *chassisTypeString[(U8)ChassisType::max+1] = {
			"unknown (0)",
			"other",
			"unknown",
			"desktop",
			"low profile desktop",
			"pizza box",
			"mini tower",
			"tower",
			"portable",
			"laptop",
			"notebook",

			"handheld",
			"docking station",
			"all-in-one",
			"sub notebook",
			"space-saving",
			"lunch box",
			"main server",
			"expansion",
			"subchassis",
			"bus expansion chassis",

			"peripheral chassis",
			"raid chassis",
			"rack mount chassis",
			"sealed-case PC",
			"multi-system chassis",
			"compact PCI",
			"advanced TCA",
			"blade",
			"blade enclosure",
			"convertible",

			"detachable",
			"IoT gateway",
			"embedded PC",
			"mini PC",
			"stick PC"
		};

		auto to_string(ChassisType type) -> const char * {
			switch(type) {
				case ChassisType::unknown0 ... ChassisType::max:
				return chassisTypeString[(U8)type];

				default:
				return "unknown";
			}
		}

		struct __attribute__((packed)) SystemEnclosure: StructureTable {
			String manufacturer;
			struct __attribute__((packed)) {
				ChassisType chassisType:7;
				bool chassisLock:1;
			} type;
			String version;
			String serialNumber;
			String assetTagNumber;
			U8 bootupState;
			U8 powerSupplyState;
			U8 thermalState;
			U8 securityStatus;
			U16 oemDefined;
			U8 height; // height in Us (1.75")
			U8 numberOfCords; // 0 for unknown
			U8 numberOfContainedElements;
			U8 containedElementRecordLength;
			U8 containedElements;//[numberOfContainedElements*containedElementRecordLength];
			// String skuNumber;
			auto skuNumber() -> String {
				return *(String*)(&containedElements+numberOfContainedElements*containedElementRecordLength);
			}
		};

		enum ProcessorType: U8 {
			unknown0,
			other,
			unknown,
			central,
			math,
			dsp,
			video,
			max = video
		};

		const char *processorTypeString[(U8)ProcessorType::max+1] = {
			"unknown (0)",
			"other",
			"unknown",
			"central processor",
			"math processor",
			"dsp processor",
			"video processor"
		};

		auto to_string(ProcessorType type) -> const char * {
			switch(type){
				case ProcessorType::unknown0 ... ProcessorType::max:
					return processorTypeString[(U8)type];
				default:
					return "unknown";
			}
		}

		struct __attribute__((packed)) ProcessorInformation: StructureTable {
			// 2.0+
			String socketDesignation;
			ProcessorType processorType;
			U8 processorFamily;
			String processorManufacturer;
			U64 processorId;
			String processorVersion;
			U8 voltage;
			U16 externalClock; // Mhz
			U16 maxSpeed; // Mhz
			U16 currentSpeed; // Mhz
			U8 status;
			U8 processorUpgrade;

			// 2.1+
			U16 l1cacheHandle;
			U16 l2cacheHandle;
			U16 l3cacheHandle;

			// 2.3+
			String serialNumber;
			String assetTag;
			String partNumber;

			// 2.5+
			U8 coreCount; // 0 for unknown
			U8 coreEnabledCount; // 0 for unknown
			U8 threadCount; // 0 for unknown
			U16 processorCharacteristics;

			// 2.6+
			U16 processorFamily2;

			// 3.0+
			U16 coreCount2;
			U16 coreEnabledCount2;
			U16 threadCount2;

			// 3.6+
			U16 threadEnabled;

			// 3.8+
			String socketType;
		};

		// struct __attribute__((packed)) PhysicalMemory: StructureTable {

		auto find_30_manually() -> EntryPointStructure30* {
			for(auto eps=(U8*)0x000f0000; eps<(U8*)0x000fffff; eps += 16){
				if(!memcmp(eps, "_SM3_", 5)){
					auto eps3 = (EntryPointStructure30*)(eps+5);
					U8 checksum = 0;
					for(auto i=0u;i<eps3->length;i++){
						checksum += eps[i];
					}
					if(checksum==0){
						return eps3;
					}
				}
			}

			return nullptr;
		}

		auto find_21_manually() -> EntryPointStructure21* {
			for(auto eps=(U8*)0x000f0000; eps<(U8*)0x000fffff; eps += 16){
				if(!memcmp(eps, "_SM_", 4)){
					auto eps1 = (EntryPointStructure21*)(eps+4);
					U8 checksum = 0;
					for(auto i=0u;i<eps1->length;i++){
						checksum += eps[i];
					}
					if(checksum==0){
						return eps1;
					}
				}
			}

			return nullptr;
		}

		StructureTable *structTables = nullptr;
		U32 structTablesLength = 0;

		auto isaSupported = Maybe::maybe;
		auto pciSupported = Maybe::maybe;
	}

	auto Smbios::_on_start() -> Try<> {
		if(auto eps3 = find_30_manually()){
			log.print_info("EPS version: ", eps3->version.majorVersion, '.', eps3->version.minorVersion);
			structTables = (StructureTable*)eps3->structTableAddress;
			structTablesLength = eps3->structTableMaxSize;
		}
		if(auto eps1 = find_21_manually()){
			log.print_info("EPS version: ", eps1->version.majorVersion, '.', eps1->version.minorVersion);
			structTables = (StructureTable*)eps1->structTableAddress;
			structTablesLength = eps1->structTableLength;
		}

		if(!structTables) return Failure{"not found"};

		for(auto addr=(U8*)structTables;addr<(U8*)structTables+structTablesLength;){
			auto table = (StructureTable*)addr;
			switch(table->type){
				case StructureTableType::firmwareInfo: {
					auto section = log.section("firmware info:");
					auto &info = *(FirmwareInformation*)table;
					log.print_info("vendor = ", info.get_string(info.vendor));
					log.print_info("firmwareVersion = ", info.get_string(info.firmwareVersion));
					log.print_info("firmwareReleaseDate = ", info.get_string(info.firmwareReleaseDate));
					log.print_info("characteristicsNotSupported = ", info.firmwareCharacteristics.characteristicsNotSupported);
					if(info.firmwareCharacteristics.characteristicsNotSupported){
						log.print_warning("firmware characteristics not available - info unknown");
						break;
					}
					log.print_info("isaSupported = ", info.firmwareCharacteristics.isaSupported);
					log.print_info("mcaSupported = ", info.firmwareCharacteristics.mcaSupported);
					log.print_info("eisaSupported = ", info.firmwareCharacteristics.eisaSupported);
					log.print_info("pciSupported = ", info.firmwareCharacteristics.pciSupported);
					log.print_info("pcmciaSupported = ", info.firmwareCharacteristics.pcmciaSupported);
					log.print_info("plugAndPlaySupported = ", info.firmwareCharacteristics.plugAndPlaySupported);
					log.print_info("apmSupported = ", info.firmwareCharacteristics.apmSupported);
					log.print_info("firmwareUpgradable = ", info.firmwareCharacteristics.firmwareUpgradable);
					log.print_info("firmwareShadowingAllowed = ", info.firmwareCharacteristics.firmwareShadowingAllowed);
					log.print_info("vlVesaSupported = ", info.firmwareCharacteristics.vlVesaSupported);
					log.print_info("escdSupport = ", info.firmwareCharacteristics.escdSupport);
					log.print_info("cdBootSupported = ", info.firmwareCharacteristics.cdBootSupported);
					log.print_info("selectableBootSupported = ", info.firmwareCharacteristics.selectableBootSupported);
					log.print_info("firmwareRomSocketed = ", info.firmwareCharacteristics.firmwareRomSocketed);
					log.print_info("pcmciaBootSupported = ", info.firmwareCharacteristics.pcmciaBootSupported);
					log.print_info("eddSupported = ", info.firmwareCharacteristics.eddSupported);
					log.print_info("nec9800FloppySupported = ", info.firmwareCharacteristics.nec9800FloppySupported);
					log.print_info("toshibaFloppySupported = ", info.firmwareCharacteristics.toshibaFloppySupported);
					log.print_info("5\"25 360KB floppy supported = ", info.firmwareCharacteristics._525_360kFloppySupported);
					log.print_info("5\"25 1.20MB floppy supported = ", info.firmwareCharacteristics._525_120mFloppySupported);
					log.print_info("3\"50 720KB floppy supported = ", info.firmwareCharacteristics._350_720kFloppySupported);
					log.print_info("3\"50 2.88MB floppy supported = ", info.firmwareCharacteristics._350_288mFloppySupported);
					log.print_info("printscreenServiceSupported = ", info.firmwareCharacteristics.printscreenServiceSupported);
					log.print_info("_8042KeyboardServicesSupported = ", info.firmwareCharacteristics._8042KeyboardServicesSupported);
					log.print_info("serialServicesSupported = ", info.firmwareCharacteristics.serialServicesSupported);
					log.print_info("printerServicesSupported = ", info.firmwareCharacteristics.printerServicesSupported);
					log.print_info("cgaMonoVideoSupported = ", info.firmwareCharacteristics.cgaMonoVideoSupported);
					log.print_info("necPc98 = ", info.firmwareCharacteristics.necPc98);

					isaSupported = info.firmwareCharacteristics.isaSupported?Maybe::yes:Maybe::no;
					pciSupported = info.firmwareCharacteristics.pciSupported?Maybe::yes:Maybe::no;
				} break;
				case StructureTableType::systemInfo: {
					auto section = log.section("system info");
					auto &info = *(SystemInformation*)table;

					log.print_info("manufacturer = ", info.get_string(info.manufacturer));
					log.print_info("productName = ", info.get_string(info.productName));
					log.print_info("version = ", info.get_string(info.version));
					log.print_info("serialNumber = ", info.get_string(info.serialNumber));
					log.print_info_start();
						log.print_inline("UUID = ");
						for(auto i=0;i<16;i++){ log.print_inline(format::Hex8{info.uuid[i], false});}
					log.print_end();
					log.print_info("wakeupType = ", to_string(info.wakeupType));
					log.print_info("skuNumber = ", info.get_string(info.skuNumber));
					log.print_info("family = ", info.get_string(info.family));
				} break;
				case StructureTableType::systemEnclosure: {
					auto section = log.section("system enclosure");
					auto &info = *(SystemEnclosure*)table;

					log.print_info("manufacturer = ", info.get_string(info.manufacturer));
					log.print_info("chassis type = ", to_string(info.type.chassisType));
					log.print_info("chassis locked = ", info.type.chassisLock);
					log.print_info("version = ", info.get_string(info.version));
					log.print_info("serialNumber = ", info.get_string(info.serialNumber));
					log.print_info("assetTagNumber = ", info.get_string(info.assetTagNumber));
					log.print_info("skuNumber = ", info.get_string(info.skuNumber()));
				} break;
				case StructureTableType::processorInformation: {
					auto section = log.section("processor information");
					auto &info = *(ProcessorInformation*)table;

					log.print_info("type = ", to_string(info.processorType));
					log.print_info("socketDesignation = ", info.get_string(info.socketDesignation));
					log.print_info("processorManufacturer = ", info.get_string(info.processorManufacturer));
					log.print_info("processorVersion = ", info.get_string(info.processorVersion));
					log.print_info("serialNumber = ", info.get_string(info.serialNumber));
					log.print_info("assetTag = ", info.get_string(info.assetTag));
					log.print_info("partNumber = ", info.get_string(info.partNumber));
					log.print_info("socketType = ", info.get_string(info.socketType));
				} break;
				case StructureTableType::cacheInformation: {
					auto section = log.section("cache information");
				} break;
				case StructureTableType::systemSlots: {
					auto section = log.section("system slots");
				} break;
				case StructureTableType::physicalMemoryArea: {
					auto section = log.section("physical memory area");
				} break;
				case StructureTableType::memoryDevice: {
					auto section = log.section("memory device");
				} break;
				case StructureTableType::memoryArrayMappedDevice: {
					auto section = log.section("memory array mapped device");
				} break;
				case StructureTableType::systemBootInformation: {
					auto section = log.section("system boot information");
				} break;
				case StructureTableType::inactive:
				case StructureTableType::endOfTable:
					// ignored
				break;
				default:
					log.print_warning("unknown table type: ", (U8)table->type);
			}

			addr += table->length;
			while(addr[0]!='\0'||addr[1]!='\0') addr++;
			addr+=2;
		}

		return {};
	}

	auto Smbios::_on_stop() -> Try<> {
		return {};
	}

	auto Smbios::is_isa_supported() -> Maybe {
		return isaSupported;
	}
	auto Smbios::is_pci_supported() -> Maybe {
		return pciSupported;
	}
}
