#include <common/config.h>

#define KERNEL_STACK_SIZE (4096*4)
#define LOW_MEMORY_SIZE (4094)

#define ASM_FUNCTION32(NAME) /**/;\
    .code32;\
    .section .text;\
    .global NAME;\
    .type NAME, @function;\
    NAME
