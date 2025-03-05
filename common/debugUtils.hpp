#pragma once

#include "LList.hpp"

#include <kernel/logging.hpp>
#include <kernel/panic.hpp>

template <typename Type>
void debug_llist(LList<Type> &list, const char *label = "list") {
	// logging::Section section(label, ':');

	unsigned length = 0;
	LListItem<Type> *last = nullptr;
	for(auto item=list.head; item; item=item->next){
		if(item->prev!=last){
			panic::panic()
				.print_details("Error: Item ", item, " has incorrect prev record when walking forwards")
				.print_stacktrace()
			;
		}
		length++;
		last = item;
	}

	if(length!=list.size){
		panic::panic()
			.print_details("Error: Forward walked length of ", length, " did not match expected size of ", list.size)
			.print_stacktrace()
		;
	}

	if(last!=list.tail){
		panic::panic()
			.print_details("Error: Last forward item ", last, " does not match the tail ", list.tail)
			.print_stacktrace()
		;
	}

	last = nullptr;
	for(auto item=list.tail; item; item=item->prev){
		if(item->next!=last){
			panic::panic()
				.print_details("Error: Item ", item, " has incorrect next record when walking backwards")
				.print_stacktrace()
			;
		}
		last = item;
	}

	if(last!=list.head){
		panic::panic()
			.print_details("Error: Last backward item ", last, " does not match the head ", list.head)
			.print_stacktrace()
		;
	}

	// logging::print_info("size: ", list.size,  " / ", length);
	// logging::print_info("head: ", list.head);
	// logging::print_info("tail: ", list.tail);

	// for(auto item=list.head; item; item=item->next){
	// 	logging::print_info("item: ", item, " prev = ", item->prev, " next = ", item->next);
	// }
}
