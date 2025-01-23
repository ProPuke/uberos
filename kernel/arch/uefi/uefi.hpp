#pragma once

#include <common/types.hpp>

namespace uefi {
	#ifdef _64BIT
		#define UEFIAPI __attribute__((ms_abi))
	#else
		#define UEFIAPI __attribute__((stdcall))
	#endif

	enum struct Status: U32 {
		success = 0b00000000,

		warning_unknownGlyph = 0b00000001, 
		warning_deleteFailure, 
		warning_writeFailure, 
		warning_bufferTooSmall, 
		warning_staleData, 
		warning_fileSystem, 
		warning_resetRequired, 

		loadError = 0b10000001,
		invalidParameter,
		unsupported,
		badBufferSize,
		bufferTooSmall,
		notReady,
		deviceError,
		writeProtected,
		outOfResources,
		volumeCorrupted,
		volumeFull,
		noMedia,
		mediaChanged,
		notFound,
		accessDenied,
		noResponse,
		noMapping,
		timeout,
		notStarted,
		alreadyStarted,
		aborted,
		icmpError,
		tftpError,
		protocolError,
		incompatibleVersion,
		securityViolation,
		crcError,
		endOfMedia,
		_unused29,
		_unused30,
		endOfFile,
		invalidLanguage,
		compromisedData,
		ipAddressConflict,
		httpError
	};

	typedef U32 PhysicalAddress;
	typedef U32 VirtualAddress;

	typedef void *Handle;
	typedef void *Event;

	struct Guid {
		U32 data1;
		U16 data2;
		U16 data3;
		U8  data4[8];
	};

	struct TableHeader {
		U64 signature;
		U32 revision;
		U32 headerSize;
		U32 crc32;
		U32 reserved;
	};

	typedef UMax Tpl;

	enum struct AllocateType {
		anyPages,
		maxAddress,
		address
	};

	enum struct MemoryType {
    	reservedMemoryType,
    	loaderCode,
    	loaderData,
    	bootServicesCode,
    	bootServicesData,
    	runtimeServicesCode,
    	runtimeServicesData,
    	conventionalMemory,
    	unusableMemory,
    	aCPIReclaimMemory,
    	aCPIMemoryNVS,
    	memoryMappedIO,
    	memoryMappedIOPortSpace,
    	palCode,
    	persistentMemory
	};

	struct MemoryDescriptor {
		U32 type;
		PhysicalAddress physicalStart;
		VirtualAddress virtualStart;
		U64 numberOfPages;
		U64 attribute;
	};

	typedef void (*EventNotify)(Event, void *context);

	// ORed as a bitmask
	enum struct EventType: U32 {
		timer         = 0x80000000,
		runtime       = 0x40000000,

		notify_wait   = 0x00000100,
		notify_signal = 0x00000200,

		signal_exit_boot_services     = 0x00000201,
		signal_virtual_address_change = 0x60000202
	};

	enum struct TimerDelayType {
		cancel,
		periodic,
		relative
	};

	struct RuntimeServices {
		TableHeader header;

		void *getTime;
		void *setTime;
		void *getWakeupTime;
		void *setWakeupTime;

		void *setVirtualAddressMap;
		void *convertPointer;

		void *getVariable;
		void *getNextVariableName;
		void *setVariable;

		void *getNextHighMonotonicCount;
		void *resetSystem;

		// 2.0..

		void *updateCapsule;
		void *queryCapsuleCapabilities;

		void *queryVariableInfo;
	};

	struct BootServices {
		TableHeader header;
		Tpl (UEFIAPI *raiseTPL)(Tpl *newTpl);
		void (UEFIAPI *restoreTPL)(Tpl *oldTpl);

		Status (UEFIAPI *allocatePages)(AllocateType, MemoryType, UMax pages, PhysicalAddress*);
		Status (UEFIAPI *freePages)(PhysicalAddress, UMax pages);
		Status (UEFIAPI *getMemoryMap)(UMax *memoryMapSize, MemoryDescriptor*, UMax *outMapKey, UMax *outDescriptorSize, U32 *outDescriptorVersion);
		Status (UEFIAPI *allocatePool)(MemoryType, UMax size, void **outBuffer);
		Status (UEFIAPI *freePool)(void *buffer);

		Status (UEFIAPI *createEvent)(EventType type, Tpl NotifyTpl, EventNotify notifyFunction, void *context, Event *outEvent);
		Status (UEFIAPI *setTimer)(Event, TimerDelayType, U64 triggerTime);
		Status (UEFIAPI *waitForEvent)(UMax numberOfEvents, Event *event, UMax *outIndex);
		Status (UEFIAPI *signalEvent)(Event);
		Status (UEFIAPI *closeEvent)(Event);
		Status (UEFIAPI *checkEvent)(Event);

		void *installProtocolInterface;
		void *reinstallProtocolInterface;
		void *uninstallProtocolInterface;
		void *handleProtocol;
		void *_reserved;
		void *registerProtocolNotify;
		void *locateHandle;
		void *locateDevicePath;
		void *installConfigurationTable;

		void *loadImage;
		void *startImage;
		void *exit;
		void *unloadImage;
		Status (UEFIAPI *exitBootServices)(Handle imageHandle, UMax mapKey);

		// ...
	};

	enum struct TextAttribute {
		// dark
		blackForeground = 0x00,
		blueForeground,
		greenForeground,
		cyanForeground,
		redForeground,
		magentaForeground,
		brownForeground,
		lightGreyForeground,

		// bright
		darkGreyForeground,
		lightBlueForeground,
		lightGreenForeground,
		lightCyanForeground,
		lightRedForeground,
		lightMagentaForeground,
		yellowForeground,
		whiteForeground,

		blackBackground = 0x00,
		blueBackground = 0x10,
		greenBackground = 0x20,
		cyanBackground = 0x30,
		redBackground = 0x40,
		magentaBackground = 0x50,
		brownBackground = 0x60,
		lightGrayBackground = 0x70,
	};

	struct InputKey {
		U16 scanCode;
		C16 unicodeChar;
	};

	struct SimpleTextInputProtocol {
		Status (UEFIAPI *_reset)(SimpleTextInputProtocol*, bool extendedVerification);
		Status (UEFIAPI *_readKeyStroke)(SimpleTextInputProtocol*, InputKey *outKey);

		Status reset(bool extendedVerification) { return _reset(this, extendedVerification); }
		Status readKeyStroke(InputKey *outKey) { return _readKeyStroke(this, outKey); }

		Event waitForKey;
	};
	
	struct SimpleTextOutputMode {
		I32 maxMode;
		I32 mode;
		I32 attribute;
		I32 cursorColumn;
		I32 cursorRow;
		bool cursorVisible;
	};

	struct SimpleTextOutputProtocol {
		Status (UEFIAPI *_reset) (SimpleTextOutputProtocol*, bool extendedVerification);
		Status (UEFIAPI *_outputString) (SimpleTextOutputProtocol*, C16 *string);
		Status (UEFIAPI *_testString) (SimpleTextOutputProtocol*, C16 *string);
		Status (UEFIAPI *_queryMode) (SimpleTextOutputProtocol*, UMax modeNumber, UMax *outColumns, UMax *outRows);
		Status (UEFIAPI *_setMode) (SimpleTextOutputProtocol*, UMax modeNumber);
		Status (UEFIAPI *_setAttribute) (SimpleTextOutputProtocol*, TextAttribute attribute);
		Status (UEFIAPI *_clearScreen) (SimpleTextOutputProtocol*);
		Status (UEFIAPI *_setCursorPosition) (SimpleTextOutputProtocol*, UMax column, UMax row);
		Status (UEFIAPI *_enableCursor) (SimpleTextOutputProtocol*, bool visible);

		Status reset(bool extendedVerification) { return _reset(this, extendedVerification); }
		Status outputString(C16 *string) { return _outputString(this, string); }
		Status testString(C16 *string) { return _testString(this, string); }
		Status queryMode(UMax modeNumber, UMax *outColumns, UMax *outRows) { return _queryMode(this, modeNumber, outColumns,  outRows); }
		Status setMode(UMax modeNumber) { return _setMode(this, modeNumber); }
		Status setAttribute(TextAttribute attribute) { return _setAttribute(this, attribute); }
		Status clearScreen() { return _clearScreen(this); }
		Status setCursorPosition(UMax column, UMax row) { return _setCursorPosition(this, column, row); }
		Status enableCursor(bool visible) { return _enableCursor(this, visible); }

		SimpleTextOutputMode *mode;
	};

	struct ConfigurationTable {
		Guid vendorGuid;
		void *vendorTable;
	};

	struct SystemTable {
		TableHeader header;
		C16 *firmwareVendor;
		U32 firmwareRevision;
		Handle consoleInHandle;
		SimpleTextInputProtocol *conIn;
		Handle consoleOutHandle;
		SimpleTextOutputProtocol *conOut;
		Handle standardErrorHandle;
		SimpleTextOutputProtocol *stdErr;
		RuntimeServices *runtimeServices;
		BootServices *bootServices;
		UMax numberOfTableEntries;
		ConfigurationTable *configurationTable;
	};

	extern SystemTable *systemTable;

	void init(SystemTable*);
}
