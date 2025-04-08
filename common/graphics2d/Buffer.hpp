#pragma once

#include <common/types.hpp>

#include <common/graphics2d/BufferFormat.hpp>
#include <common/graphics2d/Rect.hpp>

namespace graphics2d {
	struct Font;

	struct DrawTextResult {
		I32 x, y; // the final x & y
		I32 capHeight; // how far above is the top of a capital letter
		U32 lineHeight;
		I32 blockWidth, blockHeight; // the width and height of the rendered text block, excluding overhangs
		Rect updatedArea; // the actual area of pixels updated (including overhang)
	};

	struct Buffer {
		constexpr /**/ Buffer():
			format(BufferFormat::rgba8),
			order(BufferFormatOrder::argb),
			address(nullptr),
			stride(0),
			width(0),
			height(0)
		{}
		
		/**/ Buffer(U8 *address, U32 stride, U32 width, U32 height, BufferFormat format, BufferFormatOrder order):
			format(format),
			order(order),
			address(address),
			stride(stride),
			width(width),
			height(height)
		{}

		BufferFormat format;
		BufferFormatOrder order;
		U8 *address;
		U32 stride;
		U32 width, height;

		void set(I32 x, I32 y, U32 colour, U32 length = 1);
		void set(U32 x, U32 y, U32 colour, U32 length = 1);
		void set_blended(I32 x, I32 y, U32 colour, U8 opacity=255);
		void set_blended(U32 x, U32 y, U32 colour, U8 opacity=255);
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

		void draw_rect(Rect rect, U32 colour, U32 topLeftCorners[] = nullptr, U32 topRightCorners[] = nullptr, U32 bottomLeftCorners[] = nullptr, U32 bottomRightCorners[] = nullptr) { draw_rect(rect.x1, rect.y1, rect.x2-rect.x1, rect.y2-rect.y1, colour, topLeftCorners, topRightCorners, bottomLeftCorners, bottomRightCorners); }
		void draw_rect_blended(Rect rect, U32 colour, U32 topLeftCorners[] = nullptr, U32 topRightCorners[] = nullptr, U32 bottomLeftCorners[] = nullptr, U32 bottomRightCorners[] = nullptr) { draw_rect_blended(rect.x1, rect.y1, rect.x2-rect.x1, rect.y2-rect.y1, colour, topLeftCorners, topRightCorners, bottomLeftCorners, bottomRightCorners); }
		void draw_rect_outline(Rect rect, U32 colour, U32 borderWidth = 1, U32 topLeftCorners[] = nullptr, U32 topRightCorners[] = nullptr, U32 bottomLeftCorners[] = nullptr, U32 bottomRightCorners[] = nullptr) { draw_rect_outline(rect.x1, rect.y1, rect.x2-rect.x1, rect.y2-rect.y1, colour, borderWidth, topLeftCorners, topRightCorners, bottomLeftCorners, bottomRightCorners); }
		void draw_rect(U32 x, U32 y, U32 width, U32 height, U32 colour, U32 topLeftCorners[] = nullptr, U32 topRightCorners[] = nullptr, U32 bottomLeftCorners[] = nullptr, U32 bottomRightCorners[] = nullptr);
		void draw_rect_blended(U32 x, U32 y, U32 width, U32 height, U32 colour, U32 topLeftCorners[] = nullptr, U32 topRightCorners[] = nullptr, U32 bottomLeftCorners[] = nullptr, U32 bottomRightCorners[] = nullptr);
		void draw_rect_outline(U32 x, U32 y, U32 width, U32 height, U32 colour, U32 borderWidth = 1, U32 topLeftCorners[] = nullptr, U32 topRightCorners[] = nullptr, U32 bottomLeftCorners[] = nullptr, U32 bottomRightCorners[] = nullptr);
		void draw_line(U32 x, U32 y, U32 x2, U32 y2, U32 colour);
		void draw_line_aa(U32 x, U32 y, U32 x2, U32 y2, U32 colour);
		void draw_msdf(I32 x, I32 y, U32 width, U32 height, Buffer &source, I32 source_x, I32 source_y, U32 source_width, U32 source_height, U32 colour, U32 skipSourceLeft=0, U32 skipSourceTop=0, U32 skipSourceRight=0, U32 skipSourceBottom=0);

		struct FontSettings {
			Font &font;
			U32 size = 14;
			I32 lineSpacing = 0;
			I32 charSpacing = 0;
		};

		auto draw_text(FontSettings fontSettings, const char *text, I32 x, I32 y, U32 width, U32 colour) { return draw_text(fontSettings, text, x, y, width, colour, x); }
		auto draw_text(FontSettings fontSettings, const char *text, I32 x, I32 y, U32 width, U32 colour, I32 cursorX) -> DrawTextResult;
		auto measure_text(FontSettings fontSettings, const char *text, U32 width = ~0, I32 cursorX = 0) -> DrawTextResult;
		void draw_buffer(I32 x, I32 y, U32 sourceX, U32 sourceY, U32 width, U32 height, Buffer &image);
		void draw_buffer_blended(I32 x, I32 y, U32 sourceX, U32 sourceY, U32 width, U32 height, Buffer &image, U8 opacity = 0xff);
		void draw_4slice(I32 x, I32 y, U32 width, U32 height, Buffer &image);

		void scroll(I32 x, I32 y);

		auto cropped(U32 left, U32 top, U32 right, U32 bottom) -> Buffer;
		auto region(U32 x, U32 y, U32 width, U32 height) -> Buffer;
	};

	auto blend_rgb(U32 from, U32 to, float phase) -> U32;
	auto blend_rgb(U32 from, U32 to, U8 phase) -> U32;
}

#include "Buffer.inl"
