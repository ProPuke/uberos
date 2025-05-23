#include "BochsVga.hpp"

#include <drivers/x86/system/Pci.hpp>

#include <kernel/arch/x86/ioPort.hpp>
#include <kernel/arch/x86/PciDevice.hpp>
// #include <kernel/arch/x86/vm86.hpp>
#include <kernel/drivers.hpp>

#include <common/Box.hpp>
#include <common/common.hpp>
#include <common/format.hpp>
#include <common/graphics2d/BufferFormat.hpp>
#include <common/PodArray.hpp>
#include <common/stdlib.hpp>

namespace driver::graphics {
	namespace {
		U16 maxWidth = 2560;
		U16 maxHeight = 1600;
		U16 maxBpp = 32;

		graphics2d::Buffer framebuffer;
		Physical<U8> physicalFramebufferAddress;
		U32 physicalFramebufferSize;

		Graphics::Mode modes[] = {
			#define COMMON_MODES(WIDTH, HEIGHT) /**/\
				{WIDTH, HEIGHT, graphics2d::BufferFormat::grey8},\
				{WIDTH, HEIGHT, graphics2d::BufferFormat::rgb565},\
				{WIDTH, HEIGHT, graphics2d::BufferFormat::rgba8},

				COMMON_MODES(320, 200)
				COMMON_MODES(320, 240)
				COMMON_MODES(640, 480)
				COMMON_MODES(800, 600)
				COMMON_MODES(800, 1280)
				COMMON_MODES(1280, 720)
				COMMON_MODES(1280, 800)
				COMMON_MODES(1280, 1024)
				COMMON_MODES(1360, 768)
				COMMON_MODES(1366, 768)
				COMMON_MODES(1440, 900)
				COMMON_MODES(1600, 900)
				COMMON_MODES(1680, 1050)
				COMMON_MODES(1920, 1080)
				COMMON_MODES(1920, 1200)
				COMMON_MODES(2560, 1080)
				COMMON_MODES(2560, 1440)
				COMMON_MODES(2560, 1600)

			#undef COMMON_MODES
		};

		const U16 ioIndex = 0x01ce;
		const U16 ioData = 0x01cf;

		enum struct Register: U16 {
			id = 0x0,
			xRes = 0x1,
			yRes = 0x2,
			bpp = 0x3,
			enable = 0x4,
			bank = 0x5,
			virtWidth = 0x6,
			virtHeight = 0x7,
			xOffset = 0x8,
			yOffset = 0x9
		};

		enum struct Enable: U16 {
			enable = 1<<0,
			getCaps = 1<<1,
			bankGraularity32k = 1<<4,
			_8bitDac = 1<<5,
			linearFramebuffer = 1<<6,
			noClearMem = 1<<7,
		};

		auto get16(Register index) -> U16 {
			arch::x86::ioPort::write16(ioIndex, (U16)index);
			return arch::x86::ioPort::read16(ioData);
		}

		void set16(Register index, U16 value) {
			arch::x86::ioPort::write16(ioIndex, (U16)index);
			arch::x86::ioPort::write16(ioData, value);
		}

		void assign_framebuffer(U32 width, U32 height, U8 bpp) {
			//FIXME: there is a race condition between nulling the framebuffer, and broadcasting the invalid event. The framebuffer access should likely be wrapped with a spinlock to avoid this
			auto framebufferAddress = framebuffer.address;
			framebuffer.address = nullptr;

			BochsVga::instance.events.trigger({
				instance: &BochsVga::instance,
				type: driver::Graphics::Event::Type::framebufferChanging,
				framebufferChanging: { index: 0 }
			});
			BochsVga::allEvents.trigger({
				instance: &BochsVga::instance,
				type: driver::Graphics::Event::Type::framebufferChanging,
				framebufferChanging: { index: 0 }
			});

			framebuffer.width = width;
			framebuffer.height = height;
			
			graphics2d::BufferFormat format = graphics2d::BufferFormat::rgba8;
			graphics2d::BufferFormatOrder order = graphics2d::BufferFormatOrder::argb;

			switch(bpp){
				case 8u:
					format = graphics2d::BufferFormat::grey8;
					order = graphics2d::BufferFormatOrder::argb;
				break;
				case 16u:
					format = graphics2d::BufferFormat::rgb565;
					order = graphics2d::BufferFormatOrder::argb;
				break;
				case 24u:
					format = graphics2d::BufferFormat::rgb8;
					order = graphics2d::BufferFormatOrder::bgra;
				break;
				case 32u:
					format = graphics2d::BufferFormat::rgba8;
					order = graphics2d::BufferFormatOrder::bgra;
				break;
			}

			// log.print_info("got address ", format::Hex64{tags[0].data.allocate_res.fb_addr});

			framebuffer.address = framebufferAddress;
			framebuffer.stride = width*(bpp/8);
			framebuffer.format = format;
			framebuffer.order = order;

			BochsVga::instance.events.trigger({
				instance: &BochsVga::instance,
				type: driver::Graphics::Event::Type::framebufferChanged,
				framebufferChanged: { index: 0 }
			});
			BochsVga::allEvents.trigger({
				instance: &BochsVga::instance,
				type: driver::Graphics::Event::Type::framebufferChanged,
				framebufferChanged: { index: 0 }
			});
		}
	}

	auto BochsVga::_on_start() -> Try<> {
		auto pci = drivers::find_and_activate<driver::system::Pci>(this);
		if(!pci) return Failure{"PCI unavailable"};

		auto pciDevice = pci->find_device_by_id(0x11111234)?:pci->find_device_by_id(0xbeef80ee);
		if(!pciDevice) return Failure{"device not present"};

		TRY(api.subscribe_ioPort(ioIndex));
		TRY(api.subscribe_ioPort(ioData));
		TRY(api.subscribe_pci(*pciDevice, {.ioSpace=true, .memorySpace=true}));

		auto id = get16(Register::id);

		if(id!=0xb0c5){
			log.print_warning("unrecognised version: ", format::Hex16{id});
			return Failure{"unrecognised version"};
		}

		// check framebuffer address first before enabling, so we don't trample other drivers
		physicalFramebufferAddress = pciDevice->bar[0].memoryAddress.as_native_type<U8>();
		physicalFramebufferSize = pciDevice->bar[0].memorySize;
		framebuffer.address = TRY_RESULT(api.subscribe_memory<U8>(physicalFramebufferAddress, physicalFramebufferSize, mmu::Caching::writeCombining));

		set16(Register::enable, (U16)Enable::getCaps|(U16)Enable::noClearMem);
		maxWidth = get16(Register::xRes);
		maxHeight = get16(Register::yRes);
		maxBpp = get16(Register::bpp);
		set16(Register::enable, (U16)Enable::noClearMem);

		log.print_info("max supported: ", maxWidth, " x ", maxHeight, " @ ", maxBpp, " bpp (within ", physicalFramebufferSize/1024, "KB)");

		// note that bochs ISN'T set as enabled at this point, so set mode is guarentted to apply a modechange, as the current enable mode will not match target
		// return set_mode(0, 1280, 720, graphics2d::BufferFormat::rgba8, true);
		return set_mode(0, 1920, 1080, graphics2d::BufferFormat::rgba8, true);
		// return set_mode(0, 640, 480, graphics2d::BufferFormat::rgba8, true);
	}

	auto BochsVga::_on_stop() -> Try<> {
		return {};
	}

	auto BochsVga::get_mode_count(U32 framebufferId) -> U32 {
		if(framebufferId>0) return 0; // not supported

		return sizeof(modes)/(sizeof(modes[0]));
	}

	auto BochsVga::get_mode(U32 framebufferId, U32 index) -> Mode {
		if(framebufferId>0) return { 0 }; // not supported
		if(index>=sizeof(modes)/(sizeof(modes[0]))) return { 0 };

		return modes[index];
	}

	auto BochsVga::set_mode(U32 framebufferId, U32 index) -> Try<> {
		if(framebufferId>0) return Failure{"Invalid framebuffer id"};
		if(index>=sizeof(modes)/(sizeof(modes[0]))) return Failure{"Invalid mode id"};

		auto &mode = modes[index];

		return set_mode(framebufferId, mode.width, mode.height, mode.format, false);
	}

	auto BochsVga::set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion) -> Try<> {
		if(framebufferId>0) return Failure{"Invalid framebuffer id"};

		if(width&7) return Failure{"Horizontal resolution must be a multiple of 8"};

		U16 bpp = 32;
		switch(format){
			//NOTE: 4bpp (16 hours) and 15bpp (rgb555) are also supported
			case graphics2d::BufferFormat::grey8 : bpp = 8;
			case graphics2d::BufferFormat::rgb565: bpp = 16;
			case graphics2d::BufferFormat::rgb8  : bpp = 24;
			case graphics2d::BufferFormat::rgba8 : bpp = 32;
		}

		auto enables = (U16)Enable::enable|(U16)Enable::linearFramebuffer;

		auto initialWidth = get16(Register::xRes);
		auto initialHeight = get16(Register::yRes);
		auto initialBpp = get16(Register::bpp);
		auto initialEnables = get16(Register::enable);

		if(enables==initialEnables&&width==initialWidth&&height==initialHeight&&bpp==initialBpp) return {}; // we're good fam

		const auto maxMemory = physicalFramebufferSize;

		const auto requestBpp = min(bpp, maxBpp);
		auto requestWidth = min<U32>(width, maxWidth) & ~7;
		auto requestHeight = min<U32>(height, maxHeight);

		if(requestWidth<1||requestHeight<1) return Failure{"Mode not supported"};
		if(!acceptSuggestion&&(requestWidth!=width||requestHeight!=height||requestBpp!=bpp||width*height*(bpp/8)>maxMemory)) return Failure{"Mode not supported"};

		if(width*height*(bpp/8)>maxMemory){
			const auto totalPixels = maxMemory/(requestBpp/8);
			requestHeight = maths::sqrt(totalPixels/width*height);
			requestWidth = (requestHeight*width/height) & ~7;
		}

		set16(Register::enable, 0);
		set16(Register::xRes, requestWidth);
		set16(Register::yRes, requestHeight);
		set16(Register::bpp, requestBpp);

		auto appliedWidth = get16(Register::xRes);
		auto appliedHeight = get16(Register::yRes);
		auto appliedBpp = get16(Register::bpp);

		// roll it back if this mode isn't good enough..
		if(!acceptSuggestion&&(appliedWidth!=width||appliedHeight!=height||appliedBpp!=bpp)){
			set16(Register::xRes, initialWidth);
			set16(Register::yRes, initialHeight);
			set16(Register::bpp, initialBpp);
			set16(Register::enable, enables);

			assign_framebuffer(initialWidth, initialHeight, initialBpp);

			return Failure{"Mode not supported"};
		}

		assign_framebuffer(appliedWidth, appliedHeight, appliedBpp);

		set16(Register::enable, enables);

		return {};
	}

	auto BochsVga::get_framebuffer_count() -> U32 {
		return 1;
	}

	auto BochsVga::get_framebuffer(U32 index) -> graphics2d::Buffer* {
		if(index>0) return nullptr; // not supported

		return framebuffer.address?&framebuffer:nullptr;
	}

	auto BochsVga::get_framebuffer_name(U32 index) -> const char* {
		if(index>0) return nullptr; // not supported

		return "framebuffer";
	}
}
