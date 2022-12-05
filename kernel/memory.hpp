#pragma once

#include "stdio.hpp"
#include <common/types.hpp>
#include <common/LList.hpp>

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
		static inline const size_t pageSize = 16*1024; //16KB
	#else
		#error Unsupported architecture
	#endif
	// extern MemoryPool<32> *heap;

	// not thread-safe thread safe
	auto _kmalloc(size_t size) -> void*;
	void _kfree(void *address);

	auto _allocate_page() -> Page*;
	auto _allocate_pages(U32 count) -> Page*;
	void _free_page(Page &page);
	auto _get_memory_page(void *address) -> Page&;

	void _check_dangerous_address(void *from, void *to);

	// inline void memset(U8 *address, U8 value, U32 size) {
	// 	while(--size) *address++ = value;
	// }

	struct Transaction {
		/**/ Transaction() { lock(); }
		/**/~Transaction() { unlock(); }

		static void lock();
		static void unlock();

		auto allocate_page() -> Page* {
			return _allocate_page();
		}
		auto allocate_pages(U32 count) -> Page* {
			return _allocate_pages(count);
		}
		void free_page(Page &page) {
			return _free_page(page);
		}
		void free_page_with_address(void *address) {
			return free_page(get_memory_page(address));
		}
		auto get_memory_page(void *address) -> Page& {
			return _get_memory_page(address);
		}

		auto kmalloc(size_t size) -> void* {
			return _kmalloc(size);
		}
		void kfree(void *address) {
			return _kfree(address);
		}

		void check_dangerous_address(void *from, void *to) {
			return _check_dangerous_address(from, to);
		}
	};
}

// void* operator new(size_t size) noexcept;
// void* operator new[](size_t size) noexcept;
inline void* operator new(size_t size, size_t align) = delete;
inline void* operator new[](size_t size, size_t align) = delete;

// void operator delete(void* p) noexcept;
// void operator delete(void* p, size_t) noexcept;

inline void* operator new(size_t size) noexcept { memory::Transaction transaction; return transaction.kmalloc(size); }
inline void* operator new[](size_t size) noexcept { memory::Transaction transaction; return transaction.kmalloc(size); }

inline void operator delete(void* p) noexcept { memory::Transaction transaction; transaction.kfree(p); }
inline void operator delete(void* p, size_t) noexcept { memory::Transaction transaction; transaction.kfree(p); }

#include "memory.inl"
