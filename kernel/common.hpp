#pragma once

inline void halt() { asm volatile("cli\n1: jmp 1b"); }
