#pragma once

#include <kernel/IdentityMappedPointer.hpp>
#include <kernel/PhysicalPointer.hpp>

#include <common/Box.hpp>
#include <common/LList.hpp>
#include <common/types.hpp>

#include <new>

// template <unsigned alignment>
// struct MemoryPool;

namespace memory {
	struct Page;
	
	extern constinit U64 totalMemory;

	extern constinit IdentityMapped<void> lowMemory;
	extern constinit size_t lowMemorySize;

	extern constinit IdentityMapped<void> code;
	extern /*constinit*/ size_t codeSize;

	extern constinit IdentityMapped<void> stack;
	extern constinit size_t stackSize;

	extern constinit Physical<void> heap;
	extern constinit size_t heapSize;

	#if defined(ARCH_ARM32)
		static inline const size_t pageSize = 4*1024; //4KB
	#elif defined(ARCH_ARM64)
		static inline const size_t pageSize = 16*1024; //16KB
	#elif defined(ARCH_X86)
		static inline const size_t pageSize = 4*1024; //4KB
	#elif defined(ARCH_HOSTED)
		static inline const size_t pageSize = 4*1024; //4KB
	#else
		#error Unsupported architecture
	#endif
	// extern MemoryPool<32> *heap;

	void init();

	auto get_used_heap() -> size_t;
	auto get_available_heap() -> size_t;

	// not thread-safe thread safe
	auto _allocate(size_t size) -> void*;
	void _free(void *address);

	auto _allocate_pages(U32 count) -> Page*;
	void _free_pages(Page&, U32 count);
	// void _free_pages(Physical<Page>, U32 count);
	// auto _get_physical_memory_page(Physical<void>) -> Physical<Page>;

	void _check_dangerous_address(void *from, void *to);

	void _compact();

	auto read_physical(Physical<void>, UPtr size) -> Box<U8>;
	void write_physical(Physical<void>, U8 *data, UPtr size);

	// inline void memset(U8 *address, U8 value, U32 size) {
	// 	while(--size) *address++ = value;
	// }

	struct Transaction: NonCopyable<Transaction> {
		/**/ Transaction() { lock(); }
		/**/~Transaction() { unlock(); }

		static void lock();
		static void unlock();

		auto allocate_page() -> Page* {
			return _allocate_pages(1);
		}
		auto allocate_pages(U32 count) -> Page* {
			return _allocate_pages(count);
		}
		void free_page(Page &page) {
			return _free_pages(page, 1);
		}
		void free_pages(Page &page, U32 count) {
			return _free_pages(page, count);
		}
		// void free_pages(Physical<Page> page, U32 count) {
		// 	return _free_pages(page, count);
		// }
		// void free_page_with_physical_address(Physical<void> physical, UPtr size = 1) {
		// 	auto from = get_physical_memory_page(physical);
		// 	auto to = get_physical_memory_page(physical+size);
		// 	//TODO: get the real/virtual address of this page
		// 	return free_pages(from, (to.address-from.address+pageSize-1)/pageSize);
		// }
		// auto get_physical_memory_page(Physical<void> physical) -> Physical<Page> {
		// 	return _get_physical_memory_page(physical);
		// }

		auto allocate(size_t size) -> void* {
			auto allocation = _allocate(size);
			debug::assert(allocation);
			return allocation;
		}
		void free(void *address) {
			return _free(address);
		}

		void check_dangerous_address(void *from, void *to) {
			return _check_dangerous_address(from, to);
		}

		void compact() {
			return _compact();
		}
	};

	void debug();
}

// void* operator new(size_t size) noexcept;
// void* operator new[](size_t size) noexcept;
inline void* operator new(size_t size, size_t align) = delete;
inline void* operator new[](size_t size, size_t align) = delete;

// void operator delete(void *p) noexcept;
// void operator delete(void *p, size_t) noexcept;

inline void* operator new(size_t size) noexcept { memory::Transaction transaction; return transaction.allocate(size); }
inline void* operator new[](size_t size) noexcept { memory::Transaction transaction; return transaction.allocate(size); }

inline void operator delete(void *p) noexcept { if(!p) return; memory::Transaction transaction; transaction.free(p); }
inline void operator delete(void *p, size_t) noexcept { if(!p) return; memory::Transaction transaction; transaction.free(p); }

inline void* allocate(size_t size) noexcept { if(!size) return nullptr; memory::Transaction transaction; return transaction.allocate(size); }
inline void  free(void *p) noexcept { if(!p) return; memory::Transaction transaction; return transaction.free(p); }

#include "memory.inl"
