#pragma once
#include "traits.h"
#include <type_traits>

namespace ecs::traits {

	template<typename t, typename> struct is_resource : std::false_type { };
	template<typename t> struct is_resource<t, std::void_t<
		decltype(std::declval<t>().acquire()),
		decltype(std::declval<t>().release()),
		decltype(std::declval<const t>().acquire()), 
		decltype(std::declval<const t>().release())>> : std::true_type { };

		
	template<typename t, typename> struct is_resource_storage : std::false_type { };
	template<typename t> struct is_resource_storage<t, std::void_t<
		typename t::resource_storage_set
		>> : std::true_type { };


	template<typename t, typename> struct is_comp : std::false_type { };
	template<typename t> struct is_comp<t, std::void_t<t>> : std::true_type { };

	template<typename t, typename> struct is_pool : std::false_type { };
	template<typename t> struct is_pool<t, std::void_t<t>> : is_resource_storage<t> { };

	template<typename t, typename> struct is_view : std::false_type { };
	template<typename t> struct is_view<t, std::void_t<t>> : std::true_type { };

	template<typename t, typename> struct is_pipeline : std::false_type { };
	template<typename t> struct is_pipeline<t, std::void_t<t>> : std::true_type { };

}