#pragma once

#include "LList.hpp"

#include <kernel/logging.hpp>

template <typename Type>
void debug_llist(LList<Type> &list, const char *label = "list") {
	logging::Section section(label, ':');

	unsigned length = 0;
	LListItem<Type> *last = nullptr;
	for(auto item=list.head; item; item=item->next){
		if(last&&item->prev!=last){
			logging::print_error("Error: Item ", item, " has missing prev record");
		}
		length++;
		last = item;
	}

	if(length!=list.size){
		logging::print_error("Error: Walked length of ", length, " did not match expected size of ", list.size);
	}

	logging::print_info("size: ", list.size,  " / ", length);
	logging::print_info("head: ", list.head);
	logging::print_info("tail: ", list.tail);

	for(auto item=list.head; item; item=item->next){
		logging::print_info("item: ", item, " prev = ", item->prev, " next = ", item->next);
	}
}
