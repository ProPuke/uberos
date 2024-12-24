#include "VBE.hpp"

#include <kernel/Log.hpp>
#include <kernel/PodArray.hpp>
// #include <kernel/arch/x86/vm86.hpp>

#include <common/Box.hpp>
#include <common/graphics2d/BufferFormat.hpp>
#include <common/stdlib.hpp>

static Log log("vbe");

namespace driver {
	namespace graphics {
		namespace {
			namespace vbe {
				struct __attribute__((packed)) ControllerInfo {
					U8 vbeSignature[4];  // "VESA"
					U16 vbeVersion;      // major/minor. i.e 0x0300 for VBE 3.0
					P32<U8> oemString;
					U32 capabilities;    // bitfield
					P32<U16> videoModes; // U16 array, ending in 0xffff
					U16 totalMemory;     // measured in 64KB blocks

					// VESA 2.x
					U16 oemSoftwareRev;
					P32<U8> oemVendorName;
					P32<U8> oemProductName;
					P32<U8> oemProductRev;
					U8 reserved[222];

					// VBE 2.0
					U8 oemData[256];
				};

				struct __attribute__((packed)) ModeInfo {
					struct __attribute__((packed)) {
						U8 supported:1;
						U8 _reserved1:1;   // in vbe 1.x this meant extra info, but that's now mandatory
						U8 ttySupported:1;
						U8 colourMode:1;   // vs monochrome
						U8 graphicsMode:1; // vs text
						U8 vgaIncompatible:1;
						U8 vgaWindowModeIncompatible:1;
						U8 linear:1;
						U8 _unused2:8;
					} attributes;

					U8 window_a_attributes;  // deprecated
					U8 window_b_attributes;  // deprecated
					U16 granularity;	     // deprecated; used while calculating bank numbers
					U16 window_size;
					U16 segment_a;
					U16 segment_b;
					U32 win_func_ptr;	     // deprecated; used to switch banks from protected mode without returning to real mode

					U16 stride;
					U16 width;			     // width in pixels
					U16 height;			     // height in pixels

					U8 w_char;			     // unused...
					U8 y_char;			     // ...
					U8 planes;
					U8 bpp;
					U8 banks;			     // deprecated; total number of banks in this mode
					enum struct MemoryModel: U8 {
						textMode,
						cgaGraphics,
						herculesGraphics,
						planar,
						packedPixel,
						nonChain4_256Colour,
						directColour,
						yuv
					} memoryModel;
					U8 bank_size;		     // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
					U8 image_pages;
					U8 reserved0;

					U8 red_mask;
					U8 red_position;
					U8 green_mask;
					U8 green_position;
					U8 blue_mask;
					U8 blue_position;
					U8 reserved_mask;
					U8 reserved_position;
					U8 direct_color_attributes;

					U32 framebuffer;		 // physical address of the linear frame buffer; write here to draw to the screen
					U32 off_screen_mem_off;
					U16 off_screen_mem_size; // size of memory in the framebuffer but not being displayed on the screen

					U8 reserved1[206];
				};

				struct Mode {
					unsigned index;
					bool linearBuffer; // uses a linear framebuffer
					bool clear; // guarenteed to be cleared on acquisition (often auto-cleared anyway)
				};

				Framebuffer framebuffer;

				auto init() -> Box32<ControllerInfo> {
					// Box32<ControllerInfo> info = new ControllerInfo;

					// ((long)info.get()&0xff0000)>>16

					auto info = (ControllerInfo*)memory::lowMemory;

					U16 result;
					asm(
						"mov ax, 0\n"
						"mov es, ax\n"
						"mov ax, 0x100\n"
						"mov es, ax\n"
						"mov ax, %1\n"
						"mov di, %3\n"
						"mov es, %2\n"
						"int 0x10\n"
						"mov %0, ax\n"
						: "=r" (result)
						: "r" ((U16)0x4f00), "r" (U16((long)info>>16&0xffff)), "r" (U16((long)info&0xffff))
					);

					if(result){
						return {};
					}

					return info;
				}

				auto get_mode_info(int modeIndex) -> Box32<ModeInfo> {
					Box32<ModeInfo> info = new ModeInfo;

					U16 result;
					asm(
						"mov ax, %1\n"
						"mov es, %2\n"
						"mov di, %3\n"
						"int 0x10\n"
						"mov %0, ax\n"
						: "=r" (result)
						: "r" ((U16)0x4f01), "r" (U16((long)(info.get())>>16&0xffff)), "r" (U16((long)(info.get())&0xffff))
					);

					if(result){
						return {};
					}

					return info;
				}

				auto set_mode(Mode mode) -> bool {
					U16 result;
					asm(
						"mov ax, 0x4f02\n"
						"mov ebx, %1\n"
						"int 0x10\n"
						"mov %0, ax\n"
						: "=r" (result)
						: "r" (mode.index&0b0011111111111111 | mode.linearBuffer&0b0100000000000000 | mode.clear&0b1000000000000000)
					);

					if(result){
						return false;
					}

					return true;
				}

				auto get_current_mode() -> Optional<Mode> {
					U16 error;
					U16 result;

					asm(
						"mov ax, 0x4f03\n"
						"int 0x10\n"
						"mov %0, ax\n"
						"mov %1, bx\n"
						: "=r" (error), "=r" (result)
						:
					);

					if(error){
						return {};
					}

					Mode mode;
					mode.index = bits(result, 0, 13);
					mode.linearBuffer = bits(result, 14, 14);
					mode.clear = bits(result, 15, 15);

					return mode;
				}
			}

			struct Mode {
				U16 vbeModeIndex;
				framebuffer::Mode framebufferMode;
			};
		
			Framebuffer framebuffer;
			PodArray<Mode> modes;
		}

		DriverType vbeGraphicsType{"VBE"};

		/**/ VBE::VBE():
			Graphics("VESA BIOS Extensions (VBE)", "video driver")
		{
			vbeGraphicsType.parentType = type;
			type = &vbeGraphicsType;
		}

		auto VBE::_on_start() -> bool {
			if(!api.subscribe_memory((void*)0x4f00, 0x123)) return false;

			//TODO: only allow one of these drivers active at once?
			framebuffer.driver = nullptr;

			modes.clear();

			auto info = vbe::init();
			if(!info){
				//TODO: error
				return false;
			}

			return false;


			for(auto modeIndex = info->videoModes.get(); *modeIndex!=0xff; modeIndex++) {
				auto modeInfo = vbe::get_mode_info(*modeIndex);
				if(modeInfo){
					if(!modeInfo->attributes.linear) continue;
					if(modeInfo->memoryModel!=vbe::ModeInfo::MemoryModel::directColour) continue;

					//TODO: check this memory region is available and not in use by other drivers?

					graphics2d::BufferFormat format;

					switch(modeInfo->bpp){
						case 8:
							if(
								modeInfo->red_mask==8&&modeInfo->red_position==0
							){
								format = graphics2d::BufferFormat::grey8;
							}else{
								goto next;
							}
						break;
						case 16:
							if(
								modeInfo->red_mask==5&&modeInfo->red_position==11&&
								modeInfo->green_mask==6&&modeInfo->green_position==5&&
								modeInfo->blue_mask==5&&modeInfo->blue_position==0
							){
								format = graphics2d::BufferFormat::rgb565;
							}else{
								goto next;
							}
						break;
						case 24:
							if(
								modeInfo->red_mask==8&&modeInfo->red_position==16&&
								modeInfo->green_mask==8&&modeInfo->green_position==8&&
								modeInfo->blue_mask==8&&modeInfo->blue_position==0
							){
								format = graphics2d::BufferFormat::rgb8;
							}else{
								goto next;
							}
						break;
						case 32:
							if(
								modeInfo->red_mask==8&&modeInfo->red_position==24&&
								modeInfo->green_mask==8&&modeInfo->green_position==16&&
								modeInfo->blue_mask==8&&modeInfo->blue_position==8
							){
								format = graphics2d::BufferFormat::rgba8;
							}else{
								goto next;
							}
						break;
						default:
							goto next;
					}

					log.print_warning("VBE: Mode ", modeIndex, " unsupported - ", modeInfo->width, 'x', modeInfo->height, ", ", modeInfo->bpp, " bpp");

					modes.push_back(Mode {
						vbeModeIndex: *modeIndex,
						framebufferMode: {
							width: modeInfo->width,
							height: modeInfo->height,
							format: format
						}
					});

					next:;
				}
			}

			set_mode(0, 1280, 720, graphics2d::BufferFormat::rgba8, true);

			return true;
		}

		auto VBE::_on_stop() -> bool {
			if(framebuffer.driver==this){
				framebuffer.driver = nullptr;
			}

			return true;
		}

		auto VBE::get_mode_count(U32 framebufferId) -> U32 {
			if(framebufferId>0) return 0; // not supported

			return modes.length;
		}

		auto VBE::get_mode(U32 framebufferId, U32 index) -> framebuffer::Mode {
			if(framebufferId>0) return { 0 }; // not supported
			if(index>=modes.length) return { 0 };

			return modes[index].framebufferMode;
		}

		auto VBE::set_mode(U32 framebufferId, U32 index) -> bool {
			if(framebufferId>0) return false; // not supported
			if(index>=modes.length) return false;

			auto &mode = modes[index];

			//TODO: unsub previous mmeory and then try to subscribe to this new region. If that fails, attempt a rollback, or die

			return vbe::set_mode({mode.vbeModeIndex, true, true});
		}

		auto VBE::get_framebuffer_count() -> U32 {
			if(framebuffer.driver!=this) return 0;

			return 1;
		}

		auto VBE::get_framebuffer(U32 index) -> Framebuffer* {
			return index==0&&framebuffer.driver==this?&framebuffer:nullptr;
		}

		auto VBE::get_framebuffer_name(U32 index) -> const char* {
			if(index>0) return nullptr; // not supported
			if(framebuffer.driver!=this) return nullptr;

			return "framebuffer";
		}
	}
}
