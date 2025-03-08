#pragma once

#include <drivers/DisplayManager.hpp>
#include <drivers/Software.hpp>

#include <kernel/keyboard.hpp>
#include <kernel/Thread.hpp>

#include <common/EventEmitter.hpp>
#include <common/LList.hpp>

namespace driver {
	struct Keyboard;
	struct Mouse;
}

namespace driver {
	//TODO: should graphics drivers also include an api for querying their active processor(s) drivers if present? This would allow us to work out what processor speeds and temps relate to this graphics adapter, which might be useful/neat
	struct DesktopManager: Software {
		DRIVER_INSTANCE(DesktopManager, 0x3a11d5b3, "desktop", "DesktopManager", Software);

		struct Window;

		struct Event {
			Mouse *instance;

			enum struct Type {
				windowAdded,
				windowRemoved,
				windowFocused
			} type;

			union {
				struct {
					Window *window;
				} windowAdded;

				struct {
					Window *window;
				} windowRemoved;

				struct {
					Window *window;
				} windowFocused;
			};
		};

		static inline EventEmitter<Event> allEvents;
		EventEmitter<Event> events;
		
		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		struct CustomWindow;
		struct StandardWindow;

		struct Window {
			enum struct State {
				floating,
				docked
			};

			enum struct DockedType {
				top,
				bottom,
				left,
				right,
				full
			};

			enum struct Layer {
				desktopBackground,
				desktop,
				regular,
				topmost
			};

			struct Event {
				enum struct Type {
					clientAreaChanged,

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
					} clientAreaChanged;

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
			virtual auto get_title() -> const char* = 0;

			virtual auto get_x() -> I32 = 0;
			virtual auto get_y() -> I32 = 0;
			virtual auto get_width() -> I32 = 0;
			virtual auto get_height() -> I32 = 0;
			virtual auto get_client_area() -> graphics2d::Buffer& = 0;
			virtual void raise() = 0;
			virtual void focus() = 0;
			virtual auto is_top() -> bool = 0;
			virtual void show() = 0;
			virtual void hide() = 0;
			virtual void dock(DockedType) = 0;
			virtual void restore() = 0;
			virtual void move_to(I32 x, I32 y) = 0;
			virtual void resize_to(U32 width, U32 height) = 0;
			virtual void set_layer(Layer) = 0;
			virtual void set_max_docked_size(U32 width, U32 height) = 0;
			virtual void move_and_resize_to(I32 x, I32 y, U32 width, U32 height) = 0;

			virtual void redraw() = 0;
			virtual void redraw_area(graphics2d::Rect) = 0;

			virtual auto get_state() -> State = 0;
			virtual auto get_docked_type() -> DockedType = 0;

			virtual auto as_standardWindow() -> StandardWindow* { return nullptr; }
			virtual auto as_customWindow() -> CustomWindow* { return nullptr; }
		};

		struct CustomWindow: virtual Window {
			virtual auto get_window_area() -> graphics2d::Buffer& = 0;

			virtual void set_titlebar_area(graphics2d::Rect set) = 0;
			virtual void set_solid_area(graphics2d::Rect set) = 0;
			virtual void set_interact_area(graphics2d::Rect set) = 0;
			virtual void set_margin(U32 left, U32 top, U32 right, U32 bottom) = 0;

			auto as_customWindow() -> CustomWindow* override { return this; }
		};

		struct StandardWindow: virtual Window {
			virtual auto get_background_colour() -> U32 = 0;
			virtual auto get_border_colour() -> U32 = 0;
			virtual void set_status(const char*) = 0;

			auto as_standardWindow() -> StandardWindow* override { return this; }
		};

		auto create_standard_window(const char *title, I32 width, I32 height, I32 x = 1<<31, I32 y = 1<<31) -> StandardWindow&;
		auto create_custom_window(const char *title, I32 width, I32 height, I32 x = 1<<31, I32 y = 1<<31) -> CustomWindow&;
		auto get_window_from_display(DisplayManager::Display&) -> Window*;
		auto get_total_area() -> graphics2d::Rect;
		auto get_window_area() -> graphics2d::Rect;

		auto get_default_window_colour() -> U32;
		auto get_default_window_border_colour() -> U32;

		auto get_window_count() -> U32;
		auto get_window(U32) -> Window*;

		auto get_focused_window() -> Window*;
	};
}
