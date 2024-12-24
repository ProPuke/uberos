#pragma once

#include "types.hpp"

struct Bool256 {
	U64 mask[4] = {};

	auto get(U8 bit) -> bool {
		return mask[bit/64]&(1<<(bit%64));
	}

	void set(U8 bit, bool value) {
		unsigned bucket = bit/64;
		U64 bucketMask = (U64)1<<(bit%64);

		mask[bucket] = mask[bucket] & ~bucketMask | (value?1:0) << bucketMask;
	}

	void clear() {
		mask[0] = 0;
		mask[1] = 0;
		mask[2] = 0;
		mask[3] = 0;
	}

	auto has_any() -> bool {
		return mask[0]!=0||mask[1]!=0||mask[2]!=0||mask[3]!=0;
	}
};
