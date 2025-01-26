#pragma once

#include <kernel/drivers/Hardware.hpp>
#include <kernel/keyboard.hpp>

#include <common/EventEmitter.hpp>

namespace driver {
	struct Keyboard: Hardware {
		DRIVER_TYPE_CUSTOM_CTOR(Keyboard, "keyboard", "Keyboard Driver", Hardware)

		/**/ Keyboard(DriverApi::Startup &startup):
			Super(startup)
		{
			layout = keyboard::layouts.head;
			DRIVER_DECLARE_INIT();
		}

		struct Event {
			Keyboard *instance;

			enum struct Type {
				pressed,
				released,
				actionPressed,
				characterTyped
			} type;

			union {
				struct {
					keyboard::Scancode scancode;
					bool repeat;
				} pressed;

				struct {
					keyboard::Scancode scancode;
				} released;

				struct {
					const char *action;
				} actionPressed;

				struct {
					C16 character;
				} characterTyped;
			};
		};

		keyboard::Layout *layout;

		virtual auto is_pressed(keyboard::Scancode) -> bool = 0;
		static auto is_pressed_on_any(keyboard::Scancode) -> bool;

		static inline EventEmitter<Event> allEvents;
		EventEmitter<Event> events;
	};
}

#include "Keyboard.inl"
