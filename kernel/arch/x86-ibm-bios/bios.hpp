#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/IdentityMappedPointer.hpp>

namespace arch {
	namespace x86_ibm_bios {
		namespace bios {
			enum IoPort {
				com1,
				com2,
				com3,
				com4,
				lpt1,
				lpt2,
				lpt3
			};

			enum VideoType {
				none,
				monochrome80,
				colour40,
				colour80
			};

			void init();

			auto get_io(IoPort) -> arch::x86::IoPort;
			auto get_video_type() -> VideoType;
			auto get_possible_ebda() -> IdentityMapped<void>;

			void set_mode(U32);
		}
	}
}
