#include "framebuffer.hpp"

#include "mailbox.hpp"
#include "mmio.hpp"
#include <kernel/graphics2d.hpp>
#include <kernel/memory.hpp>
#include <common/types.hpp>
#include <common/stdlib.hpp>
#include <kernel/stdio.hpp>

namespace arm {
	namespace framebuffer {
		extern void (*_set)(U32 x, U32 y, U32 colour);
	}
}

namespace mailbox {
	using namespace arch::raspi::mailbox;
}

namespace mmio {
	using namespace arch::raspi;
}

extern U8 __end;

namespace framebuffer {
	namespace arch {
		namespace raspi {
			void init() {
				stdio::Section section("framebuffer::arch::raspi::init...");

				detect_default_resolution();

				framebuffer::framebuffer_count = 1;
				framebuffer::framebuffers[0].index = 0;

				if(!framebuffer::set_mode(0, default_resolution[0], default_resolution[1], FramebufferFormat::rgb565)){
					framebuffer::framebuffer_count = 0;
				}

				// {
				// 	auto &framebuffer = framebuffers[0];
				// 	auto buffer = graphics2d::get_screen_buffer(0, {0, 0, (I32)framebuffer.width, (I32)framebuffer.height});
				// 	while(true){
				// 		graphics2d::update_background();
				// 		for(U32 x=0;x<framebuffer.width/4;x++){
				// 			buffer.scroll(-4, -2);
				// 		}
				// 	}
				// }
			}
		}
	}

	bool set_mode(U32 framebufferId, U32 width, U32 height, FramebufferFormat format, bool acceptSuggestion) {
		if(framebufferId>0) return false;

		stdio::Section section("framebuffer::arch::raspi::set_mode(", framebufferId, ", ", width, "x", height, ", ", format, ")...");

		auto &framebuffer = ::framebuffer::framebuffers[framebufferId];

		unsigned bitdepth;
		switch(format){
			case FramebufferFormat::rgb565:
				bitdepth = 16u;
			break;
			case FramebufferFormat::rgb8:
				bitdepth = 24u;
			break;
			case FramebufferFormat::rgba8:
				bitdepth = 32u;
			break;
			default:
				stdio::print_error("Error: Unsupported format");
				return false;
		}

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

		// stdio::print_info("got address ", tags[0].data.allocate_res.fb_addr);

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

		return true;
	}

	bool detect_default_resolution() {
		mailbox::PropertyMessage tags[2];
		tags[0].tag = mailbox::PropertyTag::get_physical_dimensions;
		tags[1].tag = mailbox::PropertyTag::null_tag;

		if(!send_messages(tags)){
			//TODO:pick better defaults? This seems awfully low
			default_resolution[0] = 640;
			default_resolution[1] = 480;
			stdio::print_error("Error: unable to query physical display resolution");
			return false;
		}

		default_resolution[0] = tags[0].data.screenSize.width;
		default_resolution[1] = tags[0].data.screenSize.height;
		stdio::print_info("detected physical display resolution of ", default_resolution[0], "x", default_resolution[1]);

		// stdio::print_info("addresses ", &framebuffer_count, ", ", &framebuffers[0], ", ", &default_resolution);

		return true;
	}
}
