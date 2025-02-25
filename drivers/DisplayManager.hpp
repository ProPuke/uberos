#pragma once

#include <drivers/Software.hpp>

#include <kernel/Thread.hpp>

#include <common/EventEmitter.hpp>
#include <common/LList.hpp>
#include <common/graphics2d/Buffer.hpp>
#include <common/graphics2d/BufferFormat.hpp>
#include <common/graphics2d/Rect.hpp>

namespace driver {
	//TODO: should graphics drivers also include an api for querying their active processor(s) drivers if present? This would allow us to work out what processor speeds and temps relate to this graphics adapter, which might be useful/neat
	struct DisplayManager: Software {
		DRIVER_INSTANCE(DisplayManager, 0xdc52bf38, "display", "DisplayManager", Software);
		
		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		struct Event {
			enum struct Type {
				framebuffersChanged
			} type;

			struct {

			} framebuffersChanged;
		};

		EventEmitter<Event> events;

		enum struct DisplayMode {
			solid
		};

		enum struct DisplayLayer: U8 {
			// app levels
			background = 0,
			bottomMost = 32,
			regular = 64,
			topMost = 96,

			// elevated levels
			kernelWindow = 128,
			kernelTopmostWindow = 160,
			cursor = 192,
			cursorOverlay = 224
		};

		struct Display: LListItem<Display> {
			/**/ Display(Thread *thread, U8 *address, DisplayLayer layer, graphics2d::BufferFormat format, graphics2d::BufferFormatOrder order, U32 x, U32 y, U32 width, U32 height, U8 scale = 1, DisplayMode mode = DisplayMode::solid):
				thread(thread),
				x(x),
				y(y),
				layer(layer),
				scale(scale),
				buffer(address, width*graphics2d::bufferFormat::size[(U32)format], width, height, format, order),
				solidArea(0, 0, width, height),
				interactArea(0, 0, width, height),
				mode(mode)
			{}

			/**/~Display();

			Thread *thread;

			I32 x, y;
			DisplayLayer layer;
			U8 scale;
			graphics2d::Buffer buffer;
			graphics2d::Rect solidArea;
			graphics2d::Rect interactArea;
			DisplayMode mode;
			bool isVisible = true;

			U32 topLeftCorner[16] = {};
			U32 topRightCorner[16] = {};
			U32 bottomLeftCorner[16] = {};
			U32 bottomRightCorner[16] = {};

			void move_to(I32 x, I32 y, bool update=true);
			void resize_to(U32 width, U32 height, bool update=true);
			void place_above(Display&);
			void place_below(Display&);
			void raise();
			void set_layer(DisplayLayer);
			auto is_top() -> bool; // if true, top of its layer and not raisable
			void show();
			void hide();
			void update();
			void update_area(graphics2d::Rect rect);

			auto get_width() -> U32 { return buffer.width*scale; }
			auto get_height() -> U32 { return buffer.height*scale; }

			auto get_left_margin(U32 y) -> U32 {
				if(y<16&&y<=buffer.height/2){
					return maths::min(topLeftCorner[y], get_width()/2);

				}else if(auto bottom=get_height()-1-y; bottom<16){
					return maths::min(bottomLeftCorner[bottom], get_width()/2);

				}else{
					return 0;
				}
			}

			auto get_right_margin(U32 y) -> U32 {
				if(y<16&&y<=buffer.height/2){
					return maths::min(topRightCorner[y], get_width()/2);

				}else if(auto bottom=get_height()-1-y; bottom<16){
					return maths::min(bottomRightCorner[bottom], get_width()/2);

				}else{
					return 0;
				}
			}
		};

		void set_background_colour(U32 colour);

		auto create_display(Thread *thread, DisplayLayer layer, U32 x, U32 y, U32 width, U32 height, U8 scale=1) -> Display*;
		void update_background();
		void update_background_area(graphics2d::Rect rect);
		void update_area(graphics2d::Rect rect, Display *below = nullptr);

		auto get_display_at(I32 x, I32 y, bool includeNonInteractive, Display *below = nullptr) -> Display*;
		auto get_screen_count() -> U32;
		auto get_screen_buffer(U32 framebuffer) -> graphics2d::Buffer*; // this may be missing while the framebuffer is changing
		auto get_screen_buffer(U32 framebuffer, graphics2d::Rect rect) -> Optional<graphics2d::Buffer>; // this may be missing while the framebuffer is changing
		auto get_screen_area(U32 framebuffer) -> graphics2d::Rect;

		auto get_width() -> U32;
		auto get_height() -> U32;
	};
}
