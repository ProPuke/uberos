#include <common/config.h>

#define KERNEL_STACK_SIZE (4096*4)

#define ASM_FUNCTION32(NAME) /**/;\
    .code32;\
    .section .text;\
    .global NAME;\
    .type NAME, @function;\
    NAME
