#pragma once

#include <common/types.hpp>

struct __attribute__((packed)) Infoblock {
	#define RESERVED_WORDS(COUNT) U16 CONCAT(_reserved_, __COUNTER__)[COUNT];

	union {
		struct {
			U8:1;
			U8:1; // retired
			U8 responseIncomplete:1;
			U8:3; // retired
			U8:1; // obsolete
			U8 removableMediaDevice:1;
			U8:7; // retired
			U8 ataDevice:1; // 0 means ATA
		} ata;

		struct {
			U8 packetLength:2; // 0 = 12, 1 = 16 bytes
			U8 responseIncomplete:1;
			U8:2; // reserved
			U8 dataRequestDelay:2; // 0 = 3ms, 2 = 50us
			U8 removableMediaDevice:1;
			U8 commandPacketSet:5;
			U8:1; // reserved
			U8 atapiDevice:2; // 2 means ATAPI
		} atapi;
	};

	U16:16; // obsolete
	U16 specificConfiguration;
	U16:16; // obsolete
	U16 _retired[2];
	U16:16; // obsolete
	RESERVED_WORDS(2); // reserved for CompactFlash association
	U16:16; // retired
	C8 serialNumber[20];
	U16:16; // retired
	U16:16; // retired
	U16:16; // obsolete
	C8 firmwareRevision[8];
	C8 modelNumber[40];

	U16 maxSectorsPerInterrupt:8;
	U16:8; // 0x80

	RESERVED_WORDS(1);

	U16:8; // retired
	U16 dmaSupported:1;
	U16 lbaSupported:1;
	U16 ioReadyDisable:1;
	U16 ioReadySupported:1;
	U16:1; // obsolete
	U16 standbyTimerStandard:1;
	U16 atapiCommandQueuingSupported:1;
	U16 atapiInterleavedDmaSupported:1;

	U16 standbyTimerValueMin:1;
	U16:1; // obsolete
	U16:12; // reserved
	U16:1; // 1
	U16:1; // 0

	U16:16; // obsolete
	U16:16; // obsolete

	U16:1; // obsolete
	U16 word_64_70_valid:1;
	U16 word_88_valid:1;
	U16 word_53_bit_3_15:13;

	U16 word_54_58_obsolete[5];

	U16 currentSectorsPerInterrupt:8;
	U16 multipleSectorSettingValid:1;
	U16:7; // reserved

	U32	lbaSectorCount;
	U16:16; // obsolete

	U16 multiwordDma0Supported:1;
	U16 multiwordDma1Supported:1;
	U16 multiwordDma2Supported:1;
	U16:5; // reserved
	U16 multiwordDma0Selected:1;
	U16 multiwordDma1Selected:1;
	U16 multiwordDma2Selected:1;
	U16:5; // reserved

	U16 pioModesSupported:8;
	U16:8; // reserved

	U16 minMultiwordDmaCycleTime;
	U16 recommendedMultiwordDmaCycleTime;
	U16 minPioCycleTime;
	U16 minPioCycleTimeIoReady;

	U16:5; // reserved
	U16 supportsReadZeroAfterTrim:1;
	U16 supportsAta28Commands:1;
	U16:1; // reserved
	U16 supportsDownloadMicrocodeDma:1;
	U16 supportsSetMaxPasswordUnlockDma:1;
	U16 supportsWriteBufferDma:1;
	U16 supportsReadBufferDma:1;
	U16 supportsDeviceConfigurationIdentifyDma:1;
	U16 supportsLongPhysicalSectorErrorReporting:1;
	U16 supportsDeterministicReadAfterTrim:1;
	U16 supportsCfastSpecification:1;

	RESERVED_WORDS(1);
	U16 atapiPacketReceivedToBusReleaseTimeNs;
	U16 atapiServiceCommandToBusyClearTimeNs;
	RESERVED_WORDS(2);

	U16 maxQueueDepthMinusOne:5;
	U16:11; // reserved

	RESERVED_WORDS(4);

	U16:1; // reserved
	U16:3; // obsolete
	U16 supportsAtaAtapi4:1;
	U16 supportsAtaAtapi5:1;
	U16 supportsAtaAtapi6:1;
	U16 supportsAtaAtapi7:1;
	U16 supportsAtaAtapi8:1;
	U16 supportsAtaAtapi9:1;
	U16 supportsAtaAtapi10:1;
	U16 supportsAtaAtapi11:1;
	U16 supportsAtaAtapi12:1;
	U16 supportsAtaAtapi13:1;
	U16 supportsAtaAtapi14:1;
	U16:1; // reserved

	U16 minorVersion;

	U16 smartSupported:1;
	U16 securityModeSupported:1;
	U16 removableMediaSupported:1;
	U16 mandatoryPowerManagementSupported:1;
	U16 packetSupported:1;
	U16 writeCacheSupported:1;
	U16 lookAheadSupported:1;
	U16 releaseInterruptSupported:1;
	U16 serviceInterruptSupported:1;
	U16 deviceResetSupported:1;
	U16 hostProtectedAreaSupported:1;
	U16:1; // obsolete
	U16 writeBufferCommandSupported:1;
	U16 readBufferCommandSupported:1;
	U16 nopSupported:1;
	U16:1; // obsolete

	U16 downloadMicrocodeSupported:1;
	U16 readWriteDmaQueuedSupported:1;
	U16 compactFlashAssocSupported:1;
	U16 advancedPowerManagementSupported:1;
	U16 removableMediaStatusSupported:1;
	U16 powerUpInStandbySupported:1;
	U16 setFeaturesRequiredForSpinup:1;
	U16:1; // reserved
	U16 setMaxSecurityExtensionSupported:1;
	U16 automaticAcousticManagementSupported:1;
	U16 lba48Supported:1;
	U16 deviceConfigurationOverlaySupported:1;
	U16 mandatoryFlushCacheSupported:1;
	U16 flushCacheExtSupported:1;
	U16:1; // 1
	U16:1; // 0

	U16 smartErrorLoggingSupported:1;
	U16 smartSelfTestSupported:1;
	U16 mediaSerialNumberSupported:1;
	U16 mediaCardPassThroughSupported:1;
	U16:1; // reserved
	U16 generalPurposeLoggingSupported:1;
	U16:8; // reserved
	U16:1; // 1
	U16:1; // 0

	U16 smartEnabled:1;
	U16 securityModeEnabled:1;
	U16 removableMediaEnabled:1;
	U16 mandatoryPowerManagementEnabled:1;
	U16 packetEnabled:1;
	U16 writeCacheEnabled:1;
	U16 lookAheadEnabled:1;
	U16 releaseInterruptEnabled:1;
	U16 serviceInterruptEnabled:1;
	U16 deviceResetEnabled:1;
	U16 hostProtectedAreaEnabled:1;
	U16:1; // obsolete
	U16 writeBufferCommandEnabled:1;
	U16 readBufferCommandEnabled:1;
	U16 nopEnabled:1;
	U16:1; // obsolete

	U16 downloadMicrocodeSupported2:1;
	U16 readWriteDmaQueuedSupported2:1;
	U16 compactFlashAssocEnabled:1;
	U16 advancedPowerManagementEnabled:1;
	U16 removableMediaStatusEnabled:1;
	U16 powerUpInStandbyEnabled:1;
	U16 setFeaturesRequiredForSpinup2:1;
	U16:1; // reserved
	U16 setMaxSecurityExtensionEnabled:1;
	U16 automaticAcousticManagementEnabled:1;
	U16 lba48Supported2:1;
	U16 deviceConfigurationOverlaySupported2:1;
	U16 mandatoryFlushCacheSupported2:1;
	U16 flushCacheExtSupported2:1;
	U16:2; // reserved

	U16 smartErrorLoggingSupported2:1;
	U16 smartSelfTestSupported2:1;
	U16 mediaSerialNumberValid:1;
	U16 mediaCardPassThroughEnabled:1;
	U16:1; // reserved
	U16 generalPurposeLoggingSupported2:1;
	U16:8; // reserved
	U16:1; // 1
	U16:1; // 0

	U16 ultraDma0Supported:1;
	U16 ultraDma1Supported:1;
	U16 ultraDma2Supported:1;
	U16 ultraDma3Supported:1;
	U16 ultraDma4Supported:1;
	U16 ultraDma5Supported:1;
	U16 ultraDma6Supported:1;
	U16:1; // reserved
	U16 ultraDma0Selected:1;
	U16 ultraDma1Selected:1;
	U16 ultraDma2Selected:1;
	U16 ultraDma3Selected:1;
	U16 ultraDma4Selected:1;
	U16 ultraDma5Selected:1;
	U16 ultraDma6Selected:1;
	U16:1; // reserved

	U16 securityEraseUnitDuration;
	U16 enhancedSecurityEraseDuration;
	U16 currentAdvancedPowerManagementValue;
	U16 masterPasswordRevisionCode;

	U16 device0HardwareResetResult:8;
	U16 device1HardwareResetResult:5;
	U16 cableIdDetected:1;
	U16:1; // 1
	U16:1; // 0

	U16 currentAcousticManagementValue:8;
	U16 recommendedAcousticManagementValue:8;

	RESERVED_WORDS(5);
	U64	lba48SectorCount;
	RESERVED_WORDS(1);
	U16 maxDataSetManagementLbaRangeBlocks;

	U16 logicalSectorsPerPhysicalSector:4; // 2^x exponent
	U16:8; // reserved
	U16 logicalSectorNot512Bytes:1;
	U16 multipleLogicalPerPhysicalSectors:1;
	U16 blockSizeValid1:1; // 1
	U16 blockSizeValid0:1; // 0

	RESERVED_WORDS(10);

	U32 logicalSectorSize; // in words, see 106

	RESERVED_WORDS(8);

	U16 removableMediaStatusSupported2:2; // 1 = supported
	U16:14; // reserved

	U16 securitySupported:1;
	U16 securityEnabled:1;
	U16 securityLocked:1;
	U16 securityFrozen:1;
	U16 securityCountExpired:1;
	U16 enhancedSecurityEraseSupported:1;
	U16:2; // reserved
	U16 securityLevel:1; // 0 = high, 1 = max
	U16:7; // reserved

	U16 vendorSpecific[31];

	U16 cfaMaxCurrentMilliAmpers:12;
	U16 cfaPowerMode1Disabled:1;
	U16 cfaPowerMode1Required:1;
	U16:1; // reserved
	U16 word160Supported:1;

	RESERVED_WORDS(7); // reserved for CompactFlash association

	U16 deviceNominalFormFactor:4;
	U16:12; // reserved

	U16 dataSetManagementSupport:1;
	U16:15; // reserved

	U16 additionalProductIdentifier[4];
	RESERVED_WORDS(2);
	C8 currentMediaSerialNumber[60];
	RESERVED_WORDS(3);

	U16 logicalSectorOffset:14;
	U16 logicalBlockOffsetConfigValid1:1; // 1
	U16 logicalBlockOffsetConfigValid0:1; // 0

	RESERVED_WORDS(45);

	U16 signature:8;
	U16 checksum:8;

	template <size_t length>
	auto _copy_string(C8 (&data)[length]) -> const char* {
		static char buffer[length+1] = {};
		auto c = buffer;

		for(auto i=0u;i<length;i+=2){
			*(c++) = data[i+1];
			*(c++) = data[i+0];
		}
		*c = '\0';

		// trim ending spaces
		while(c[-1]==' '){
			*(--c) = '\0';
		}

		return buffer;
	}


	auto get_serialNumber() -> const char * { return _copy_string(serialNumber); }
	auto get_firmwareRevision() -> const char * { return _copy_string(firmwareRevision); }
	auto get_modelNumber() -> const char * { return _copy_string(modelNumber); }
	auto get_currentMediaSerialNumber() -> const char * { return _copy_string(currentMediaSerialNumber); }

	auto get_sector_count(bool &use48Bits, bool force = false) -> U64 {
		if(lba48Supported &&lba48SectorCount>=lbaSectorCount) {
			use48Bits = true;
			return lba48SectorCount;
		}

		use48Bits = force?lba48Supported:false;
		return lbaSectorCount;
	}

	auto get_physical_sector_size() -> U32 {
		if(!blockSizeValid1||blockSizeValid0) return 512; // invalid block size config

		U32 blockSize = 512;

		if(logicalSectorNot512Bytes){
			blockSize = logicalSectorSize*2;
		}

		if(multipleLogicalPerPhysicalSectors){
			return blockSize << logicalSectorsPerPhysicalSector;
		}

		return blockSize;
	}

	auto get_sector_size() -> U32 {
		if(!blockSizeValid1||blockSizeValid0) return 512; // invalid block size config

		if(logicalSectorNot512Bytes){
			return logicalSectorSize*2;
		}

		return 512;
	}

	auto get_block_offset() -> U32 {
		if(!logicalBlockOffsetConfigValid1||logicalBlockOffsetConfigValid0) return 0; // invalid logical block offset config

		return logicalSectorOffset;
	}

	#undef RESERVED_WORDS
};
