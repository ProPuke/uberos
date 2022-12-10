#pragma once

#include <common/types.hpp>

#include "BufferFormat.hpp"
#include "Rect.hpp"

namespace graphics2d {
	struct Font;

	struct DrawTextResult {
		I32 x, y;
		I32 maxX;
		Rect updatedArea;
	};

	struct Buffer {
		/**/ Buffer(U8 *address, U32 size, U32 stride, U32 width, U32 height, BufferFormat format, BufferFormatOrder order):
			format(format),
			order(order),
			address(address),
			size(size),
			stride(stride),
			width(width),
			height(height)
		{}

		BufferFormat format;
		BufferFormatOrder order;
		U8 *address;
		U32 size;
		U32 stride;
		U32 width, height;

		void set(I32 x, I32 y, U32 colour);
		void set(U32 x, U32 y, U32 colour);
		void set_grey8(U32 x, U32 y, U32 colour);
		void set_rgb565(U32 x, U32 y, U32 colour);
		void set_bgr565(U32 x, U32 y, U32 colour);
		void set_rgb8(U32 x, U32 y, U32 colour);
		void set_bgr8(U32 x, U32 y, U32 colour);
		void set_rgba8(U32 x, U32 y, U32 colour);
		void set_bgra8(U32 x, U32 y, U32 colour);

		auto get(I32 x, I32 y) -> U32;
		auto get(U32 x, U32 y) -> U32;
		auto get_grey8(U32 x, U32 y) -> U32;
		auto get_rgb565(U32 x, U32 y) -> U32;
		auto get_bgr565(U32 x, U32 y) -> U32;
		auto get_rgb8(U32 x, U32 y) -> U32;
		auto get_bgr8(U32 x, U32 y) -> U32;
		auto get_rgba8(U32 x, U32 y) -> U32;
		auto get_bgra8(U32 x, U32 y) -> U32;

		void draw_rect(U32 x, U32 y, U32 width, U32 height, U32 colour);
		void draw_rect_outline(U32 x, U32 y, U32 width, U32 height, U32 colour, U32 borderWidth=1);
		void draw_msdf(I32 x, I32 y, U32 width, U32 height, Buffer &source, I32 source_x, I32 source_y, U32 source_width, U32 source_height, U32 colour, U32 skipSourceLeft=0, U32 skipSourceTop=0, U32 skipSourceRight=0, U32 skipSourceBottom=0);
		auto draw_text(Font &font, const char *text, I32 x, I32 y, U32 size, U32 lineHeight, U32 colour) { return draw_text(font, text, x, y, size, lineHeight, colour, x); }
		auto draw_text(Font &font, const char *text, I32 x, I32 y, U32 size, U32 lineHeight, U32 colour, I32 cursorX) -> DrawTextResult;

		void scroll(I32 x, I32 y);
	};

	auto blend_rgb(U32 from, U32 to, float phase) -> U32;
	auto blend_rgb(U32 from, U32 to, U8 phase) -> U32;
}

#include "Buffer.inl"
