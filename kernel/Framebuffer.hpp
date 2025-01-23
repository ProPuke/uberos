#pragma once

#include <common/graphics2d/BufferFormat.hpp>
#include <common/graphics2d/Buffer.hpp>
#include <common/types.hpp>

namespace driver {
	struct Graphics;
}

struct Framebuffer {
	U32 index;
	graphics2d::Buffer buffer; //address might be invalid if direct access not available

	void clear();

	void set(U32 x, U32 y, U32 colour, U32 length = 1);
	void (Framebuffer::*_set)(U32 x, U32 y, U32 colour, U32 length) = &Framebuffer::set_rgb8;
	void set_grey8(U32 x, U32 y, U32 colour, U32 length = 1);
	void set_rgb565(U32 x, U32 y, U32 colour, U32 length = 1);
	void set_bgr565(U32 x, U32 y, U32 colour, U32 length = 1);
	void set_rgb8(U32 x, U32 y, U32 colour, U32 length = 1);
	void set_bgr8(U32 x, U32 y, U32 colour, U32 length = 1);
	void set_rgba8(U32 x, U32 y, U32 colour, U32 length = 1);
	void set_bgra8(U32 x, U32 y, U32 colour, U32 length = 1);

	bool set_mode(U32 width, U32 height, graphics2d::BufferFormat format);
};

#include "Framebuffer.inl"
