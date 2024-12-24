#pragma once

template <typename Type>
struct Optional {
	/**/ Optional(){}

	/**/ Optional(const Type &data):
		present(true),
		data(data)
	{}

	/**/ Optional(const Type &&data):
		present(true),
		data(data)
	{}

	void clear() { present = false; }

	operator bool() const { return present; }
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
