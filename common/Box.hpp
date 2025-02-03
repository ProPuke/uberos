#pragma once

#include "Pointer.hpp"

template <typename Type>
struct Box {
	/**/ Box() {}

	/**/ Box(Type *data):
		data(data)
	{}

	/**/~Box() {
		delete data;
	}

	/**/ Box(Box &&copy):
		data(copy.data)
	{
		copy.data = nullptr;
	}
	
	/**/ Box(const Box&) = delete;
	Box& operator=(const Box&) = delete;

	auto operator->() { return data; }
	auto operator->() const { return data; }
	explicit operator bool() const { return data; }
	operator Type*() { return data; }
	operator const Type*() const { return data; }

	Type* get() { return data; }
	
	Type* release() {
		auto old = data;
		data = nullptr;
		return old;
	}

private:
	Type *data = nullptr;
};

template <typename PointerType, typename Type>
struct BoxT {
	/**/ BoxT() {}

	/**/ BoxT(Type *data):
		data(data)
	{}

	/**/~BoxT() {
		delete data.get();
	}

	/**/ BoxT(BoxT &&copy):
		data(copy.data)
	{
		copy.data = nullptr;
	}
	
	/**/ BoxT(const BoxT&) = delete;
	BoxT& operator=(const BoxT&) = delete;

	auto operator->() { return data; }
	auto operator->() const { return data; }
	explicit operator bool() const { return data; }
	operator Type*() { return data; }
	operator const Type*() const { return data; }

	Type* get() { return data; }
	
	Type* release() {
		auto old = data;
		data = nullptr;
		return old;
	}

private:
	Pointer<PointerType, Type> data = nullptr;
};

template <typename Type>
using Box8 = BoxT<U8, Type>;

template <typename Type>
using Box16 = BoxT<U16, Type>;

template <typename Type>
using Box32 = BoxT<U32, Type>;

template <typename Type>
using Box64 = BoxT<U64, Type>;
