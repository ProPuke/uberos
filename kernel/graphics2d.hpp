#pragma once

#include "Framebuffer.hpp"
#include "Thread.hpp"
#include <common/types.hpp>
#include <common/LList.hpp>
#include <common/Callback.hpp>

namespace graphics2d {
	struct Font;

	struct Buffer {
		/**/ Buffer(U8 *address, U32 size, U32 width, U32 height, FramebufferFormat format):
			format(format),
			address(address),
			size(size),
			width(width),
			height(height)
		{}

		FramebufferFormat format;
		U8 *address;
		U32 size;
		U32 width, height;

		void set(U32 x, U32 y, U32 colour);
		void set_rgb565(U32 x, U32 y, U32 colour);
		void set_rgb8(U32 x, U32 y, U32 colour);
		void set_rgba8(U32 x, U32 y, U32 colour);

		U32 get(U32 x, U32 y);
		U32 get_rgb565(U32 x, U32 y);
		U32 get_rgb8(U32 x, U32 y);
		U32 get_rgba8(U32 x, U32 y);

		void draw_rect(U32 x, U32 y, U32 width, U32 height, U32 colour);
		void draw_msdf(I32 x, I32 y, U32 width, U32 height, Buffer &source, U32 source_x, U32 source_y, U32 source_width, U32 source_height, U32 colour);
		void draw_text(Font &font, const char *text, I32 x, I32 y, U32 size, U32 colour);
	};

	U32 blend_rgb(U32 from, U32 to, float phase);
	U32 blend_rgb(U32 from, U32 to, U8 phase);

	enum struct ViewMode {
		solid,
		transparent
	};

	struct View: LListItem<View> {
		/**/ View(Thread &thread, U8 *address, U32 size, FramebufferFormat format, U32 x, U32 y, U32 width, U32 height, U8 scale = 1, ViewMode mode = ViewMode::solid):
			thread(thread),
			x(x),
			y(y),
			scale(scale),
			buffer(address, size, width, height, format),
			mode(mode)
		{}

		/**/~View();

		Thread &thread;

		I32 x, y;
		U8 scale;
		Buffer buffer;
		ViewMode mode;

		struct Handle_thread_deleted:Callback {
			/**/ Handle_thread_deleted(View &view):Callback(&view){}

			void call(void *data) override;
		} handle_thread_deleted {*this};
	};

	struct Rect {
		I32 x1, y1, x2, y2;

		void offset(I32 x, I32 y) {
			x1 += x;
			x2 += x;
			y1 += y;
			y2 += y;
		}
	};

	extern LList<View> views;

	void init();

	void set_background_colour(U32 colour);

	View* create_view(Thread &thread, U32 x, U32 y, U32 width, U32 height, U8 scale=1);
	void move_view_to(View &view, I32 x, I32 y);
	void update_background();
	void update_background_area(Rect rect);
	void update_area(Rect rect, View *below = nullptr);
	void update_view(View &view);
	void update_view_area(View &view, Rect rect);
}

#include "graphics2d.inl"
