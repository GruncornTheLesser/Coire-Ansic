#pragma once
#include <vector>
#include "traits.h"

namespace ecs {

	template<typename T, typename Alloc_T>
	class entity_manager
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
T ecs::entity_manager<T, Alloc_T>::create()
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
void ecs::entity_manager<T, Alloc_T>::destroy(T e) {
	value_type value = e.value();
	version_type version = e.version();

	if (version != active[value]) 
		return;

	unused.push_back(value);

	if (++active[value] == 0)
		++active[value];
}

template<typename T, typename Alloc_T>
bool ecs::entity_manager<T, Alloc_T>::alive(T e) const {
	return e.version() == active[e.value()];
}