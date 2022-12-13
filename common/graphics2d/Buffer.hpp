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
		/**/ Buffer():
			format(BufferFormat::rgba8),
			order(BufferFormatOrder::rgb),
			address(nullptr),
			size(0),
			stride(0),
			width(0),
			height(0)
		{}
		
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

		void set(I32 x, I32 y, U32 colour, U32 length = 1);
		void set(U32 x, U32 y, U32 colour, U32 length = 1);
		void set_grey8(U32 x, U32 y, U32 colour, U32 length = 1);
		void set_rgb565(U32 x, U32 y, U32 colour, U32 length = 1);
		void set_bgr565(U32 x, U32 y, U32 colour, U32 length = 1);
		void set_rgb8(U32 x, U32 y, U32 colour, U32 length = 1);
		void set_bgr8(U32 x, U32 y, U32 colour, U32 length = 1);
		void set_rgba8(U32 x, U32 y, U32 colour, U32 length = 1);
		void set_bgra8(U32 x, U32 y, U32 colour, U32 length = 1);

		auto get(I32 x, I32 y) -> U32;
		auto get(U32 x, U32 y) -> U32;
		auto get_grey8(U32 x, U32 y) -> U32;
		auto get_rgb565(U32 x, U32 y) -> U32;
		auto get_bgr565(U32 x, U32 y) -> U32;
		auto get_rgb8(U32 x, U32 y) -> U32;
		auto get_bgr8(U32 x, U32 y) -> U32;
		auto get_rgba8(U32 x, U32 y) -> U32;
		auto get_bgra8(U32 x, U32 y) -> U32;

		void draw_rect(U32 x, U32 y, U32 width, U32 height, U32 colour, U32 topLeftCorners[] = nullptr, U32 topRightCorners[] = nullptr, U32 bottomLeftCorners[] = nullptr, U32 bottomRightCorners[] = nullptr);
		void draw_rect_outline(U32 x, U32 y, U32 width, U32 height, U32 colour, U32 borderWidth = 1, U32 topLeftCorners[] = nullptr, U32 topRightCorners[] = nullptr, U32 bottomLeftCorners[] = nullptr, U32 bottomRightCorners[] = nullptr);
		void draw_msdf(I32 x, I32 y, U32 width, U32 height, Buffer &source, I32 source_x, I32 source_y, U32 source_width, U32 source_height, U32 colour, U32 skipSourceLeft=0, U32 skipSourceTop=0, U32 skipSourceRight=0, U32 skipSourceBottom=0);
		auto draw_text(Font &font, const char *text, I32 x, I32 y, U32 size, U32 colour) { return draw_text(font, text, x, y, size, colour, size*5/4, x); }
		auto draw_text(Font &font, const char *text, I32 x, I32 y, U32 size, U32 colour, U32 lineHeight) { return draw_text(font, text, x, y, size, colour, lineHeight, x); }
		auto draw_text(Font &font, const char *text, I32 x, I32 y, U32 size, U32 colour, U32 lineHeight, I32 cursorX) -> DrawTextResult;
		auto measure_text(Font &font, const char *text, I32 x, I32 y, U32 size) { return measure_text(font, text, x, y, size, size*5/4, x); }
		auto measure_text(Font &font, const char *text, I32 x, I32 y, U32 size, U32 lineHeight) { return measure_text(font, text, x, y, size, lineHeight, x); }
		auto measure_text(Font &font, const char *text, I32 x, I32 y, U32 size, U32 lineHeight, I32 cursorX) -> DrawTextResult;
		void draw_buffer_area(I32 x, I32 y, U32 sourceX, U32 sourceY, U32 width, U32 height, Buffer &image);
		void draw_4slice(I32 x, I32 y, U32 width, U32 height, Buffer &image);

		void scroll(I32 x, I32 y);

		static void create_round_corner(U32 radius, U32 corner[]);
		static void create_diagonal_corner(U32 radius, U32 corner[]);
	};

	auto blend_rgb(U32 from, U32 to, float phase) -> U32;
	auto blend_rgb(U32 from, U32 to, U8 phase) -> U32;
}

#include "Buffer.inl"
