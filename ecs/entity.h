#pragma once
#include <cstdint>

namespace ecs {
	class entity {
	public:
		uint32_t value;
		constexpr entity() : value(-1) { }
		entity(uint32_t v) : value(v) { }
		constexpr operator uint32_t() { return value; }
	};

	const entity tombstone;
}