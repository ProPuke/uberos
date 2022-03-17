#pragma once

#include "graphics2d.hpp"

namespace graphics2d {
	inline void _update_view(View &view) { return _update_view_area(view, {0, 0, (I32)view.buffer.width*(I32)view.scale, (I32)view.buffer.height*(I32)view.scale}); };

	inline void Buffer::set(U32 x, U32 y, U32 colour) {
		// return _set(x, y, colour);
		
		switch(format){ //the switch is faster ¯\_(ツ)_/¯
			case FramebufferFormat::rgb565: return set_rgb565(x, y, colour);
			case FramebufferFormat::rgb8  : return set_rgb8(x, y, colour);
			case FramebufferFormat::rgba8 : return set_rgba8(x, y, colour);
		}
	}

	inline void Buffer::set_rgb565(U32 x, U32 y, U32 colour) {
		//TODO:dither? (based on frame, once there is such a concept?)
		
		auto offset = (y*width+x)*2;
		*(U16*)&address[offset] =
			 (((colour&0x0000ff)>> 0)>>3)<< 0
			|(((colour&0x00ff00)>> 8)>>2)<< 5
			|(((colour&0xff0000)>>16)>>3)<<11
		;
	}

	inline void Buffer::set_rgb8(U32 x, U32 y, U32 colour) {
		auto offset = (y*width+x)*3;
		address[offset+0] = (colour&0x0000ff)>> 0;
		address[offset+1] = (colour&0x00ff00)>> 8;
		address[offset+2] = (colour&0xff0000)>>16;
	}

	inline void Buffer::set_rgba8(U32 x, U32 y, U32 colour) {
		auto offset = (y*width+x)*4;
		address[offset+0] = (colour&0x000000ff)>> 0;
		address[offset+1] = (colour&0x0000ff00)>> 8;
		address[offset+2] = (colour&0x00ff0000)>>16;
		address[offset+3] = (colour&0xff000000)>>24;
	}

	inline U32 Buffer::get(U32 x, U32 y) {
		// return _get(x, y);
		
		switch(format){ //the switch is faster ¯\_(ツ)_/¯
			case FramebufferFormat::rgb565: return get_rgb565(x, y);
			case FramebufferFormat::rgb8  : return get_rgb8(x, y);
			case FramebufferFormat::rgba8 : return get_rgba8(x, y);
		}

		return 0x000000;
	}

	inline U32 Buffer::get_rgb565(U32 x, U32 y) {
		U16 data = *(U16*)&address[(y*width+x)*2];
		return
			 (((data>>11)&0x1f)<<3<<16)
			|(((data>> 5)&0x3f)<<2<< 8)
			|(((data>> 0)&0x1f)<<3<< 0)
		;
	}

	inline U32 Buffer::get_rgb8(U32 x, U32 y) {
		auto offset = (y*width+x)*3;
		return 
			 (address[offset+0]<<16)
			|(address[offset+1]<< 8)
			|(address[offset+2]<< 0)
		;
	}

	inline U32 Buffer::get_rgba8(U32 x, U32 y) {
		auto offset = (y*width+x)*4;
		return 
			 address[offset+0]<<16
			|address[offset+1]<< 8
			|address[offset+2]<< 0
			|address[offset+3]<<24
		;
	}

	inline void Buffer::draw_rect(U32 startX, U32 startY, U32 width, U32 height, U32 colour) {
		for(U32 y=0;y<height;y++) for(U32 x=0;x<width;x++) {
			set(startX+x, startY+y, colour);
		}
	}

	inline void Buffer::draw_msdf(I32 startX, I32 startY, U32 width, U32 height, Buffer &source, U32 source_x, U32 source_y, U32 source_width, U32 source_height, U32 colour) {
		const I32 sdfRange = 1;
		const I32 scale = (width/source_width + height/source_height)/2;
		const I32 sdfPixels = sdfRange * scale;

		source_x *= 256;
		source_y *= 256;

		for(U32 y=(U32)max((I32)0,-startY); y<height&&startY+y<this->height; y++) for(U32 x=(U32)max((I32)0,-startX); x<width&&startX+x<this->width; x++) {
			U32 sX = source_x+x*256*source_width/width;
			U32 sY = source_y+y*256*source_height/height;

			U32 x1y1 = source.get((sX+  0)/256, (sY+  0)/256);
			U32 x2y1 = source.get((sX+255)/256, (sY+  0)/256);
			U32 x1y2 = source.get((sX+  0)/256, (sY+255)/256);
			U32 x2y2 = source.get((sX+255)/256, (sY+255)/256);

			U8 pX = sX-(sX/256*256);
			U8 pY = sY-(sY/256*256);

			U32 msdf = blend_rgb(blend_rgb(x1y1, x2y1, pX), blend_rgb(x1y2, x2y2, pX), pY);

			U8 b = (msdf&0xff0000)>>16;
			U8 g = (msdf&0x00ff00)>> 8;
			U8 r = (msdf&0x0000ff)>> 0;

			U8 median = max(min(r, g), min(max(r, g), b));

			I32 screenPxDistance = ((I32)median-128)*sdfPixels;

			U8 alpha = clamp(screenPxDistance + 128, 0, 255);

			set(startX+x, startY+y, blend_rgb(get(startX+x, startY+y), colour, alpha));
		}
	}

	inline void Buffer::scroll(I32 scrollX, I32 scrollY) {
		if(!scrollX&&!scrollY) return;
		if(abs(scrollX)>=width||abs(scrollY)>=height) return;

		const auto bpp = framebufferFormat::size[(U8)format];
		U32 x1, x2;
		U32 width;
		if(scrollX>=0){
			x1 = scrollX*bpp;
			x2 = 0;
			width = stride-scrollX*bpp;
		}else{
			x1 = 0;
			x2 = -scrollX*bpp;
			width = stride+scrollX*bpp;
		}

		if(scrollY==0){
			if(scrollX>0){
				for(U32 y=0;y<height+scrollY;y++){
					memcpy_backwards(&address[y*stride+x1], &address[y*stride+x2], width);
				}
			}else{
				for(U32 y=0;y<height+scrollY;y++){
					memcpy_forwards(&address[y*stride+x1], &address[y*stride+x2], width);
				}
			}

		}else if(scrollY>0){
			for(U32 y=height-1;y>(U32)scrollY;y--){
				memcpy_forwards(&address[y*stride+x1], &address[(y-scrollY)*stride+x2], stride);
			}
		}else{
			for(U32 y=0;y<height+scrollY;y++){
				memcpy_forwards(&address[y*stride+x1], &address[(y-scrollY)*stride+x2], stride);
			}
		}
	}

	inline U32 blend_rgb(U32 a, U32 b, float phase) {
		return
			 ((U32)(((a&0xff0000)>>16)*(1-phase) + ((b&0xff0000)>>16)*(0+phase)))<<16
			|((U32)(((a&0x00ff00)>> 8)*(1-phase) + ((b&0x00ff00)>> 8)*(0+phase)))<< 8
			|((U32)(((a&0x0000ff)>> 0)*(1-phase) + ((b&0x0000ff)>> 0)*(0+phase)))<< 0
		;
	}

	inline U32 blend_rgb(U32 a, U32 b, U8 phase) {
		return
			 ((U32)(((a&0xff0000)>>16)*(255-phase)/255 + ((b&0xff0000)>>16)*(0+phase)/255))<<16
			|((U32)(((a&0x00ff00)>> 8)*(255-phase)/255 + ((b&0x00ff00)>> 8)*(0+phase)/255))<< 8
			|((U32)(((a&0x0000ff)>> 0)*(255-phase)/255 + ((b&0x0000ff)>> 0)*(0+phase)/255))<< 0
		;
	}
}
