#pragma once

#include "mailbox.hpp"

#include "mmio.hpp"
#include <common/types.hpp>

namespace arch {
	namespace raspi {
		namespace mailbox {
			enum Status {
				empty = 0x40000000,
				full = 0x80000000,
			};

			namespace mmio {
				static volatile U32 &mail0_read = *(U32*)raspi::mmio::Address::mail0_read;
				static volatile U32 &mail0_status = *(U32*)raspi::mmio::Address::mail0_status;
				static volatile U32 &mail0_write = *(U32*)raspi::mmio::Address::mail0_write;
			}

			union Message {
				struct {
					Channel channel:4;
					U32 data:28;
				};

				U32 as_int;
			};

			inline U32 read(Channel channel, U32 defaultValue) {
				Message message;
				
				while(true){
					U32 waits = 0;
					while(mmio::mail0_status & Status::empty){
						if(++waits>1<<25){
							return defaultValue;
						}
					}

					message.as_int = mmio::mail0_read;
					if(message.channel==channel) break;
				};

				return message.data;
			}

			inline void send(Channel channel, U32 data) {
				Message message = {channel, data};

				while(mmio::mail0_status & Status::full);

				mmio::mail0_write = message.as_int;
			}
		}
	}
}
