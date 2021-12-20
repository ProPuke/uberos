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

				// get_firmware_revision = 0x00001,
				// set_cursor_info = 0x08010,
				// set_cursor_state = 0x08011,
				// get_board_model = 0x10001,
				get_board_revision = 0x10002,
				// get_mac_address = 0x10003,
				// get_board_serial = 0x10004,
				get_arm_memory = 0x10005,
				get_vc_memory = 0x10006,
				// set_power_state = 0x28001,
				// get_clock_rate = 0x30002,
				// get_max_clock_rate = 0x30004,
				// get_temperature = 0x30006,
				// get_min_clock_rate = 0x30007,
				// get_turbo = 0x30009,
				// get_max_temperature = 0x3000A,
				// get_edid_block = 0x30020,
				set_clock_rate = 0x38002,
				// set_turbo = 0x38009,
				// set_set_gpio_state = 0x38041,

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

			// struct __attribute__((packed)) PropertyMessage {
			struct PropertyMessage {
				PropertyTag tag;

				union {
					U32 boardRevision;

					struct {
						U32 clockId;
						U32 rate; //hz
						U32 skipSettingTurbo;
					} clock_rate;

					U32 allocate_align;
					struct {
						U32 fb_addr;
						U32 fb_size;
					} allocate_res;

					struct {
						U32 address;
						U32 size;
					} memory;

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
