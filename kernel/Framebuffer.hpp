#pragma once

#include <common/graphics2d/BufferFormat.hpp>
#include <common/types.hpp>

namespace driver {
	struct Graphics;
}

struct Framebuffer {
	driver::Graphics *driver;
	U32 index;
	U8 *address;
	U32 size;
	U32 width __attribute__ ((aligned (8)));
	U32 height;
	graphics2d::BufferFormat format;
	graphics2d::BufferFormatOrder order;

	void clear();

	void set(U32 x, U32 y, U32 colour);
	void (Framebuffer::*_set)(U32 x, U32 y, U32 colour) = &Framebuffer::set_rgb8;
	void set_grey8(U32 x, U32 y, U32 colour);
	void set_rgb565(U32 x, U32 y, U32 colour);
	void set_bgr565(U32 x, U32 y, U32 colour);
	void set_rgb8(U32 x, U32 y, U32 colour);
	void set_bgr8(U32 x, U32 y, U32 colour);
	void set_rgba8(U32 x, U32 y, U32 colour);
	void set_bgra8(U32 x, U32 y, U32 colour);

	bool set_mode(U32 width, U32 height, graphics2d::BufferFormat format);
};

#include "Framebuffer.inl"
