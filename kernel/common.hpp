#pragma once

inline void halt() { asm("cli\n1: jmp 1b"); }
