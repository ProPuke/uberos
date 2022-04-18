#pragma once

#include "Framebuffer.hpp"

#include <common/stdlib.hpp>

inline void Framebuffer::clear() {
	bzero(address, size);
}

inline void Framebuffer::set(U32 x, U32 y, U32 colour) {
	// return _set(x, y, colour);
	
	switch(format){ //the switch is faster ¯\_(ツ)_/¯
		case graphics2d::BufferFormat::grey8 : return set_grey8(x, y, colour);
		case graphics2d::BufferFormat::rgb565: return set_rgb565(x, y, colour);
		case graphics2d::BufferFormat::rgb8  : return set_rgb8(x, y, colour);
		case graphics2d::BufferFormat::rgba8 : return set_rgba8(x, y, colour);
	}
}

inline void Framebuffer::set_grey8(U32 x, U32 y, U32 colour) {
	//TODO:dither? (based on frame, once there is such a concept?)
	
	auto offset = y*width+x;
	*(U16*)&address[offset] = (0
		+ (int)((colour&0x0000ff)>> 0)
		+ (int)((colour&0x00ff00)>> 8)
		+ (int)((colour&0xff0000)>>16)
	) / 3;
}

inline void Framebuffer::set_rgb565(U32 x, U32 y, U32 colour) {
	//TODO:dither? (based on frame, once there is such a concept?)

	auto offset = (y*width+x)*2;
	*(U16*)&address[offset] =
			(((colour&0x0000ff)>> 0)>>3)<< 0
		|(((colour&0x00ff00)>> 8)>>2)<< 5
		|(((colour&0xff0000)>>16)>>3)<<11
	;
}

inline void Framebuffer::set_rgb8(U32 x, U32 y, U32 colour) {
	auto offset = (y*width+x)*3;
	address[offset+0] = (colour&0x0000ff)>> 0;
	address[offset+1] = (colour&0x00ff00)>> 8;
	address[offset+2] = (colour&0xff0000)>>16;
}

inline void Framebuffer::set_rgba8(U32 x, U32 y, U32 colour) {
	auto offset = (y*width+x)*4;
	address[offset+0] = (colour&0x000000ff)>> 0;
	address[offset+1] = (colour&0x0000ff00)>> 8;
	address[offset+2] = (colour&0x00ff0000)>>16;
	address[offset+3] = (colour&0xff000000)>>24;
}

