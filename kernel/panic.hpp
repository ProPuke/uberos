#pragma once

#include <kernel/PhysicalPointer.hpp>

#include <common/graphics2d/Buffer.hpp>

namespace graphics2d {
	struct Buffer;
}

namespace panic {
	struct Panic;

	void init();

	auto panic() -> Panic;
	auto panic(const char *title, const char *subtitle) -> Panic;

	struct Panic: NonCopyable<Panic> {
		/**/~Panic();
		
		template<typename Type>
		auto _print_details_inline(Type) -> Panic&;
		auto _print_details_inline(const char*) -> Panic&;
		auto _print_details_inline(char) -> Panic&;
		auto _print_details_inline(char*) -> Panic&;
	
		template<typename ...Params>
		auto print_details(Params ...params) -> Panic&;
		auto print_details_start() -> Panic&;
		auto print_details_end() -> Panic&;
		template<typename ...Params>
		auto print_details_inline(Params ...params) -> Panic&;

		auto print_stacktrace() -> Panic&;
		auto print_stacktrace(UPtr stackframe) -> Panic&;

	protected:
		/**/ Panic(Physical<graphics2d::Buffer>, const char *title, const char *subtitle);

		friend auto panic() -> Panic;
		friend auto panic(const char *title, const char *subtitle) -> Panic;

		graphics2d::Buffer *framebuffer = nullptr; // physical address
		U32 x, y;
		U32 width;
		I32 cursorX;
		bool hasDetails = false;
	};
}

#include "panic.inl"
