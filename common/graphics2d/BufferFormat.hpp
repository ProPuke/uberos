#pragma once

#include <common/types.hpp>

namespace graphics2d {
	enum struct BufferFormat {
		grey8,
		rgb565,
		rgb8,
		rgba8,
		max = rgba8
	};

	namespace bufferFormat {
		static const char *const name[(U32)BufferFormat::max+1] = {
			"8bit greyscale",
			"16bit colour",
			"24bit colour",
			"32bit colour"
		};

		static const U8 size[(U32)BufferFormat::max+1] = {
			1,
			2,
			3,
			4
		};
	}
}

#include <common/stdlib.hpp>

template<> inline auto to_string(graphics2d::BufferFormat format) -> const char* {
	return (U32)format<=(U32)graphics2d::BufferFormat::max?graphics2d::bufferFormat::name[(U32)format]: ::to_string((U32)format);
}
