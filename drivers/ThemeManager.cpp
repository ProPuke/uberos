#include "ThemeManager.hpp"

#include <common/Box.hpp>
#include <common/ui2d/theme/Clean.hpp>
#include <common/ui2d/theme/System8.hpp>
#include <common/ui2d/theme/Win9x.hpp>

namespace driver {
	namespace {
		ui2d::Theme *currentTheme = nullptr;

		ui2d::theme::Clean theme;
		// ui2d::theme::Win9x theme;
		// ui2d::theme::System8 theme;
	}

	auto ThemeManager::_on_start() -> Try<> {
		set_theme(theme);

		return {};
	}

	auto ThemeManager::_on_stop() -> Try<> {
		return {};
	}

	void ThemeManager::set_theme(ui2d::Theme &theme) {
		currentTheme = &theme;

		events.trigger({
			type: Event::Type::themeChanged,
			themeChanged: { }
		});
	}

	auto ThemeManager::get_theme() -> ui2d::Theme& {
		return *currentTheme;
	}
}
