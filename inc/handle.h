#pragma once
#include <stdint.h>
#include "traits.h"
#include "resource.h"

namespace ecs {
	template<typename T>
	struct create { using event_tag = ecs::tag::event::sync; T handle; };
	template<typename T> struct destroy { using event_tag = ecs::tag::event::sync; T handle; };

	template<typename T>
	struct handle_traits<T, ecs::tag::handle::custom> { 
		using generator_type = typename T::generator_type;
	};

	template<typename T>
	struct handle_traits<T, ecs::tag::handle::versioned> {
	private:
		static constexpr uint32_t version_incr = 0x00100000;
		static constexpr uint32_t version_mask = 0xfff00000;
		static constexpr uint32_t index_mask =   0x000fffff;
	public:
		using generator_type = generator<ecs::entity>;
		using value_type = uint32_t;
		static constexpr inline value_type tombstone(uint32_t curr = index_mask) { 
			return index_mask | version_mask;
		}
		static constexpr inline value_type create(uint32_t curr, uint32_t prev = 0) {
			return (curr & index_mask) | (prev & version_mask);
		}
		static constexpr inline value_type resurrect(uint32_t curr, uint32_t prev) {
			return create(curr, prev) + version_incr;
		}
		static constexpr inline value_type destroy(uint32_t handle) { 
			return handle & version_mask;
		}
		static constexpr inline bool is_current(uint32_t lhs, uint32_t rhs) {
			return ((lhs ^ rhs) & version_mask) == 0;
		}
		static constexpr inline value_type get_index(uint32_t v) {
			return v & index_mask;
		}
		static constexpr inline value_type get_version(uint32_t v) { 
			return v & version_mask;
		}
	};

	struct entity {
	public:
		using handle_tag = ecs::tag::handle::versioned;
		
		constexpr entity() = default;
		constexpr entity(uint32_t v) : value(v) { }
		constexpr entity& operator=(uint32_t v) { value = v; return *this; }
		
		constexpr operator uint32_t() { return value; }
		
		constexpr bool operator==(const entity& other) const { return value == other.value; }
		constexpr bool operator!=(const entity& other) const { return value != other.value; }
	private:
		uint32_t value;
	};
}

namespace ecs {
	template<typename T>
	struct generator {
		static_assert(!std::is_const_v<T>);
		// resource traits
		using resource_tag = ecs::tag::resource::custom;
		using component_set = std::tuple<>;
		using event_set = std::tuple<create<T>, destroy<T>>;
		// handle traits
		using value_type = typename handle_traits<T>::value_type;

		template<typename reg_T>
		static T create(reg_T* reg) {
			if (reg->template get<generator<T>>().inactive.empty()) {
				auto ent = handle_traits<T>::create(reg->template get<generator<T>>().active.size());
				return reg->template get<generator<T>>().active.emplace_back(ent); // initialize with value_type
			}
			
			value_type val = reg->template get<generator<T>>().inactive.back();
			reg->template get<generator<T>>().inactive.pop_back();

			auto& ent = reg->template get<generator<T>>().active[val];
			ent = handle_traits<T>::resurrect(val, ent);
			reg->template on<ecs::create<T>>().invoke({ent});
			return ent;
		}
		
		template<typename reg_T>
		static void destroy(reg_T* reg, T handle) {
			auto key = handle_traits<T>::get_index(handle);
			if (reg->template get<generator<T>>().active[key] != handle) return;

			reg->template on<ecs::destroy<T>>().invoke({handle});

			reg->template get<generator<T>>().inactive.push_back(key);
			reg->template get<generator<T>>().active[key] = handle_traits<T>::destroy(handle);
		}

		template<typename reg_T>
		static bool alive(const reg_T* reg, T handle) {
			return reg->template get<generator<T>>().active[handle_traits<T>::get_index(handle)] == handle;
		}

	private:
		std::vector<value_type> inactive; // packed
		std::vector<T> active; // sparse
	};
}