#pragma once

#include "Buffer.hpp"

#include <common/maths.hpp>

namespace graphics2d {

	inline void Buffer::set(I32 x, I32 y, U32 colour, U32 length) {
		if(x<0||y<0||(U32)x>=width||(U32)y>=height) return;

		return set((U32)x, (U32)y, colour, length);
	}

	inline void Buffer::set(U32 x, U32 y, U32 colour, U32 length) {
		// return _set(x, y, colour);

		static_assert((int)BufferFormatOrder::max<2);
		
		switch((int)format<<1|(int)order){ //the switch is faster ¯\_(ツ)_/¯
			case (int)BufferFormat::grey8 <<1|(int)BufferFormatOrder::rgb:
			case (int)BufferFormat::grey8 <<1|(int)BufferFormatOrder::bgr: return set_grey8(x, y, colour, length);
			case (int)BufferFormat::rgb565<<1|(int)BufferFormatOrder::rgb: return set_rgb565(x, y, colour, length);
			case (int)BufferFormat::rgb565<<1|(int)BufferFormatOrder::bgr: return set_bgr565(x, y, colour, length);
			case (int)BufferFormat::rgb8  <<1|(int)BufferFormatOrder::rgb: return set_rgb8(x, y, colour, length);
			case (int)BufferFormat::rgb8  <<1|(int)BufferFormatOrder::bgr: return set_bgr8(x, y, colour, length);
			case (int)BufferFormat::rgba8 <<1|(int)BufferFormatOrder::rgb: return set_rgba8(x, y, colour, length);
			case (int)BufferFormat::rgba8 <<1|(int)BufferFormatOrder::bgr: return set_bgra8(x, y, colour, length);
		}
	}

	inline void Buffer::set_grey8(U32 x, U32 y, U32 colour, U32 length) {
		//TODO:dither? (based on frame, once there is such a concept?)
		
		auto data = (U16*)&address[y*width+x];
		U8 value = (0
			+ (int)((colour&0x0000ff)>> 0)
			+ (int)((colour&0x00ff00)>> 8)
			+ (int)((colour&0xff0000)>>16)
		) / 3;

		memset(data, value, length);
	}

	inline void Buffer::set_rgb565(U32 x, U32 y, U32 colour, U32 length) {
		//TODO:dither? (based on frame, once there is such a concept?)
		
		auto data = (U16*)&address[(y*width+x)*2];
		U16 value = 0
			|(((colour&0x0000ff)>> 0)>>3)<< 0
			|(((colour&0x00ff00)>> 8)>>2)<< 5
			|(((colour&0xff0000)>>16)>>3)<<11
		;

		//TODO:optimise
		while(length--) *data++ = value;
	}

	inline void Buffer::set_bgr565(U32 x, U32 y, U32 colour, U32 length) {
		//TODO:dither? (based on frame, once there is such a concept?)
		
		auto data = (U16*)&address[(y*width+x)*2];
		U16 value = 0
			|(((colour&0x0000ff)>> 0)>>3)<<11
			|(((colour&0x00ff00)>> 8)>>2)<< 5
			|(((colour&0xff0000)>>16)>>3)<< 0
		;

		//TODO:optimise
		while(length--) *data++ = value;
	}

	inline void Buffer::set_rgb8(U32 x, U32 y, U32 colour, U32 length) {
		auto data = (U8*)&address[(y*width+x)*3];

		//TODO:optimise
		while(length--){
			data[0] = (colour&0xff0000)>>16;
			data[1] = (colour&0x00ff00)>> 8;
			data[2] = (colour&0x0000ff)>> 0;
			data += 3;
		}
	}

	inline void Buffer::set_bgr8(U32 x, U32 y, U32 colour, U32 length) {
		auto data = (U8*)&address[(y*width+x)*3];

		//TODO:optimise
		while(length--){
			data[0] = (colour&0x0000ff)>> 0;
			data[1] = (colour&0x00ff00)>> 8;
			data[2] = (colour&0xff0000)>>16;
			data += 3;
		}
	}

	inline void Buffer::set_rgba8(U32 x, U32 y, U32 colour, U32 length) {
		auto data = (U32*)&address[(y*width+x)*4];
		U32 value = 0
			|((colour&0x00ff0000)>>16)<< 0
			|((colour&0x0000ff00)>> 8)<< 8
			|((colour&0x000000ff)>> 0)<<16
			|((colour&0xff000000)>>24)<<24
		;

		if(length>=16/4&&(uintptr_t)address&3==0){
			while((uintptr_t)address&15){
				*data++ = value;
				length--;
			}

			auto phatValue = (U128)value|(U128)value<<32|(U128)value<<64|(U128)value<<96;
			auto phatData = (U128*)data;

			while(length>=16/4){
				*phatData++ = phatValue;
				length-=16/4;
			}

			data = (U32*)phatData;
		}

		while(length--) *data++ = value;
	}

	inline void Buffer::set_bgra8(U32 x, U32 y, U32 colour, U32 length) {
		auto data = (U32*)&address[(y*width+x)*4];
		U32 value = 0
			|((colour&0x000000ff)>> 0)<< 0
			|((colour&0x0000ff00)>> 8)<< 8
			|((colour&0x00ff0000)>>16)<<16
			|((colour&0xff000000)>>24)<<24
		;

		if(length>=16/4&&(uintptr_t)address&3==0){
			while((uintptr_t)address&15){
				*data++ = value;
				length--;
			}

			auto phatValue = (U128)value|(U128)value<<32|(U128)value<<64|(U128)value<<96;
			auto phatData = (U128*)data;

			while(length>=16/4){
				*phatData++ = phatValue;
				length-=16/4;
			}

			data = (U32*)phatData;
		}

		while(length--) *data++ = value;
	}

	inline U32 Buffer::get(I32 x, I32 y) {
		if(x<0||y<0||(U32)x>=width||(U32)y>=height) return 0x000000;

		return get((U32)x, (U32)y);
	}

	inline U32 Buffer::get(U32 x, U32 y) {
		// return _get(x, y);

		static_assert((int)BufferFormatOrder::max<2);
		
		switch((int)format<<1|(int)order){ //the switch is faster ¯\_(ツ)_/¯
			case (int)BufferFormat::grey8 <<1|(int)BufferFormatOrder::rgb:
			case (int)BufferFormat::grey8 <<1|(int)BufferFormatOrder::bgr: return get_grey8(x, y);
			case (int)BufferFormat::rgb565<<1|(int)BufferFormatOrder::rgb: return get_rgb565(x, y);
			case (int)BufferFormat::rgb565<<1|(int)BufferFormatOrder::bgr: return get_bgr565(x, y);
			case (int)BufferFormat::rgb8  <<1|(int)BufferFormatOrder::rgb: return get_rgb8(x, y);
			case (int)BufferFormat::rgb8  <<1|(int)BufferFormatOrder::bgr: return get_bgr8(x, y);
			case (int)BufferFormat::rgba8 <<1|(int)BufferFormatOrder::rgb: return get_rgba8(x, y);
			case (int)BufferFormat::rgba8 <<1|(int)BufferFormatOrder::bgr: return get_bgra8(x, y);
		}

		return 0x000000;
	}

	inline U32 Buffer::get_grey8(U32 x, U32 y) {
		U8 value = address[y*width+x];
		return value<<16|value<<8|value;
	}

	inline U32 Buffer::get_rgb565(U32 x, U32 y) {
		U16 data = *(U16*)&address[(y*width+x)*2];
		return 0
			|(((data>>11)&0x1f)<<3<<16)
			|(((data>> 5)&0x3f)<<2<< 8)
			|(((data>> 0)&0x1f)<<3<< 0)
		;
	}

	inline U32 Buffer::get_bgr565(U32 x, U32 y) {
		U16 data = *(U16*)&address[(y*width+x)*2];
		return 0
			|(((data>> 0)&0x1f)<<3<<16)
			|(((data>> 5)&0x3f)<<2<< 8)
			|(((data>>11)&0x1f)<<3<< 0)
		;
	}

	inline U32 Buffer::get_rgb8(U32 x, U32 y) {
		auto offset = (y*width+x)*3;
		return 0
			|(address[offset+0]<<16)
			|(address[offset+1]<< 8)
			|(address[offset+2]<< 0)
		;
	}

	inline U32 Buffer::get_bgr8(U32 x, U32 y) {
		auto offset = (y*width+x)*3;
		return 0
			|(address[offset+2]<<16)
			|(address[offset+1]<< 8)
			|(address[offset+0]<< 0)
		;
	}

	inline U32 Buffer::get_rgba8(U32 x, U32 y) {
		auto offset = (y*width+x)*4;
		return 0
			|address[offset+0]<<16
			|address[offset+1]<< 8
			|address[offset+2]<< 0
			|address[offset+3]<<24
		;
	}

	inline U32 Buffer::get_bgra8(U32 x, U32 y) {
		auto offset = (y*width+x)*4;
		return 0
			|address[offset+2]<<16
			|address[offset+1]<< 8
			|address[offset+0]<< 0
			|address[offset+3]<<24
		;
	}

	inline void Buffer::draw_rect(U32 startX, U32 startY, U32 width, U32 height, U32 colour, U32 topLeftCorners[], U32 topRightCorners[], U32 bottomLeftCorners[], U32 bottomRightCorners[]) {
		U32 topLeftRadius = 0; if(topLeftCorners) while(topLeftRadius[topLeftCorners]!=~0u) topLeftRadius++;
		U32 topRightRadius = 0; if(topRightCorners) while(topRightRadius[topRightCorners]!=~0u) topRightRadius++;
		U32 bottomLeftRadius = 0; if(bottomLeftCorners) while(bottomLeftRadius[bottomLeftCorners]!=~0u) bottomLeftRadius++;
		U32 bottomRightRadius = 0; if(bottomRightCorners) while(bottomRightRadius[bottomRightCorners]!=~0u) bottomRightRadius++;

		U32 y = 0;
		for(;y<height;y++){
			const auto left = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			const auto right = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);
			if(left<right){
				set(startX+left, startY+y, colour, right-left);
			}
		}
	}

	inline void Buffer::draw_rect_outline(U32 startX, U32 startY, U32 width, U32 height, U32 colour, U32 borderWidth, U32 topLeftCorners[], U32 topRightCorners[], U32 bottomLeftCorners[], U32 bottomRightCorners[]) {
		if(width<1||height<1) return;
		if(borderWidth<1) return;

		U32 topLeftRadius = 0; if(topLeftCorners) while(topLeftRadius[topLeftCorners]!=~0u) topLeftRadius++;
		U32 topRightRadius = 0; if(topRightCorners) while(topRightRadius[topRightCorners]!=~0u) topRightRadius++;
		U32 bottomLeftRadius = 0; if(bottomLeftCorners) while(bottomLeftRadius[bottomLeftCorners]!=~0u) bottomLeftRadius++;
		U32 bottomRightRadius = 0; if(bottomRightCorners) while(bottomRightRadius[bottomRightCorners]!=~0u) bottomRightRadius++;

		U32 y = 0;

		U32 lastLeft1 = startX;
		U32 lastLeft2 = startX+width;
		U32 lastRight1 = lastLeft1;
		U32 lastRight2 = lastLeft2;

		for(;y<height/2&&y<borderWidth;y++){
			const auto left = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			const auto right = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);

			if(left<right){
				set(startX+left, startY+y, colour, right-left);
			}

			lastLeft1 = left;
			lastLeft2 = right;
			lastRight1 = left;
			lastRight2 = right;
		}

		for(;y<height&&y<height-borderWidth;y++){
			const auto left1 = maths::min(width-1, maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0));
			const auto left2 = maths::min(width, left1+borderWidth);
			const auto right2 = (U32)maths::max(0, (I32)width-(I32)maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0));
			const auto right1 = (U32)maths::max(0, (I32)right2-(I32)borderWidth);

			{
				U32 left  = maths::min(left1, lastLeft2);
				U32 right = maths::max(left2, lastLeft1);
				if(left<right){
					set(startX+left, startY+y, colour, right-left);
				}
			}
			
			{
				U32 left  = maths::min(right1, lastRight2);
				U32 right = maths::max(right2, lastRight1);
				if(left<right){
					set(startX+left, startY+y, colour, right-left);
				}
			}

			lastLeft1 = left1;
			lastLeft2 = left2;
			lastRight1 = right1;
			lastRight2 = right2;
		}

		bool topRow = true;
		for(;y<height;y++){
			const auto left1 = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			const auto right2 = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);

			if(topRow){
				if(lastLeft2<left1){
					set(startX+left1, startY-1+y, colour, left1-lastLeft2);
				}

				if(right2<lastRight1){
					set(startX+lastRight1, startY-1+y, colour, lastRight1-right2);
				}

				{
					U32 left = maths::max(left1, lastLeft2);
					U32 right = maths::min(right2, lastRight2);
					if(left<right){
						set(startX+left, startY+y, colour, right-left);
					}
				}
				
				topRow = false;

			}else{
				if(left1<right2){
					set(startX+left1, startY+y, colour, right2-left1);
				}
			}
		}
	}

	inline void Buffer::draw_msdf(I32 startX, I32 startY, U32 width, U32 height, Buffer &source, I32 source_x, I32 source_y, U32 source_width, U32 source_height, U32 colour, U32 skipSourceLeft, U32 skipSourceTop, U32 skipSourceRight, U32 skipSourceBottom) {
		if(width<1||height<1) return;

		//minify by sampling multiple times (looks best when <= halfsize)
		if(width<=source_width/2&&height<=source_height/2){
			const auto samplesX = (source_width+source_width/2-1)/width;
			const auto samplesY = (source_height+source_height/2-1)/height;

			for(I32 y=0; y<(I32)height&&startY+y<(I32)this->height; y++) for(I32 x=0; x<(I32)width&&startX+x<(I32)this->width; x++) {
				I32 sX = source_x+x*source_width/width;
				I32 sY = source_y+y*source_height/height;

				U32 coverage = 0;

				for(I32 y2=y==0?skipSourceTop:0;y2<(I32)samplesY-(y==(I32)height-1?(I32)skipSourceBottom:0);y2++) for(I32 x2=x==0?skipSourceLeft:0;x2<(I32)samplesX-(x==(I32)width-1?(I32)skipSourceRight:0);x2++) {
					auto sample = source.get(sX+x2, sY+y2);

					const auto r = bits(sample, 16, 23);
					const auto g = bits(sample, 8, 15);
					const auto b = bits(sample, 0, 7);

					// we have to calculate the median for every sample, not from the end average, otherwise we get artifacts betweeen 2 different coloured edges that don't touch
					auto median =
						r>g?g>b?g:b
						:b>g?g:b>r?b:r
					;

					coverage += median;
				}

				coverage /= (samplesX*samplesY);

				coverage += coverage*(255-coverage)/512; //push up "gamma" for better contrast

				set(startX+x, startY+y, blend_rgb(get(startX+x, startY+y), colour, (U8)coverage));
			}

		//bilinear filter the msdf (looks best at half size and up, although blurs some letters slightly)
		}else{
			const I32 sdfRange = 1;
			const I32 scale = maths::max(1u, ((width+source_width-1)/source_width + (height+source_height-1)/source_height)/2);
			// const I32 scale = 2;
			const I32 sdfPixels = sdfRange * scale;

			source_x *= 256;
			source_y *= 256;

			for(I32 y=max((I32)0,-startY); y<(I32)height&&startY+y<(I32)this->height; y++) for(I32 x=max((I32)0,-startX); x<(I32)width&&startX+x<(I32)this->width; x++) {
				I32 sX = source_x+(x*256)*source_width/width;
				I32 sY = source_y+(y*256)*source_height/height;

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

				U8 alpha = maths::clamp(screenPxDistance + 128, 0, 255);

				set(startX+x, startY+y, blend_rgb(get(startX+x, startY+y), colour, alpha));
				// set(startX+x, startY+y, blend_rgb(0x0, colour, alpha));
			}
		}
	}

	inline void Buffer::draw_buffer_area(I32 destX, I32 destY, U32 sourceX, U32 sourceY, U32 width, U32 height, Buffer &image) {
		if(destX>=(I32)this->width||destY>=(I32)this->height||destX+width<=0||destY+height<=0) return;

		for(U32 y=maths::max(0, -destX); y<maths::min(maths::min(height, image.height-sourceX), height-destX); y++){
			for(U32 x=maths::max(0, -destY); x<maths::min(maths::min(width, image.width-sourceY), width-destY); x++){
				set(destX+x, destY+y, image.get(sourceX+x, sourceY+y));
			}
		}
	}

	inline void Buffer::draw_4slice(I32 startX, I32 startY, U32 width, U32 height, Buffer &image) {
		if(startX>=(I32)this->width||startY>=(I32)this->height||startX+width<=0||startY+height<=0) return;

		U32 cornerWidth = maths::min(width/2, image.width);
		U32 cornerHeight = maths::min(height/2, image.height);

		draw_buffer_area(startX, startY, 0, 0, cornerWidth, cornerHeight, image);
		draw_buffer_area(startX+width-cornerWidth, startY, image.width-cornerWidth, 0, cornerWidth, cornerHeight, image);
		draw_buffer_area(startX, startY+height-cornerHeight, 0, image.height-cornerHeight, cornerWidth, cornerHeight, image);
		draw_buffer_area(startX+width-cornerWidth, startY+height-cornerHeight, image.width-cornerWidth, image.height-cornerHeight, cornerWidth, cornerHeight, image);

		for(U32 x=startX+cornerWidth;x<width-cornerWidth;x++){
			for(U32 y=0;y<cornerHeight;y++){
				set(x, startY+y, image.get(image.width-1,y));
			}
			for(U32 y=cornerHeight;y<height-cornerHeight;y++){
				set(x, startY+y, image.get(image.width-1,image.height-1));
			}
			for(U32 y=0;y<height-cornerHeight+cornerHeight;y++){
				set(x, startY+height-cornerHeight+y, image.get(image.width-1,cornerHeight+y));
			}
		}

		for(U32 y=startY+cornerHeight;y<height-cornerHeight;y++){
			for(U32 x=0;x<cornerWidth;x++){
				set(startX+x, y, image.get(x, image.height-1));
			}
			for(U32 x=0;x<width-cornerWidth+cornerWidth;x++){
				set(startX+width-cornerWidth+x, y, image.get(cornerWidth+x, image.height-1));
			}
		}
	}
	
	inline void Buffer::scroll(I32 scrollX, I32 scrollY) {
		if(!scrollX&&!scrollY) return;
		if(maths::abs(scrollX)>=width||maths::abs(scrollY)>=height) return;

		const auto bpp = bufferFormat::size[(U8)format];
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
					#ifdef USE_STDLIB_ASM
						memmove(&address[y*stride+x1], &address[y*stride+x2], width);
					#else
						memcpy_backwards(&address[y*stride+x1], &address[y*stride+x2], width);
					#endif
				}
			}else{
				for(U32 y=0;y<height+scrollY;y++){
					#ifdef USE_STDLIB_ASM
						memmove(&address[y*stride+x1], &address[y*stride+x2], width);
					#else
						memcpy_forwards(&address[y*stride+x1], &address[y*stride+x2], width);
					#endif
				}
			}

		}else if(scrollY>0){
			for(U32 y=height-1;y>(U32)scrollY;y--){
				#ifdef USE_STDLIB_ASM
					memmove(&address[y*stride+x1], &address[(y-scrollY)*stride+x2], stride);
				#else
					memcpy(&address[y*stride+x1], &address[(y-scrollY)*stride+x2], stride);
				#endif
			}
		}else{
			// for(I32 y=height+scrollY-1;y>=0;y--){
			for(U32 y=0;y<height+scrollY;y++){
				#ifdef USE_STDLIB_ASM
					memmove(&address[y*stride+x1], &address[(y-scrollY)*stride+x2], stride);
				#else
					memcpy(&address[y*stride+x1], &address[(y-scrollY)*stride+x2], stride);
				#endif
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

	inline void Buffer::create_round_corner(U32 radius, U32 corner[]) {
		for(auto i=0u;i<radius;i++){
			auto y = radius-i;
			corner[i] = radius-U32(maths::sqrt(radius*radius-((y*y))));
		}
		corner[radius] = -1;
	}

	inline void Buffer::create_diagonal_corner(U32 radius, U32 corner[]) {
		for(auto i=0u;i<radius;i++){
			corner[i] = radius-i;
		}
		corner[radius] = -1;
	}
}
