#pragma once

#include <kernel/drivers/common/system/DisplayManager.hpp>
#include <kernel/drivers/Software.hpp>
#include <kernel/keyboard.hpp>
#include <kernel/Thread.hpp>

#include <common/EventEmitter.hpp>
#include <common/LList.hpp>

namespace driver {
	struct Keyboard;
	struct Mouse;
}

namespace driver::system {
	//TODO: should graphics drivers also include an api for querying their active processor(s) drivers if present? This would allow us to work out what processor speeds and temps relate to this graphics adapter, which might be useful/neat
	struct DesktopManager: Software {
		DRIVER_INSTANCE(DesktopManager, "desktop", "DesktopManager", Software);
		
		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		struct Window {
			graphics2d::Buffer clientArea;

			struct Event {
				enum struct Type {
					keyPressed,
					keyReleased,
					actionPressed,
					characterTyped,

					mouseMoved,
					mousePressed,
					mouseReleased,
					mouseScrolled,
				} type;

				union {
					struct {
						driver::Keyboard *keyboard;
						keyboard::Scancode scancode;
						bool repeat;
						U8 modifiers;
					} keyPressed;

					struct {
						driver::Keyboard *keyboard;
						keyboard::Scancode scancode;
					} keyReleased;

					struct {
						driver::Keyboard *keyboard;
						const char *action;
					} actionPressed;

					struct {
						driver::Keyboard *keyboard;
						C16 character;
					} characterTyped;

					struct {
						driver::Mouse *mouse;
						I32 x;
						I32 y;
						I32 motionX;
						I32 motionY;
					} mouseMoved;

					struct {
						driver::Mouse *mouse;
						I32 x;
						I32 y;
						U32 button;
					} mousePressed;

					struct {
						driver::Mouse *mouse;
						I32 x;
						I32 y;
						U32 button;
					} mouseReleased;

					struct {
						driver::Mouse *mouse;
						I32 x;
						I32 y;
						I32 distance;
					} mouseScrolled;
				};
			};

			EventEmitter<Event> events;

			virtual void set_title(const char*) = 0;
			virtual void set_status(const char*) = 0;

			virtual auto get_x() -> I32 = 0;
			virtual auto get_y() -> I32 = 0;
			virtual auto get_width() -> I32 = 0;
			virtual auto get_height() -> I32 = 0;
			virtual auto get_background_colour() -> U32 = 0;
			virtual void raise() = 0;
			virtual auto is_top() -> bool = 0;
			virtual void show() = 0;
			virtual void hide() = 0;
			virtual void move_to(I32 x, I32 y) = 0;

			virtual void redraw() = 0;
			virtual void redraw_area(graphics2d::Rect) = 0;
		};

		auto create_window(const char *title, I32 width, I32 height, I32 x = ~0, I32 y = ~0) -> Window&;
		auto get_window_from_display(DisplayManager::Display&) -> Window*;
	};
}
