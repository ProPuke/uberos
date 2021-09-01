#pragma once

#include "stdio.hpp"
#include <common/types.hpp>
#include <common/LList.hpp>
#include <cstddef>

// template <unsigned alignment>
// struct MemoryPool;

namespace memory {
	struct Page;
	
	extern size_t totalMemory;
	extern Page *pageData;
	extern size_t pageDatasize;

	static inline const size_t pageSize = 4096;
	// extern MemoryPool<32> *heap;

	void is_dangerous_address(void *from, void *to);

	// threadsafe variants
	Page* allocate_page();
	Page* allocate_pages(U32 count);
	void free_page(Page &page);
	Page* get_memory_page(void *address);

	void* kmalloc(size_t size);
	void kfree(void*);

	// only to be used internally, not threadsafe
	Page* _allocate_page();
	Page* _allocate_pages(U32 count);
	void _free_page(Page &page);
	Page* _get_memory_page(void *address);

	// inline void memset(U8 *address, U8 value, U32 size) {
	// 	while(--size) *address++ = value;
	// }
}

#include <cstddef>

inline void* operator new(size_t size) noexcept { return memory::kmalloc(size); }
inline void* operator new[](size_t size) noexcept { return memory::kmalloc(size); }
inline void* operator new(size_t size, size_t align) = delete;
inline void* operator new[](size_t size, size_t align) = delete;

inline void operator delete(void* p) noexcept { memory::kfree(p); }
inline void operator delete(void* p, size_t) noexcept { memory::kfree(p); }

#include "memory.inl"
