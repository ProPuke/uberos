#include <SDL2/SDL.h>

#include <common/types.hpp>
#include <common/graphics2d/Buffer.hpp>
#include <common/graphics2d/font.hpp>
#include <common/maths.hpp>
#include <common/maths/Fixed.hpp>

#include <unistd.h>

namespace ui2d {
	extern graphics2d::Buffer test;
}

void draw_window(graphics2d::Buffer &buffer, U32 x, U32 y, U32 width, U32 height) {
	// const auto windowColour = 0xeeeeee;
	const auto borderColor = 0xcccccc;
	const auto borderWidth = 1;
	const auto titleHeight = 29;
	const auto titleFontSize = 15;
	// const auto titleColour = 0xffffff;
	const auto titleTextColour = 0x222222;
	// const auto titleAlpha = (U8)0xff;

	U32 windowCorners20[20+1];
	buffer.create_round_corner(20, windowCorners20);

	U32 windowCorners40[40+1];
	buffer.create_round_corner(40, windowCorners40);

	buffer.draw_rect_outline(x, y, width, height, borderColor, borderWidth, windowCorners20, windowCorners20, windowCorners20, windowCorners20);
	// buffer.draw_rect(x+borderWidth, y+borderWidth, width-borderWidth*2, titleHeight, graphics2d::blend_rgb(windowColour, titleColour, titleAlpha), windowCorners20, windowCorners20, nullptr, nullptr);
	// buffer.draw_text(*graphics2d::font::default_sans, "Test Window", x, y+borderWidth+titleHeight-4, 100, titleTextColour);
	buffer.draw_text(*graphics2d::font::default_sans, "Test Window", x, y+borderWidth+titleHeight-(titleHeight-(titleFontSize*3/5))/2, titleFontSize, titleTextColour);
	// buffer.draw_rect(x+borderWidth, y+borderWidth+titleHeight, width-borderWidth*2, height-titleHeight-borderWidth*2, windowColour, nullptr, nullptr, windowCorners20, windowCorners20);

	buffer.draw_rect(200,200, 40,40, borderColor, windowCorners20, windowCorners20, windowCorners20, windowCorners20);

	buffer.draw_rect(350,350, 80,80, borderColor, windowCorners40, windowCorners40, windowCorners40, windowCorners40);

	buffer.draw_rect_outline(500,500, 80,80, borderColor, 10, windowCorners40, windowCorners40, windowCorners40, windowCorners40);
}

void draw_text(graphics2d::Buffer &buffer) {
	auto y = 2;
	for(U32 size=6;size<256;size+=size>10?size/5:1){
		auto nextY = y+size;
		buffer.draw_text(*graphics2d::font::default_sans, to_string(size), 2, nextY, size, 0xffffff);
		buffer.draw_text(*graphics2d::font::default_sans, "This is example text", 300, nextY, size, 0xffffff);
		y = nextY;
	}
}

void draw_windows(graphics2d::Buffer &buffer) {
	buffer.draw_rect(20,20,40,40,0x0000ff);
	buffer.draw_text(*graphics2d::font::default_sans, "This is example text", 50, 50, 50, 0xffffff);

	draw_window(buffer, 50, 50, 400, 350);
	draw_window(buffer, 160, 160, 400, 350);
	draw_window(buffer, 600, 600, 400, 350);

// buffer.draw_4slice(900, 900, 200, 200, ui2d::test);
}

void draw(graphics2d::Buffer &buffer) {
	// draw_text(buffer);
	draw_windows(buffer);
}

int main() {
	if(SDL_Init(SDL_INIT_VIDEO)<0){
		printf("Error starting SDL\n");
		return 1;
	}

	auto width = 1920;
	auto height = 1080;

	auto window = SDL_CreateWindow("ui-rendering test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	if(!window){
		printf("Error creating SDL window\n");
		return 1;
	}

	auto windowSurface = SDL_GetWindowSurface(window);
	if(!windowSurface){
		printf("Error getting SDL window surface\n");
		return 1;
	}

	auto surface = SDL_CreateRGBSurface(0, windowSurface->w, windowSurface->h, 32, 0xff0000, 0x00ff00, 0x0000ff, 0xff000000);
	if(!surface){
		printf("Error creating drawing surface\n");
		return 1;
	}

	graphics2d::Buffer buffer((U8*)surface->pixels, surface->w*surface->h*4, surface->w*4, surface->w, surface->h, graphics2d::BufferFormat::rgba8, graphics2d::BufferFormatOrder::rgb);

	for(bool active=true;active;){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					active = false;
				break;
			}
		}

		buffer.draw_rect(0,0,buffer.width,buffer.height, 0x000000);
		draw(buffer);

		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
		SDL_BlitSurface(surface, nullptr, windowSurface, nullptr);
		SDL_UpdateWindowSurface(window);

		sleep(1);
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
