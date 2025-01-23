#include "mailbox.hpp"

#include <kernel/Log.hpp>
#include <kernel/mmio.hpp>

#include <alloca.h>

static Log log("mailbox");

namespace arch {
	namespace raspi {
		namespace mailbox {
			enum struct BufferReqResCode: U32 {
				request = 0x00000000,
				response_success = 0x80000000,
				response_error = 0x80000001,
			};

			struct __attribute__((packed)) PropertyMessageBuffer {
				U32 size;
				BufferReqResCode req_res_code;
				U32 tags[1];
			};

			unsigned get_PropertyTag_size(PropertyTag tag) {
				switch(tag){
					case PropertyTag::get_board_revision:
						return 4;

					case PropertyTag::get_arm_memory:
					case PropertyTag::get_vc_memory:
						return 8;

					case PropertyTag::allocate_buffer: 
					case PropertyTag::get_physical_dimensions:
					case PropertyTag::test_physical_dimensions:
					case PropertyTag::set_physical_dimensions:
					case PropertyTag::get_virtual_dimensions:
					case PropertyTag::test_virtual_dimensions:
					case PropertyTag::set_virtual_dimensions:
						return 8;

					case PropertyTag::get_clock:
					case PropertyTag::get_actual_clock:
					case PropertyTag::get_min_clock:
					case PropertyTag::get_max_clock:
						return sizeof(PropertyMessage::Data::getClockResult);

					case PropertyTag::set_clock:
						return sizeof(PropertyMessage::Data::setClock);

					case PropertyTag::get_voltage:
					case PropertyTag::get_min_voltage:
					case PropertyTag::get_max_voltage:
						return sizeof(PropertyMessage::Data::getVoltageResult);

					case PropertyTag::set_voltage:
						return sizeof(PropertyMessage::Data::setVoltage);

					case PropertyTag::get_temperature:
					case PropertyTag::get_max_temperature:
						return sizeof(PropertyMessage::Data::getTemperatureResult);

					case PropertyTag::get_bits_per_pixel:
					case PropertyTag::test_bits_per_pixel:
					case PropertyTag::set_bits_per_pixel:
					case PropertyTag::get_pixel_order:
					case PropertyTag::test_pixel_order:
					case PropertyTag::set_pixel_order:
					case PropertyTag::get_alpha_mode:
					case PropertyTag::test_alpha_mode:
					case PropertyTag::set_alpha_mode:
					case PropertyTag::get_bytes_per_row:
						return 4;

					case PropertyTag::get_virtual_offset:
					case PropertyTag::test_virtual_offset:
					case PropertyTag::set_virtual_offset:
						return sizeof(PropertyMessage::Data::virtualOffset);

					case PropertyTag::get_overscan_offset:
					case PropertyTag::test_overscan_offset:
					case PropertyTag::set_overscan_offset:
						return sizeof(PropertyMessage::Data::screenOverscan);

					// case PropertyTag::get_palette:
					// 	return sizeof(PropertyMessage::Data::existingPalette);
					// case PropertyTag::test_palette:
					// case PropertyTag::set_palette:
					// 	return sizeof(PropertyMessage::Data::newPalette); //FIXME: Technically 24..1032. We might need to handle this dynamically based on data

					case PropertyTag::set_cursor_info:
						return sizeof(PropertyMessage::Data::cursorInfo);
					case PropertyTag::set_cursor_state:
						return sizeof(PropertyMessage::Data::cursorState);

					#ifdef ARCH_RASPI3
						case PropertyTag::set_display_gamma:
							return sizeof(PropertyMessage::Data::gamma);
					#endif

					case PropertyTag::release_buffer:
					case PropertyTag::null_tag:
						return 0;
				}

				return 0; //switch above should catch all cases
			}

			bool send_messages(PropertyMessage *tags) {
				auto messageSize = 0u;

				for(auto tag=tags;tag->tag!=PropertyTag::null_tag;tag++){
					messageSize += sizeof(U32)*3 + get_PropertyTag_size(tag->tag);
				}

				messageSize += sizeof(U32)*3; //end tag header
				messageSize = align(messageSize, 16);

				auto *message = align((PropertyMessageBuffer*)alloca(messageSize+15), 16);

				message->size = messageSize;
				message->req_res_code = BufferReqResCode::request;

				{
					auto i = 0u;
					for(auto tag=tags;tag->tag!=PropertyTag::null_tag;tag++){
						auto size = get_PropertyTag_size(tag->tag);
						message->tags[i++] = (U32)tag->tag;
						message->tags[i++] = size;
						message->tags[i++] = 0;
						memcpy(&message->tags[i], &tag->data, size);
						i += size/4;
					}
					message->tags[i] = 0;
				}

				// asm volatile("" ::: "memory"); //ensure message is written to

				{
					mmio::PeripheralWriteGuard _guard;
					send(Channel::property, ((size_t)message)>>4);
				}

				{
					mmio::PeripheralReadGuard _guard;
					if(read(Channel::property, 0xffffffff) == 0xffffffff){
						log.print_warning("Error: Mailbox did not respond");
						return false;
					}
				}

				if(message->req_res_code==BufferReqResCode::request){
					log.print_warning("Error: Mailbox did not respond with anything useful");
					return false;
				}

				if(message->req_res_code==BufferReqResCode::response_error){
					log.print_warning("Error: Mailbox returned an error"); //TODO:print code?
					return false;
				}

				{
					auto i = 0u;
					for(auto tag=tags;tag->tag!=PropertyTag::null_tag;tag++){
						auto size = get_PropertyTag_size(tag->tag);
						i += 3;
						memcpy(&tag->data, &message->tags[i], size);
						i += size/4;
					}
				}

				return true;
			}
		}
	}
}
