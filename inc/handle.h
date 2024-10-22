#pragma once
#include <stdint.h>
#include "traits.h"
#include "resource.h"

namespace ecs {
	template<typename T>
	struct create { 
		using event_tag = ecs::tag::event::sync; 
		T value; 
	};
	template<typename T> struct destroy { using event_tag = ecs::tag::event::sync; T value; };

	template<typename T>
	struct generator {
		using resource_tag = ecs::tag::resource::custom;
		using component_set = std::tuple<>;
		using event_set = std::tuple<create<T>, destroy<T>>;
	};

	template<typename T>
	struct handle_traits<T, ecs::tag::handle::custom> {
		using generator_type = generator<T>;
	};

	template<typename T>
	struct handle_traits<T, ecs::tag::handle::versioned> {
		using generator_type = generator<T>;
	};

	template<typename T>
	struct handle_traits<T, ecs::tag::handle::unversioned> {
		using generator_type = generator<T>;
	};
}

namespace ecs {
	struct entity {
		using handle_tag = ecs::tag::handle::versioned;
		entity() = default;
		entity(tombstone) : data(-1) { }
		entity(uint32_t v) : data(v) { }
		
		constexpr uint32_t value() const { return data & 0x0000ffff; } 
		constexpr uint32_t version() const { return data & 0xffff0000; } 

	private:
		uint32_t data;
	};
}



/*
#include <vector>
#include "traits.h"

namespace ecs {

	template<typename T, typename Alloc_T>
	class handle_manager
	{
		using value_type = typename T::value_type;
		using version_type = typename T::version_type;

		using value_allocator_type = std::allocator_traits<Alloc_T>::template rebind_alloc<value_type>;
		using version_allocator_type = std::allocator_traits<Alloc_T>::template rebind_alloc<version_type>;

	public:
		[[nodiscard]] T create();
		void destroy(T e);
		[[nodiscard]] bool alive(T e) const;
	private:
		std::vector<value_type, value_allocator_type> active;
		std::vector<version_type, version_allocator_type> unused;
	};

}

template<typename T, typename Alloc_T>
T ecs::handle_manager<T, Alloc_T>::create()
{
	value_type value;
	version_type version;

	if (unused.size() == 0)
	{
		value = active.size();
		version = 1;
		active.emplace_back(version);
	}
	else
	{
		value = unused.back();
		unused.pop_back();
	}

	return { value, version };
}


template<typename T, typename Alloc_T>
void ecs::handle_manager<T, Alloc_T>::destroy(T e) {
	value_type value = e.value();
	version_type version = e.version();

	if (version != active[value]) 
		return;

	unused.push_back(value);

	if (++active[value] == 0)
		++active[value];
}

template<typename T, typename Alloc_T>
bool ecs::handle_manager<T, Alloc_T>::alive(T e) const {
	return e.version() == active[e.value()];
}
*/