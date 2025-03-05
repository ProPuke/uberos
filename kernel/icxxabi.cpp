#include "icxxabi.hpp"

#include <kernel/Log.hpp>
#include <kernel/panic.hpp>

static Log log("cxa");

extern "C" {

void __cxa_pure_virtual() {
	panic::panic()
		.print_details("Error: Pure virtual method called")
		.print_stacktrace()
	;
}

atexitFuncEntry_t __atexitFuncs[ATEXIT_FUNC_MAX];
uarch_t __atexitFuncCount = 0;

// void *__dso_handle = 0;

int __aeabi_atexit(void *object, void (*destructor)(void *), void *dso){
  return __cxa_atexit(destructor, object, dso);
}

int __cxa_atexit(void (*destructor)(void *), void *object, void *dso){
	if(__atexitFuncCount >= ATEXIT_FUNC_MAX){
		return -1;
	}
	__atexitFuncs[__atexitFuncCount].destructor = destructor;
	__atexitFuncs[__atexitFuncCount].object = object;
	__atexitFuncs[__atexitFuncCount].dso = dso;
	__atexitFuncCount++;
	return 0;
}

void __cxa_finalize(void *f){
	signed i = __atexitFuncCount;
	if(!f){
		while(i--){
			if(__atexitFuncs[i].destructor){
				(*__atexitFuncs[i].destructor)(__atexitFuncs[i].object);
			}
		}
		return;
	}

	for(; i >= 0; i--){
		if(__atexitFuncs[i].destructor == f){
			(*__atexitFuncs[i].destructor)(__atexitFuncs[i].object);
			__atexitFuncs[i].destructor = 0;
		}
	}
}

__extension__ typedef int __guard __attribute__((mode(__DI__)));

//TODO: support multithreading

extern "C" auto __cxa_guard_acquire (volatile __guard *guard) -> int {
	return !*(int*)(guard);
}

extern "C" void __cxa_guard_release (volatile __guard *guard) {
	*(int*)guard = 1;
}

extern "C" void __cxa_guard_abort (volatile __guard *guard) {
	*(int*)guard = 0;
}

}
