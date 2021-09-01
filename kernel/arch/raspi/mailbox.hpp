#pragma once

#include "mmio.hpp"
#include <common/types.hpp>

namespace arch {
	namespace raspi {
		namespace mailbox {
			enum struct Channel: U8 {
				framebuffer = 1,
				property = 8
			};

			enum struct PropertyTag: U32 {
				null_tag = 0,

				set_clock_rate = 0x38002,

				allocate_buffer = 0x40001,
				release_buffer  = 0x48001,
				get_physical_dimensions = 0x40003,
				set_physical_dimensions = 0x48003,
				get_virtual_dimensions = 0x40004,
				set_virtual_dimensions = 0x48004,
				get_bits_per_pixel = 0x40005,
				set_bits_per_pixel = 0x48005,
				set_pixel_order = 0x48006,
				get_bytes_per_row = 0x40008
			};

			struct __attribute__((packed)) PropertyMessage {
				PropertyTag tag;

				union {
					struct {
						U32 clockId;
						U32 rate; //hz
						U32 skipSettingTurbo;
					} clock_rate;

					U32 allocate_align;
					struct {
						void * fb_addr;
						U32 fb_size;
					} allocate_res;

					struct {
						U32 width;
						U32 height;
					} screenSize;

					U32 bitsPerPixel;
					U32 bytesPerRow;
				} data;
			};

			U32 read(Channel channel, U32 defaultValue);

			void send(Channel channel, U32 data);

			bool send_messages(PropertyMessage *tags);
		}
	}
}

#include "mailbox.inl"
