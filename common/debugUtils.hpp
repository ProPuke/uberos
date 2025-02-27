#pragma once

#include "LList.hpp"

#include <kernel/logging.hpp>

template <typename Type>
void debug_llist(LList<Type> &list, const char *label = "list") {
	logging::Section section(label, ':');

	unsigned length = 0;
	LListItem<Type> *last = nullptr;
	for(auto item=list.head; item; item=item->next){
		if(item->prev!=last){
			logging::print_error("Error: Item ", item, " has incorrect prev record when walking forwards");
			debug::halt();
		}
		length++;
		last = item;
	}

	if(length!=list.size){
		logging::print_error("Error: Forward walked length of ", length, " did not match expected size of ", list.size);
		debug::halt();
	}

	if(last!=list.tail){
		logging::print_error("Error: Last forward item ", last, " does not match the tail ", list.tail);
		debug::halt();
	}

	last = nullptr;
	for(auto item=list.tail; item; item=item->prev){
		if(item->next!=last){
			logging::print_error("Error: Item ", item, " has incorrect next record when walking backwards");
			debug::halt();
		}
		last = item;
	}

	if(last!=list.head){
		logging::print_error("Error: Last backward item ", last, " does not match the head ", list.head);
		debug::halt();
	}

	// logging::print_info("size: ", list.size,  " / ", length);
	// logging::print_info("head: ", list.head);
	// logging::print_info("tail: ", list.tail);

	// for(auto item=list.head; item; item=item->next){
	// 	logging::print_info("item: ", item, " prev = ", item->prev, " next = ", item->next);
	// }
}
