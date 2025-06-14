#pragma once

inline void halt [[noreturn]] () { halt: asm volatile("cli"); goto halt; }
template <typename Type>
inline auto assert(Type test) -> Type { if(!test) halt(); return test; }
