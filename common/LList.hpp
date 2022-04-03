#pragma once

#include "types.hpp"

template <typename Type>
struct LListItem;

template <typename Type>
struct LList {
	Type *head = nullptr, *tail = nullptr;
	U32 size = 0;

	void push_front(Type &item){
		if(head){
			head->prev = &item;
		}
		item.prev = nullptr;
		item.next = head;

		head = &item;

		if(!tail){
			tail = &item;
		}

		size++;
	}

	void push_back(Type &item){
		if(tail){
			tail->next = &item;
		}

		item.prev = tail;
		item.next = nullptr;

		tail = &item;

		if(!head){
			head = &item;
		}

		size++;
	}

	Type* pop_front(){
		if(!head) return nullptr;

		auto item = head;

		head = (Type*)item->next;
		if(head){
			head->prev = nullptr;
		}else if(tail==item){
			tail = nullptr;
		}

		size--;

		return item;
	}

	Type* pop_back(){
		if(!tail) return nullptr;

		auto item = tail;
		item->list = nullptr;

		tail = (Type*)item->prev;
		if(tail){
			tail->next = nullptr;
		}else if(head==item){
			head = nullptr;
		}

		size--;

		return item;
	}

	Type* pop(Type &item){
		if(item.prev){
			item.prev->next = item.next;
		}else{
			head = (Type*)item.next;
		}

		if(item.next){
			item.next->prev = item.prev;
		}else{
			tail = (Type*)item.prev;
		}

		size--;

		return &item;
	}

	void insert_before(Type &after, Type &item){
		item.next = &after;
		item.prev = after.prev;

		after.prev = &item;
		if(item.prev){
			item.prev->next = &item;
		}else{
			head = &item;
		}

		size++;
	}

	void insert_after(Type &before, Type &item){
		item.prev = &before;
		item.next = before.next;
		
		before.next = &item;
		if(item.next){
			item.next->prev = &item;
		}else{
			tail = &item;
		}

		size++;
	}

	bool contains(Type &object){
		for(auto item=head;item;item=item->next){
			if(item==&object) return true;
		}
		return false;
	}

	U32 length() {
		U32 length = 0;
		for(auto item=head;item;item=item->next) length++;
		return length;
	}
};

template <typename Type>
struct LListItem {
	Type *next = nullptr, *prev = nullptr;
};
