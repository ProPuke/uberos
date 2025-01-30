#pragma once

#include "types.hpp"

struct Bitmask32 {
	U32 mask = {};

	/**/ Bitmask32() {}
	/**/ Bitmask32(U32 mask): mask(mask) {}

	auto get(U8 bit) -> bool {
		return mask&((U32)1<<bit);
	}

	void set(U8 bit, bool value) {
		U64 mask = (U64)1<<bit;

		mask = mask & ~mask | (value?mask:0);
	}

	void clear() {
		mask = 0;
	}

	auto has_any() -> bool {
		return mask!=0;
	}
};

struct Bitmask64 {
	U64 mask = {};

	/**/ Bitmask64() {}
	/**/ Bitmask64(U64 mask): mask(mask) {}

	auto get(U8 bit) -> bool {
		return mask&((U64)1<<bit);
	}

	void set(U8 bit, bool value) {
		U64 mask = (U64)1<<bit;

		mask = mask & ~mask | (value?mask:0);
	}

	void clear() {
		mask = 0;
	}

	auto has_any() -> bool {
		return mask!=0;
	}
};

struct Bitmask256 {
	U64 mask[4] = {};

	/**/ Bitmask256() {}
	/**/ Bitmask256(U64 mask[4]) {
		for(auto i=0;i<4;i++) this->mask[i] = mask[i];
	}
	/**/ Bitmask256(U64 mask0, U64 mask1, U64 mask2, U64 mask3) {
		this->mask[0] = mask0;
		this->mask[1] = mask1;
		this->mask[2] = mask2;
		this->mask[3] = mask3;
	}

	auto get(U8 bit) -> bool {
		return mask[bit/64]&((U64)1<<(bit%64));
	}

	void set(U8 bit, bool value) {
		unsigned bucket = bit/64;
		U64 bucketMask = (U64)1<<(bit%64);

		mask[bucket] = mask[bucket] & ~bucketMask | (value?bucketMask:0);
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
