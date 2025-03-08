#pragma once

#include "../Control.hpp"

namespace graphics2d {
	struct Buffer;
}

namespace ui2d {
	namespace control {
		struct Button: Control {
			typedef Control Super;

			enum struct Type {
				regular,
				toggle
			};

			/**/ Button(Gui &gui, graphics2d::Rect rect, const char *text):
				Super(gui, rect),
				text(text)
			{}

			const char *text;
			graphics2d::Buffer *icon = nullptr;
			Type type = Type::regular;
			bool toggleActive = false;

			virtual void set_regular() { type = Type::regular; redraw(); }
			virtual void set_toggle(bool active) { type = Type::toggle; toggleActive = active; redraw(); }

			void on_mouse_pressed(I32 x, I32 y, U32 button) override;
			void on_mouse_released(I32 x, I32 y, U32 button) override;

			virtual void on_pressed(){};
			virtual void on_released(){};
			virtual void on_clicked(){};

			void redraw(bool flush = true) override;
		};
	}
}
