#pragma once

#include <common/types.hpp>

enum struct FramebufferFormat {
	rgb565,
	rgb8,
	rgba8,
	max = rgba8
};

namespace framebufferFormat {
	static const unsigned max = (U32)FramebufferFormat::rgba8;

	static const char *const name[max+1] = {
		"16bit colour",
		"24bit colour",
		"32bit colour"
	};

	static const U8 size[max+1] = {
		2,
		3,
		4
	};
}

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
	FramebufferFormat format;

	void clear();

	void set(U32 x, U32 y, U32 colour);
	void (Framebuffer::*_set)(U32 x, U32 y, U32 colour) = &Framebuffer::set_rgb8;
	void set_rgb565(U32 x, U32 y, U32 colour);
	void set_rgb8(U32 x, U32 y, U32 colour);
	void set_rgba8(U32 x, U32 y, U32 colour);

	bool set_mode(U32 width, U32 height, FramebufferFormat format);
};

#include <common/stdlib.hpp>

inline auto to_string(FramebufferFormat format) -> const char* {
	return (U32)format<=framebufferFormat::max?framebufferFormat::name[(U32)format]: ::to_string((U32)format);
}

#include "Framebuffer.inl"
