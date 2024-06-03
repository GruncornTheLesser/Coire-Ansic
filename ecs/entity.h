#pragma once
#include <cstdint>

namespace ecs {
	typedef uint32_t entity;
	const entity tombstone = static_cast<uint32_t>(-1);
}
