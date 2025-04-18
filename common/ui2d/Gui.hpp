#pragma once

#include <common/graphics2d/Buffer.hpp>
#include <common/PodArray.hpp>
#include <common/ui2d/Theme.hpp>

namespace ui2d {
	struct Control;

	struct Gui {
		/*   */ /**/ Gui(graphics2d::Buffer, Theme&);
		virtual /**/~Gui();

		graphics2d::Buffer buffer;
		Theme &theme;
		U32 backgroundColour;
		PodArray<Control*> controls;
		U32 isFrozen = 0;

		virtual void on_mouse_left();
		virtual void on_mouse_moved(I32 x, I32 y);
		virtual void on_mouse_pressed(I32 x, I32 y, U32 button);
		virtual void on_mouse_released(I32 x, I32 y, U32 button);

		virtual void redraw(bool flush = true);
		virtual void _freeze() { isFrozen++; }
		virtual void _unfreeze() { isFrozen--; }

		virtual void update_area(graphics2d::Rect) = 0;

		struct Freeze: NonCopyable<Freeze> {
			Gui &gui;
			/**/ Freeze(Gui &gui):
				gui(gui)
			{
				gui._freeze();
			}
			/**/~Freeze(){
				gui._unfreeze();
			}
		};

		auto freeze() -> Freeze { return {*this}; }
	};
}
