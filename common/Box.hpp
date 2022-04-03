#pragma once

template <typename Type>
struct Box {
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
	operator bool() const { return data; }
	operator Type*() { return data; }
	operator const Type*() const { return data; }

	Type* get() { return data; }
	
	Type* release() {
		auto old = data;
		data = nullptr;
		return old;
	}

private:
	Type *data;
};
