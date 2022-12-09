#pragma once

#include "LList.hpp"

#include <kernel/log.hpp>

template <typename Type>
void debug_llist(LList<Type> &list, const char *label = "list") {
	log::Section section(label, ':');

	unsigned length = 0;
	LListItem<Type> *last = nullptr;
	for(auto item=list.head; item; item=item->next){
		if(last&&item->prev!=last){
			log::print_error("Error: Item ", item, " has missing prev record");
		}
		length++;
		last = item;
	}

	if(length!=list.size){
		log::print_error("Error: Walked length of ", length, " did not match expected size of ", list.size);
	}

	log::print_info("size: ", list.size,  " / ", length);
	log::print_info("head: ", list.head);
	log::print_info("tail: ", list.tail);

	for(auto item=list.head; item; item=item->next){
		log::print_info("item: ", item, " prev = ", item->prev, " next = ", item->next);
	}
}
