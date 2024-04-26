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

	template<typename T, typename Pip_T> struct is_pipeline_accessible : 
		util::mtc::build::allof<
			util::mtc::build::element_of<Pip_T,
				util::cmp::build::disjunction<
					util::cmp::build::transformed_rhs<std::is_same, std::add_const>::template type,
					util::cmp::build::transformed_rhs<std::is_same, std::add_const>::template type
				>::template type
			>::template type
		>::type<ecs::traits::get_resource_set_t<T>> { };

}

#endif