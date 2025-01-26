#pragma once

#include "Keyboard.hpp"

#include <kernel/drivers.hpp>

namespace driver {
	inline auto Keyboard::is_pressed_on_any(keyboard::Scancode scancode) -> bool {
		for(auto &driver:drivers::iterate<driver::Keyboard>()) {
			if(driver.is_pressed(scancode)) return true;
		}

		return false;
	}
}
