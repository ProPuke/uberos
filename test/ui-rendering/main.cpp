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

void draw_window(graphics2d::Buffer &buffer, U32 x, U32 y, U32 width, U32 height, bool showStatusbar = false) {
	const auto windowColour = 0xeeeeee;
	const auto borderColor = 0xc5c5c5;
	const auto borderWidth = 1;
	const auto borderRadius = 4;
	const auto titleHeight = 29;
	const auto titleFontSize = 15;
	const auto titleColour = 0xf9f9f9;
	const auto titleTextColour = 0x333333;
	const auto statusbarHeight = showStatusbar?24:borderRadius+borderWidth*2;

	U32 windowCorners[borderRadius+1];
	buffer.create_round_corner(borderRadius, windowCorners);

	//titlebar
	buffer.draw_rect(x, y+borderWidth, width, titleHeight, titleColour, windowCorners, windowCorners, nullptr, nullptr);

	//body
	buffer.draw_rect(x, y+borderWidth+titleHeight+1, width, height-borderWidth*2-titleHeight, windowColour, nullptr, nullptr, windowCorners, windowCorners);

	//statusbar
	buffer.draw_rect(x, y+height-borderWidth-statusbarHeight, width, statusbarHeight, titleColour, nullptr, nullptr, windowCorners, windowCorners);

	//outline
	buffer.draw_rect_outline(x, y, width, height, borderColor, borderWidth, windowCorners, windowCorners, windowCorners, windowCorners);

	//title
	auto titleSize = buffer.measure_text(*graphics2d::font::default_sans, "Test Window", x, y+borderWidth+titleHeight-(titleHeight-(titleFontSize*3/5))/2, titleFontSize);
	buffer.draw_text(*graphics2d::font::default_sans, "Test Window", x+width/2-(titleSize.maxX-x)/2, y+borderWidth+titleHeight-(titleHeight-(titleFontSize*3/5))/2, titleFontSize, titleTextColour);

	//titlebar bottom border
	buffer.draw_rect(x+borderWidth, y+borderWidth+titleHeight, width-borderWidth*2, 1, borderColor);

	//statusbar bottom border
	buffer.draw_rect(x+borderWidth, y+height-borderWidth-statusbarHeight-1, width-borderWidth*2, 1, borderColor);
}

void draw_text(graphics2d::Buffer &buffer) {
	auto y = 2;
	U32 size = 6;
	for(;size<=32;size++){
		auto nextY = y+size;
		buffer.draw_text(*graphics2d::font::default_sans, to_string(size), 2, nextY, size, 0xffffff);
		buffer.draw_text(*graphics2d::font::default_sans, "This is example text", 300, nextY, size, 0xffffff);
		y = nextY;
	}
	for(;size<256;size+=size>10?size/5:1){
		auto nextY = y+size;
		buffer.draw_text(*graphics2d::font::default_sans, to_string(size), 2, nextY, size, 0xffffff);
		buffer.draw_text(*graphics2d::font::default_sans, "This is example text", 300, nextY, size, 0xffffff);
		y = nextY;
	}
}

void draw_windows(graphics2d::Buffer &buffer) {
	draw_window(buffer, 650, 50, 400, 350);
	draw_window(buffer, 760, 160, 400, 350);
	draw_window(buffer, 1100, 600, 400, 350);

// buffer.draw_4slice(900, 900, 200, 200, ui2d::test);
}

void draw(graphics2d::Buffer &buffer) {
	draw_text(buffer);
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

		buffer.draw_rect(0,0,buffer.width,buffer.height, 0x666666);
		draw(buffer);

		SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_NONE);
		SDL_BlitSurface(surface, nullptr, windowSurface, nullptr);
		SDL_UpdateWindowSurface(window);

		SDL_Delay(500);
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
