#include "Console.hpp"

#include <kernel/console.hpp>

namespace driver {
	namespace {
		auto abs(I32 x) -> I32 { return x<0?-x:x; }
	}

	auto Console::get_nearest_colour(U32 colour) -> U32 {
		U32 nearestColour = 0;
		U32 nearestDistance = ~0;

		const int r = (colour >> 16) & 0xff;
		const int g = (colour >>  8) & 0xff;
		const int b = (colour >>  0) & 0xff;

		const I32 colour_count = get_colour_count();
		for(auto i=0;i<colour_count;i++){
			const auto compare = get_colour(i);
			const int compareR = (compare >> 16) & 0xff;
			const int compareG = (compare >>  8) & 0xff;
			const int compareB = (compare >>  0) & 0xff;

			const U32 distance = abs(r-compareR)+abs(g-compareG)+abs(b-compareB);
			if(distance<nearestDistance){
				nearestColour = i;
				nearestDistance = distance;
			}
		}

		return nearestColour;
	}

	void Console::bind_to_console() {
		if(!api.is_active()) return;

		consoleOut.clear_current_escape();

		console::bind(this,
			[](void *_console, char c) {
				auto &console = *(driver::Console*)_console;
				C8 string[2] = {c, '\0'};
				console.consoleOut.write_text(string);
			},
			nullptr,
			nullptr,
			[](void *_console, const char *str) {
				auto &console = *(driver::Console*)_console;
				console.consoleOut.write_text(str);
			},
			nullptr
		);
	}

	auto Console::_on_start() -> Try<> {
		// consoleOut.defaultBackgroundColour = get_nearest_colour(0x000000);
		// consoleOut.defaultForegroundColour = get_nearest_colour(0xa0a0a0);
		
		consoleOut.defaultBackgroundColour = get_nearest_colour(0x000080);
		// consoleOut.defaultForegroundColour = get_nearest_colour(0x0000ff);
		consoleOut.defaultForegroundColour = get_nearest_colour(0xa0a0a0);

		consoleOut.backgroundColour = consoleOut.defaultBackgroundColour;
		consoleOut.foregroundColour = consoleOut.defaultForegroundColour;

		consoleOut.ansiColours[0] = get_nearest_colour(0x000000);
		consoleOut.ansiColours[1] = get_nearest_colour(0x990000);
		consoleOut.ansiColours[2] = get_nearest_colour(0x00a600);
		consoleOut.ansiColours[3] = get_nearest_colour(0x999900);
		consoleOut.ansiColours[4] = get_nearest_colour(0x0000b2);
		consoleOut.ansiColours[5] = get_nearest_colour(0xb200b2);
		consoleOut.ansiColours[6] = get_nearest_colour(0x00a6b2);
		consoleOut.ansiColours[7] = get_nearest_colour(0xa0a0a0);

		consoleOut.brightAnsiColours[0] = get_nearest_colour(0x666666);
		consoleOut.brightAnsiColours[1] = get_nearest_colour(0xff0000);
		consoleOut.brightAnsiColours[2] = get_nearest_colour(0x00ff00);
		consoleOut.brightAnsiColours[3] = get_nearest_colour(0xffff00);
		consoleOut.brightAnsiColours[4] = get_nearest_colour(0x0000ff);
		consoleOut.brightAnsiColours[5] = get_nearest_colour(0xff00ff);
		consoleOut.brightAnsiColours[6] = get_nearest_colour(0x00ffff);
		consoleOut.brightAnsiColours[7] = get_nearest_colour(0xffffff);

		return {};
	}

	void Console::ConsoleOut::clear() {
		clear_foreground_colour();
		clear_background_colour();
		clear_current_escape();
		_console().clear(defaultForegroundColour, defaultBackgroundColour);
	}
}
