#pragma once

inline void halt [[noreturn]] () { halt: asm volatile("cli"); goto halt; }
inline void assert(bool test) { if(!test) halt(); }
