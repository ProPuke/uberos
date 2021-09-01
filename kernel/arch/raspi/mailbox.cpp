#include "mailbox.hpp"

#include <kernel/stdio.hpp>
#include <alloca.h>

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
					case PropertyTag::set_clock_rate:
						return 12;

					case PropertyTag::allocate_buffer: 
					case PropertyTag::get_physical_dimensions:
					case PropertyTag::set_physical_dimensions:
					case PropertyTag::get_virtual_dimensions:
					case PropertyTag::set_virtual_dimensions:
						return 8;
					case PropertyTag::get_bits_per_pixel:
					case PropertyTag::set_bits_per_pixel:
					case PropertyTag::get_bytes_per_row:
					case PropertyTag::set_pixel_order:
						return 4;
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

				messageSize += (messageSize%16)?16-(messageSize%16):0; //16 byte align

				PropertyMessageBuffer *message = (PropertyMessageBuffer*)alloca(messageSize+15);
				message = (PropertyMessageBuffer*)(((size_t)message)+(true?16-(((size_t)message)%16):0));

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

				asm volatile("" ::: "memory"); //ensure message is written to

				send(Channel::property, ((size_t)message)>>4);

				if(read(Channel::property, 0xffffffff) == 0xffffffff){
					stdio::print_info("Error: Mailbox did not respond");
					return false;
				}

				if(message->req_res_code==BufferReqResCode::request){
					stdio::print_info("Error: Mailbox did not respond with anything useful");
					return false;
				}

				if(message->req_res_code==BufferReqResCode::response_error){
					stdio::print_info("Error: Mailbox returned an error"); //TODO:print code?
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
