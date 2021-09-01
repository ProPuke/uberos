#include "framebuffer.hpp"

#include "mailbox.hpp"
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

namespace framebuffer {
	namespace arch {
		namespace raspi {
			void init() {
				stdio::Section section("framebuffer::arch::raspi::init...");

				detect_default_resolution();

				framebuffer::framebuffer_count = 1;
				framebuffer::framebuffers[0].index = 0;

				//find a mode that works..
				if(!framebuffer::set_mode(0, default_resolution[0], default_resolution[1], FramebufferFormat::rgb565)
					&&!framebuffer::set_mode(0, default_resolution[0], default_resolution[1], FramebufferFormat::rgb8)
				){
					framebuffer::framebuffer_count = 0;
				}
			}
		}
	}

	bool set_mode(U32 framebufferId, U32 width, U32 height, FramebufferFormat format) {
		if(framebufferId>0) return false;

		stdio::Section section("raspi::framebuffer::set_mode(", framebufferId, ", ", width, "x", height, ", ", format, ")...");

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
			return false;
		}

		auto &framebuffer = ::framebuffer::framebuffers[framebufferId];

		framebuffer.address = (U8*)((size_t)tags[0].data.allocate_res.fb_addr&0x3FFFFFFF);
		framebuffer.size = tags[0].data.allocate_res.fb_size;
		framebuffer.width = tags[1].data.screenSize.width;
		framebuffer.height = tags[1].data.screenSize.height;
		framebuffer.format = format;

		tags[0].tag = mailbox::PropertyTag::get_bytes_per_row;

		tags[1].tag = mailbox::PropertyTag::null_tag;

		if(send_messages(tags)){
			//TODO:support?

			if(tags[0].data.bytesPerRow!=framebuffer.width*bitdepth/8){
				stdio::print_error("Error: Custom pitch of ", tags[0].data.bytesPerRow, " required for framebuffer. This is not supported");
				return false;
			}
		}

		if(framebuffer.width!=width||framebuffer.height!=height){
			stdio::print_info("fell back to resolution of ", framebuffer.width, "x", framebuffer.height);
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

		return true;
	}
}
