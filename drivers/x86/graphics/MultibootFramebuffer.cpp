#include "MultibootFramebuffer.hpp"

#include <drivers/x86/system/Pci.hpp>

#include <kernel/arch/x86/ioPort.hpp>
// #include <kernel/arch/x86/vm86.hpp>
#include <kernel/multiboot.hpp>
#include <kernel/drivers.hpp>
#include <kernel/PodArray.hpp>

#include <common/Box.hpp>
#include <common/common.hpp>
#include <common/format.hpp>
#include <common/graphics2d/BufferFormat.hpp>
#include <common/stdlib.hpp>

namespace driver::graphics {
	namespace {
		graphics2d::Buffer framebuffer;
	}

	auto MultibootFramebuffer::_on_start() -> Try<> {
		auto multiboot = ::multiboot::multiboot1;

		if(!multiboot) return {"multiboot not available"};
		if(!multiboot->framebuffer_addr) return {"no multiboot framebuffer was provided"};

		switch(multiboot->framebuffer_type){
			case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
				return {"multiboot framebuffer is indexes, which is not supported "}; // :( We should probably support this mapped as RGB242 or grey8 something?
			break;
			case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
				// okay!
			break;
			case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
				return {"no graphical multiboot framebuffer was provided"};
			break;
		}

		graphics2d::BufferFormat format;
		graphics2d::BufferFormatOrder formatOrder;

		switch(multiboot->framebuffer_bpp){
			case 16:
				if(
					multiboot->framebuffer_red_mask_size==5&&
					multiboot->framebuffer_green_mask_size==6&&
					multiboot->framebuffer_blue_mask_size==5
				){
					if(
						multiboot->framebuffer_red_field_position==11&&
						multiboot->framebuffer_green_field_position==5&&
						multiboot->framebuffer_blue_field_position==0	
					){
						format = graphics2d::BufferFormat::rgb565;
						formatOrder = graphics2d::BufferFormatOrder::bgra;
						break;

					}else if(
						multiboot->framebuffer_blue_field_position==11&&
						multiboot->framebuffer_green_field_position==5&&
						multiboot->framebuffer_red_field_position==0	
					){
						format = graphics2d::BufferFormat::rgb565;
						formatOrder = graphics2d::BufferFormatOrder::argb;
						break;
					}
				}

				return {"unsupported 16bit framebuffer rgb configuration"};
			break;
			case 24:
				if(
					multiboot->framebuffer_red_mask_size==8&&
					multiboot->framebuffer_green_mask_size==8&&
					multiboot->framebuffer_blue_mask_size==8
				){
					if(
						multiboot->framebuffer_red_field_position==16&&
						multiboot->framebuffer_green_field_position==8&&
						multiboot->framebuffer_blue_field_position==0
					){
						format = graphics2d::BufferFormat::rgb8;
						formatOrder = graphics2d::BufferFormatOrder::bgra;
						break;

					}else if(
						multiboot->framebuffer_blue_field_position==16&&
						multiboot->framebuffer_green_field_position==8&&
						multiboot->framebuffer_red_field_position==0
					){
						format = graphics2d::BufferFormat::rgb8;
						formatOrder = graphics2d::BufferFormatOrder::argb;
						break;
					}
				}

				return {"unsupported 16bit framebuffer rgb configuration"};
			break;
			case 32:
				if(
					multiboot->framebuffer_red_mask_size==8&&
					multiboot->framebuffer_green_mask_size==8&&
					multiboot->framebuffer_blue_mask_size==8
				){
					if(
						multiboot->framebuffer_red_field_position==16&&
						multiboot->framebuffer_green_field_position==8&&
						multiboot->framebuffer_blue_field_position==0
					){
						format = graphics2d::BufferFormat::rgba8;
						formatOrder = graphics2d::BufferFormatOrder::bgra;
						break;

					}else if(
						multiboot->framebuffer_blue_field_position==24&&
						multiboot->framebuffer_green_field_position==16&&
						multiboot->framebuffer_red_field_position==8
					){
						format = graphics2d::BufferFormat::rgba8;
						formatOrder = graphics2d::BufferFormatOrder::argb;
						break;
					}
				}

				return {"unsupported 16bit framebuffer rgb configuration"};
			break;
			default:
				return {"unsupported 16bit framebuffer rgb configuration"};
			break;
		}

		TRY(api.subscribe_memory((void*)multiboot->framebuffer_addr, multiboot->framebuffer_pitch*multiboot->framebuffer_width, mmu::Caching::writeCombining));

		framebuffer.format = format;
		framebuffer.order = formatOrder;
		framebuffer.address = (U8*)multiboot->framebuffer_addr;
		framebuffer.stride = multiboot->framebuffer_pitch;
		framebuffer.width = multiboot->framebuffer_width;
		framebuffer.height = multiboot->framebuffer_height;

		return {};
	}

	auto MultibootFramebuffer::_on_stop() -> Try<> {
		return {};
	}

	auto MultibootFramebuffer::get_mode_count(U32 framebufferId) -> U32 {
		if(framebufferId>0) return 0; // not supported

		return 1;
	}

	auto MultibootFramebuffer::get_mode(U32 framebufferId, U32 index) -> Mode {
		if(framebufferId>0) return { 0 }; // not supported
		if(index>0) return { 0 };

		return { framebuffer.width, framebuffer.height, framebuffer.format };
	}

	auto MultibootFramebuffer::set_mode(U32 framebufferId, U32 index) -> Try<> {
		if(framebufferId>0) return {"Invalid framebuffer id"};
		if(index>0) return {"Invalid mode id"};

		// nothing to do, keeping mode 0
		return {};
	}

	auto MultibootFramebuffer::get_framebuffer_count() -> U32 {
		return 1;
	}

	auto MultibootFramebuffer::get_framebuffer(U32 index) -> graphics2d::Buffer* {
		if(index>0) return nullptr; // not supported

		return framebuffer.address?&framebuffer:nullptr;
	}

	auto MultibootFramebuffer::get_framebuffer_name(U32 index) -> const char* {
		if(index>0) return nullptr; // not supported

		return "framebuffer";
	}
}
