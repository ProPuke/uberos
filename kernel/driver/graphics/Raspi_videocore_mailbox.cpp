#include "Raspi_videocore_mailbox.hpp"

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
				set_mode(0, 1280, 720, FramebufferFormat::rgb8);
			#else
				set_mode(0, default_resolution[0], default_resolution[1], FramebufferFormat::rgb8);
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

		bool Raspi_videocore_mailbox::set_mode(U32 framebufferId, U32 width, U32 height, FramebufferFormat format, bool acceptSuggestion) {
			if(framebufferId>0) return false;

			stdio::Section section("device::", name, "::set_mode(", framebufferId, ", ", width, "x", height, ", ", format, ")...");

			framebuffer.driver = nullptr; // initially invalid

			unsigned bitdepth = framebufferFormat::size[(U8)format]*8;

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
				stdio::print_info("Unable to initialise");
				return false;
			}

			framebuffer.width = tags[1].data.screenSize.width;
			framebuffer.height = tags[1].data.screenSize.height;

			tags[0].tag = mailbox::PropertyTag::allocate_buffer;
			tags[0].data.screenSize.width = 0;
			tags[0].data.screenSize.height = 0;

			tags[1].tag = mailbox::PropertyTag::null_tag;

			if(!send_messages(tags)){
				stdio::print_info("Unable to allocate framebuffer");
				return false;
			}

			if(tags[2].data.bitsPerPixel!=bitdepth){
				stdio::print_warning("Warning: Unable to allocate framebuffer with bitdepth ", bitdepth);
				if(!tags[2].data.bitsPerPixel){
					return false;
				}
				
				if(!acceptSuggestion){
					if(framebuffer.width==width&&framebuffer.height==height){
						stdio::print_warning("a ", bitdepth, " bit buffer was suggested, instead");
					}else{
						stdio::print_warning("a ", framebuffer.width, "x", framebuffer.height, " ", bitdepth, " bit buffer was suggested, instead");
					}

					return false;
				}

				bitdepth = tags[2].data.bitsPerPixel;
				switch(bitdepth){
					case 16u:
						format = FramebufferFormat::rgb565;
					break;
					case 24u:
						format = FramebufferFormat::rgb8;
					break;
					case 32u:
						format = FramebufferFormat::rgba8;
					break;
					default:
						return false;
				}

				stdio::print_info("accepting a bitdepth of ", bitdepth);
			}

			// stdio::print_info("got address ", format::Hex64{tags[0].data.allocate_res.fb_addr});

			framebuffer.address = (U8*)(size_t)(tags[0].data.allocate_res.fb_addr&0x3FFFFFFF);
			framebuffer.size = tags[0].data.allocate_res.fb_size;
			framebuffer.format = format;

			tags[0].tag = mailbox::PropertyTag::get_bytes_per_row;

			tags[1].tag = mailbox::PropertyTag::null_tag;

			if(!send_messages(tags)){
				stdio::print_error("Error: Something went wrong requesting framebuffer pitch");
				return false;
			}


			if(tags[0].data.bytesPerRow!=framebuffer.width*bitdepth/8){
				stdio::print_error("Error: Custom pitch of ", tags[0].data.bytesPerRow, " required for framebuffer. This is not supported");
				return false;
			}

			if(framebuffer.width!=width||framebuffer.height!=height){
				stdio::print_warning("Warning: Unable to allocate framebuffer of resolution ", width, "x", height);

				if(!acceptSuggestion){
					stdio::print_warning("a resolution of ", framebuffer.width, "x", framebuffer.height, " was suggested, instead");
					return false;
				}
				stdio::print_info("accepting a resolution of ", framebuffer.width, "x", framebuffer.height);
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

		U32  Raspi_videocore_mailbox::get_mode_count() {
			return sizeof(possibleResolutions)/sizeof(possibleResolutions[0]) * ((U32)FramebufferFormat::max+1);
		}

		framebuffer::Mode Raspi_videocore_mailbox::get_mode(U32 framebufferId, U32 index) {
			auto modeIndex = index/((U32)FramebufferFormat::max+1);
			if(modeIndex>=sizeof(possibleResolutions)/sizeof(possibleResolutions[0])) {
				return { 0 };
			}
			FramebufferFormat format = (FramebufferFormat)(index%((U32)FramebufferFormat::max+1));
			auto resolution = possibleResolutions[modeIndex];
			auto formatSize = framebufferFormat::size[(U8)format]*8u;

			mailbox::PropertyMessage tags[3];
			tags[0].tag = mailbox::PropertyTag::test_physical_dimensions;
			tags[0].data.screenSize.width = resolution[0];
			tags[0].data.screenSize.height = resolution[1];

			tags[1].tag = mailbox::PropertyTag::test_bits_per_pixel;
			tags[1].data.bitsPerPixel = formatSize;

			tags[2].tag = mailbox::PropertyTag::null_tag;

			if(!send_messages(tags)){
				stdio::print_error("tag error");
				//TODO:ERROR
				return { 0 };
			}

			if(tags[0].data.screenSize.width!=resolution[0]||tags[0].data.screenSize.height!=resolution[1]||tags[1].data.bitsPerPixel!=formatSize){
				return { 0 };
			}

			return { resolution[0], resolution[1], format };
		}

		bool Raspi_videocore_mailbox::detect_default_mode() {
			//TODO:pick better defaults? This seems awfully low
			defaultMode.width = 640;
			defaultMode.height = 480;
			defaultMode.format = FramebufferFormat::rgb8;

			{ //get default resolution
				mailbox::PropertyMessage tags[2];
				tags[0].tag = mailbox::PropertyTag::get_physical_dimensions;
				tags[1].tag = mailbox::PropertyTag::null_tag;

				if(!send_messages(tags)){
					stdio::print_error("Error: unable to query physical display resolution");
					return false;
				}

				defaultMode.width = tags[0].data.screenSize.width;
				defaultMode.height = tags[0].data.screenSize.height;
				stdio::print_debug("detected physical display resolution of ", defaultMode.width, "x", defaultMode.height);
			}

			{ //get default format

				mailbox::PropertyMessage tags[2];
				tags[0].tag = mailbox::PropertyTag::test_bits_per_pixel;
				tags[0].data.bitsPerPixel = framebufferFormat::size[(U8)FramebufferFormat::rgb8]*8;

				tags[1].tag = mailbox::PropertyTag::null_tag;

				if(!send_messages(tags)){
					stdio::print_error("Error: unable to query default bitdepth");

				}else{
					auto defaultBitdepth = tags[0].data.bitsPerPixel;
					
					stdio::print_debug("detected bitdepth of ", defaultBitdepth);

					bool found = false;

					for(U32 i=0;i<=(U8)FramebufferFormat::max;i++){
						if(framebufferFormat::size[i]*8==defaultBitdepth){
							defaultMode.format = (FramebufferFormat)i;
							found = true;
							break;
						}
					}

					if(!found){
						stdio::print_error("Bitdepth of ", defaultBitdepth, " not understood");
					}
				}
			}

			return true;
		}

		framebuffer::Mode Raspi_videocore_mailbox::get_default_mode() {
			return defaultMode;
		}

		U32 Raspi_videocore_mailbox::get_framebuffer_count() {
			return framebuffer.driver==this?1:0;
		}

		Framebuffer* Raspi_videocore_mailbox::get_framebuffer(U32 index) {
			return index==0&&framebuffer.driver==this?&framebuffer:nullptr;
		}
	}
}
