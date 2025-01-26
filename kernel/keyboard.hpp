#pragma once

#include <common/LList.hpp>
#include <common/types.hpp>

namespace keyboard {
	struct Position {
		U8 row;
		U8 col;
		const static inline auto maxRows = 8;
		const static inline auto maxCols = 29;
	};

	typedef U8 Scancode;

	struct Layout;

	inline LList<Layout> layouts;

	struct Layout: LListItem<Layout> {
		const char *name;
		C16 charmap[256];
		C16 charmapShift[256];
		C16 charmapAltGr[256];
		C16 charmapShiftAltGr[256];

		/**/ Layout(const char *name, C16 charmap[256], C16 charmapShift[256], C16 charmapAltGr[256], C16 charmapShiftAltGr[256]):
			name(name)
		{
			for(auto i=0;i<256;i++) this->charmap[i] = charmap[(7-i/32)*32+i%32];
			for(auto i=0;i<256;i++) this->charmapShift[i] = charmapShift[(7-i/32)*32+i%32];
			for(auto i=0;i<256;i++) this->charmapAltGr[i] = charmapAltGr[(7-i/32)*32+i%32];
			for(auto i=0;i<256;i++) this->charmapShiftAltGr[i] = charmapShiftAltGr[(7-i/32)*32+i%32];
			layouts.push_back(*this);
		}
	};

	constexpr auto positionToScancode(Position position) -> Scancode { return (position.row&0b111)<<5 | (position.col&0b11111); }
	constexpr auto scancodeToPosition(Scancode scancode) -> Position { return {(U8)((scancode>>5)&0b111), (U8)(scancode&0b11111)}; }

	enum struct ActionCode {
		mediaSelect,
		mediaPlayPause,
		mediaStop,
		mediaTrackPrevious,
		mediaTrackNext,
		volumeMute,
		volumeUp,
		volumeDown,
		browserHome,
		browserSearch,
		browserFavourites,
		browserRefresh,
		browserStop,
		browserBack,
		browserForward,
		power,
		launchMail,
	};
}

#include <kernel/keyboard/layout/uk.hpp>
#include <kernel/keyboard/layout/us.hpp>

namespace keyboard {
	#define KEY(NAME,...) NAME,
	#define EMPTY CONCAT(_empty, __COUNTER__),
	#define EXTEND_UP CONCAT(_extendUp, __COUNTER__),
	#define EXTEND_DOWN CONCAT(_extendDown, __COUNTER__),
	#define EXTEND_LEFT CONCAT(_extendLeft, __COUNTER__),
	#define EXTEND_RIGHT CONCAT(_extendRight, __COUNTER__),

	enum struct ScancodeUk: Scancode {
		/*_beforeRow0 = (U8)positionToScancode({0, 0})-1,*/ KEYBOARD_LAYOUT_UK_ROW0
		_beforeRow1 = (U8)positionToScancode({1, 0})-1, KEYBOARD_LAYOUT_UK_ROW1
		_beforeRow2 = (U8)positionToScancode({2, 0})-1, KEYBOARD_LAYOUT_UK_ROW2
		_beforeRow3 = (U8)positionToScancode({3, 0})-1, KEYBOARD_LAYOUT_UK_ROW3
		_beforeRow4 = (U8)positionToScancode({4, 0})-1, KEYBOARD_LAYOUT_UK_ROW4
		_beforeRow5 = (U8)positionToScancode({5, 0})-1, KEYBOARD_LAYOUT_UK_ROW5
		_beforeRow6 = (U8)positionToScancode({6, 0})-1, KEYBOARD_LAYOUT_UK_ROW6
		_beforeRow7 = (U8)positionToScancode({7, 0})-1, KEYBOARD_LAYOUT_UK_ROW7
	};

	enum struct ScancodeUs: Scancode {
		/*_beforeRow0 = (U8)positionToScancode({0, 0})-1,*/ KEYBOARD_LAYOUT_US_ROW0
		_beforeRow1 = (U8)positionToScancode({1, 0})-1, KEYBOARD_LAYOUT_US_ROW1
		_beforeRow2 = (U8)positionToScancode({2, 0})-1, KEYBOARD_LAYOUT_US_ROW2
		_beforeRow3 = (U8)positionToScancode({3, 0})-1, KEYBOARD_LAYOUT_US_ROW3
		_beforeRow4 = (U8)positionToScancode({4, 0})-1, KEYBOARD_LAYOUT_US_ROW4
		_beforeRow5 = (U8)positionToScancode({5, 0})-1, KEYBOARD_LAYOUT_US_ROW5
		_beforeRow6 = (U8)positionToScancode({6, 0})-1, KEYBOARD_LAYOUT_US_ROW6
		_beforeRow7 = (U8)positionToScancode({7, 0})-1, KEYBOARD_LAYOUT_US_ROW7
	};

	#undef KEY
	#undef EMPTY
	#undef UNUSED
	#undef EXTEND_UP
	#undef EXTEND_DOWN
	#undef EXTEND_LEFT
	#undef EXTEND_RIGHT
}
