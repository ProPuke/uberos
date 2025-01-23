#pragma once

#include <kernel/drivers/Hardware.hpp>

#include <common/EventEmitter.hpp>

namespace driver {
	struct Keyboard: Hardware {
		DRIVER_TYPE(Keyboard, "keyboard", "Keyboard Driver", Hardware)

		struct Event {
			enum struct Type {
				pressed,
				released,
			} type;

			union {
				struct {
					U32 scancode;
					bool repeat;
				} pressed;

				struct {
					U32 scancode;
				} released;
			};
		};

		EventEmitter<Event> events;
	};
}
