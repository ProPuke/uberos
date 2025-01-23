#pragma once

#include <common/types.hpp>

struct ConsoleOut {
	char _currentEscape[64] = "";
	unsigned _currentEscapeLength = 0;
	
	U32 ansiColours[8];
	U32 brightAnsiColours[8];

	U32 defaultForegroundColour = 1;
	U32 foregroundColour = defaultForegroundColour;
	U32 defaultBackgroundColour = 0;
	U32 backgroundColour = defaultBackgroundColour;
	bool boldText = false;

	void clear_current_escape() { _currentEscapeLength = 0; }

	virtual auto get_colour_count() -> U32 = 0;
	virtual auto get_colour(U32) -> U32 = 0;
	virtual auto get_nearest_colour(U32) -> U32 = 0;

	virtual void set_foreground_colour(U32 x) { foregroundColour = x;}
	virtual void set_background_colour(U32 x) { backgroundColour = x;}
	virtual void set_bold(bool set) { boldText = set;}
	virtual void clear_foreground_colour() { foregroundColour = defaultForegroundColour;}
	virtual void clear_background_colour() { backgroundColour = defaultBackgroundColour;}

	virtual void clear() { clear_foreground_colour(); clear_background_colour(); clear_current_escape(); }

	/*   */ void write_text(const char *);
	virtual void write_raw_text(const char *) = 0;
};
