#pragma once

#include <kernel/arch/raspi/mmio.hpp>

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
				// get_edid_block = 0x30020,
				// set_turbo = 0x38009,
				// set_set_gpio_state = 0x38041,

				allocate_buffer = 0x40001,
				release_buffer  = 0x48001,

				get_clock = 0x30002,
				get_actual_clock = 0x30047,
				get_min_clock = 0x30007,
				get_max_clock = 0x30004,
				set_clock = 0x38002,

				get_voltage = 0x30003,
				get_min_voltage = 0x30008,
				get_max_voltage = 0x30005,
				set_voltage = 0x38003,

				get_temperature = 0x30006,
				get_max_temperature = 0x3000a,

				get_physical_dimensions = 0x40003,
				test_physical_dimensions = 0x44003,
				set_physical_dimensions = 0x48003,

				get_virtual_dimensions = 0x40004,
				test_virtual_dimensions = 0x44004,
				set_virtual_dimensions = 0x48004,

				get_bits_per_pixel = 0x40005,
				test_bits_per_pixel = 0x44005,
				set_bits_per_pixel = 0x48005,

				get_pixel_order = 0x40006,
				test_pixel_order = 0x44006,
				set_pixel_order = 0x48006,

				get_alpha_mode = 0x40007,
				test_alpha_mode = 0x44007,
				set_alpha_mode = 0x48007,

				get_bytes_per_row = 0x40008,

				get_virtual_offset = 0x40009,
				test_virtual_offset = 0x44009,
				set_virtual_offset = 0x48009,

				get_overscan_offset = 0x4000a,
				test_overscan_offset = 0x4400a,
				set_overscan_offset = 0x4800a,

				// disabled as a bit too large in the union, for now (will need to pack differently)
				// get_palette = 0x4000b,
				// test_palette = 0x4400b,
				// set_palette = 0x4800b,

				set_cursor_info = 0x8010,
				set_cursor_state = 0x8011,

				#ifdef ARCH_RASPI3 //NOT 1-2 OR 4
					set_display_gamma = 0x8012,
				#endif
			};

			// struct __attribute__((packed)) PropertyMessage {
			struct PropertyMessage {
				PropertyTag tag;

				union Data {
					U32 boardRevision;

					U32 allocate_align;
					struct __attribute__((packed)) {
						U32 fb_addr;
						U32 fb_size;
					} allocate_res;

					struct __attribute__((packed)) {
						U32 address;
						U32 size;
					} memory;

					struct __attribute__((packed)) {
						U32 width;
						U32 height;
					} screenSize;

					enum struct __attribute__((packed)) PixelOrder:U32 {
						bgr,
						rgb
					} pixelOrder;

					enum struct __attribute__((packed)) AlphaMode:U32 {
						enabled  = 0x0, // opacity is 1..0
						reversed = 0x1, // opacity is 0..1
						ignore   = 0x2
					} alphaMode;

					struct __attribute__((packed)) {
						U32 x;
						U32 y;
					} virtualOffset;

					struct __attribute__((packed)) {
						U32 top;
						U32 bottom;
						U32 left;
						U32 right;
					} screenOverscan;

					// U32 existingPalette[256];

					// struct __attribute__((packed)) {
					// 	U32 offset; //0..255
					// 	U32 length; //1..255
					// 	U32 palette[256];
					// } newPalette;

					U32 bitsPerPixel;
					U32 bytesPerRow;

					struct __attribute__((packed)) {
						U32 width;
						U32 height;
						U32 _unused1;
						U32 pixelData; //pointer
						U32 hotspotX;
						U32 hotspotY;
					} cursorInfo;

					struct __attribute__((packed)) {
						U32 enable; //0..1
						U32 x;
						U32 y;
						U32 flags;
					} cursorState;

					enum struct __attribute__((packed)) Clock:U32 {
						min = 1,
						emmc = min,
						uart,
						arm,
						core,
						v3d,
						h264,
						isp,
						sdram,
						pixel,
						pwm,
						hevc,
						emmc2,
						m2mc,
						pixel_bvb,
						max = pixel_bvb
					};

					Clock getClock;

					struct __attribute__((packed)) {
						Clock clock;
						U32 rate; //hz
					} getClockResult;

					struct __attribute__((packed)) {
						Clock clock;
						U32 rate; //hz
						U32 skipSettingTurbo; //voltage, sdram and gpu freq will be boosted when setting above average rate UNLESS this flag is set
					} setClock;

					enum struct __attribute__((packed)) Voltage:U32 {
						min = 1,
						core = min,
						sdram_c,
						sdram_p,
						sdram_i,
						max = sdram_i
					};

					Voltage getVoltage;

					struct __attribute__((packed)) SetVoltage {
						Voltage voltage;
						U32 value; //microvolts

						/* NOTE: when setting, `value` has some special rules:
							     0..16    : relative to platform-specific typical voltage, in 25mV steps
							    17..499999: relative to platform-specific typical voltage, in microvolts
							500000..*     : absolute voltage in microvolts
						*/
					} setVoltage;

					SetVoltage getVoltageResult;

					enum struct __attribute__((packed)) Temperature:U32 {
						min = 0,
						soc = min, //presumably?
						max = soc
					};

					Temperature getTemperature;

					struct __attribute__((packed)) SetTemperature {
						Temperature temperature;
						U32 value; //millidegrees celcius
					};

					SetTemperature getTemperatureResult;

					#ifdef ARCH_RASPI3
						struct __attribute__((packed)) {
							U32 display;
							U32 data; //address of 768 byte gamma table: 256 bytes red, 256 bytes green, 256 blue
						} gamma;
					#endif
				} data;
			};

			U32 read(Channel channel, U32 defaultValue);

			void send(Channel channel, U32 data);

			bool send_messages(PropertyMessage *tags);
		}
	}
}

#include "mailbox.inl"
