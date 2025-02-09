#pragma once

#include <kernel/Spinlock.hpp>

#include <common/ListOrdered.hpp>

#include <atomic>

template <typename Type>
struct EventEmitter {
	typedef void(*Callback)(const Type&, void*);
	typedef void(*Callback2)(const Type&);

	constexpr /**/ EventEmitter(){}

	void subscribe(Callback, void *data);
	void subscribe(Callback2 callback) { return subscribe((Callback) callback, nullptr); }
	void unsubscribe(Callback, void *data);
	void unsubscribe(Callback2 callback) { return unsubscribe((Callback) callback, nullptr); }
	void unsubscribe_all();

	void trigger(const Type&);
	
protected:

	struct Subscription {
		Callback callback;
		void *data;
	};

	ListOrdered<Subscription> subscribers;
	U32 subscribersGeneration = 0;

	ListOrdered<Subscription> callableSubscribers;
	U32 callableSubscribersGeneration = 0;

	std::atomic<int> isTriggering{false};
	Spinlock lockEdit;
};

template <typename Type>
void EventEmitter<Type>::subscribe(Callback callback, void *data) {
	Spinlock_Guard guard{lockEdit};

	subscribers.push_back({callback, data});
	subscribersGeneration++;
}

template <typename Type>
void EventEmitter<Type>::unsubscribe(Callback callback, void *data) {
	Spinlock_Guard guard{lockEdit};

	for(auto i=0u;i<subscribers.length;i++) {
		auto &subscription = subscribers[i];
		if(subscription.callback==callback&&subscription.data==data){
			subscribers.remove(i);
			subscribersGeneration++;
			break;
		}
	}
}

template <typename Type>
void EventEmitter<Type>::unsubscribe_all() {
	Spinlock_Guard guard{lockEdit};

	subscribers.clear();
	subscribersGeneration++;
}

template <typename Type>
void EventEmitter<Type>::trigger(const Type &data) {
	if(isTriggering.fetch_or(1)){ // if reentrant, use a local copy
		Spinlock_Guard guard{lockEdit};

		auto subscribersCopy = subscribers; // pull from editable subscribers direct
		for(auto &subscription:subscribersCopy){
			(subscription.callback)(data, subscription.data);
		}

	}else{
		{
			Spinlock_Guard guard{lockEdit};

			if(subscribersGeneration!=callableSubscribersGeneration){
				callableSubscribers = subscribers;
				callableSubscribersGeneration = subscribersGeneration;
			}
		}

		for(auto &subscription:callableSubscribers){
			(subscription.callback)(data, subscription.data);
		}

		isTriggering = 0;
	}
}
