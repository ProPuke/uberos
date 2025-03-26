#include "IbmBios.hpp"

#include <kernel/arch/x86-ibm/memory.hpp>

namespace driver::system {
	namespace {
		// IbmBios Data Area (BDA)
		struct __attribute__((packed)) DataArea {
			struct __attribute__((packed)) ComPortInfo {
				U16 com1;
				U16 com2;
				U16 com3;
				U16 com4;
			} comPortInfo;

			struct __attribute__((packed)) LptInfo {
				U16 lpt1;
				U16 lpt2;
				U16 lpt3;
			} lptInfo;

			U16 ebdaShiftedAddress; // (_USUALLY_ true) must be left-shifted by 4

			struct __attribute__((packed)) {
				bool iplDiskette:1;
				bool mathCoprocessor:1;
				bool _unused1:2; // something to do with old system board with < 256k and ps2 ports?
				U8 initialVideoMode:2;
				U8 diskDriveCount:1; // 1 less than the number of drives,
			} equipmentFlags;
		};

		const auto &data = *(DataArea*)arch::x86_ibm::memory::biosDataArea.address;
		void *ebda = nullptr;
	}

	auto IbmBios::_on_start() -> Try<> {
		ebda = (void*)((size_t)data.ebdaShiftedAddress<<4);

		if((size_t)ebda+0x400 > 0xA0000) {
			log.print_warning("EBDA address is invalid (", ebda ,')');
			ebda = nullptr;
		}

		// set_mode(0x2);

		return {};
	}

	auto IbmBios::_on_stop() -> Try<> {
		return {};
	}

	auto IbmBios::get_io(IoPort port) -> arch::x86::IoPort {
		switch(port) {
			case IoPort::com1: return data.comPortInfo.com1;
			case IoPort::com2: return data.comPortInfo.com2;
			case IoPort::com3: return data.comPortInfo.com3;
			case IoPort::com4: return data.comPortInfo.com4;
			case IoPort::lpt1: return data.lptInfo.lpt1;
			case IoPort::lpt2: return data.lptInfo.lpt2;
			case IoPort::lpt3: return data.lptInfo.lpt3;
		}

		return 0;
	}

	auto IbmBios::get_video_type() -> VideoType {
		switch(data.equipmentFlags.initialVideoMode){
			case 0b00: return VideoType::none;
			case 0b01: return VideoType::colour40;
			case 0b10: return VideoType::colour80;
			case 0b11: return VideoType::monochrome80;
		}

		return VideoType::none; //unknown
	}

	auto IbmBios::get_possible_ebda() -> void* {
		return ebda;
	}

	void IbmBios::set_mode(U32 mode) {
		asm volatile(
			"mov ah, 0x00\n"
			"mov al, %0\n"
			"int 0x10\n"
			:
			: "r"((U8)mode)
			: "ax"
		);
	}
}
