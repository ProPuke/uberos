#pragma once

inline void halt() { asm volatile("cli\n1: jmp 1b"); }
inline void assert(bool test) { if(!test) halt(); }
