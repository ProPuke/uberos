#include "Vbe.hpp"

#include <drivers/x86/system/Gdt.hpp>

#include <kernel/arch/x86-ibm/memory.hpp>
#include <kernel/drivers.hpp>
// #include <kernel/arch/x86/vm86.hpp>

#include <common/Box.hpp>
#include <common/graphics2d/BufferFormat.hpp>
#include <common/PodArray.hpp>
#include <common/stdlib.hpp>

// #define VBE_ATI_WORKAROUND
// #define VBE_NVIDIA_WORKAROUND

namespace driver::graphics {
	namespace {
		namespace vbe {
			struct __attribute__((packed)) ControllerInfo {
				C8 vbeSignature[4];  // "VESA"
				U16 vbeVersion;      // major/minor. i.e 0x0300 for VBE 3.0
				P32<C8> oemString;
				U32 capabilities;    // bitfield
				P32<U16> videoModes; // U16 array, ending in 0xffff
				U16 totalMemory;     // measured in 64KB blocks

				// VESA 2.x
				U16 oemSoftwareRev;
				P32<C8> oemVendorName;
				P32<C8> oemProductName;
				P32<C8> oemProductRev;
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

			struct __attribute__((packed)) ProtectedModeInfo {
				C8 signature[4]; // "PMID"
				P16<void(*)()> entrypoint;
				P16<void(*)()> initialise;
				U16 biosDataSelector;
				U16 a0000Selector;
				U16 b0000Selector;
				U16 b8000Selector;
				U16 codeSegmentSelector;
				bool inProtectedMode;
				U8 checksum;
			};

			struct __attribute__((packed)) Edid {
				U16 dotClock;          // pixel clock / 10000
				U8 xresLo;
				U16 hblank:12;        // blank = front porch + sync width + back porch
				U8 xresHi:4;
				U8 yresLo;
				U16 vblank:12;
				U8 yresHi:4;
				U8 hoverplusLo;       // overplus = front porch
				U8 hsyncwidthLo;
				U8 vsyncwidthLo:4;
				U8 voverplusLo:4;
				U8 vsyncwidthHi:2;
				U8 voverplusHi:2;
				U8 hsyncwidthHi:2;
				U8 hoverplusHi:2;
				U8 _reserved[6];
			};

			enum struct CRTC_flags:U8 {
				doubleScan = 1<<0,
				interlaced = 1<<1,
				hsyncNegative = 1<<2,
				vsyncNegative = 1<<3
			};

			struct __attribute__((packed)) CRTC_infoblock {
				U16 horizontalTotal;
				U16 horizontalSyncStart;
				U16 horizontalSyncEnd;
				U16 verticalTotal;
				U16 verticalSyncStart;
				U16 verticalSyncEnd;
				CRTC_flags flags;
				U32 pixelClock;  // hz
				U16 refreshRate; // * 0.01Hz
				U8  _reserved[40];
			};

			const auto vesa_code_size = 0x10000;
			const auto vesa_stack_size = 0x2000;
			const auto vesa_data_size = 0x600;

			U8 bios[vesa_code_size];

			// Edid *atiEdid = nullptr;
			ProtectedModeInfo *protectedModeInfo = nullptr;

			auto init() -> void {
				memcpy(bios, (void*)arch::x86_ibm::memory::videoBios, sizeof(bios));

				for(auto i=0u;i<sizeof(bios);i++){
					auto &modeInfo = *(ProtectedModeInfo*)&bios[i];
					if(modeInfo.signature[0]!='P'||modeInfo.signature[1]!='M'||modeInfo.signature[2]!='I'||modeInfo.signature[3]!='D') continue;

					U8 checksum = 0;
					for(auto i2=0u;i2<sizeof(ProtectedModeInfo);i2++){
						checksum += ((U8*)&modeInfo)[i2];
					}

					// invalid checksum
					if(checksum!=0) {
						#ifdef VBE_ATI_WORKAROUND
							// some old ati's cock up their checksum, so specially handle these
							if(memmem(bios, sizeof(bios), "ATI Technologies Inc.", 21)){
								log.print_warning("Likely broken ATI implementation found - Workaround deployed");
								auto edidAddress = *(U16*)&bios[*(U16*)&bios[*(U16*)&bios[0x48]+0x34]+0x2];
								vesa3SetMode = vesa3SetMode_ati;
								atiEdid = (Edid*)&bios[edidAddress];
								*(U32*)(&bios[4]) = 0;
							}else{
								continue;
							}
						#else
							continue;
						#endif
					}

					// found!
					protectedModeInfo = &modeInfo;
					break;
				}

				if(protectedModeInfo){
					Vbe::instance.log.print_info("3.0 protected mode info found @ ", protectedModeInfo);

					#ifdef VBE_NVIDIA_WORKAROUND
						const auto jmp = 0xe9;
						const auto retf = 0xcb;
						const auto ret_data16 = 0xc366;

						auto isNvidiaPatch = false;

						// detect possibly broken nvidia implementation...
						if(bios[protectedModeInfo->initialise]==jmp){
							auto ins = &bios[protectedModeInfo->initialise+3] + *(U16*)&bios[protectedModeInfo->initialise+1];
							// mov ds,[cs:0x6e]; call ...; mov ax,[cs:0x66]
							if (*(U32*)ins==0x6e1e8e2e && *(U16*)(ins+4)==0xe800 && *(U32*)(ins+8)==0x0066a12e) {
								ins += 12;

								// if it uses a ret next instead of a retf, we've found the bad case
								while(ins<=bios+0x8000-2 &&*ins!=retf &&*(U16*)ins!=ret_data16) ins++;

								if(*(U16*)ins==ret_data16){
									log.print_warning("Likely broken NVIDIA implementation. Attempting to patch...");
									vesa3SetMode = vesa3SetMode_nvidia;
									isNvidiaPatch = true;
								}
							}
						}
					#endif

					// auto biosData = new U8[vesa_data_size]; memset(biosData, 0, vesa_data_size);
					// auto biosStack = new U8[vesa_stack_size];

					// auto gdt = drivers::find_and_activate<driver::system::Gdt>(&Graphics::Vbe::instance);
					// if(!gdt){
					// 	return {"Unable to connect to GDT"};
					// }

					// protectedModeInfo->biosDataSelector = gdt->add_entry((U32)biosData, vesa_data_size-1, 0b10010010, driver::system::Gdt::DescriptorSize::_16bit, false);
					// protectedModeInfo->codeSegmentSelector = gdt->add_entry((U32)bios, vesa_code_size-1, 0b10010010, driver::system::Gdt::DescriptorSize::_16bit, false);

					// auto codeSelector = gdt->add_entry((U32)bios, vesa_code_size-1, 0b10011010, driver::system::Gdt::DescriptorSize::_16bit, false);
					// auto stackSelector = gdt->add_entry((U32)biosStack, vesa_stack_size-1, 0b10010010, driver::system::Gdt::DescriptorSize::_16bit, false);

					// protectedModeInfo->a0000Selector = gdt->add_entry(video_mem, 0x10000-1, 0b10010010, driver::system::Gdt::DescriptorSize::_16bit, false);
					// protectedModeInfo->b0000Selector = gdt->add_entry(video_mem+0x10000, 0x10000-1, 0b10010010, driver::system::Gdt::DescriptorSize::_16bit, false);
					// protectedModeInfo->b8000Selector = gdt->add_entry(video_mem+0x18000, 0x8000-1, 0b10010010, driver::system::Gdt::DescriptorSize::_16bit, false);
				}
			}

			auto initRealmode() -> Box32<ControllerInfo> {
				// Box32<ControllerInfo> info = new ControllerInfo;

				// ((long)info.get()&0xff0000)>>16

				auto info = (ControllerInfo*)memory::lowMemory.address;

				U16 result;
				asm volatile(
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

			struct __attribute__((packed)) s_regs {
				U16 ax;
				U16 bx;
				U16 cx;
				U16 dx;
				U16 si;
				U16 di;
				U16 es;
			};

			// void call(s_regs *regs) {
			// 	asm volatile(R"(
			// 		pushf
			// 		cli
			// 		push es
			// 		pusha
			// 		mov esi, esp
			// 		push sel_stack
			// 		push (VESA_STACK_SIZE-4)
			// 		call i386_stack_switch
			// 		push KERNEL_DATA_SEG
			// 		push esi
			// 		push edi
			// 		push ds
			// 		mov dx, [edi + 12]
			// 		mov es, dx
			// 		mov ax, [edi]
			// 		mov bx, [edi + 2]
			// 		mov cx, [edi + 4]
			// 		mov dx, [edi + 6]
			// 		mov si, [edi + 8]
			// 		mov di, [edi + 10]
			// 		lcall farcall
			// 		pop ds
			// 		pop ebp
			// 		mov ds:[ebp], ax
			// 		mov eax, ebp
			// 		mov [eax + 2], bx
			// 		mov [eax + 4], cx
			// 		mov [eax + 6], dx
			// 		mov [eax + 8], si
			// 		mov [eax + 10], di
			// 		mov bx, es
			// 		mov [eax + 12], bx
			// 		call i386_stack_switch
			// 		popa
			// 		pop es
			// 		popf
			// 		:
			// 		: "D" (regs)
			// 	)");
			// }

			// auto set_mode_CRTC_generic(U16 mode, CRTC_infoblock *crtc) -> U16 {
			// /*  vesa3_CRTC_infoblock CRTC_info_block = {
			// 		640+16+64+80, 640+16, 640+16+64,
			// 		480+3+4+13, 480+3, 480+3+4,
			// 		VESA3_CRTC_HSYNC_NEGATIVE, 23750000, 5938, {} };*/
			// 	struct s_regs regs;

			// 	regs.ax = 0x4f02;
			// 	regs.bx = mode | 0xc000; //| 0xc800;
			// 	regs.es = 0;
			// /*  regs.es = i386_selector_add( SELECTOR(&CRTC_info_block,sizeof(CRTC_info_block)-1,DATA_w,false) );
			// 	regs.di = 0;*/
			// 	call(&regs);

			// 	return regs.ax;
			// }

			auto get_mode_info(int modeIndex) -> Box32<ModeInfo> {
				Box32<ModeInfo> info = new ModeInfo;

				U16 result;
				asm volatile(
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

			auto set_mode(Mode mode) -> Try<> {
				U16 result;
				asm volatile(
					"mov ax, 0x4f02\n"
					"mov ebx, %1\n"
					"int 0x10\n"
					"mov %0, ax\n"
					: "=r" (result)
					: "r" (mode.index&0b0011111111111111 | mode.linearBuffer&0b0100000000000000 | mode.clear&0b1000000000000000)
				);

				if(result){
					return {"Unable to switch to mode"}; // ?
				}

				return {};
			}

			auto get_current_mode() -> Optional<Mode> {
				U16 error;
				U16 result;

				asm volatile(
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
			Graphics::Mode framebufferMode;
		};
	
		graphics2d::Buffer framebuffer;
		PodArray<Mode> modes;
	}

	auto Vbe::_on_start() -> Try<> {
		framebuffer.address = TRY_RESULT(api.subscribe_memory<U8>(Physical<void>{0x4f00}, 0x123, mmu::Caching::writeCombining));

		modes.clear();

		auto info = vbe::initRealmode();
		if(!info){
			return {"Unable to initialise VBE"};
		}

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

				log.print_warning("Mode ", modeIndex, " unsupported - ", modeInfo->width, 'x', modeInfo->height, ", ", modeInfo->bpp, " bpp");

				modes.push_back(graphics::Mode{
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

		return set_mode(0, 1280, 720, graphics2d::BufferFormat::rgba8, true);
	}

	auto Vbe::_on_stop() -> Try<> {
		return {};
	}

	auto Vbe::get_mode_count(U32 framebufferId) -> U32 {
		if(framebufferId>0) return 0; // not supported

		return modes.length;
	}

	auto Vbe::get_mode(U32 framebufferId, U32 index) -> Mode {
		if(framebufferId>0) return { 0 }; // not supported
		if(index>=modes.length) return { 0 };

		return modes[index].framebufferMode;
	}

	auto Vbe::set_mode(U32 framebufferId, U32 index) -> Try<> {
		if(framebufferId>0) return {"Invalid framebuffer id"};
		if(index>=modes.length) return {"Invalid mode id"};

		auto &mode = modes[index];

		//TODO: unsub previous mmeory and then try to subscribe to this new region. If that fails, attempt a rollback, or die

		return vbe::set_mode({mode.vbeModeIndex, true, true});
	}

	auto Vbe::get_framebuffer_count() -> U32 {
		return 1;
	}

	auto Vbe::get_framebuffer(U32 index) -> graphics2d::Buffer* {
		if(index>0) return nullptr; // not supported

		return &framebuffer;
	}

	auto Vbe::get_framebuffer_name(U32 index) -> const char* {
		if(index>0) return nullptr; // not supported

		return "framebuffer";
	}
}
