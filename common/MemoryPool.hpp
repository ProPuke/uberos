#pragma once

#include "debugUtils.hpp"
#include "LList.hpp"
#include "stdlib.hpp"
#include "types.hpp"

#include <kernel/logging.hpp>

#include <new>

//These must always be aligned correctly in memory
struct MemoryPoolBlock:LListItem<MemoryPoolBlock> {
	/**/ MemoryPoolBlock(size_t size):
		size(size - offsetof(MemoryPoolBlock, MemoryPoolBlock::_data))
	{}

	size_t size;
	U8 _data; //since we're offsetting by `size_t`, we'll assume this shares `size_t_` alignment and is thus always at a valid alignment
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
			logging::Section section("malloc ", size);

			memory::debug();
		#endif

		//TODO:align properly. This only aligns the chunk position, not the data inside it

		unsigned requiredSize = align(size, alignment);

		auto isLarge = requiredSize>=memory::pageSize; //TODO:mark is large if it's higher than the median size (do this with a guessed term that adapts on each search when more than half blocks are searched)

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
			logging::Section section("claim block ", &reclaim, " of size ", reclaim.size);
			memory::debug();
		#endif

		debug::assert(!availableBlocks.contains(reclaim));

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
							logging::print_info("insert_before ", &reclaim);
						#endif
						availableBlocks.insert_before(*block, reclaim);
						goto inserted;
					}
				}

				availableBlocks.push_back(reclaim);

			}else{
				for(auto block=availableBlocks.tail; block; block=block->prev){
					if(block->size<=reclaim.size){
						#ifdef MEMORY_CHECKS
							logging::print_info("insert_after ", &reclaim);
						#endif
						availableBlocks.insert_after(*block, reclaim);
						goto inserted;
					}
				}

				availableBlocks.push_front(reclaim);
			}
		}

		inserted:
		;

		#ifdef MEMORY_CHECKS
			{
				bool found = false;
				for(auto block=availableBlocks.head; block; block=block->next){
					if(block==&reclaim){
						found = true;
					}
				}

				if(!found){
					logging::print_error("Error: NOT FOUND");
					memory::debug();
				}
			}
		#endif
	}

	MemoryPoolBlock* get_block(void *address){
		return (MemoryPoolBlock*)(((uint8_t*)address)-offsetof(MemoryPoolBlock, _data));
	}

	void free(void *address){
		auto block = get_block(address);
		const auto size = block->size;
		claim_block(*block);
		used -= size;
		needsCompacting = true;
	}

	bool compact() {
		//TODO: write something faster

		{ // brute search and merge adjacent memory blocks
			for(auto block=availableBlocks.head;block;block=block->next){
				searchForNext:
				for(auto other=availableBlocks.head;other;other=other->next){
					if(other==block) continue;
					if((U8*)other==&block->_data+block->size){
						block->size += offsetof(MemoryPoolBlock, MemoryPoolBlock::_data) + other->size;
						availableBlocks.pop(*other);
						goto searchForNext;
					}
				}
			}
		}

		{ // sort blocks by size
			LList<MemoryPoolBlock> sortedBlocks;
			while(auto block = availableBlocks.pop_front()){
				for(auto other=sortedBlocks.head;other;other=other->next){
					if(other->size>=block->size){
						sortedBlocks.insert_before(*other, *block);
						goto sortedBlockInserted;
					}
				}

				sortedBlocks.push_back(*block);

				sortedBlockInserted:;
			}

			availableBlocks = sortedBlocks;
		}

		needsCompacting = false;

		return false; //return if successful
	}
};
