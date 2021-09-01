#include "graphics2d.hpp"

#include "graphics2d/Font.hpp"
#include "framebuffer.hpp"
#include "memory.hpp"
#include "scheduler.hpp"
#include <common/stdlib.hpp>

#ifdef ARCH_RASPI
	#include <kernel/arch/raspi/mmio.hpp>

	namespace mmio {
		using namespace arch::raspi::mmio;
	}
#endif

namespace graphics2d {
	LList<View> views;

	U32 backgroundColour = 0x202020;
	U32 backgroundColour2 = 0x282828;

	void init() {
		update_background();
		stdio::print_info("cleared bg");
	}

	void set_background_colour(U32 colour) {
		if(backgroundColour==colour) return;

		backgroundColour = colour;
		update_background();
	}

	View* create_view(Thread &thread, U32 x, U32 y, U32 width, U32 height, U8 scale) {
		scheduler::Guard guard;

		#ifdef DEBUG_MEMORY
			stdio::Section section("create_view ", x, ", ", y, " ", width, "x", height, "\n");
		#endif

		auto &framebuffer = framebuffer::framebuffers[0];
		auto bpp = framebufferFormat::size[(U8)framebuffer.format];

		#ifdef DEBUG_MEMORY
			stdio::print("new U8 ", width, "x", height, "@", bpp, "\n");
		#endif

		auto buffer = new U8[width*height*bpp];

		#ifdef DEBUG_MEMORY
			stdio::print("new U8 ", width, "x", height, "@", bpp, " = ", buffer, "\n");
		#endif

		if(!buffer) return nullptr;

		bzero(buffer, width*height*bpp);

		auto view = new View(thread, buffer, width*height*bpp, framebuffer.format, x, y, width, height, scale);
		if(!view) return nullptr;
		views.push_back(*view);

		thread.on_deleted.push_front(view->handle_thread_deleted);

		#ifdef DEBUG_MEMORY
			stdio::print("created view ", view, "\n");
		#endif

		return view;
	}

	/**/ View::~View() {
		//TODO:lock
		if(!views.contains(*this)) return; //if not in the list it's already been recycled

		Rect area = {x, y, x+(I32)buffer.width*(I32)scale, y+(I32)buffer.height*(I32)scale};

		thread.on_deleted.pop(handle_thread_deleted);
		delete buffer.address;
		views.pop(*this);

		update_area(area);
	}

	void update_area(Rect rect, View *below) {
		update_background_area(rect);

		for(auto view=views.head; view; view=view->next){
			if(view==below) break;
			update_view_area(*view, {rect.x1-view->x, rect.y1-view->y, rect.x2-view->x, rect.y2-view->y});
		}
	}

	void update_background() {
		auto &framebuffer = framebuffer::framebuffers[0];

		update_background_area({0, 0, (I32)framebuffer.width, (I32)framebuffer.height});
	}

	void update_background_area(Rect rect) {
		mmio::PeripheralAccessGuard guard;

		// stdio::print_info("update background area");

		for(auto i=0u; i<framebuffer::framebuffer_count; i++){
			auto &framebuffer = framebuffer::framebuffers[i];
			// auto bpp = framebufferFormat::size[(U8)framebuffer.format];

			rect.x1 = max<I32>(rect.x1, 0);
			rect.y1 = max<I32>(rect.y1, 0);
			rect.x2 = min<I32>(rect.x2, framebuffer.width);
			rect.y2 = min<I32>(rect.y2, framebuffer.height);

			// stdio::print_info("paint background ", rect.y1, " to ", rect.y2);

			for(auto y=rect.y1;y<rect.y2;y++){
				// stdio::print("y = ", y, "\n");
				auto startX = rect.x1;
				while(startX<rect.x2){
					auto endX = rect.x2;
					auto nextX = startX;
					for(auto view=views.head; view; view=view->next){
						if(view->y>y||view->y+(I32)view->buffer.height*(I32)view->scale<=y) continue;

						if(view->x<=startX){
							startX = max(startX, view->x+(I32)view->buffer.width*(I32)view->scale);
							if(startX>=endX) break;

						}else if(view->x<endX){
							endX = view->x;
							nextX = max(nextX, view->x+(I32)view->buffer.width*(I32)view->scale);
						}

					}

					if(endX>startX){
						for(auto x=startX;x<endX;x++) {
							framebuffer.set(x, y, (x/30+y/30)%2?backgroundColour:backgroundColour2);
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

	void move_view_to(View &view, I32 x, I32 y) {
		if(view.x==x&&view.y==y) return;

		const auto oldX = view.x;
		const auto oldY = view.y;

		view.x = x;
		view.y = y;
		update_view(view);
		update_area({oldX, oldY, oldX+(I32)view.buffer.width*(I32)view.scale, oldY+(I32)view.buffer.height*(I32)view.scale}, &view);
	}

	template <unsigned scale>
	void _update_view_area(View &view, Rect rect);

	void update_view_area(View &view, Rect rect) {
		switch(view.scale){
			case 1: _update_view_area<1>(view, rect); break;
			case 2: _update_view_area<2>(view, rect); break;
			case 3: _update_view_area<3>(view, rect); break;
			case 4: _update_view_area<4>(view, rect); break;
		}
	}

	template <unsigned scale>
	void _update_view_area(View &view, Rect rect) {
		//TODO: do not occlude against transparent views, and after loop, continue forward through each transparent view, updating them over the same local rect
		
		for(auto i=0u; i<framebuffer::framebuffer_count; i++){
			auto &framebuffer = framebuffer::framebuffers[i];
			auto bpp = framebufferFormat::size[(U8)framebuffer.format];

			rect.x1 = max<I32>(rect.x1, max<I32>(0, -view.x));
			rect.y1 = max<I32>(rect.y1, max<I32>(0, -view.y));
			rect.x2 = min<I32>(rect.x2, min<I32>(view.buffer.width*(I32)view.scale, ((I32)framebuffer.width)-view.x));
			rect.y2 = min<I32>(rect.y2, min<I32>(view.buffer.height*(I32)view.scale, ((I32)framebuffer.height)-view.y));

			for(auto y=rect.y1; y<rect.y2; y++){
				auto startX = rect.x1;
				while(startX<rect.x2){
					auto endX = rect.x2;
					auto nextX = startX;
					for(auto viewAbove=view.next; viewAbove; viewAbove=viewAbove->next){
						Rect aboveArea = { viewAbove->x, viewAbove->y, viewAbove->x+(I32)viewAbove->buffer.width*(I32)viewAbove->scale, viewAbove->y+(I32)viewAbove->buffer.height*(I32)viewAbove->scale };
						aboveArea.offset(-view.x, -view.y);

						if(aboveArea.y1>y||aboveArea.y2<=y) continue;

						if(aboveArea.x1<=startX){
							startX = max(startX, aboveArea.x2);
							if(startX>=endX) break;

						}else if(aboveArea.x1<endX){
							endX = aboveArea.x1;
							nextX = max(nextX, aboveArea.x2);
						}

					}

					if(endX>startX){

						// U8 *target = &framebuffer.address[((view.y+y)*framebuffer.width+view.x+startX)*bpp];
						// U8 *source = &view.buffer.address[((y/scale)*view.buffer.width+startX/scale)*bpp];
						// U32 length = (endX-startX)*bpp;
						// if(source<view.buffer.address||source+length>view.buffer.address+view.buffer.size){
						// 	stdio::print("READ OUT OF BUFFER! ", source, " -> ", source+length, " vs ", view.buffer.address, " -> ", view.buffer.address+view.buffer.size, "\n");
						// }
						// stdio::print(source, " -> ", source+length, "\n");
						// memcpy(target, source, length);

						if(scale==1){
							U8 *target = &framebuffer.address[((view.y+y)*framebuffer.width+view.x+startX)*bpp];
							U8 *source = &view.buffer.address[((y/scale)*view.buffer.width+startX/scale)*bpp];
							U32 length = (endX-startX)*bpp;

							// if(source<view.buffer.address||source+length>view.buffer.address+view.buffer.size){
							// 	stdio::print("READ OUT OF BUFFER! ", source, " -> ", source+length, " vs ", view.buffer.address, " -> ", view.buffer.address+view.buffer.size, "\n");
							// }
							memcpy(target, source, length);

						}else{
							U8 *target = &framebuffer.address[((view.y+y)*framebuffer.width+view.x+startX)*bpp];
							U8 *source = &view.buffer.address[((y/scale)*view.buffer.width)*bpp];

							for(I32 x=startX;x<endX;x++){
								U8 *sourceData = &source[(x/scale)*bpp]; 
								for(unsigned i=0;i<bpp;i++){
									// if(sourceData<view.buffer.address||source>=view.buffer.address+view.buffer.size){
									// 	stdio::print("READ OUT OF BUFFER! ", source, " vs ", view.buffer.address, " -> ", view.buffer.address+view.buffer.size, "\n");
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

	void View::Handle_thread_deleted::call(void *data) {
		auto &view = *(View*)data;

		delete &view;
	}

	void Buffer::draw_text(Font &font, const char *text, I32 startX, I32 startY, U32 size, U32 colour) {
		I32 x = startX;
		I32 y = startY;

		I32 scale = 256 * size/font.size;

		for(const char *c=text;*c;c++){
			auto character = font.get_character(*c);
			if(!character){
				continue;
			}

			// draw_rect(x, y, 10, 10, 0xff0000);

			draw_msdf(x+character->offsetX*(I32)size/256, y-character->offsetY*(I32)size/256, character->atlasWidth*scale/256, character->atlasHeight*scale/256, font.atlas, character->atlasX, character->atlasY, character->atlasWidth, character->atlasHeight, colour);

			x += character->advance*(I32)font.size*scale/256/256;
		}
	}
}
