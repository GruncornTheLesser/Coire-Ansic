#pragma once
#include <cstdint>
#include "util/tuple_util.h"
#include "util/sparse_map.h"
#include "resource.h"

namespace ecs {
	class entity_manager;

	struct entity {
		friend class entity_manager;
		using resource_set = std::tuple<entity_manager>;

		entity() : value(static_cast<uint32_t>(-1)) { }
		operator uint32_t() { return value; }

		entity(const entity& other) = default;
		entity& operator=(const entity& other) = default;
		
		entity(entity&& other) = default;
		entity& operator=(entity&& other) = default;

	private:
		template<std::unsigned_integral T>
	 	entity(T v) : value(static_cast<uint32_t>(v)) { }
		uint32_t value; 
	};

	const entity tombstone{};
}

namespace ecs::traits {		
	template<typename T>
	struct is_entity : util::cmp::is_ignore_cvref_same<ecs::entity, T> { };
	template<typename T>
	static constexpr bool is_entity_v = is_entity<T>::value;

}

#include "entity_manager.h"