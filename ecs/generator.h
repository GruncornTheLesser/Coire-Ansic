#pragma once
#include "traits.h"
#include "resource.h"

namespace ecs {
	struct entity {
	public:
		using ecs_tag = ecs::tag::handle;
		
		constexpr inline entity() = default;
		constexpr inline entity(uint32_t v) : value(v) { }
		constexpr inline entity& operator=(uint32_t v) { value = v; return *this; }
		
		constexpr inline operator uint32_t() const { return value; }
		
		constexpr inline bool operator==(const entity& other) const { return value == other.value; }
		constexpr inline bool operator!=(const entity& other) const { return value != other.value; }
	private:
		uint32_t value;
	};

	struct tombstone { };
}

namespace ecs {
	template<typename T>
	struct generator : ecs::restricted_resource {
		// handle traits
		using value_type = typename handle_traits<T>::value_type;
		static constexpr value_type empty_flag = handle_traits<T>::tomb;

		constexpr inline T create() {
			if (inactive == empty_flag)
				return active.emplace_back(active.size()); // initialize with new handle active.size() and version 0
			

			size_t next = handle_traits<T>::get_data(active[inactive]); // get next in inactive list

			value_type handle;
			handle = handle_traits<T>::create(inactive, active[inactive]); // create, copy previous version
			handle = handle_traits<T>::resurrect(handle);
			active[inactive] = handle;
			
			inactive = next;

			return handle;
		}
		
		constexpr inline void destroy(T handle) {
			value_type curr = handle_traits<T>::get_data(handle);
			if (active[curr] != handle) return;
			
			active[curr] = handle_traits<T>::create(inactive, handle);
			inactive = curr;
		}

		constexpr inline bool alive(T handle) const {
			return active[handle_traits<T>::get_data(handle)] == handle;
		}

	private:
		size_t inactive = empty_flag;
		std::vector<T> active; // sparse
	};

	// generator service

}