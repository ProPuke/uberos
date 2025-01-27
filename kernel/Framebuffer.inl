#pragma once

#include "Framebuffer.hpp"

#include <common/stdlib.hpp>

inline void Framebuffer::clear() {
	buffer.draw_rect(0, 0, buffer.width, buffer.height, 0x000000);
}

inline void Framebuffer::set(U32 x, U32 y, U32 colour, U32 length) {
	// return _set(x, y, colour);

	static_assert((int)graphics2d::BufferFormatOrder::max<2);
	
	switch((int)buffer.format<<1|(int)buffer.order){ //the switch is faster ¯\_(ツ)_/¯
		case (int)graphics2d::BufferFormat::grey8 <<1|(int)graphics2d::BufferFormatOrder::argb:
		case (int)graphics2d::BufferFormat::grey8 <<1|(int)graphics2d::BufferFormatOrder::bgra: return set_grey8(x, y, colour, length);
		case (int)graphics2d::BufferFormat::rgb565<<1|(int)graphics2d::BufferFormatOrder::argb: return set_rgb565(x, y, colour, length);
		case (int)graphics2d::BufferFormat::rgb565<<1|(int)graphics2d::BufferFormatOrder::bgra: return set_bgr565(x, y, colour, length);
		case (int)graphics2d::BufferFormat::rgb8  <<1|(int)graphics2d::BufferFormatOrder::argb: return set_rgb8(x, y, colour, length);
		case (int)graphics2d::BufferFormat::rgb8  <<1|(int)graphics2d::BufferFormatOrder::bgra: return set_bgr8(x, y, colour, length);
		case (int)graphics2d::BufferFormat::rgba8 <<1|(int)graphics2d::BufferFormatOrder::argb: return set_rgba8(x, y, colour, length);
		case (int)graphics2d::BufferFormat::rgba8 <<1|(int)graphics2d::BufferFormatOrder::bgra: return set_bgra8(x, y, colour, length);
	}
}

inline void Framebuffer::set_grey8(U32 x, U32 y, U32 colour, U32 length) {
	buffer.set_grey8(x, y, colour, length);
}

inline void Framebuffer::set_rgb565(U32 x, U32 y, U32 colour, U32 length) {
	buffer.set_rgb565(x, y, colour, length);
}

inline void Framebuffer::set_bgr565(U32 x, U32 y, U32 colour, U32 length) {
	buffer.set_bgr565(x, y, colour, length);
}

inline void Framebuffer::set_rgb8(U32 x, U32 y, U32 colour, U32 length) {
	buffer.set_rgb8(x, y, colour, length);
}

inline void Framebuffer::set_bgr8(U32 x, U32 y, U32 colour, U32 length) {
	buffer.set_bgr8(x, y, colour, length);
}

inline void Framebuffer::set_rgba8(U32 x, U32 y, U32 colour, U32 length) {
	buffer.set_rgba8(x, y, colour, length);
}

inline void Framebuffer::set_bgra8(U32 x, U32 y, U32 colour, U32 length) {
	buffer.set_bgra8(x, y, colour, length);
}
