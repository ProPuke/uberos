#include "DisplayManager.hpp"

#include <drivers/Graphics.hpp>

#include <kernel/drivers.hpp>
#include <kernel/Lock.hpp>
#include <kernel/logging.hpp>
#include <kernel/memory.hpp>
#include <kernel/mmio.hpp>

#include <common/stdlib.hpp>

// #define BACKGROUND_GRID
#define BACKGROUND_STRIP

namespace driver {
	namespace {
		struct Framebuffer {
			driver::Graphics *driver;
			U32 driverFramebuffer;
			graphics2d::Buffer *buffer;
			graphics2d::Rect area;
		};

		graphics2d::Rect totalArea;

		ListOrdered<Framebuffer> framebuffers;
		LList<DisplayManager::Display> displays;

		U32 windowBackgroundColour = 0x202020;

		#if defined(BACKGROUND_GRID)
			U32 backgroundColour2 = 0x282828;
			// U32 backgroundColour = 0xa0a0a0;
			// U32 backgroundColour2 = 0xa8a8a8;

		#elif defined(BACKGROUND_STRIP)
			U8 backgroundStrip[256*3+1] = "<\222\263<\222\263=\223\263=\223\264>\224\264>\224\264?\225\264@\225\264@\225\264A\226\264B\226\264C\227\265D\227\265E\230\265F\230\265G\231\266H\231\266I\232\266K\233\267L\234\267M\234\270N\235\270P\235\270Q\236\271R\236\271S\237\271T\240\271U\240\272V\241\272W\242\272X\242\273Z\243\273[\244\274]\245\274^\246\275_\246\275a\247\276b\250\276d\251\277e\251\277f\252\300g\253\300h\254\300i\254\301j\255\301k\255\301l\256\301m\257\302n\260\302p\260\303q\261\303r\262\304t\263\304u\264\305v\264\305x\265\305y\266\306z\266\306{\267\306|\267\307~\270\307\177\271\310\200\272\310\201\272\310\202\273\311\203\274\311\204\274\312\206\275\312\207\276\313\210\277\314\211\300\314\212\300\315\214\301\315\215\302\315\216\302\316\217\303\316\220\304\316\220\304\317\221\305\317\223\305\317\224\306\317\225\307\320\226\307\320\227\310\320\230\311\321\231\311\321\232\312\321\233\312\322\234\313\322\235\313\322\236\314\323\237\315\323\240\315\323\242\316\324\243\316\324\244\317\325\245\317\325\246\320\325\247\320\325\250\320\325\251\321\326\251\321\326\252\322\326\253\322\326\254\323\327\255\323\327\257\324\327\260\324\327\261\325\330\262\325\330\263\326\330\265\326\331\266\327\331\267\330\331\270\330\332\271\330\332\272\331\332\273\331\332\274\332\333\274\332\333\275\332\333\276\332\333\277\333\333\277\333\333\300\333\333\301\333\333\302\334\333\303\334\333\304\334\333\306\335\333\307\335\334\310\335\334\311\336\334\312\336\334\313\337\334\314\337\335\316\337\335\317\340\335\320\340\335\321\341\335\321\341\335\322\341\335\323\341\335\324\342\335\325\342\335\325\342\335\326\342\335\326\343\334\327\343\334\327\343\334\327\342\334\326\342\333\324\340\332\321\336\331\315\333\326\306\327\324\275\322\320\240\303\307\220\274\302\210\267\276\210\267\276\212\270\277}\260\272\200\262\274\204\265\276\201\263\275{\260\273w\255\270v\253\270v\252\267{\255\267|\256\267{\255\270|\256\271x\252\265x\252\265w\251\264v\250\263v\246\262s\243\260o\240\256m\237\255L\177\227Fz\222Ex\222Cv\221G}\225Cv\220@r\215@r\215>p\213@r\215Bq\215Gr\213Ks\214Hp\211Su\213Ru\214Iq\212Al\207@l\211\067f\206\065d\204\063c\203\061b\203\060b\203/a\202/`\202.`\201._\201._\201._\200-_\200,^\177+]~+\\}*[|)Z{)Yz)Xy(Vx'Uv&St%Qr#No#Lm\"Lm!Jj\"Kk\040Ih\040Ii\037Ff\037Ed\036Db\036Ca\036B`\037B`\033>Z\034?[\032<W\033=X\033=X\033<W\033;V\033:T\033:S\033\071Q\032\070O\032\067N\031\066L\030\062G\030\061G\027\060F\027\060E\026/D\027\060E";
		#endif

		Lock<LockType::recursive> lock;
		Lock<LockType::flat> displaysLock;

		void _set_background_colour(U32 colour);

		auto _create_view(Thread *thread, DisplayManager::DisplayLayer layer, U32 width, U32 height, U8 scale=1) -> DisplayManager::Display*;
		void _move_display_to(DisplayManager::Display&, I32 x, I32 y, bool update);
		void _resize_display_to(DisplayManager::Display&, U32 width, U32 height, bool update);
		void _place_above(DisplayManager::Display&, DisplayManager::Display &other);
		void _place_below(DisplayManager::Display&, DisplayManager::Display &other);
		void _raise_display(DisplayManager::Display&);
		void _set_display_layer(DisplayManager::Display&, DisplayManager::DisplayLayer);
		auto _is_display_top(DisplayManager::Display&) -> bool;
		void _show_display(DisplayManager::Display&);
		void _hide_display(DisplayManager::Display&);
		auto _find_display_section_in_row(I32 &x, I32 y, I32 x2, bool &isTransparent, DisplayManager::Display *current = nullptr) -> DisplayManager::Display*;
		void _update_framebuffer_positions();
		void _update_background();
		void _update_background_area(graphics2d::Rect);
		void _update_area(graphics2d::Rect, DisplayManager::Display *below = nullptr);
		void _update_area_transparency(graphics2d::Rect);
		void _update_area_solid(graphics2d::Rect, DisplayManager::Display *below = nullptr);
		void _update_display_solid(DisplayManager::Display&);
		void _update_display_area_solid(DisplayManager::Display&, graphics2d::Rect);
		void _calculate_blending_at(Framebuffer&, U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay);
		auto _get_screen_buffer(U32 framebuffer, graphics2d::Rect) -> Optional<graphics2d::Buffer>;

		void _set_background_colour(U32 colour) {
			if(windowBackgroundColour==colour) return;

			windowBackgroundColour = colour;
			_update_background();
		}

		void _on_thread_event(const Thread::Event &event, void *_display) {
			auto &display = *(DisplayManager::Display*)_display;

			switch(event.type){
				case Thread::Event::Type::terminated:
					delete &display;
				break;
			}
		}

		DisplayManager::Display* _create_view(Thread *thread, DisplayManager::DisplayLayer layer, U32 width, U32 height, U8 scale) {
			#ifdef DEBUG_MEMORY
				logging::Section section("create_view ", width, "x", height);
			#endif

			if(framebuffers.length<1) return nullptr; //TODO: handle this more elegantly? Make this function a Try?

			auto &framebuffer = framebuffers[0]; //TODO: pick out the correct framebuffer based on position
			auto bpp = graphics2d::bufferFormat::size[(U8)framebuffer.buffer->format];

			#ifdef DEBUG_MEMORY
				debug::trace("new U8 ", width, "x", height, "@", bpp, "\n");
			#endif

			auto buffer = new U8[width*height*bpp]; //TODO:allocate from pages and map to current thread

			#ifdef DEBUG_MEMORY
				debug::trace("new U8 ", width, "x", height, "@", bpp, " = ", buffer, "\n");
			#endif

			if(!buffer) return nullptr;

			bzero(buffer, width*height*bpp);

			auto display = new DisplayManager::Display(thread, buffer, layer, framebuffer.buffer->format, framebuffer.buffer->order, width, height, scale);
			display->x = ((I32)framebuffer.buffer->width-(I32)width)/2;
			display->y = max(0, ((I32)framebuffer.buffer->height-(I32)height)/2);
			if(!display) return nullptr;

			if(displays.size<1){
				displays.push_back(*display);
			}else{
				for(auto other = displays.tail;other;other = other->prev){
					if(other->layer<=display->layer){
						displays.insert_after(*other, *display);
						goto inserted;
					}
				}
				displays.insert_before(*displays.head, *display);
				inserted:;
			}

			if(thread){
				thread->events.subscribe(_on_thread_event, display);
			}

			#ifdef DEBUG_MEMORY
				debug::trace("created display ", display);
			#endif

			return display;
		}

		auto _sample_background_at(I32 x, I32 y) -> U32;
		#if defined(BACKGROUND_STRIP)
			auto _sample_background_strip_at(U32 y) -> U32;
		#endif

		template <graphics2d::BufferFormatOrder>
		void __calculate_blending_at(U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay);

		void _calculate_blending_at(Framebuffer &framebuffer, U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay) {
			switch(framebuffer.buffer->order){
				case graphics2d::BufferFormatOrder::argb:
					__calculate_blending_at<graphics2d::BufferFormatOrder::argb>(buffer, x, y, topDisplay);
				break;
				case graphics2d::BufferFormatOrder::bgra:
				default:
					__calculate_blending_at<graphics2d::BufferFormatOrder::bgra>(buffer, x, y, topDisplay);
				break;
			}
		}

		template <graphics2d::BufferFormatOrder order>
		union __attribute__((packed)) PackedPixel;

		template <>
		union __attribute__((packed)) PackedPixel<graphics2d::BufferFormatOrder::bgra> {
			struct __attribute__((packed)) {
				U8 b;
				U8 g;
				U8 r;
				U8 a;
			};
			U32 value;
		};

		template <>
		union __attribute__((packed)) PackedPixel<graphics2d::BufferFormatOrder::argb> {
			struct __attribute__((packed)) {
				U8 a;
				U8 r;
				U8 g;
				U8 b;
			};
			U32 value;
		};

		//TODO: implement alpha testing instead on non rgba modes (palette index 0 or 0xff00ff perhaps?)
		template <graphics2d::BufferFormatOrder formatOrder>
		void __calculate_blending_at(U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay) {
			PackedPixel<formatOrder> result{0,0,0,0};
			U8 visibility = 255;

			for(auto display=topDisplay; display; display=display->prev){
				if(!display->isVisible) continue;

				// check just y
				if(display->y>y||display->y+(I32)display->get_height()<=y) continue;

				// check x (+ corners)
				const auto displayY = y-display->y;
				if(
					display->x+(I32)display->get_left_margin(displayY)>x||
					display->x+(I32)display->get_width()-(I32)display->get_right_margin(displayY)<=x
				) continue;

				const auto displayX = x-display->x;

				auto &read = *(PackedPixel<formatOrder>*)&display->buffer.address[(displayY/display->scale)*display->buffer.stride+(displayX/display->scale)*4];

				result.r = min(result.r + read.r*(U32)visibility/255, 255u);
				result.g = min(result.g + read.g*(U32)visibility/255, 255u);
				result.b = min(result.b + read.b*(U32)visibility/255, 255u);

				auto displayTransparent = !display->solidArea.contains(displayX, displayY);

				// return early if we hit something solid
				if(!displayTransparent) goto done;

				// all visibility used up
				if(visibility<=255-read.a) goto done;

				visibility -= 255-read.a;
			}

			// if(visibility==255) return; //nothing was found

			{
				auto bg = _sample_background_at(x, y);

				result.r += min((bg>>16 & 0xff) * (U32)visibility/255u, 255u);
				result.g += min((bg>> 8 & 0xff) * (U32)visibility/255u, 255u);
				result.b += min((bg>> 0 & 0xff) * (U32)visibility/255u, 255u);
			}

			done:

			*buffer++ = result.value;
		}

		auto _find_display_section_in_row(I32 &x, I32 y, I32 x2, bool &isTransparent, DisplayManager::Display *current) -> DisplayManager::Display* {
			I32 foundLeft = x2;

			if(current){
				auto currentX = x-current->x;
				auto currentY = y-current->y;

				if(currentY<current->solidArea.y1||currentY>=current->solidArea.y2){
					; // single section top and bottom

				}else if(current->solidArea.x1>0&&currentX<current->solidArea.x1){
					foundLeft = current->x+current->solidArea.x1;

				}else if(currentX<current->solidArea.x2){
					foundLeft = current->x+current->solidArea.x2;

				}else{
					foundLeft = current->x+current->get_width();
				}

				foundLeft = maths::clamp(foundLeft, current->x+(I32)current->get_left_margin(currentY), current->x+(I32)current->get_width()-(I32)current->get_right_margin(currentY));
			}
			
			DisplayManager::Display *found = nullptr;
			auto foundTransparent = false;

			for(auto display=displays.tail; display; display=display->prev){
				if(!display->isVisible) goto next;

				if(display==current){
					// abort, we've now reached the current window. There is nothing more on top of it to check.
					break;
				}

				// check just y
				if(display->y>y||display->y+(I32)display->get_height()<=y) goto next;

				{
					// check x (+ corners)
					const auto displayX = x-display->x;
					const auto displayY = y-display->y;
					auto displayLeft = display->x+(I32)display->get_left_margin(displayY);
					auto displayRight = display->x+(I32)display->get_width()-(I32)display->get_right_margin(displayY);

					if(displayRight<=x) goto next;

					auto transparent = false;

					if(true){ // chop into separate regions when using solidArea

						if(displayY<display->solidArea.y1||displayY>=display->solidArea.y2){
							// top and bottom transparent areas
							transparent = true;

						}else{
							if(display->solidArea.x1>0&&displayX<display->solidArea.x1){
								// left transparent edge
								displayRight = min(display->x+display->solidArea.x1, displayRight);
								transparent = true;

							}else if(displayX<display->solidArea.x2){
								// centre solid area
								displayLeft = max(displayLeft, display->x+display->solidArea.x1);
								displayRight = min(display->x+display->solidArea.x2, displayRight);

							}else{
								// right transparent edge
								displayLeft = max(displayLeft, display->x+display->solidArea.x2);
								transparent = true;
							}
						}
					}

					if(displayLeft<=x){
						// we're already inside it. Return immediately
						foundTransparent = transparent;
						found = display;
						goto done;

					}else if(displayLeft<foundLeft){
						// it's further to the right, so set if the best match so far
						foundLeft = displayLeft;
						foundTransparent = transparent;
						found = display;
					}
				}

				next:
				;
			}

			x = min(foundLeft, x2);

			done:

			isTransparent = foundTransparent;
			return found;
		}

		void _update_area(graphics2d::Rect rect, DisplayManager::Display *below) {
			_update_area_solid(rect, below);
			_update_area_transparency(rect); //TODO: pass this the below rect to extrude
		}

		//TODO: take a rect area to exclude
		void _update_area_transparency(graphics2d::Rect screenRect) {
			for(auto &framebuffer:framebuffers){
				if(!framebuffer.buffer) continue;

				auto rect = screenRect.intersect(framebuffer.area);

				for(auto y=rect.y1; y<rect.y2; y++){
					auto display = (DisplayManager::Display*)nullptr;
					auto isTransparent = false;

					for(auto scanX=rect.x1; scanX<rect.x2;){
						auto nextScanX = scanX;
						auto nextIsTransparent = false;
						auto nextDisplay = _find_display_section_in_row(nextScanX, y, rect.x2, nextIsTransparent, display);
						if(!display&&!nextDisplay) break; // we didn't end a current display, and didn't find a new one - this row is done

						if(display){
							if(isTransparent){
								// render transparent strip to framebuffer
								// do so in buffered chunks, to hopefully speed up the process

								auto framebufferAddress = &framebuffer.buffer->address[(y-framebuffer.area.y1)*framebuffer.buffer->stride+(scanX-framebuffer.area.x1)*4];
								U32 buffer[256];
								auto bufferPosition = buffer;

								for(auto x=scanX; x<nextScanX; x++){
									_calculate_blending_at(framebuffer, bufferPosition, x, y, display);

									if(bufferPosition>=&buffer[sizeof(buffer)/sizeof(buffer[0])]){
										memcpy(framebufferAddress, buffer, (U8*)bufferPosition-(U8*)buffer);
										framebufferAddress += (U8*)bufferPosition-(U8*)buffer;
										bufferPosition = buffer;
									}
								}

								if(bufferPosition>buffer){
									memcpy(framebufferAddress, buffer, (U8*)bufferPosition-(U8*)buffer);
								}
							}
						}

						display = nextDisplay;
						isTransparent = nextIsTransparent;
						scanX = nextScanX;
					}
				}
			}
		}

		void _update_area_solid(graphics2d::Rect rect, DisplayManager::Display *below) {
			_update_background_area(rect);

			for(auto display=displays.head; display; display=display->next){
				if(display==below) break;
				if(!display->isVisible) continue;

				_update_display_area_solid(*display, rect.offset(-display->x, -display->y));
			}
		}

		void _update_framebuffer_positions() {
			auto x = 0;
			auto y = 0;

			totalArea.clear();

			for(auto &framebuffer:framebuffers){
				if(!framebuffer.buffer) break; // a buffer is in a transitional state, so abort updating later stuff that depends on it!

				auto newArea = graphics2d::Rect{x, y, x+(I32)framebuffer.buffer->width, y+(I32)(I32)framebuffer.buffer->height};

				totalArea = totalArea.include(newArea);

				if(framebuffer.area != newArea){
					framebuffer.area = newArea;
					_update_area(framebuffer.area);
				}

				x = framebuffer.area.x2;
			}
		}

		void _update_background() {
			_update_background_area(totalArea);
		}

		#pragma GCC push_options
		#pragma GCC optimize ("-O3")
		#if defined(BACKGROUND_GRID)
			auto _sample_background_at(I32 x, I32 y) -> U32 {
				const I32 edge = 64;
				U32 colour = (x/30+y/30)%2?windowBackgroundColour:backgroundColour2;

				if(x<edge) {
					const U32 fade = edge-x;
					I32 r = max<I32>(0, ((colour>>16)&0xff)-fade/5);
					I32 g = max<I32>(0, ((colour>> 8)&0xff)-fade/5);
					I32 b = max<I32>(0, ((colour>> 0)&0xff)-fade/5);
					return r<<16|g<<8|b;

				} else {
					const auto right = totalArea.x2-(I32)edge;

					if(x<right){
						return colour;

					} else {
						const U32 fade = edge-(totalArea.x2-x);
						I32 r = max<I32>(0, ((colour>>16)&0xff)-fade/5);
						I32 g = max<I32>(0, ((colour>> 8)&0xff)-fade/5);
						I32 b = max<I32>(0, ((colour>> 0)&0xff)-fade/5);
						return r<<16|g<<8|b;
					}
				}
			}
		#elif defined(BACKGROUND_STRIP)
			auto _sample_background_at(I32 x, I32 y) -> U32 {
				return _sample_background_strip_at(y);
			}

			auto _sample_background_strip_at(U32 y) -> U32 {
				y -= totalArea.y1;
				auto pos1 = y*256/totalArea.height();
				auto pos2 = min(255u, y*256/totalArea.height()+1);

				auto y1 = pos1*totalArea.height()/256;
				auto y2 = pos2*totalArea.height()/256;

				U32 sample1 =
					backgroundStrip[pos1*3+0]<<16|
					backgroundStrip[pos1*3+1]<< 8|
					backgroundStrip[pos1*3+2]<< 0
				;

				// if(y2==y1) return sample1;

				U32 sample2 =
					backgroundStrip[pos2*3+0]<<16|
					backgroundStrip[pos2*3+1]<< 8|
					backgroundStrip[pos2*3+2]<< 0
				;

				// return sample1;
				// return sample2;

				return y2>y1&&maths::abs(((sample1&0xff0000)>>16)-((sample2&0xff0000)>>16))<0x20?graphics2d::blend_rgb(sample1, sample2, (U8)(255*(y-y1)/(y2-y1))):sample1;
				// return y2>y1?graphics2d::blend_rgb(sample1, sample2, (256*(y-y1)/(y2-y1))/256.0f):sample1;
			}
		#endif

		void _update_background_area(graphics2d::Rect screenRect) {
			// mmio::PeripheralWriteGuard guard;

			// debug::trace("update background area");

			for(auto &framebuffer:framebuffers){
				if(!framebuffer.buffer) continue;

				// auto bpp = graphics2d::bufferFormat::size[(U8)framebuffer.buffer->format];

				auto rect = screenRect.intersect(framebuffer.area);

				// debug::trace("paint background ", rect.y1, " to ", rect.y2);

				for(auto y=rect.y1;y<rect.y2;y++){
					// debug::trace("y = ", y, "\n");
					auto startX = rect.x1;
					while(startX<rect.x2){
						auto endX = rect.x2;
						auto nextX = startX;
						for(auto display=displays.head; display; display=display->next){
							if(!display->isVisible) continue;

							auto viewArea = graphics2d::Rect{ display->x, display->y, display->x+(I32)display->get_width(), display->y+(I32)display->get_height() };

							if(viewArea.y1>y||viewArea.y2<=y) continue;

							const auto areaX1 = viewArea.x1+(I32)display->get_left_margin(y-display->y);
							const auto areaX2 = viewArea.x2-(I32)display->get_right_margin(y-display->y);

							if(areaX1<=startX){
								startX = maths::max(startX, areaX2);
								if(startX>=endX) break;

							}else if(areaX1<endX){
								endX = areaX1;
								nextX = maths::max(nextX, areaX2);
							}
						}

						if(endX>startX){
							#if defined(BACKGROUND_GRID)
								const I32 edge = 64;
								I32 x = startX;
								for(;x<min(endX,edge);x++) {
									const U32 fade = edge-x;
									const U32 colour = (x/30+y/30)%2?windowBackgroundColour:backgroundColour2;
									I32 r = max<I32>(0, ((colour>>16)&0xff)-fade/5);
									I32 g = max<I32>(0, ((colour>> 8)&0xff)-fade/5);
									I32 b = max<I32>(0, ((colour>> 0)&0xff)-fade/5);
									framebuffer.set(x, y, r<<16|g<<8|b);
								}
								{
									auto right = min(endX, (I32)totalArea.x2-(I32)edge);
									while(x<right){
										const U32 colour = (x/30+y/30)%2?windowBackgroundColour:backgroundColour2;
										auto next = min(((x/30)+1)*30, right);
										framebuffer.set(x, y, colour, next-x);
										x = next;
									}
								}
								for(;x<endX;x++) {
									const U32 fade = edge-(totalArea.x2-x);
									const U32 colour = (x/30+y/30)%2?windowBackgroundColour:backgroundColour2;
									I32 r = max<I32>(0, ((colour>>16)&0xff)-fade/5);
									I32 g = max<I32>(0, ((colour>> 8)&0xff)-fade/5);
									I32 b = max<I32>(0, ((colour>> 0)&0xff)-fade/5);
									framebuffer.set(x, y, r<<16|g<<8|b);
								}

							#elif defined(BACKGROUND_STRIP)
								auto bgColour = _sample_background_strip_at(y);
								framebuffer.buffer->set(startX-framebuffer.area.x1, y-framebuffer.area.y1, bgColour, endX-startX);
							#endif
						}

						if(nextX>endX){
							startX = nextX;
						}else{
							break;
						}
					}
				}
			}
		}

		#pragma GCC pop_options

		void _move_display_to(DisplayManager::Display &display, I32 x, I32 y, bool update) {
			if(display.x==x&&display.y==y) return;

			const auto oldX = display.x;
			const auto oldY = display.y;

			display.x = x;
			display.y = y;

			if(display.isVisible&&update){
				_update_display_solid(display);
				_update_area_solid({oldX, oldY, oldX+(I32)display.get_width(), oldY+(I32)display.get_height()}, &display);
				_update_area_transparency(
					graphics2d::Rect{oldX, oldY, oldX+(I32)display.get_width(), oldY+(I32)display.get_height()}
					.include({display.x, display.y, display.x+(I32)display.get_width(), display.y+(I32)display.get_height()})
				);
			}
		}

		void _resize_display_to(DisplayManager::Display &display, U32 width, U32 height, bool update) {
			width = max(16u, width);
			height = max(16u, height);
			if(display.get_width()==width&&display.get_height()==height) return;

			const auto oldWidth = display.get_width();
			const auto oldHeight = display.get_height();

			auto bpp = graphics2d::bufferFormat::size[(U8)display.buffer.format];

			delete display.buffer.address;
			display.buffer.address = new U8[width*height*bpp];
			display.buffer.width = width;
			display.buffer.height = height;
			display.buffer.stride = width*bpp;

			if(display.isVisible&&update){
				_update_area_solid({display.x, display.y, display.x+(I32)oldWidth, display.y+(I32)oldHeight}, &display);
				_update_area_transparency(
					graphics2d::Rect{display.x, display.y, display.x+(I32)oldWidth, display.y+(I32)oldHeight}
					.include({display.x, display.y, display.x+(I32)display.get_width(), display.y+(I32)display.get_height()})
				);
			}
		}

		void _place_above(DisplayManager::Display &display, DisplayManager::Display &other) {
			displays.pop(display);

			display.layer = other.layer;
			displays.insert_after(other, display);
			_update_display_solid(display); //technically we only need to draw the parts that were previously obscured, oh well..
			_update_area_transparency({display.x, display.y, display.x+(I32)display.get_width(), display.y+(I32)display.get_height()});
		}

		void _place_below(DisplayManager::Display &display, DisplayManager::Display &other) {
			displays.pop(display);

			display.layer = other.layer;
			displays.insert_before(other, display);
			_update_display_solid(display); //technically we only need to draw the parts that were previously obscured, oh well..
			_update_area_transparency({display.x, display.y, display.x+(I32)display.get_width(), display.y+(I32)display.get_height()});
		}

		void _raise_display(DisplayManager::Display &display) {
			if(_is_display_top(display)) return; //already topmost

			displays.pop(display);

			if(displays.size<1){
				displays.push_back(display);
			}else{
				for(auto other = displays.tail;other;other = other->prev){
					if(other->layer<=display.layer){
						displays.insert_after(*other, display);
						goto inserted;
					}
				}
				displays.insert_before(*displays.head, display);
				inserted:;
			}

			const auto rect = (graphics2d::Rect){display.x, display.y, display.x+(I32)display.get_width(), display.y+(I32)display.get_height()};

			_update_display_solid(display); //technically we only need to draw the parts that were previously obscured, oh well..
			_update_area_transparency(rect);
		}

		void _set_display_layer(DisplayManager::Display &display, DisplayManager::DisplayLayer layer) {
			if(display.layer==layer) return;

			displays.pop(display);

			display.layer = layer;

			for(auto other=displays.head; other; other=other->next){
				if(other->layer>=layer){
					displays.insert_before(*other, display);
					goto inserted;
				}
			}

			displays.push_back(display);

			inserted:

			const auto rect = (graphics2d::Rect){display.x, display.y, display.x+(I32)display.get_width(), display.y+(I32)display.get_height()};

			_update_display_solid(display); //technically we only need to draw the parts that were previously obscured, oh well..
			_update_area_transparency(rect);
		}

		auto _is_display_top(DisplayManager::Display &display) -> bool {
			return !display.next || display.next->layer>display.layer;
		}

		void _show_display(DisplayManager::Display &display) {
			if(display.isVisible) return;

			display.isVisible = true;

			const auto rect = (graphics2d::Rect){display.x, display.y, display.x+(I32)display.get_width(), display.y+(I32)display.get_height()};

			_update_display_solid(display);
			_update_area_transparency(rect);
		}

		void _hide_display(DisplayManager::Display &display) {
			if(!display.isVisible) return;

			display.isVisible = false;

			const auto rect = (graphics2d::Rect){display.x, display.y, display.x+(I32)display.get_width(), display.y+(I32)display.get_height()};

			_update_area(rect, &display);
		}

		inline void _update_display_solid(DisplayManager::Display &display) {
			return _update_display_area_solid(display, display.solidArea);
		};

		template <unsigned scale>
		void __update_display_solid_area(DisplayManager::Display&, graphics2d::Rect);

		void _update_display_area_solid(DisplayManager::Display &display, graphics2d::Rect rect) {
			if(!display.isVisible) return;
			// if(display.mode==DisplayManager::DisplayMode::transparent) return; // this is handled by blended rendering, not directly, so abort here

			switch(display.scale){
				case 1: __update_display_solid_area<1>(display, rect); break;
				case 2: __update_display_solid_area<2>(display, rect); break;
				case 3: __update_display_solid_area<3>(display, rect); break;
				case 4: __update_display_solid_area<4>(display, rect); break;
			}
		}

		#pragma GCC push_options
		#pragma GCC optimize ("-O3")
		template <unsigned scale>
		void __update_display_solid_area(DisplayManager::Display &display, graphics2d::Rect displayRect) {
			// mmio::PeripheralAccessGuard guard;

			for(auto &framebuffer:framebuffers){
				if(!framebuffer.buffer) continue;

				auto bpp = graphics2d::bufferFormat::size[(U8)framebuffer.buffer->format];

				auto rect = displayRect
					.intersect(display.solidArea)
					.offset(display.x, display.y).intersect(framebuffer.area).offset(-display.x, -display.y)
				;

				for(auto y=rect.y1; y<rect.y2; y++){
					auto startX = max(rect.x1, (I32)display.get_left_margin(y));
					const auto x2 = min(rect.x2, (I32)display.get_width()-(I32)display.get_right_margin(y));

					while(startX<x2){
						auto endX = x2;
						auto nextX = startX;
						for(auto displayAbove=display.next; displayAbove; displayAbove=displayAbove->next){
							if(!displayAbove->isVisible) continue;

							auto aboveArea =
								graphics2d::Rect{displayAbove->x, displayAbove->y, displayAbove->x+(I32)displayAbove->get_width(), displayAbove->y+(I32)displayAbove->get_height() }
								.offset(-display.x, -display.y)
							;

							if(aboveArea.y1>y||aboveArea.y2<=y) continue;

							const auto areaX1 = aboveArea.x1+(I32)displayAbove->get_left_margin(display.y+y-displayAbove->y);
							const auto areaX2 = aboveArea.x2-(I32)displayAbove->get_right_margin(display.y+y-displayAbove->y);

							if(areaX1<=startX){
								startX = max(startX, areaX2);
								if(startX>=endX) break;

							}else if(areaX1<endX){
								if(areaX2<endX){ //does this display end before the next one starts again?
									// if so, there's a gap between this display and the next blocking, so resume there
									nextX = areaX2;

								}else{
									nextX = max(nextX, areaX2);
								}

								endX = areaX1; // end drawing at the start of this display
							}

						}

						if(endX>startX){

							// U8 *target = &framebuffer.buffer->address[((display.y+y)*framebuffer.buffer->width+display.x+startX)*bpp];
							// U8 *source = &display.viewBuffer.address[((y/scale)*display.viewBuffer.width+startX/scale)*bpp];
							// U32 length = (endX-startX)*bpp;
							// if(source<display.viewBuffer.address||source+length>display.viewBuffer.address+display.viewBuffer.size){
							// 	debug::trace("READ OUT OF BUFFER! ", source, " -> ", source+length, " vs ", display.viewBuffer.address, " -> ", display.viewBuffer.address+display.viewBuffer.size, "\n");
							// }
							// debug::trace(source, " -> ", source+length, "\n");
							// memcpy_aligned(target, source, length);

							if(scale==1){
								U8 *target = &framebuffer.buffer->address[(display.y+y-framebuffer.area.y1)*framebuffer.buffer->stride+(display.x+startX-framebuffer.area.x1)*bpp];
								U8 *source = &display.buffer.address[y*display.buffer.stride+startX*bpp];
								U32 length = (endX-startX)*bpp;

								// if(source<display.viewBuffer.address||source+length>display.viewBuffer.address+display.viewBuffer.size){
								// 	debug::trace("READ OUT OF BUFFER! ", source, " -> ", source+length, " vs ", display.viewBuffer.address, " -> ", display.viewBuffer.address+display.viewBuffer.size, "\n");
								// }
								memcpy_aligned(target, source, length);

							}else{
								U8 *target = &framebuffer.buffer->address[(display.y+y-framebuffer.area.y1)*framebuffer.buffer->stride+(display.x+startX-framebuffer.area.x1)*bpp];
								U8 *source = &display.buffer.address[(y/scale)*display.buffer.stride];

								for(I32 x=startX;x<endX;x++){
									U8 *sourceData = &source[(x/scale)*bpp]; 
									for(unsigned i=0;i<bpp;i++){
										// if(sourceData<display.viewBuffer.address||source>=display.viewBuffer.address+display.viewBuffer.size){
										// 	debug::trace("READ OUT OF BUFFER! ", source, " vs ", display.viewBuffer.address, " -> ", display.viewBuffer.address+display.viewBuffer.size, "\n");
										// }
										*target++ = *sourceData++;
									}
								}
							}
						}

						if(nextX>endX){
							startX = nextX;
						}else{
							break;
						}
					}
				}
			}
		}
		#pragma GCC pop_options

		auto _get_screen_buffer(U32 framebufferId, graphics2d::Rect rect) -> Optional<graphics2d::Buffer> {
			if(framebufferId>=framebuffers.length) return {};

			if(!framebuffers[framebufferId].buffer) return {};

			return framebuffers[framebufferId].buffer->region(rect.x1, rect.y1, rect.width(), rect.height());
		}

		void _on_driver_event(const drivers::Event &event) {
			switch(event.type){
				case drivers::Event::Type::driverStarted: {
					auto graphics = event.driverStarted.driver->as_type<driver::Graphics>();
					if(!graphics) break;

					// add graphics framebuffers
					for(auto i=0u;i<graphics->get_framebuffer_count();i++){
						framebuffers.push_back({
							driver: graphics,
							driverFramebuffer: i,
							buffer: graphics->get_framebuffer(i)
						});
					}

					_update_framebuffer_positions();

					DisplayManager::instance.events.trigger({
						type: DisplayManager::Event::Type::framebuffersChanged
					});
				} break;
				case drivers::Event::Type::driverStopped: {
					auto graphics = event.driverStopped.driver->as_type<driver::Graphics>();
					if(!graphics) break;

					// remove graphics framebuffers
					for(auto i=0u;i<framebuffers.length;i++){
						if(framebuffers[i].driver==graphics){
							framebuffers.remove(i--);
						}
					}

					_update_framebuffer_positions();

					DisplayManager::instance.events.trigger({
						type: DisplayManager::Event::Type::framebuffersChanged
					});
				} break;
			}
		}

		void _on_graphics_event(const driver::Graphics::Event &event) {
			switch(event.type){
				case driver::Graphics::Event::Type::framebufferChanging:
					for(auto &framebuffer:framebuffers){
						if(framebuffer.driver==event.instance&&event.framebufferChanging.index==framebuffer.driverFramebuffer){
							framebuffer.buffer = nullptr;
							break;
						}
					}

					DisplayManager::instance.events.trigger({
						type: DisplayManager::Event::Type::framebuffersChanged
					});
				break;
				case driver::Graphics::Event::Type::framebufferChanged:
					for(auto &framebuffer:framebuffers){
						if(framebuffer.driver==event.instance&&event.framebufferChanging.index==framebuffer.driverFramebuffer){
							framebuffer.buffer = event.instance->get_framebuffer(event.framebufferChanged.index);
							framebuffer.area.clear(); //invalidate

							_update_framebuffer_positions();

							DisplayManager::instance.events.trigger({
								type: DisplayManager::Event::Type::framebuffersChanged
							});

							break;
						}
					}

				break;
			}
		}
	}

	auto DisplayManager::_on_start() -> Try<> {
		Lock_Guard guard(lock);

		framebuffers.clear();

		for(auto &graphics:drivers::iterate<driver::Graphics>()){
			if(!graphics.api.is_active()){
				if(!graphics.api.is_enabled()) continue;
				if(!drivers::start_driver(graphics)) continue;
			}

			for(auto i=0u;i<graphics.get_framebuffer_count();i++){
				framebuffers.push_back({
					driver: &graphics,
					driverFramebuffer: i,
					buffer: graphics.get_framebuffer(i)
				});
			}
		}

		if(framebuffers.length<1) return {"No graphics displays available"};

		_update_framebuffer_positions();

		if(totalArea.width()<1||totalArea.height()<1) return {"No graphics drivers in a valid state"}; // drivers exist, but were all transitioning, so no actual display size available

		drivers::events.subscribe(_on_driver_event);
		driver::Graphics::allEvents.subscribe(_on_graphics_event);

		return {};
	}

	auto DisplayManager::_on_stop() -> Try<> {
		drivers::events.unsubscribe(_on_driver_event);
		driver::Graphics::allEvents.unsubscribe(_on_graphics_event);

		return {};
	}

	/**/ DisplayManager::Display::~Display() {
		graphics2d::Rect area;

		{
			Lock_Guard guard(displaysLock);

			if(!displays.contains(*this)) return; //if not in the list it's already been recycled

			area = graphics2d::Rect{x, y, x+(I32)get_width(), y+(I32)get_height()};

			if(thread){
				thread->events.unsubscribe(_on_thread_event, this);
			}
			delete buffer.address;
			buffer.address = nullptr;
			displays.pop(*this);
		}

		_update_area(area);
	}

	void DisplayManager::set_background_colour(U32 colour) {
		Lock_Guard guard(lock);

		return _set_background_colour(colour);
	}

	auto DisplayManager::create_display(Thread *thread, DisplayLayer layer, U32 width, U32 height, U8 scale) -> Display* {
		Lock_Guard guard(lock, "create_view");

		return _create_view(thread, layer, width, height, scale);
	}

	auto DisplayManager::get_display_at(I32 x, I32 y, bool includeNonInteractive, Display *below) -> Display* {
		for(auto display=below?below->prev:displays.tail; display; display=display->prev){
			if(!display->isVisible) continue;

			if(
				x>=display->x&&
				y>=display->y&&
				x<display->x+(I32)display->get_width()&&
				y<display->y+(I32)display->get_height()
			){
				if(!(includeNonInteractive?graphics2d::Rect{0, 0, (I32)display->buffer.width, (I32)display->buffer.height}:display->interactArea).contains(x-display->x, y-display->y)) continue;

				return display;
			}
		}

		return nullptr;
	}

	auto DisplayManager::get_screen_count() -> U32 {
		Lock_Guard guard(lock);

		return framebuffers.length;
	}

	auto DisplayManager::get_screen_buffer(U32 framebufferId) -> graphics2d::Buffer* {
		Lock_Guard guard(lock);

		if(framebufferId>=framebuffers.length) return nullptr;

		return framebuffers[framebufferId].buffer;
	}

	auto DisplayManager::get_screen_buffer(U32 framebufferId, graphics2d::Rect rect) -> Optional<graphics2d::Buffer> {
		Lock_Guard guard(lock);

		return _get_screen_buffer(framebufferId, rect);
	}

	auto DisplayManager::get_screen_area(U32 framebufferId) -> graphics2d::Rect {
		Lock_Guard guard(lock);

		if(framebufferId>=framebuffers.length) return {};

		return framebuffers[framebufferId].area;
	}

	void DisplayManager::update_area(graphics2d::Rect rect, Display *below) {
		Lock_Guard guard(lock);

		_update_area(rect, below);
	}

	void DisplayManager::update_background() {
		Lock_Guard guard(lock);

		return _update_background();
	}

	void DisplayManager::update_background_area(graphics2d::Rect rect) {
		Lock_Guard guard(lock);

		return _update_background_area(rect);
	}

	void DisplayManager::Display::move_to(I32 x, I32 y, bool update) {
		Lock_Guard guard(lock);

		return _move_display_to(*this, x, y, update);
	}

	void DisplayManager::Display::resize_to(U32 width, U32 height, bool update) {
		Lock_Guard guard(lock);

		return _resize_display_to(*this, width, height, update);
	}

	void DisplayManager::Display::place_above(DisplayManager::Display &other) {
		Lock_Guard guard(lock);

		return _place_above(*this, other);
	}

	void DisplayManager::Display::place_below(DisplayManager::Display &other) {
		Lock_Guard guard(lock);

		return _place_below(*this, other);
	}

	void DisplayManager::Display::raise() {
		Lock_Guard guard(lock);

		return _raise_display(*this);
	}

	void DisplayManager::Display::set_layer(DisplayLayer layer) {
		Lock_Guard guard(lock);

		return _set_display_layer(*this, layer);
	}

	auto DisplayManager::Display::is_top() -> bool {
		Lock_Guard guard(lock);

		return _is_display_top(*this);
	}

	void DisplayManager::Display::show() {
		Lock_Guard guard(lock);

		return _show_display(*this);
	}

	void DisplayManager::Display::hide() {
		Lock_Guard guard(lock);

		return _hide_display(*this);
	}

	void DisplayManager::Display::update() {
		Lock_Guard guard(lock);

		if(!isVisible) return;

		_update_display_solid(*this);
		_update_area_transparency(graphics2d::Rect{x, y, x+(I32)get_width(), y+(I32)get_height()});
	}

	void DisplayManager::Display::update_area(graphics2d::Rect rect) {
		Lock_Guard guard(lock);

		_update_display_area_solid(*this, rect);
		_update_area_transparency(rect.offset(x, y)); //TODO: avoid unneccasary redraws here if this area is obscured
	}

	auto DisplayManager::get_width() -> U32 {
		return totalArea.width();
	}

	auto DisplayManager::get_height() -> U32 {
		return totalArea.height();
	}
}
