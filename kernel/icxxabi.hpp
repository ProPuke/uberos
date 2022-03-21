#pragma once

#define ATEXIT_FUNC_MAX 128

extern "C" {

	typedef unsigned uarch_t;

	struct atexitFuncEntry_t {
		void (*destructor) (void *);
		void *object;
		void *dso;
	};

	extern void *__dso_handle;

	int __aeabi_atexit(void *object, void (*destructor)(void *), void *dso);

	int __cxa_atexit(void (*destructor)(void *), void *object, void *dso);
	void __cxa_finalize(void *destructor);

	void __cxa_pure_virtual();

}
