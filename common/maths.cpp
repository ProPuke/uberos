#include "maths.hpp"

namespace maths {
	U32 seed = 0;
	U16 rand() {
		seed = seed * 1103515245+12345;
		return (U32)(seed/65536)%32768;
	}
}
