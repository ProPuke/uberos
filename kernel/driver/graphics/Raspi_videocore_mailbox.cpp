#include "Raspi_videocore_mailbox.hpp"

#include <common/graphics2d/BufferFormat.hpp>
#include <kernel/arch/raspi/mailbox.hpp>

namespace mailbox {
	using namespace arch::raspi::mailbox;
}

namespace driver {
	namespace graphics {
		namespace {
			framebuffer::Mode defaultMode;

			Framebuffer framebuffer;

			// THERE is surely a better way than bruting a list of possibles (list from https://www.raspberrypi.com/documentation/computers/config_txt.html)
			// TODO: use the Get EDID block mailbox message to properly get the display options
			U32 possibleResolutions[][2] = {
				{640, 350},
				{640, 400},
				{640, 480},
				{720, 400},
				{800, 600},
				{848, 480},
				{1024, 768},
				{1152, 864},
				{1280, 768},
				{1280, 800},
				{1280, 1024},
				{1360, 768},
				{1366, 768},
				{1400, 1050},
				{1440, 900},
				{1600, 900},
				{1600, 1200},
				{1680, 1050},
				{1792, 1344},
				{1856, 1392},
				{1920, 1080},
				{1920, 1200},
				{1920, 1440},
				{2048, 1152},
				{2560, 1600},
			};
		}

		/**/ Raspi_videocore_mailbox::Raspi_videocore_mailbox(U64 address):
			Graphics(address, "Raspi Videocore firmware mailbox", "video driver")
		{}

		void Raspi_videocore_mailbox::_on_driver_enable() {
			if(state==State::enabled) return;

			//TODO: only allow one of these drivers active at once?
			framebuffer.driver = nullptr;

			detect_default_mode();

			#if true
				set_mode(0, 1280, 720, graphics2d::BufferFormat::rgb8);
			#else
				set_mode(0, default_resolution[0], default_resolution[1], graphics2d::BufferFormat::rgb8);
			#endif

			state = State::enabled;
		}

		void Raspi_videocore_mailbox::_on_driver_disable() {
			if(state==State::disabled) return;

			if(framebuffer.driver==this){
				framebuffer.driver = nullptr;
			}

			state = State::disabled;
		}

		auto Raspi_videocore_mailbox::set_mode(U32 framebufferId, U32 width, U32 height, graphics2d::BufferFormat format, bool acceptSuggestion) -> bool {
			if(framebufferId>0) return false;

			log::Section section("device::", name, "::set_mode(", framebufferId, ", ", width, "x", height, ", ", format, ")...");

			framebuffer.driver = nullptr; // initially invalid

			unsigned bitdepth = graphics2d::bufferFormat::size[(U8)format]*8;

			mailbox::PropertyMessage tags[4];
			tags[0].tag = mailbox::PropertyTag::set_physical_dimensions;
			tags[0].data.screenSize.width = width;
			tags[0].data.screenSize.height = height;

			tags[1].tag = mailbox::PropertyTag::set_virtual_dimensions;
			tags[1].data.screenSize.width = width;
			tags[1].data.screenSize.height = height;

			tags[2].tag = mailbox::PropertyTag::set_bits_per_pixel;
			tags[2].data.bitsPerPixel = bitdepth;

			tags[3].tag = mailbox::PropertyTag::null_tag;

			if(!send_messages(tags)){
				log::print_info("Unable to initialise");
				return false;
			}

			auto assignBitdepth = tags[2].data.bitsPerPixel;

			framebuffer.buffer.width = tags[1].data.screenSize.width;
			framebuffer.buffer.height = tags[1].data.screenSize.height;

			tags[0].tag = mailbox::PropertyTag::allocate_buffer;
			tags[0].data.screenSize.width = 0;
			tags[0].data.screenSize.height = 0;

			tags[1].tag = mailbox::PropertyTag::null_tag;

			if(!send_messages(tags)){
				log::print_info("Unable to allocate framebuffer");
				return false;
			}

			if(assignBitdepth!=bitdepth){
				log::print_warning("Warning: Unable to allocate framebuffer with bitdepth ", bitdepth);
				if(!assignBitdepth){
					return false;
				}
				
				if(!acceptSuggestion){
					if(framebuffer.buffer.width==width&&framebuffer.buffer.height==height){
						log::print_warning("a ", bitdepth, " bit buffer was suggested, instead");
					}else{
						log::print_warning("a ", framebuffer.buffer.width, "x", framebuffer.buffer.height, " ", bitdepth, " bit buffer was suggested, instead");
					}

					return false;
				}

				bitdepth = assignBitdepth;
				switch(bitdepth){
					case 16u:
						format = graphics2d::BufferFormat::rgb565;
					break;
					case 24u:
						format = graphics2d::BufferFormat::rgb8;
					break;
					case 32u:
						format = graphics2d::BufferFormat::rgba8;
					break;
					default:
						return false;
				}

				log::print_info("accepting a bitdepth of ", bitdepth);
			}

			// log::print_info("got address ", format::Hex64{tags[0].data.allocate_res.fb_addr});

			framebuffer.buffer.stride = framebuffer.buffer.width*bitdepth;
			framebuffer.buffer.address = (U8*)(size_t)(tags[0].data.allocate_res.fb_addr&0x3FFFFFFF);
			framebuffer.buffer.size = tags[0].data.allocate_res.fb_size;
			framebuffer.buffer.format = format;

			tags[0].tag = mailbox::PropertyTag::get_bytes_per_row;
			tags[1].tag = mailbox::PropertyTag::null_tag;

			if(!send_messages(tags)){
				log::print_error("Error: Something went wrong requesting framebuffer pitch");
				return false;
			}

			auto pitch = tags[0].data.bytesPerRow;

			tags[0].tag = mailbox::PropertyTag::get_pixel_order;
			tags[1].tag = mailbox::PropertyTag::null_tag;

			if(!send_messages(tags)){
				log::print_warning("Warning: Unable to query pixel order");
			}

			auto pixelOrder = tags[0].data.pixelOrder;

			switch(pixelOrder){
				case mailbox::PropertyMessage::Data::PixelOrder::rgb:
					framebuffer.buffer.order = graphics2d::BufferFormatOrder::rgb;
				break;
				case mailbox::PropertyMessage::Data::PixelOrder::bgr:
					framebuffer.buffer.order = graphics2d::BufferFormatOrder::bgr;
				break;
			}

			if(pitch!=framebuffer.buffer.width*bitdepth/8){
				log::print_error("Error: Custom pitch of ", tags[0].data.bytesPerRow, " required for framebuffer. This is not supported");
				return false;
			}

			if(framebuffer.buffer.width!=width||framebuffer.buffer.height!=height){
				log::print_warning("Warning: Unable to allocate framebuffer of resolution ", width, "x", height);

				if(!acceptSuggestion){
					log::print_warning("a resolution of ", framebuffer.buffer.width, "x", framebuffer.buffer.height, " was suggested, instead");
					return false;
				}
				log::print_info("accepting a resolution of ", framebuffer.buffer.width, "x", framebuffer.buffer.height);
			}

			switch(bitdepth){
				case 32:
					framebuffer._set = &Framebuffer::set_rgba8;
				break;
				case 24:
					framebuffer._set = &Framebuffer::set_rgb8;
				break;
				case 16:
					framebuffer._set = &Framebuffer::set_rgb565;
				break;
			}

			// graphics2d::update_background();

			framebuffer.driver = this;
			
			return true;
		}

		auto Raspi_videocore_mailbox::get_mode_count() -> U32 {
			return sizeof(possibleResolutions)/sizeof(possibleResolutions[0]) * ((U32)graphics2d::BufferFormat::max+1);
		}

		auto Raspi_videocore_mailbox::get_mode(U32 framebufferId, U32 index) -> framebuffer::Mode {
			auto modeIndex = index/((U32)graphics2d::BufferFormat::max+1);
			if(modeIndex>=sizeof(possibleResolutions)/sizeof(possibleResolutions[0])) {
				return { 0 };
			}
			graphics2d::BufferFormat format = (graphics2d::BufferFormat)(index%((U32)graphics2d::BufferFormat::max+1));
			auto resolution = possibleResolutions[modeIndex];
			auto formatSize = graphics2d::bufferFormat::size[(U8)format]*8u;

			mailbox::PropertyMessage tags[3];
			tags[0].tag = mailbox::PropertyTag::test_physical_dimensions;
			tags[0].data.screenSize.width = resolution[0];
			tags[0].data.screenSize.height = resolution[1];

			tags[1].tag = mailbox::PropertyTag::test_bits_per_pixel;
			tags[1].data.bitsPerPixel = formatSize;

			tags[2].tag = mailbox::PropertyTag::null_tag;

			if(!send_messages(tags)){
				log::print_error("tag error");
				//TODO:ERROR
				return { 0 };
			}

			if(tags[0].data.screenSize.width!=resolution[0]||tags[0].data.screenSize.height!=resolution[1]||tags[1].data.bitsPerPixel!=formatSize){
				return { 0 };
			}

			return { resolution[0], resolution[1], format };
		}

		auto Raspi_videocore_mailbox::detect_default_mode() -> bool {
			//TODO:pick better defaults? This seems awfully low
			defaultMode.width = 640;
			defaultMode.height = 480;
			defaultMode.format = graphics2d::BufferFormat::rgb8;

			{ //get default resolution
				mailbox::PropertyMessage tags[2];
				tags[0].tag = mailbox::PropertyTag::get_physical_dimensions;
				tags[1].tag = mailbox::PropertyTag::null_tag;

				if(!send_messages(tags)){
					log::print_error("Error: unable to query physical display resolution");
					return false;
				}

				defaultMode.width = tags[0].data.screenSize.width;
				defaultMode.height = tags[0].data.screenSize.height;
				log::print_debug("detected physical display resolution of ", defaultMode.width, "x", defaultMode.height);
			}

			{ //get default format

				mailbox::PropertyMessage tags[2];
				tags[0].tag = mailbox::PropertyTag::test_bits_per_pixel;
				tags[0].data.bitsPerPixel = graphics2d::bufferFormat::size[(U8)graphics2d::BufferFormat::rgb8]*8;

				tags[1].tag = mailbox::PropertyTag::null_tag;

				if(!send_messages(tags)){
					log::print_error("Error: unable to query default bitdepth");

				}else{
					auto defaultBitdepth = tags[0].data.bitsPerPixel;
					
					log::print_debug("detected bitdepth of ", defaultBitdepth);

					bool found = false;

					for(U32 i=0;i<=(U8)graphics2d::BufferFormat::max;i++){
						if(graphics2d::bufferFormat::size[i]*8==defaultBitdepth){
							defaultMode.format = (graphics2d::BufferFormat)i;
							found = true;
							break;
						}
					}

					if(!found){
						log::print_error("Bitdepth of ", defaultBitdepth, " not understood");
					}
				}
			}

			return true;
		}

		auto Raspi_videocore_mailbox::get_default_mode() -> framebuffer::Mode {
			return defaultMode;
		}

		auto Raspi_videocore_mailbox::get_framebuffer_count() -> U32 {
			if(framebuffer.driver!=this) return 0;

			return 1;
		}

		auto Raspi_videocore_mailbox::get_framebuffer(U32 index) -> Framebuffer* {
			return index==0&&framebuffer.driver==this?&framebuffer:nullptr;
		}

		auto Raspi_videocore_mailbox::get_framebuffer_name(U32 index) -> const char* {
			if(framebuffer.driver!=this) return 0;

			return "framebuffer";
		}
	}
}
