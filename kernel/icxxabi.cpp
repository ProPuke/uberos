#include "icxxabi.hpp"

#include <kernel/Log.hpp>

static Log log("cxa");

extern "C" {

void __cxa_pure_virtual() {
	log.print_error("Error: Pure virtual method called");
	//TODO:drop to an emergency debug console?
	//TODO:halt on release?
	debug::halt();
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

}
