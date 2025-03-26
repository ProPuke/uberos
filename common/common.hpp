#pragma once

#define _STRINGIFY(X) #X
#define STRINGIFY(X) _STRINGIFY(X)

#define _CONCAT(X, Y) X##Y
#define CONCAT(X, Y) _CONCAT(X, Y)

template <class Type>
struct NonCopyable {
	/**/ NonCopyable(const NonCopyable&) = delete;
	auto operator=(const NonCopyable&) -> NonCopyable& = delete;

	protected:
		/**/ NonCopyable () = default;
		/**/ ~NonCopyable () = default;
};

template <typename Type>
struct Defer: NonCopyable<Defer<Type>> {
	Type callback;

	/**/ Defer(Type callback): callback(callback) { };
	/**/~Defer() { callback(); }
};

#define defer Defer CONCAT(_defer, __COUNTER__) = [&]()

template <typename To, typename From>
inline constexpr auto reinterpret_value(From value) -> To {
	union {
		From from;
		To to;
	};
	from = value;
	return to;
}

#include <common/types.hpp>

template <typename Parent, typename Type>
inline auto parent(Type &object, Type Parent::*member) -> Parent& {
	return *(Parent*)((UPtr)&object-(UPtr)&((Parent*)(0)->*member));
}
