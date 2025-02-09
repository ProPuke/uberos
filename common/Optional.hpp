#pragma once

template <typename Type>
struct Optional {
	constexpr /**/ Optional(){}

	constexpr /**/ Optional(const Type &data):
		present(true),
		data(data)
	{}

	constexpr /**/ Optional(const Type &&data):
		present(true),
		data(data)
	{}

	void clear() { present = false; }

	explicit operator bool() const { return present; }
	auto operator->() { return data; }
	auto operator->() const { return data; }
	auto operator*() -> Type& { return data; }
	auto operator*() const -> const Type& { return data; }

	auto operator=(const Type &data) { present = true; this->data = data; return *this; }
	auto operator=(const Type &&data) { present = true; this->data = data; return *this; }

	private:

	bool present = false;
	Type data = {};
};
