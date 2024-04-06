#ifndef ECS_TRAITS_TPP
#define ECS_TRAITS_TPP
#include "traits.h"
#include <type_traits>

namespace ecs::traits {

	template<typename T, typename> struct is_resource : std::false_type { };
	template<typename T> struct is_resource<T, std::void_t<
		decltype(std::declval<T>().acquire()),
		decltype(std::declval<T>().release()),
		decltype(std::declval<const T>().acquire()), 
		decltype(std::declval<const T>().release())>> : std::true_type { };

	template<typename T, typename Tup, typename> struct is_storage : std::false_type { };
	template<typename T, typename ... Ts> struct is_storage<T, std::tuple<Ts...>, std::void_t<
		decltype(std::declval<T>().template sync<Ts>())..., 
		decltype(std::declval<T>().template get<Ts>())...>> : std::true_type { };

	/*
	 :  
	*/

	template<typename T, typename resource_set_T> struct is_acquirable : 
		std::disjunction<util::tuple_contains<std::is_same, T, resource_set_T>, 
		util::tuple_contains<std::is_same, T, util::tuple_transform_t<std::add_const, resource_set_T>>> { };

}

#endif