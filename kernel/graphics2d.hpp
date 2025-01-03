#pragma once

#include <kernel/Framebuffer.hpp>
#include <kernel/Thread.hpp>

#include <common/Callback.hpp>
#include <common/graphics2d/Buffer.hpp>
#include <common/graphics2d/BufferFormat.hpp>
#include <common/graphics2d/Rect.hpp>
#include <common/LList.hpp>
#include <common/types.hpp>

namespace graphics2d {
	struct Font;

	enum struct ViewMode {
		solid,
		transparent
	};

	enum struct ViewLayer: U8 {
		background = 0,
		bottomMost = 64,
		regular = 128,
		topMost = 192,
		kernelOverlay = 255
	};

	struct View: LListItem<View> {
		/**/ View(Thread *thread, U8 *address, ViewLayer layer, U32 size, graphics2d::BufferFormat format, graphics2d::BufferFormatOrder order, U32 x, U32 y, U32 width, U32 height, U8 scale = 1, ViewMode mode = ViewMode::solid):
			thread(thread),
			x(x),
			y(y),
			layer(layer),
			scale(scale),
			buffer(address, size, width*graphics2d::bufferFormat::size[(U32)format], width, height, format, order),
			mode(mode)
		{}

		/**/~View();

		Thread *thread;

		I32 x, y;
		ViewLayer layer;
		U8 scale;
		Buffer buffer;
		ViewMode mode;
		bool isVisible = true;

		struct Handle_thread_deleted:Callback {
			/**/ Handle_thread_deleted(View &view):Callback(&view){}

			void call(void *data) override;
		} handle_thread_deleted {*this};
	};

	extern LList<View> views;

	void init();

	// threadsafe functions

	void set_background_colour(U32 colour);

	auto create_view(Thread *thread, ViewLayer layer, U32 x, U32 y, U32 width, U32 height, U8 scale=1) -> View*;
	void move_view_to(View &view, I32 x, I32 y);
	void raise_view(View &view);
	void show_view(View &view);
	void hide_view(View &view);
	void update_background();
	void update_background_area(Rect rect);
	void update_area(Rect rect, View *below = nullptr);
	void update_view(View &view);
	void update_view_area(View &view, Rect rect);

	auto get_screen_buffer(U32 framebuffer, Rect rect) -> Buffer;

	// non-threadsafe internals

	void _set_background_colour(U32 colour);

	auto _create_view(Thread *thread, ViewLayer layer, U32 x, U32 y, U32 width, U32 height, U8 scale=1) -> View*;
	void _move_view_to(View &view, I32 x, I32 y);
	void _raise_view(View &view);
	void _show_view(View &view);
	void _hide_view(View &view);
	void _update_background();
	void _update_background_area(Rect rect);
	void _update_area(Rect rect, View *below = nullptr);
	void _update_view(View &view);
	void _update_view_area(View &view, Rect rect);

	auto _get_screen_buffer(U32 framebuffer, Rect rect) -> Buffer;
}

#include "graphics2d.inl"
