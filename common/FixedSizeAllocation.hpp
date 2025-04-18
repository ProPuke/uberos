#pragma once

template <typename Type>
struct FixedSizeAllocation final: NonCopyable<FixedSizeAllocation<Type>> {
	Type data;

	auto operator->() /* */ -> /* */ Type& { return data; }
	auto operator->() const -> const Type& { return data; }
};
