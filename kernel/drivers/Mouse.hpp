#pragma once

#include <kernel/drivers/Hardware.hpp>

#include <common/EventEmitter.hpp>

namespace driver {
	struct Mouse: Hardware {
		DRIVER_TYPE(Mouse, "mouse", "Mouse Driver", Hardware)

		struct Event {
			Mouse *instance;

			enum struct Type {
				moved,
				pressed,
				released,
				scrolled,
			} type;

			union {
				struct {
					I32 x;
					I32 y;
				} moved;

				struct {
					U32 button;
				} pressed;

				struct {
					U32 button;
				} released;

				struct {
					I32 distance;
				} scrolled;
			};
		};

		static inline EventEmitter<Event> allEvents;
		EventEmitter<Event> events;

		static const U32 maxButtons = 5;

		bool buttonState[maxButtons];
	};
}
