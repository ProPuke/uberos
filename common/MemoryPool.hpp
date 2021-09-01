#pragma once

#include "types.hpp"
#include "LList.hpp"
#include "stdlib.hpp"
#include <common/debugUtils.hpp>

#include <new>

struct MemoryPoolBlock:LListItem<MemoryPoolBlock> {
	/**/ MemoryPoolBlock(U32 size):
		size(size - offsetof(MemoryPoolBlock, MemoryPoolBlock::_data))
	{}

	U32 size;
	U8 _data;
};

//TODO:safely lock this so the kernel can inspect?

template <unsigned alignment>
struct MemoryPool {
	/**/ MemoryPool(void *address, size_t size):
		available(size),
		used(0)
	{
		if(address){
			auto block = new ((MemoryPoolBlock*)address) MemoryPoolBlock(size);
			availableBlocks.push_back(*block);
		}
	}

	size_t available;
	size_t used;
	bool needsCompacting = false;
	LList<MemoryPoolBlock> availableBlocks;

	void* malloc(size_t size) {
		#ifdef MEMORY_CHECKS
			stdio::Section section("malloc ", size, "\n");

			debug_llist(availableBlocks, "availableBlocks in malloc 0");
		#endif

		//TODO:align properly. This only aligns the chunk position, not the data inside it

		auto isLarge = size>=memory::pageSize; //TODO:mark is large if it's higher than the median size (do this with a guessed term that adapts on each search when more than half blocks are searched)

		unsigned requiredSize = size;
		requiredSize += requiredSize%alignment?alignment-(requiredSize%alignment):0;

		if(!isLarge){ //search forwards
			for(auto block=availableBlocks.head;block;block=block->next){
				if(block->size>=requiredSize){
					block = availableBlocks.pop(*block);
					available -= block->size; //we subtract the whole block first, as if we reclaim the tail that will add back onto available

					const unsigned remainingSize = block->size-requiredSize;
					if(remainingSize>32&&remainingSize>=alignment){
						block->size = requiredSize;

						#pragma GCC diagnostic push
						#pragma GCC diagnostic ignored "-Wplacement-new"
							auto remainingSlot = new (&block->_data+requiredSize) MemoryPoolBlock(remainingSize);
						#pragma GCC diagnostic pop
						claim_block(*remainingSlot);
					}

					used += block->size; //we add just the used final block size, in case the tail gets chopped off
					return &block->_data;
				}
			}

		}else{ //search backwards
			for(auto block=availableBlocks.tail;block;block=block->prev){
				if(block->size>=requiredSize&&(!block->prev||block->prev&&block->prev->size<requiredSize)){
					block = availableBlocks.pop(*block);
					available -= block->size; //we subtract the whole block first, as if we reclaim the tail that will add back onto available

					const unsigned remainingSize = block->size-requiredSize;
					if(remainingSize>32&&remainingSize>=alignment){
						block->size = requiredSize;

						#pragma GCC diagnostic push
						#pragma GCC diagnostic ignored "-Wplacement-new"
							auto remainingSlot = new (&block->_data+requiredSize) MemoryPoolBlock(remainingSize);
						#pragma GCC diagnostic pop
						claim_block(*remainingSlot);
					}

					used += block->size; //we add just the used final block size, in case the tail gets chopped off
					return &block->_data;
				}
			}
		}

		if(needsCompacting&&available>=requiredSize){
			// if we can't find a chunk this big although we have space, it might be compacting memory will help
			// give it one final try if we can compact
			if(compact()){
				return malloc(size);
			}
		}

		return nullptr;
	}

	void claim_block(MemoryPoolBlock &reclaim){
		#ifdef MEMORY_CHECKS
			stdio::Section section("claim block ", &reclaim, " of size ", reclaim.size, "\n");
			debug_llist(availableBlocks, "availableBlocks before");
		#endif

		available += reclaim.size;

		auto isLarge = reclaim.size>=memory::pageSize; //TODO:compare to adaptive median size


		if(!availableBlocks.head){
			availableBlocks.push_back(reclaim);

		}else{

			if(!isLarge){ //search forwards
				MemoryPoolBlock *block;
				for(block=availableBlocks.head; block; block=block->next){
					if(block->size>=reclaim.size){
						#ifdef MEMORY_CHECKS
							stdio::print_info("insert_before ", &reclaim);
						#endif
						availableBlocks.insert_before(*block, reclaim);
						break;
					}
				}
				if(!block){
					availableBlocks.push_back(reclaim);
				}

			}else{
				MemoryPoolBlock *block;
				for(block=availableBlocks.tail; block; block=block->prev){
					if(block->size<=reclaim.size){
						#ifdef MEMORY_CHECKS
							stdio::print_info("insert_after ", &reclaim);
						#endif
						availableBlocks.insert_after(*block, reclaim);
						break;
					}
				}
				if(!block){
					availableBlocks.push_front(reclaim);
				}
			}
		}

		#ifdef MEMORY_CHECKS
			{
				bool found = false;
				for(auto block=availableBlocks.head; block; block=block->next){
					if(block==&reclaim){
						found = true;
					}
				}

				if(!found){
					stdio::print_error("Error: NOT FOUND");
					debug_llist(availableBlocks, "availableBlocks after");
				}
			}
		#endif
	}

	MemoryPoolBlock* get_block(void *address){
		return (MemoryPoolBlock*)(((uint8_t*)address)-offsetof(MemoryPoolBlock, _data));
	}

	void free(void *address){
		auto block = get_block(address);
		claim_block(*block);
		needsCompacting = true;
	}

	bool compact() {
		//TODO: walk through availableBlocks in order of memory address, and merge if adjacent, before sorting by size again
		needsCompacting = false;

		return false; //return if successful
	}
};

template <>
inline void debug_llist(LList<MemoryPoolBlock> &list, const char *label) {
	stdio::Section section(label, ':');

	unsigned length = 0;
	unsigned firstErrorPosition = ~0u;
	unsigned lastErrorPosition = 0;
	LListItem<MemoryPoolBlock> *last = nullptr;

	for(auto item=list.head; item; item=item->next){
		if(last&&item->prev!=last){
			stdio::print_error("Error: Item ", item, " has missing prev record");
			if(length<firstErrorPosition){
				firstErrorPosition = length;
			}
			if(length>lastErrorPosition){
				lastErrorPosition = length;
			}
		}
		length++;
		last = item;
	}

	if(length!=list.size){
		stdio::print_error("Error: Walked length of ", length, " did not match expected size of ", list.size);
	}

	stdio::print_info("size: ", list.size,  " / ", length);
	stdio::print_info("head: ", list.head);
	stdio::print_info("tail: ", list.tail);

	{
		unsigned i=0;
		for(auto item=list.head; item; item=item->next){
			if(i+2>=firstErrorPosition&&i<=lastErrorPosition+2){
				stdio::print_info("item: ", item, " prev = ", item->prev, " next = ", item->next, " size = ", ((MemoryPoolBlock*)item)->size);
			}
			if(i>lastErrorPosition+2) break;
			i++;
		}
	}
}
