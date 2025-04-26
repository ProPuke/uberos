#pragma once

#include <drivers/Software.hpp>

#include <common/EventEmitter.hpp>
#include <common/ui2d/Theme.hpp>

namespace driver {
	struct ThemeManager: Software {
		DRIVER_INSTANCE(ThemeManager, 0xf250c3d5, "theme", "ThemeManager", Software);

		struct Event {
			enum struct Type {
				themeChanged
			} type;

			union {
				struct {
				} themeChanged;
			};
		};

		EventEmitter<Event> events;
		
		auto _on_start() -> Try<> override;
		auto _on_stop() -> Try<> override;

		void set_theme(ui2d::Theme &theme);
		auto get_theme() -> ui2d::Theme&;
	};
}
