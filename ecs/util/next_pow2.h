#ifndef ECS_NEXT_POW2_H
#define ECS_NEXT_POW2_H
#include <stdint.h>

namespace util {
	static constexpr uint32_t next_pow2(uint32_t v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v++;
		
		return v;
	}

	static constexpr uint64_t next_pow2(uint64_t v) {
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		v++;
		
		return v;
	}
}

#endif