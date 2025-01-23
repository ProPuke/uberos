#pragma once

enum struct Maybe {
	no,
	maybe,
	yes
};

inline auto operator&&(Maybe a, Maybe b) { return a==Maybe::maybe||b==Maybe::maybe?Maybe::maybe:a==Maybe::yes&&b==Maybe::yes?Maybe::yes:Maybe::no; }
inline auto operator||(Maybe a, Maybe b) { return a==Maybe::yes||b==Maybe::yes?Maybe::yes:a==Maybe::maybe||b==Maybe::maybe?Maybe::maybe:Maybe::no; }
