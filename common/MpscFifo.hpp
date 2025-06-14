#pragma once

#include <atomic>

// multi-producer single-consumer fifo

template <typename Type>
struct MpscFifo: NonCopyable<MpscFifo<Type>> {
	struct Item {
		Type data;
		std::atomic<Item*> next{nullptr};
	};

	Item *out_{nullptr};
	std::atomic<Item*> in_{nullptr};

public:

	/**/ MpscFifo():
		out_(new Item),
		in_(out_)
	{}

	/**/~MpscFifo() {
		clear();
		delete out_;
	}

	template <typename ...Args>
	void emplace(Args &&...args) {
		push(*new Item(std::forward<Args>(args)...));
	}

	void push(Item &item) {
		auto oldIn = in_.exchange(&item, std::memory_order_acq_rel);
		oldIn->next.store(&item, std::memory_order_release);
	}

	auto pop_unknown() -> Item* {
		if(out_==in_.load(std::memory_order_relaxed)) return nullptr;

		auto item = out_->next.load(std::memory_order_relaxed);
		if(item){
			delete out_;
			out_ = item;
		}

		return item;
	}

	auto pop_known() -> Item& {
		auto item = out_->next.load(std::memory_order_relaxed);

		delete out_;
		out_ = item;

		return *item;
	}

	void clear() {
		while(pop_unknown());
	}
};
