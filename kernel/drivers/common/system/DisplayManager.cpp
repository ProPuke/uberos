#include "DisplayManager.hpp"

#include <kernel/drivers.hpp>
#include <kernel/framebuffer.hpp>
#include <kernel/logging.hpp>
#include <kernel/memory.hpp>
#include <kernel/mmio.hpp>
#include <kernel/Spinlock.hpp>

#include <common/stdlib.hpp>

namespace driver::system {
	namespace {
		LList<DisplayManager::Display> displays;

		U32 backgroundColour = 0x202020;
		U32 backgroundColour2 = 0x282828;
		// U32 backgroundColour = 0xa0a0a0;
		// U32 backgroundColour2 = 0xa8a8a8;

		Spinlock spinlock("graphics2d");

		void _set_background_colour(U32 colour);

		auto _create_view(Thread *thread, DisplayManager::DisplayLayer layer, U32 x, U32 y, U32 width, U32 height, U8 scale=1) -> DisplayManager::Display*;
		void _move_display_to(DisplayManager::Display&, I32 x, I32 y, bool update);
		void _place_above(DisplayManager::Display&, DisplayManager::Display &other);
		void _place_below(DisplayManager::Display&, DisplayManager::Display &other);
		void _raise_display(DisplayManager::Display&);
		auto _is_display_top(DisplayManager::Display&) -> bool;
		void _show_display(DisplayManager::Display&);
		void _hide_display(DisplayManager::Display&);
		auto _find_display_section_in_row(I32 &x, I32 y, I32 x2, bool &isTransparent, DisplayManager::Display *current = nullptr) -> DisplayManager::Display*;
		void _update_background();
		void _update_background_area(graphics2d::Rect);
		void _update_area_transparency(graphics2d::Rect);
		void _update_area_solid(graphics2d::Rect, DisplayManager::Display *below = nullptr);
		void _update_display_solid(DisplayManager::Display&);
		void _update_display_area_solid(DisplayManager::Display&, graphics2d::Rect);
		void _update_blending_at(Framebuffer&, U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay);
		auto _get_screen_buffer(U32 framebuffer, graphics2d::Rect) -> graphics2d::Buffer;

		void _set_background_colour(U32 colour) {
			if(backgroundColour==colour) return;

			backgroundColour = colour;
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

		DisplayManager::Display* _create_view(Thread *thread, DisplayManager::DisplayLayer layer, U32 x, U32 y, U32 width, U32 height, U8 scale) {

			#ifdef DEBUG_MEMORY
				logging::Section section("create_view ", x, ", ", y, " ", width, "x", height);
			#endif

			auto &framebuffer = *framebuffer::get_framebuffer(0); //FIXME: handle 0 framebuffers
			auto bpp = graphics2d::bufferFormat::size[(U8)framebuffer.buffer.format];

			#ifdef DEBUG_MEMORY
				trace("new U8 ", width, "x", height, "@", bpp, "\n");
			#endif

			auto buffer = new U8[width*height*bpp]; //TODO:allocate from pages and map to current thread

			#ifdef DEBUG_MEMORY
				trace("new U8 ", width, "x", height, "@", bpp, " = ", buffer, "\n");
			#endif

			if(!buffer) return nullptr;

			bzero(buffer, width*height*bpp);

			auto display = new DisplayManager::Display(thread, buffer, layer, framebuffer.buffer.format, framebuffer.buffer.order, x, y, width, height, scale);
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
				trace("created display ", display);
			#endif

			return display;
		}


		auto _sample_background_at(Framebuffer &framebuffer, I32 x, I32 y) -> U32;

		void _update_blending_at_rgb(Framebuffer&, U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay);
		void _update_blending_at_bgr(Framebuffer&, U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay);
		void _update_blending_at(Framebuffer &framebuffer, U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay) {
			switch(framebuffer.buffer.order){
				case graphics2d::BufferFormatOrder::argb:
					_update_blending_at_rgb(framebuffer, buffer, x, y, topDisplay);
				break;
				case graphics2d::BufferFormatOrder::bgra:
				default:
					_update_blending_at_bgr(framebuffer, buffer, x, y, topDisplay);
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
		void _update_blending_at_rgb(Framebuffer &framebuffer, U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay) {
			PackedPixel<graphics2d::BufferFormatOrder::argb> result{0,0,0,0};
			U8 visibility = 255;

			for(auto display=topDisplay; display; display=display->prev){
				// check just y
				if(display->y>y||display->y+(I32)display->get_height()<=y) continue;

				// check x (+ corners)
				const auto displayY = y-display->y;
				if(
					display->x+(I32)display->get_left_margin(displayY)>x||
					display->x+(I32)display->get_width()-(I32)display->get_right_margin(displayY)<=x
				) continue;

				const auto displayX = x-display->x;

				auto &read = *(PackedPixel<graphics2d::BufferFormatOrder::argb>*)&display->buffer.address[(displayY/display->scale)*display->buffer.stride+(displayX/display->scale)*4];

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
				auto bg = _sample_background_at(framebuffer, x, y);

				result.r += min((bg>>16 & 0xff) * (U32)visibility/255u, 255u);
				result.g += min((bg>> 8 & 0xff) * (U32)visibility/255u, 255u);
				result.b += min((bg>> 0 & 0xff) * (U32)visibility/255u, 255u);
			}

			done:

			*buffer = result.value;
			buffer++;
		}

		void _update_blending_at_bgr(Framebuffer &framebuffer, U32 *&buffer, I32 x, I32 y, DisplayManager::Display *topDisplay) {
			PackedPixel<graphics2d::BufferFormatOrder::bgra> result{0,0,0,0};
			U8 visibility = 255;

			for(auto display=topDisplay; display; display=display->prev){
				// check just y
				if(display->y>y||display->y+(I32)display->get_height()<=y) continue;

				// check x (+ corners)
				const auto displayY = y-display->y;
				if(
					display->x+(I32)display->get_left_margin(displayY)>x||
					display->x+(I32)display->get_width()-(I32)display->get_right_margin(displayY)<=x
				) continue;

				const auto displayX = x-display->x;

				auto &read = *(PackedPixel<graphics2d::BufferFormatOrder::bgra>*)&display->buffer.address[(displayY/display->scale)*display->buffer.stride+(displayX/display->scale)*4];

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
				auto bg = _sample_background_at(framebuffer, x, y);

				result.r += min((bg>>16 & 0xff) * (U32)visibility/255u, 255u);
				result.g += min((bg>> 8 & 0xff) * (U32)visibility/255u, 255u);
				result.b += min((bg>> 0 & 0xff) * (U32)visibility/255u, 255u);
			}

			done:

			*buffer = result.value;
			buffer++;
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
				if(!display->isVisible) continue;

				if(display==current){
					// abort, we've now reached the current window. There is nothing more on top of it to check.
					break;
				}

				// check just y
				if(display->y>y||display->y+(I32)display->get_height()<=y) continue;

				// check x (+ corners)
				const auto displayX = x-display->x;
				const auto displayY = y-display->y;
				auto displayLeft = display->x+(I32)display->get_left_margin(displayY);
				auto displayRight = display->x+(I32)display->get_width()-(I32)display->get_right_margin(displayY);

				if(displayRight<=x) continue;

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
					isTransparent = transparent;
					return display;

				}else if(displayLeft<foundLeft){
					// it's further to the right, so set if the best match so far
					foundLeft = displayLeft;
					foundTransparent = transparent;
					found = display;
				}
			}

			x = min(foundLeft, x2);
			isTransparent = foundTransparent;
			return found;
		}

		void _update_area_transparency(graphics2d::Rect screenRect) {
			// return;

			for(auto i=0u; i<framebuffer::get_framebuffer_count(); i++){
				auto &framebuffer = *framebuffer::get_framebuffer(i);

				auto rect = screenRect.intersect({0, 0, (I32)framebuffer.buffer.width, (I32)framebuffer.buffer.height});

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

								auto framebufferAddress = &framebuffer.buffer.address[y*framebuffer.buffer.stride+scanX*4];
								U32 buffer[256];
								auto bufferPosition = buffer;

								for(auto x=scanX; x<nextScanX; x++){
									_update_blending_at(framebuffer, bufferPosition, x, y, display);

									if(bufferPosition>&buffer[sizeof(buffer)/sizeof(buffer[0])]){
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

				_update_display_area_solid(*display, {rect.x1-display->x, rect.y1-display->y, rect.x2-display->x, rect.y2-display->y});
			}
		}

		void _update_background() {
			auto &framebuffer = *framebuffer::get_framebuffer(0); //FIXME:handle 0 framebuffers

			_update_background_area({0, 0, (I32)framebuffer.buffer.width, (I32)framebuffer.buffer.height});
		}

		#pragma GCC push_options
		#pragma GCC optimize ("-O3")
		auto _sample_background_at(Framebuffer &framebuffer, I32 x, I32 y) -> U32 {
			const I32 edge = 64;
			U32 colour = (x/30+y/30)%2?backgroundColour:backgroundColour2;

			if(x<edge) {
				const U32 fade = edge-x;
				I32 r = max<I32>(0, ((colour>>16)&0xff)-fade/5);
				I32 g = max<I32>(0, ((colour>> 8)&0xff)-fade/5);
				I32 b = max<I32>(0, ((colour>> 0)&0xff)-fade/5);
				return r<<16|g<<8|b;

			} else {
				const auto right = (I32)framebuffer.buffer.width-(I32)edge;

				if(x<right){
					return colour;

				} else {
					const U32 fade = edge-(framebuffer.buffer.width-x);
					I32 r = max<I32>(0, ((colour>>16)&0xff)-fade/5);
					I32 g = max<I32>(0, ((colour>> 8)&0xff)-fade/5);
					I32 b = max<I32>(0, ((colour>> 0)&0xff)-fade/5);
					return r<<16|g<<8|b;
				}
			}
		}

		void _update_background_area(graphics2d::Rect rect) {
			// mmio::PeripheralWriteGuard guard;

			// trace("update background area");

			for(auto i=0u; i<framebuffer::get_framebuffer_count(); i++){
				auto &framebuffer = *framebuffer::get_framebuffer(i);
				// auto bpp = graphics2d::bufferFormat::size[(U8)framebuffer.buffer.format];

				rect.x1 = max<I32>(rect.x1, 0);
				rect.y1 = max<I32>(rect.y1, 0);
				rect.x2 = min<I32>(rect.x2, framebuffer.buffer.width);
				rect.y2 = min<I32>(rect.y2, framebuffer.buffer.height);

				// trace("paint background ", rect.y1, " to ", rect.y2);

				for(auto y=rect.y1;y<rect.y2;y++){
					// trace("y = ", y, "\n");
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
							const I32 edge = 64;
							I32 x = startX;
							for(;x<min(endX,edge);x++) {
								const U32 fade = edge-x;
								const U32 colour = (x/30+y/30)%2?backgroundColour:backgroundColour2;
								I32 r = max<I32>(0, ((colour>>16)&0xff)-fade/5);
								I32 g = max<I32>(0, ((colour>> 8)&0xff)-fade/5);
								I32 b = max<I32>(0, ((colour>> 0)&0xff)-fade/5);
								framebuffer.set(x, y, r<<16|g<<8|b);
							}
							{
								auto right = min(endX,(I32)framebuffer.buffer.width-(I32)edge);
								while(x<right){
									const U32 colour = (x/30+y/30)%2?backgroundColour:backgroundColour2;
									auto next = min(((x/30)+1)*30, right);
									framebuffer.set(x, y, colour, next-x);
									x = next;
								}
							}
							for(;x<endX;x++) {
								const U32 fade = edge-(framebuffer.buffer.width-x);
								const U32 colour = (x/30+y/30)%2?backgroundColour:backgroundColour2;
								I32 r = max<I32>(0, ((colour>>16)&0xff)-fade/5);
								I32 g = max<I32>(0, ((colour>> 8)&0xff)-fade/5);
								I32 b = max<I32>(0, ((colour>> 0)&0xff)-fade/5);
								framebuffer.set(x, y, r<<16|g<<8|b);
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

		void _place_above(DisplayManager::Display &display, DisplayManager::Display &other) {
			displays.pop(display);

			display.layer = other.layer;
			displays.insert_after(other, display);
			_update_display_solid(display); //technically we only need to draw the parts that were previously obscured, oh well..
		}

		void _place_below(DisplayManager::Display &display, DisplayManager::Display &other) {
			displays.pop(display);

			display.layer = other.layer;
			displays.insert_before(other, display);
			_update_display_solid(display); //technically we only need to draw the parts that were previously obscured, oh well..
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

			_update_area_solid(rect, &display);
			_update_area_transparency(rect);
		}

		inline void _update_display_solid(DisplayManager::Display &display) {
			return _update_display_area_solid(display, {0, 0, (I32)display.get_width(), (I32)display.get_height()});
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
		void __update_display_solid_area(DisplayManager::Display &display, graphics2d::Rect _rect) {
			// mmio::PeripheralAccessGuard guard;

			for(auto i=0u; i<framebuffer::get_framebuffer_count(); i++){
				auto &framebuffer = *framebuffer::get_framebuffer(i);
				auto bpp = graphics2d::bufferFormat::size[(U8)framebuffer.buffer.format];

				auto rect = _rect.intersect({
					max(display.solidArea.x1, -display.x),
					max(display.solidArea.y1, -display.y),
					min(display.solidArea.x2, (I32)framebuffer.buffer.width-display.x),
					min(display.solidArea.y2, (I32)framebuffer.buffer.height-display.y)
				});

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

							// U8 *target = &framebuffer.buffer.address[((display.y+y)*framebuffer.buffer.width+display.x+startX)*bpp];
							// U8 *source = &display.viewBuffer.address[((y/scale)*display.viewBuffer.width+startX/scale)*bpp];
							// U32 length = (endX-startX)*bpp;
							// if(source<display.viewBuffer.address||source+length>display.viewBuffer.address+display.viewBuffer.size){
							// 	trace("READ OUT OF BUFFER! ", source, " -> ", source+length, " vs ", display.viewBuffer.address, " -> ", display.viewBuffer.address+display.viewBuffer.size, "\n");
							// }
							// trace(source, " -> ", source+length, "\n");
							// memcpy_aligned(target, source, length);

							if(scale==1){
								U8 *target = &framebuffer.buffer.address[(display.y+y)*framebuffer.buffer.stride+(display.x+startX)*bpp];
								U8 *source = &display.buffer.address[y*display.buffer.stride+startX*bpp];
								U32 length = (endX-startX)*bpp;

								// if(source<display.viewBuffer.address||source+length>display.viewBuffer.address+display.viewBuffer.size){
								// 	trace("READ OUT OF BUFFER! ", source, " -> ", source+length, " vs ", display.viewBuffer.address, " -> ", display.viewBuffer.address+display.viewBuffer.size, "\n");
								// }
								memcpy_aligned(target, source, length);

							}else{
								U8 *target = &framebuffer.buffer.address[(display.y+y)*framebuffer.buffer.stride+(display.x+startX)*bpp];
								U8 *source = &display.buffer.address[(y/scale)*display.buffer.stride];

								for(I32 x=startX;x<endX;x++){
									U8 *sourceData = &source[(x/scale)*bpp]; 
									for(unsigned i=0;i<bpp;i++){
										// if(sourceData<display.viewBuffer.address||source>=display.viewBuffer.address+display.viewBuffer.size){
										// 	trace("READ OUT OF BUFFER! ", source, " vs ", display.viewBuffer.address, " -> ", display.viewBuffer.address+display.viewBuffer.size, "\n");
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

		auto _get_screen_buffer(U32 framebuffer_id, graphics2d::Rect rect) -> graphics2d::Buffer {
			auto possibleFramebuffer = framebuffer::get_framebuffer(framebuffer_id);
			const auto &framebuffer = *possibleFramebuffer; //FIXME: handle invalid framebuffer
			const auto bpp = graphics2d::bufferFormat::size[(U8)framebuffer.buffer.format];
			return graphics2d::Buffer(framebuffer.buffer.address+rect.y1*framebuffer.buffer.stride+rect.x1*bpp, framebuffer.buffer.stride, rect.x2-rect.x1, rect.y2-rect.y1, framebuffer.buffer.format, framebuffer.buffer.order);
		}
	}

	auto DisplayManager::_on_start() -> Try<> {
		Spinlock_Guard guard(spinlock);

		_update_background();

		return {};
	}

	auto DisplayManager::_on_stop() -> Try<> {
		return {};
	}

	/**/ DisplayManager::Display::~Display() {
		Spinlock_Guard guard(spinlock);

		if(!displays.contains(*this)) return; //if not in the list it's already been recycled

		const auto area = graphics2d::Rect{x, y, x+(I32)get_width(), y+(I32)get_height()};

		if(thread){
			thread->events.unsubscribe(_on_thread_event, this);
		}
		delete buffer.address;
		buffer.address = nullptr;
		displays.pop(*this);

		_update_area_solid(area);
		_update_area_transparency(area);
	}

	void DisplayManager::set_background_colour(U32 colour) {
		Spinlock_Guard guard(spinlock);

		return _set_background_colour(colour);
	}

	auto DisplayManager::create_display(Thread *thread, DisplayLayer layer, U32 x, U32 y, U32 width, U32 height, U8 scale) -> Display* {
		Spinlock_Guard guard(spinlock, "create_view");

		return _create_view(thread, layer, x, y, width, height, scale);
	}

	auto DisplayManager::get_display_at(I32 x, I32 y, bool includeNonInteractive, Display *below) -> Display* {
		for(auto display=below?below->prev:displays.tail; display; display=display->prev){
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

	auto DisplayManager::get_screen_buffer(U32 framebuffer_id, graphics2d::Rect rect) -> graphics2d::Buffer {
		Spinlock_Guard guard(spinlock);

		return _get_screen_buffer(framebuffer_id, rect);
	}

	void DisplayManager::update_area(graphics2d::Rect rect, Display *below) {
		Spinlock_Guard guard(spinlock);

		_update_area_solid(rect, below);
		_update_area_transparency(rect); //FIXME: don't do this unneccasarily here. Is update_area(Rect, Display) even used?
	}

	void DisplayManager::update_background() {
		Spinlock_Guard guard(spinlock);

		return _update_background();
	}

	void DisplayManager::update_background_area(graphics2d::Rect rect) {
		Spinlock_Guard guard(spinlock);

		return _update_background_area(rect);
	}

	void DisplayManager::Display::move_to(I32 x, I32 y, bool update) {
		Spinlock_Guard guard(spinlock);

		return _move_display_to(*this, x, y, update);
	}

	void DisplayManager::Display::place_above(DisplayManager::Display &other) {
		Spinlock_Guard guard(spinlock);

		return _place_above(*this, other);
	}

	void DisplayManager::Display::place_below(DisplayManager::Display &other) {
		Spinlock_Guard guard(spinlock);

		return _place_below(*this, other);
	}

	void DisplayManager::Display::raise() {
		Spinlock_Guard guard(spinlock);

		return _raise_display(*this);
	}

	auto DisplayManager::Display::is_top() -> bool {
		Spinlock_Guard guard(spinlock);

		return _is_display_top(*this);
	}

	void DisplayManager::Display::show() {
		Spinlock_Guard guard(spinlock);

		return _show_display(*this);
	}

	void DisplayManager::Display::hide() {
		Spinlock_Guard guard(spinlock);

		return _hide_display(*this);
	}

	void DisplayManager::Display::update() {
		Spinlock_Guard guard(spinlock);

		_update_display_solid(*this);
		_update_area_transparency(graphics2d::Rect{x, y, x+(I32)get_width(), y+(I32)get_height()});
	}

	void DisplayManager::Display::update_area(graphics2d::Rect rect) {
		Spinlock_Guard guard(spinlock);

		_update_display_area_solid(*this, rect);
		_update_area_transparency(rect.offset(x, y)); //TODO: avoid unneccasary redraws here if this area is obscured
	}

	auto DisplayManager::get_width() -> U32 {
		// TODO: support multiple framebuffers as a virtual display, and missing framebuffer 0s
		auto &framebuffer = *framebuffer::get_framebuffer(0);
		return framebuffer.buffer.width;
	}

	auto DisplayManager::get_height() -> U32 {
		// TODO: support multiple framebuffers as a virtual display, and missing framebuffer 0s
		auto &framebuffer = *framebuffer::get_framebuffer(0);
		return framebuffer.buffer.height;
	}
}
