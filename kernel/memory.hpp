#pragma once

#include "stdio.hpp"
#include <common/types.hpp>
#include <common/LList.hpp>
#include <cstddef>

// template <unsigned alignment>
// struct MemoryPool;

namespace memory {
	struct Page;
	
	extern U64 totalMemory;
	extern Page *pageData;
	extern size_t pageDatasize;

	#if defined(ARCH_ARM32)
		static inline const size_t pageSize = 4*1024; //4KB
	#elif defined(ARCH_ARM64)
		static inline const size_t pageSize = 64*1024; //64KB
	#else
		#error Unsupported architecture
	#endif
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
// void* operator new(size_t size) noexcept;
// void* operator new[](size_t size) noexcept;
inline void* operator new(size_t size, size_t align) = delete;
inline void* operator new[](size_t size, size_t align) = delete;

// void operator delete(void* p) noexcept;
// void operator delete(void* p, size_t) noexcept;

void* operator new(size_t size) noexcept;
void* operator new[](size_t size) noexcept;

void operator delete(void* p) noexcept;
void operator delete(void* p, size_t) noexcept;

#include "memory.inl"
