#pragma once

#include "Pointer.hpp"

template <typename Type>
struct Box: NonCopyable<Box<Type>> {
	/**/ Box() {}

	/**/ Box(Type *data):
		data(data)
	{}

	/**/~Box() {
		delete data;
	}

	/**/ Box(Box &&other):
		data(other.data)
	{
		other.data = nullptr;
	}
	
	auto operator=(Box &&other) -> Box& {
		if(this==&other) return *this;

		delete data;
		data = other.data;
		other.data = nullptr;

		return *this;
	}

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
struct BoxT: NonCopyable<BoxT<PointerType, Type>> {
	/**/ BoxT() {}

	/**/ BoxT(Type *data):
		data(data)
	{}

	/**/~BoxT() {
		delete data.get();
	}

	/**/ BoxT(BoxT &&other):
		data(other.data)
	{
		other.data = nullptr;
	}
	
	auto operator=(BoxT &&other) -> BoxT& {
		if(this==&other) return *this;

		delete data;
		data = other.data;
		other.data = nullptr;

		return *this;
	}

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
