#pragma once

#include "Buffer.hpp"

#include <common/graphics2d.hpp>
#include <common/maths.hpp>

namespace graphics2d {
	namespace {
		inline U16 mix(U32 a, U32 b, U8 phase) {
			return a*(255-phase)/255 + b*phase/255;
		}
	}

	inline void Buffer::set(I32 x, I32 y, U32 colour, U32 length) {
		if(x<0||y<0) return;

		return set((U32)x, (U32)y, colour, length);
	}

	inline void Buffer::set(U32 x, U32 y, U32 colour, U32 length) {
		if((U32)x>=width||(U32)y>=height) return;

		// return _set(x, y, colour);

		static_assert((int)BufferFormatOrder::max<2);
		
		switch((int)format<<1|(int)order){ //the switch is faster ¯\_(ツ)_/¯
			case (int)BufferFormat::grey8 <<1|(int)BufferFormatOrder::argb:
			case (int)BufferFormat::grey8 <<1|(int)BufferFormatOrder::bgra: return set_grey8(x, y, colour, length);
			case (int)BufferFormat::rgb565<<1|(int)BufferFormatOrder::argb: return set_rgb565(x, y, colour, length);
			case (int)BufferFormat::rgb565<<1|(int)BufferFormatOrder::bgra: return set_bgr565(x, y, colour, length);
			case (int)BufferFormat::rgb8  <<1|(int)BufferFormatOrder::argb: return set_rgb8(x, y, colour, length);
			case (int)BufferFormat::rgb8  <<1|(int)BufferFormatOrder::bgra: return set_bgr8(x, y, colour, length);
			case (int)BufferFormat::rgba8 <<1|(int)BufferFormatOrder::argb: return set_rgba8(x, y, colour, length);
			case (int)BufferFormat::rgba8 <<1|(int)BufferFormatOrder::bgra: return set_bgra8(x, y, colour, length);
		}
	}

	inline void Buffer::set_blended(I32 x, I32 y, U32 colour, U8 opacity) {
		if(x<0||y<0) return;

		return set_blended((U32)x, (U32)y, colour, opacity);
	}

	inline void Buffer::set_blended(U32 x, U32 y, U32 colour, U8 opacity) {
		if((U32)x>=width||(U32)y>=height) return;

		switch(format){
			case BufferFormat::grey8:
			case BufferFormat::rgb565:
			case BufferFormat::rgb8:
				if(colour>>24<128){
					set(x, y, colour);
				}
			break;
			case BufferFormat::rgba8: {
				const U8 trans = 255-((255-(colour>>24)) * opacity/255);
				auto &data = *(U32*)&address[y*stride+x*4];

				switch(order){
					case BufferFormatOrder::argb:
						data =
							(trans - ((255-((data&0x000000ff)>>24)))*trans/255)<<0|
							(((colour&0x00ff0000)>>16)*opacity/255 + ((data&0x0000ff00)>> 8)*trans/255)<< 8|
							(((colour&0x0000ff00)>> 8)*opacity/255 + ((data&0x00ff0000)>>16)*trans/255)<<16|
							(((colour&0x000000ff)>> 0)*opacity/255 + ((data&0xff000000)>>24)*trans/255)<<24
						;
					break;
					case BufferFormatOrder::bgra:
						data =
							(((colour&0x000000ff)>> 0)*opacity/255 + ((data&0x000000ff)>> 0)*trans/255)<< 0|
							(((colour&0x0000ff00)>> 8)*opacity/255 + ((data&0x0000ff00)>> 8)*trans/255)<< 8|
							(((colour&0x00ff0000)>>16)*opacity/255 + ((data&0x00ff0000)>>16)*trans/255)<<16|
							(trans - ((255-((data&0xff000000)>>24)))*trans/255)<<24
						;
					break;
				}
			} break;
		}
	}

	inline void Buffer::set_grey8(U32 x, U32 y, U32 colour, U32 length) {
		if((U32)x>=width||(U32)y>=height) return;

		//TODO:dither? (based on frame, once there is such a concept?)
		
		auto data = (U16*)&address[y*stride+x];
		U8 value = (0
			+ (int)((colour&0x0000ff)>> 0)
			+ (int)((colour&0x00ff00)>> 8)
			+ (int)((colour&0xff0000)>>16)
		) / 3;

		memset(data, value, length);
	}

	inline void Buffer::set_rgb565(U32 x, U32 y, U32 colour, U32 length) {
		if((U32)x>=width||(U32)y>=height) return;

		//TODO:dither? (based on frame, once there is such a concept?)
		
		auto data = (U16*)&address[y*stride+x*2];
		U16 value = 0
			|(((colour&0x0000ff)>> 0)>>3)<< 0
			|(((colour&0x00ff00)>> 8)>>2)<< 5
			|(((colour&0xff0000)>>16)>>3)<<11
		;

		//TODO:optimise
		while(length--) *data++ = value;
	}

	inline void Buffer::set_bgr565(U32 x, U32 y, U32 colour, U32 length) {
		if((U32)x>=width||(U32)y>=height) return;

		//TODO:dither? (based on frame, once there is such a concept?)
		
		auto data = (U16*)&address[y*stride+x*2];
		U16 value = 0
			|(((colour&0x0000ff)>> 0)>>3)<<11
			|(((colour&0x00ff00)>> 8)>>2)<< 5
			|(((colour&0xff0000)>>16)>>3)<< 0
		;

		//TODO:optimise
		while(length--) *data++ = value;
	}

	inline void Buffer::set_rgb8(U32 x, U32 y, U32 colour, U32 length) {
		if((U32)x>=width||(U32)y>=height) return;

		auto data = (U8*)&address[y*stride+x*3];

		//TODO:optimise
		while(length--){
			data[0] = (colour&0xff0000)>>16;
			data[1] = (colour&0x00ff00)>> 8;
			data[2] = (colour&0x0000ff)>> 0;
			data += 3;
		}
	}

	inline void Buffer::set_bgr8(U32 x, U32 y, U32 colour, U32 length) {
		if((U32)x>=width||(U32)y>=height) return;

		auto data = (U8*)&address[y*stride+x*3];

		//TODO:optimise
		while(length--){
			data[0] = (colour&0x0000ff)>> 0;
			data[1] = (colour&0x00ff00)>> 8;
			data[2] = (colour&0xff0000)>>16;
			data += 3;
		}
	}

	inline void Buffer::set_rgba8(U32 x, U32 y, U32 colour, U32 length) {
		if((U32)x>=width||(U32)y>=height) return;

		auto data = (U32*)&address[y*stride+x*4];
		U32 value = 0
			|((colour&0x00ff0000)>>16)<<16
			|((colour&0x0000ff00)>> 8)<< 8
			|((colour&0x000000ff)>> 0)<< 0
			|((colour&0xff000000)>>24)<<24
		;

		#ifdef HAS_128BIT
			if(length>=16/4&&(UPtr)address&3==0){
				while((UPtr)address&15){
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
		#else
			if(length>=8/4&&(UPtr)address&3==0){
				while((UPtr)address&15){
					*data++ = value;
					length--;
				}

				auto phatValue = (U64)value|(U64)value<<32;
				auto phatData = (U64*)data;

				while(length>=8/4){
					*phatData++ = phatValue;
					length-=8/4;
				}

				data = (U32*)phatData;
			}
		#endif

		while(length--) *data++ = value;
	}

	inline void Buffer::set_bgra8(U32 x, U32 y, U32 colour, U32 length) {
		if((U32)x>=width||(U32)y>=height) return;

		auto data = (U32*)&address[y*stride+x*4];
		U32 value = 0
			|((colour&0x000000ff)>> 0)<< 0
			|((colour&0x0000ff00)>> 8)<< 8
			|((colour&0x00ff0000)>>16)<<16
			|((colour&0xff000000)>>24)<<24
		;

		#ifdef HAS_128BIT
			if(length>=16/4&&(UPtr)address&3==0){
				while((UPtr)address&15){
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
		#else
			if(length>=8/4&&(UPtr)address&3==0){
				while((UPtr)address&15){
					*data++ = value;
					length--;
				}

				auto phatValue = (U64)value|(U64)value<<32;
				auto phatData = (U64*)data;

				while(length>=8/4){
					*phatData++ = phatValue;
					length-=8/4;
				}

				data = (U32*)phatData;
			}
		#endif

		while(length--) *data++ = value;
	}

	inline U32 Buffer::get(I32 x, I32 y) {
		if(x<0||y<0) return 0x000000;

		return get((U32)x, (U32)y);
	}

	inline U32 Buffer::get(U32 x, U32 y) {
		// return _get(x, y);

		static_assert((int)BufferFormatOrder::max<2);
		
		switch((int)format<<1|(int)order){ //the switch is faster ¯\_(ツ)_/¯
			case (int)BufferFormat::grey8 <<1|(int)BufferFormatOrder::argb:
			case (int)BufferFormat::grey8 <<1|(int)BufferFormatOrder::bgra: return get_grey8(x, y);
			case (int)BufferFormat::rgb565<<1|(int)BufferFormatOrder::argb: return get_rgb565(x, y);
			case (int)BufferFormat::rgb565<<1|(int)BufferFormatOrder::bgra: return get_bgr565(x, y);
			case (int)BufferFormat::rgb8  <<1|(int)BufferFormatOrder::argb: return get_rgb8(x, y);
			case (int)BufferFormat::rgb8  <<1|(int)BufferFormatOrder::bgra: return get_bgr8(x, y);
			case (int)BufferFormat::rgba8 <<1|(int)BufferFormatOrder::argb: return get_rgba8(x, y);
			case (int)BufferFormat::rgba8 <<1|(int)BufferFormatOrder::bgra: return get_bgra8(x, y);
		}

		return 0x000000;
	}

	inline U32 Buffer::get_grey8(U32 x, U32 y) {
		if((U32)x>=width||(U32)y>=height) return 0x000000;

		U8 value = address[y*stride+x];
		return value<<16|value<<8|value;
	}

	inline U32 Buffer::get_rgb565(U32 x, U32 y) {
		if((U32)x>=width||(U32)y>=height) return 0x000000;

		U16 data = *(U16*)&address[y*stride+x*2];
		return 0
			|(((data>>11)&0x1f)<<3<<16)
			|(((data>> 5)&0x3f)<<2<< 8)
			|(((data>> 0)&0x1f)<<3<< 0)
		;
	}

	inline U32 Buffer::get_bgr565(U32 x, U32 y) {
		if((U32)x>=width||(U32)y>=height) return 0x000000;

		U16 data = *(U16*)&address[y*stride+x*2];
		return 0
			|(((data>> 0)&0x1f)<<3<<16)
			|(((data>> 5)&0x3f)<<2<< 8)
			|(((data>>11)&0x1f)<<3<< 0)
		;
	}

	inline U32 Buffer::get_rgb8(U32 x, U32 y) {
		if((U32)x>=width||(U32)y>=height) return 0x000000;

		auto offset = y*stride+x*3;
		return 0
			|(address[offset+0]<<16)
			|(address[offset+1]<< 8)
			|(address[offset+2]<< 0)
		;
	}

	inline U32 Buffer::get_bgr8(U32 x, U32 y) {
		if((U32)x>=width||(U32)y>=height) return 0x000000;

		auto offset = y*stride+x*3;
		return 0
			|(address[offset+2]<<16)
			|(address[offset+1]<< 8)
			|(address[offset+0]<< 0)
		;
	}

	inline U32 Buffer::get_rgba8(U32 x, U32 y) {
		if((U32)x>=width||(U32)y>=height) return 0x000000;

		auto offset = y*stride+x*4;
		return 0
			|address[offset+0]<<16
			|address[offset+1]<< 8
			|address[offset+2]<< 0
			|address[offset+3]<<24
		;
	}

	inline U32 Buffer::get_bgra8(U32 x, U32 y) {
		if((U32)x>=width||(U32)y>=height) return 0x000000;

		auto offset = y*stride+x*4;
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

		// clip height at bottom
		const U32 clippedHeight = max(0, min<I32>(startY+height, (I32)this->height)-(I32)startY);

		U32 y = 0;
		for(;y<clippedHeight;y++){
			I32 left = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			I32 right = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);

			// clip right at right
			right = min<I32>(startX+right, (I32)this->width)-(I32)startX;

			if(left<right){
				set(startX+left, startY+y, colour, right-left);
			}
		}
	}

	inline void Buffer::draw_rect_blended(U32 startX, U32 startY, U32 width, U32 height, U32 colour, U32 topLeftCorners[], U32 topRightCorners[], U32 bottomLeftCorners[], U32 bottomRightCorners[]) {
		U32 topLeftRadius = 0; if(topLeftCorners) while(topLeftRadius[topLeftCorners]!=~0u) topLeftRadius++;
		U32 topRightRadius = 0; if(topRightCorners) while(topRightRadius[topRightCorners]!=~0u) topRightRadius++;
		U32 bottomLeftRadius = 0; if(bottomLeftCorners) while(bottomLeftRadius[bottomLeftCorners]!=~0u) bottomLeftRadius++;
		U32 bottomRightRadius = 0; if(bottomRightCorners) while(bottomRightRadius[bottomRightCorners]!=~0u) bottomRightRadius++;

		// clip height at bottom
		const U32 clippedHeight = max(0, min<I32>(startY+height, (I32)this->height)-(I32)startY);

		U32 y = 0;
		for(;y<clippedHeight;y++){
			I32 left = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			I32 right = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);

			// clip right at right
			right = min<I32>(startX+right, (I32)this->width)-(I32)startX;

			if(left<right){
				for(auto x=(U32)left;x<(U32)right;x++){
					set_blended(startX+x, startY+y, colour);
				}
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

		// clip height at bottom
		const U32 clippedHeight = max(0, min<I32>(startY+height, (I32)this->height)-(I32)startY);

		U32 y = 0;

		I32 lastLeft1 = startX;
		I32 lastLeft2 = startX+width;
		I32 lastRight1 = lastLeft1;
		I32 lastRight2 = lastLeft2;

		for(;y<height/2&&y<borderWidth&&y<clippedHeight;y++){
			I32 left = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			I32 right = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);

			// clip right at right
			I32 clippedRight = min<I32>(startX+right, (I32)this->width)-(I32)startX;

			if(left<clippedRight){
				set(startX+left, startY+y, colour, clippedRight-left);
			}

			lastLeft1 = left;
			lastLeft2 = right;
			lastRight1 = left;
			lastRight2 = right;
		}

		for(;y<height&&y<height-borderWidth&&y<clippedHeight;y++){
			I32 left1 = maths::min(width-1, maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0));
			I32 left2 = maths::min(width, left1+borderWidth);
			I32 right2 = maths::max<I32>(0, (I32)width-(I32)maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0));
			I32 right1 = maths::max<I32>(0, (I32)right2-(I32)borderWidth);

			// clip right at right
			I32 maxRight = (I32)this->width-(I32)startX;

			{
				I32 left  = maths::min(left1, lastLeft2);
				I32 right = maths::min(maths::max(left2, lastLeft1), maxRight);
				if(left<right){
					set(startX+left, startY+y, colour, right-left);
				}
			}
			
			{
				I32 left  = maths::min(right1, lastRight2);
				I32 right = maths::min(maths::max(right2, lastRight1), maxRight);
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
		for(;y<clippedHeight;y++){
			I32 left1 = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			I32 right2 = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);

			// clip right at right
			right2 = min<I32>(startX+right2, (I32)this->width)-(I32)startX;

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

	inline void Buffer::draw_rect_outline_blended(U32 startX, U32 startY, U32 width, U32 height, U32 colour, U32 borderWidth, U32 topLeftCorners[], U32 topRightCorners[], U32 bottomLeftCorners[], U32 bottomRightCorners[]) {
		if(width<1||height<1) return;
		if(borderWidth<1) return;

		U32 topLeftRadius = 0; if(topLeftCorners) while(topLeftRadius[topLeftCorners]!=~0u) topLeftRadius++;
		U32 topRightRadius = 0; if(topRightCorners) while(topRightRadius[topRightCorners]!=~0u) topRightRadius++;
		U32 bottomLeftRadius = 0; if(bottomLeftCorners) while(bottomLeftRadius[bottomLeftCorners]!=~0u) bottomLeftRadius++;
		U32 bottomRightRadius = 0; if(bottomRightCorners) while(bottomRightRadius[bottomRightCorners]!=~0u) bottomRightRadius++;

		// clip height at bottom
		const U32 clippedHeight = max(0, min<I32>(startY+height, (I32)this->height)-(I32)startY);

		U32 y = 0;

		I32 lastLeft1 = startX;
		I32 lastLeft2 = startX+width;
		I32 lastRight1 = lastLeft1;
		I32 lastRight2 = lastLeft2;

		for(;y<height/2&&y<borderWidth&&y<clippedHeight;y++){
			I32 left = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			I32 right = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);

			// clip right at right
			I32 clippedRight = min<I32>(startX+right, (I32)this->width)-(I32)startX;

			if(left<clippedRight){
				set_blended(startX+left, startY+y, colour, clippedRight-left);
			}

			lastLeft1 = left;
			lastLeft2 = right;
			lastRight1 = left;
			lastRight2 = right;
		}

		for(;y<height&&y<height-borderWidth&&y<clippedHeight;y++){
			I32 left1 = maths::min(width-1, maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0));
			I32 left2 = maths::min(width, left1+borderWidth);
			I32 right2 = maths::max<I32>(0, (I32)width-(I32)maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0));
			I32 right1 = maths::max<I32>(0, (I32)right2-(I32)borderWidth);

			// clip right at right
			I32 maxRight = (I32)this->width-(I32)startX;

			{
				I32 left  = maths::min(left1, lastLeft2);
				I32 right = maths::min(maths::max(left2, lastLeft1), maxRight);
				if(left<right){
					set_blended(startX+left, startY+y, colour, right-left);
				}
			}
			
			{
				I32 left  = maths::min(right1, lastRight2);
				I32 right = maths::min(maths::max(right2, lastRight1), maxRight);
				if(left<right){
					set_blended(startX+left, startY+y, colour, right-left);
				}
			}

			lastLeft1 = left1;
			lastLeft2 = left2;
			lastRight1 = right1;
			lastRight2 = right2;
		}

		bool topRow = true;
		for(;y<clippedHeight;y++){
			I32 left1 = maths::max(topLeftCorners&&y<topLeftRadius?topLeftCorners[y]:0, bottomLeftCorners&&height-1-y<bottomLeftRadius?bottomLeftCorners[height-1-y]:0);
			I32 right2 = width-maths::max(topRightCorners&&y<topRightRadius?topRightCorners[y]:0, bottomRightCorners&&height-1-y<bottomRightRadius?bottomRightCorners[height-1-y]:0);

			// clip right at right
			right2 = min<I32>(startX+right2, (I32)this->width)-(I32)startX;

			if(topRow){
				if(lastLeft2<left1){
					set_blended(startX+left1, startY-1+y, colour, left1-lastLeft2);
				}

				if(right2<lastRight1){
					set_blended(startX+lastRight1, startY-1+y, colour, lastRight1-right2);
				}

				{
					U32 left = maths::max(left1, lastLeft2);
					U32 right = maths::min(right2, lastRight2);
					if(left<right){
						set_blended(startX+left, startY+y, colour, right-left);
					}
				}
				
				topRow = false;

			}else{
				if(left1<right2){
					set_blended(startX+left1, startY+y, colour, right2-left1);
				}
			}
		}
	}

	inline void Buffer::draw_line(U32 x1, U32 y1, U32 x2, U32 y2, U32 colour) {
		const I32 xVec = (I32)x2-(I32)x1;
		const I32 yVec = (I32)y2-(I32)y1;
		const I32 xLength = maths::abs(xVec);
		const I32 yLength = maths::abs(yVec);

		if(xLength>=yLength){
			const auto dir = maths::sign((I32)x2-(I32)x1);
			for(auto step=0;step<=xLength;step++){
				const auto phase = step/(float)xLength;
				const auto x = x1+step*dir;
				const auto y = y1 + phase*yVec;
				set(x, y, colour);
			}
		}else{
			const auto dir = maths::sign((I32)y2-(I32)y1);
			for(auto step=0;step<=yLength;step++){
				const auto phase = step/(float)yLength;
				const auto y = y1+step*dir;
				const auto x = x1 + phase*xVec;
				set(x, y, colour);
			}
		}
	}

	inline void Buffer::draw_line_blended(U32 x1, U32 y1, U32 x2, U32 y2, U32 colour) {
		const I32 xVec = (I32)x2-(I32)x1;
		const I32 yVec = (I32)y2-(I32)y1;
		const I32 xLength = maths::abs(xVec);
		const I32 yLength = maths::abs(yVec);

		if(xLength>=yLength){
			const auto dir = maths::sign((I32)x2-(I32)x1);
			for(auto step=0;step<=xLength;step++){
				const auto phase = step/(float)xLength;
				const auto x = x1+step*dir;
				const auto y = y1 + phase*yVec;
				set_blended(x, y, colour);
			}
		}else{
			const auto dir = maths::sign((I32)y2-(I32)y1);
			for(auto step=0;step<=yLength;step++){
				const auto phase = step/(float)yLength;
				const auto y = y1+step*dir;
				const auto x = x1 + phase*xVec;
				set_blended(x, y, colour);
			}
		}
	}

	inline void Buffer::draw_line_aa(U32 x1, U32 y1, U32 x2, U32 y2, U32 colour) {
		auto xVec = (I32)x2-(I32)x1;
		auto yVec = (I32)y2-(I32)y1;
		const I32 xLength = maths::abs(xVec);
		const I32 yLength = maths::abs(yVec);

		if(xLength==0||yLength==0) return draw_line(x1, y1, x2, y2, colour);

		auto xDir = maths::sign((I32)x2-(I32)x1);
		auto yDir = maths::sign((I32)y2-(I32)y1);

		if(xLength>=yLength){
			if(yDir<0){
				// swap
				auto x3=x1; x1=x2; x2=x3;
				auto y3=y1; y1=y2; y2=y3;
				xVec = -xVec;
				yVec = -yVec;
				xDir = -xDir;
				yDir = -yDir;
			}

			for(auto step=0;step<=xLength;step++){
				const auto phase = step/(float)xLength;
				const auto x = x1+step*xDir;
				const auto y = y1 + phase*yVec;
				const U32 trans = 255*(y-(U32)y);
				set_blended(x, y, premultiply_colour(colour|(trans<<24)));
				set_blended(x, y+yDir, premultiply_colour(colour|((255-trans)<<24)));
			}
		}else{
			if(xDir<0){
				// swap
				auto x3=x1; x1=x2; x2=x3;
				auto y3=y1; y1=y2; y2=y3;
				xVec = -xVec;
				yVec = -yVec;
				xDir = -xDir;
				yDir = -yDir;
			}

			for(auto step=0;step<=yLength;step++){
				const auto phase = step/(float)yLength;
				const auto y = y1+step*yDir;
				const auto x = x1 + phase*xVec;
				const U32 trans = 255*(x-(U32)x);
				set_blended(x, y, premultiply_colour(colour|(trans<<24)));
				set_blended(x+xDir, y, premultiply_colour(colour|((255-trans)<<24)));
			}
		}
	}

	inline void Buffer::draw_msdf(I32 startX, I32 startY, U32 width, U32 height, Buffer &source, I32 source_x, I32 source_y, U32 source_width, U32 source_height, U32 colour, U32 skipSourceLeft, U32 skipSourceTop, U32 skipSourceRight, U32 skipSourceBottom) {
		if(width<1||height<1) return;

		//minify by sampling multiple times (looks best when <= halfsize)
		if(width<=source_width&&height<=source_height){
			// // floor: (this is sharp, but a bit too sharp)
			// // const auto samplesX = source_width/width;
			// // const auto samplesY = source_height/height;

			// // floor+1/2-2: (this is a bit too soft)
			// // const auto samplesX = (source_width+source_width/2-1)/width;
			// // const auto samplesY = (source_height+source_height/2-1)/height;

			// // floor+1/6: (this seems a good mix)
			// const auto samplesX = (source_width+source_width*1/6)/width;
			// const auto samplesY = (source_height+source_height*1/6)/height;

			for(I32 y=0; y<(I32)height&&startY+y<(I32)this->height; y++) for(I32 x=0; x<(I32)width&&startX+x<(I32)this->width; x++) {
				I32 sX = source_x+x*source_width/width;
				I32 sY = source_y+y*source_height/height;

				// calculcate per pixel instead: (correct mixing)
				const auto samplesX = (source_x+(x+1)*source_width/width)-sX;
				const auto samplesY = (source_y+(y+1)*source_height/height)-sY;

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

				// note that we divide by the _expected_ number of samples, not the actual (minus those skipped). We _want_ the edges to fade out there, as they are not full covering those pixels
				coverage /= samplesX*samplesY;

				U8 alphaSmall = maths::clamp(coverage*3/2, 0u, 255u); // technically this should maybe be *2, but we'll go for a mid of *3/2
				U8 alphaLarge = maths::clamp(((I32)coverage-128)*2 + 128, 0, 255);

				U8 alpha = mix(alphaSmall, alphaLarge, height<source_height/2?0:255*(height-source_height/2)/(source_height/2));

				if(alpha<1) continue;

				set_blended(startX+x, startY+y, premultiply_colour((colour&0xffffff)|((255-alpha)*(255-(colour>>24))/255)<<24));
			}

		//bilinear filter the msdf (looks best at half size and up, although blurs some letters slightly)
		}else{
			const I32 sdfRange = 1;
			const I32 scale = maths::max<U32>(1u, ((width+source_width-1)/source_width + (height+source_height-1)/source_height)/2);
			// const I32 scale = 2;
			const I32 sdfPixels = sdfRange * scale;

			source_x *= 256;
			source_y *= 256;

			for(I32 y=max((I32)0,-startY); y<(I32)height&&startY+y<(I32)this->height; y++) for(I32 x=max((I32)0,-startX); x<(I32)width&&startX+x<(I32)this->width; x++) {
				I32 sX = source_x+(x*256)*source_width/width;
				I32 sY = source_y+(y*256)*source_height/height;

				U32 x1y1 = source.get((sX+  0)/256, (sY+  0)/256);
				U32 x2y1 = source.get((sX+256)/256, (sY+  0)/256);
				U32 x1y2 = source.get((sX+  0)/256, (sY+256)/256);
				U32 x2y2 = source.get((sX+256)/256, (sY+256)/256);

				U8 pX = sX%256;
				U8 pY = sY%256;

				U32 msdf = blend_rgb(blend_rgb(x1y1, x2y1, pX), blend_rgb(x1y2, x2y2, pX), pY);
				U8 r = (msdf&0xff0000)>>16;
				U8 g = (msdf&0x00ff00)>> 8;
				U8 b = (msdf&0x0000ff)>> 0;

				auto median = max(min(r, g), min(max(r, g), b));

				I32 screenPxDistance = ((I32)median-128)*sdfPixels;

				U8 alpha = maths::clamp(screenPxDistance*3/2 + 128, 0, 255); // technically this should maybe be *2, but we'll go for a mid of *3/2 
				// U8 alpha = maths::clamp(screenPxDistance*2 + 128, 0, 255);

				if(alpha<1) continue;

				set_blended(startX+x, startY+y, premultiply_colour((colour&0xffffff)|((255-alpha)*(255-(colour>>24))/255)<<24));
			}
		}
	}

	// TODO: optimise this to use memcpy strips? (if it's the same format)
	inline void Buffer::draw_buffer(I32 destX, I32 destY, U32 sourceX, U32 sourceY, U32 width, U32 height, Buffer &image) {
		if(destX>=(I32)this->width||destY>=(I32)this->height||destX+width<=0||destY+height<=0) return;

		for(U32 y=maths::max((I32)0, -destX); y<maths::min(maths::min(height, image.height-sourceX), height-destX); y++){
			for(U32 x=maths::max((I32)0, -destX); x<maths::min(maths::min(width, image.width-sourceX), width-destX); x++){
				set(destX+x, destY+y, image.get(sourceX+x, sourceY+y));
			}
		}
	}

	inline void Buffer::draw_scaled_buffer(U32 x, U32 y, U32 width, U32 height, Buffer &image, U32 imageX, U32 imageY, U32 imageWidth, U32 imageHeight, DrawScaledBufferOptions options) {
		if(options.minFiltered){
			const auto samplesX = maths::max(1u, imageWidth/width);
			const auto samplesY = maths::max(1u, imageHeight/height);
			const auto samples = samplesX*samplesY;

			for(auto offsetY=0u; offsetY<height; offsetY++)
			for(auto offsetX=0u; offsetX<width; offsetX++) {
				auto r = 0;
				auto g = 0;
				auto b = 0;
				auto a = 0;

				for(auto sampleY=0u; sampleY<samplesY; sampleY++)
				for(auto sampleX=0u; sampleX<samplesX; sampleX++) {
					const auto sample = image.get(imageWidth*offsetX/width+sampleX, imageHeight*offsetY/height+sampleY);
					a += sample>>24 & 0xff;
					r += sample>>16 & 0xff;
					g += sample>> 8 & 0xff;
					b += sample>> 0 & 0xff;
				}

				r /= samples;
				g /= samples;
				b /= samples;
				a /= samples;

				set(x+offsetX, y+offsetY, a<<24 | r<<16 | g<<8 | b<<0);
			}

		}else{
			for(auto offsetY=0u; offsetY<height; offsetY++)
			for(auto offsetX=0u; offsetX<width; offsetX++) {
				set(x+offsetX, y+offsetY, image.get((imageWidth-1)*offsetX/(width-1), (imageHeight-1)*offsetY/(height-1)));
			}
		}
	}

	inline void Buffer::draw_scaled_buffer_blended(U32 x, U32 y, U32 width, U32 height, Buffer &image, U32 imageX, U32 imageY, U32 imageWidth, U32 imageHeight, DrawScaledBufferOptions options, U8 opacity) {
		if(width<1||height<1) return;

		if(options.minFiltered){
			auto samplesX = maths::max(1u, (imageWidth+width-1)/width);
			auto samplesY = maths::max(1u, (imageHeight+height-1)/height);
			const auto samples = samplesX*samplesY;

			for(auto offsetY=0u; offsetY<height; offsetY++)
			for(auto offsetX=0u; offsetX<width; offsetX++) {
				auto r = 0;
				auto g = 0;
				auto b = 0;
				auto a = 0;

				// const auto samplesX = maths::max(1u, (imageWidth*(offsetX+1)/width)-(imageWidth*offsetX/width));
				// const auto samplesY = maths::max(1u, (imageHeight*(offsetY+1)/height)-(imageHeight*offsetY/height));
				// const auto samples = samplesX*samplesY;

				for(auto sampleY=0u; sampleY<samplesY; sampleY++)
				for(auto sampleX=0u; sampleX<samplesX; sampleX++) {
					const auto sample = image.get(imageX+imageWidth*offsetX/width+sampleX, imageY+imageHeight*offsetY/height+sampleY);
					a += sample>>24 & 0xff;
					r += sample>>16 & 0xff;
					g += sample>> 8 & 0xff;
					b += sample>> 0 & 0xff;
				}

				r /= samples;
				g /= samples;
				b /= samples;
				a /= samples;

				set_blended(x+offsetX, y+offsetY, a<<24 | r<<16 | g<<8 | b<<0, opacity);
			}

		}else{
			for(auto offsetY=0u; offsetY<height; offsetY++)
			for(auto offsetX=0u; offsetX<width; offsetX++) {
				set_blended(x+offsetX, y+offsetY, image.get(imageX+imageWidth*offsetX/width, imageY+imageHeight*offsetY/height), opacity);
			}
		}
	}

	inline void Buffer::draw_buffer_blended(I32 destX, I32 destY, U32 sourceX, U32 sourceY, U32 width, U32 height, Buffer &image, U8 opacity) {
		if(destX>=(I32)this->width||destY>=(I32)this->height||destX+width<=0||destY+height<=0) return;

		for(U32 y=maths::max((I32)0, -destX); y<maths::min(maths::min(height, image.height-sourceX), height-destX); y++){
			for(U32 x=maths::max((I32)0, -destX); x<maths::min(maths::min(width, image.width-sourceX), width-destX); x++){
				set_blended(destX+x, destY+y, image.get(sourceX+x, sourceY+y), opacity);
			}
		}
	}

	inline void Buffer::draw_4slice(I32 startX, I32 startY, U32 width, U32 height, Buffer &image) {
		if(startX>=(I32)this->width||startY>=(I32)this->height||startX+width<=0||startY+height<=0) return;

		U32 cornerWidth = maths::min(width/2, image.width);
		U32 cornerHeight = maths::min(height/2, image.height);

		draw_buffer(startX, startY, 0, 0, cornerWidth, cornerHeight, image);
		draw_buffer(startX+width-cornerWidth, startY, image.width-cornerWidth, 0, cornerWidth, cornerHeight, image);
		draw_buffer(startX, startY+height-cornerHeight, 0, image.height-cornerHeight, cornerWidth, cornerHeight, image);
		draw_buffer(startX+width-cornerWidth, startY+height-cornerHeight, image.width-cornerWidth, image.height-cornerHeight, cornerWidth, cornerHeight, image);

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
					#ifdef HAS_UNALIGNED_ACCESS
						memmove(&address[y*stride+x1], &address[y*stride+x2], width);
					#else
						memcpy_backwards_aligned(&address[y*stride+x1], &address[y*stride+x2], width);
					#endif
				}
			}else{
				for(U32 y=0;y<height+scrollY;y++){
					#ifdef HAS_UNALIGNED_ACCESS
						memmove(&address[y*stride+x1], &address[y*stride+x2], width);
					#else
						memcpy_forwards_aligned(&address[y*stride+x1], &address[y*stride+x2], width);
					#endif
				}
			}

		}else if(scrollY>0){
			for(U32 y=height-1;y>(U32)scrollY;y--){
				memcpy(&address[y*stride+x1], &address[(y-scrollY)*stride+x2], stride);
			}
		}else{
			// for(I32 y=height+scrollY-1;y>=0;y--){
			for(U32 y=0;y<height+scrollY;y++){
				memcpy(&address[y*stride+x1], &address[(y-scrollY)*stride+x2], stride);
			}
		}
	}

	inline U32 blend_rgb(U32 a, U32 b, float phase) {
		return
			 (U32)(((a&0xff0000)>>16)*(1-phase) + ((b&0xff0000)>>16)*(0+phase))<<16
			|(U32)(((a&0x00ff00)>> 8)*(1-phase) + ((b&0x00ff00)>> 8)*(0+phase))<< 8
			|(U32)(((a&0x0000ff)>> 0)*(1-phase) + ((b&0x0000ff)>> 0)*(0+phase))<< 0
		;
	}

	inline U32 blend_rgb(U32 a, U32 b, U8 phase) {
		return
			 (U32)(((a&0xff0000)>>16)*(255-phase)/255 + ((b&0xff0000)>>16)*(0+phase)/255)<<16
			|(U32)(((a&0x00ff00)>> 8)*(255-phase)/255 + ((b&0x00ff00)>> 8)*(0+phase)/255)<< 8
			|(U32)(((a&0x0000ff)>> 0)*(255-phase)/255 + ((b&0x0000ff)>> 0)*(0+phase)/255)<< 0
		;
	}

	inline auto Buffer::cropped(U32 left, U32 top, U32 right, U32 bottom) -> Buffer {
		return region(left, top, width-left-right, height-top-bottom);
	}

	inline auto Buffer::region(U32 x, U32 y, U32 width, U32 height) -> Buffer {
		const auto bpp = bufferFormat::size[(U8)format];

		return Buffer(
			address+y*stride+x*bpp,
			stride,
			width,
			height,
			format, order
		);
	}
}
