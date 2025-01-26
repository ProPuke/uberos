#pragma once

#include <common/ListOrdered.hpp>

template <typename Type>
class EventEmitter {
	typedef void(*Callback)(const Type&, void*);
	struct Subscription {
		Callback callback;
		void *data;
	};

	ListOrdered<Subscription> subscribers{0};

public:

	void subscribe(Callback, void *data);
	void unsubscribe(Callback, void *data);
	void unsubscribe_all();

	void trigger(const Type&);
};

template <typename Type>
void EventEmitter<Type>::subscribe(Callback callback, void *data) {
	subscribers.push_back({callback, data});
}

template <typename Type>
void EventEmitter<Type>::unsubscribe(Callback callback, void *data) {
	for(auto i=0u;i<subscribers.length;i++)	{
		auto &subscription = subscribers[i];
		if(subscription.callback==callback&&subscription.data==data){
			subscribers.remove(i);
			break;
		}
	}
}

template <typename Type>
void EventEmitter<Type>::unsubscribe_all() {
	subscribers.clear();
}

template <typename Type>
void EventEmitter<Type>::trigger(const Type &data) {
	for(auto &subscription:subscribers){
		(subscription.callback)(data, subscription.data);
	}
}
