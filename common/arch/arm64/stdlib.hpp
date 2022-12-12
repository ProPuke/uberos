#include <common/stdlib.h>

#ifdef USE_STDLIB_ASM

	extern "C" {

		#include <lib/arm-optimized-routines/string/include/stringlib.h>

		inline auto __attribute__ ((optimize(2))) memcpy(void *__restrict dest, const void *__restrict src, size_t bytes) -> void* {
			return __memcpy_aarch64(dest, src, bytes);
			// return __memcpy_aarch64_simd(dest, src, bytes);
		}

		inline auto __attribute__ ((optimize(2))) memmove(void *__restrict dest, const void *__restrict src, size_t bytes) -> void* {
			return __memmove_aarch64(dest, src, bytes);
		}

		inline auto __attribute__ ((optimize(2))) memcmp(const void *a, const void *b, size_t bytes) -> int {
			return __memcmp_aarch64(a, b, bytes);
		}

		inline auto __attribute__ ((optimize(2))) memset(void *dest, int value, size_t bytes) -> void* {
			return __memset_aarch64(dest, value, bytes);
		}

		inline auto __attribute__ ((optimize(2))) strlen(const char *str) -> size_t {
			return __strlen_aarch64(str);
		}

		inline auto __attribute__ ((optimize(2))) strcpy(char *__restrict destination, const char *__restrict source) -> char* {
			return __strcpy_aarch64(destination, source);
		}
		inline auto __attribute__ ((optimize(2))) strcmp(const char* str1, const char* str2) -> int {
			return __strcmp_aarch64(str1, str2);
		}

	}

#endif
