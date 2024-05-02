#ifndef ECS_TRAITS_TPP
#define ECS_TRAITS_TPP
#include "traits.h"
#include <type_traits>
#include "entity.h"
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

	template<typename T, typename Pip_T> struct is_accessible_resource : 
		util::mtc::allof<
			util::trn::propergate_const_each_t<T, ecs::traits::get_resource_set>,
			util::mtc::build::element_of<ecs::traits::get_resource_set_t<Pip_T>,
				util::cmp::build::disjunction<std::is_same,
					util::cmp::build::transformed_rhs<std::is_same, std::add_const>::template type
				>::template type
			>::template type
		> { };
	
	/*
	static_assert(ecs::traits::is_accessible_resource_v<const ecs::pool<A>::index, ecs::pipeline<const ecs::pool<A>::index>>, "get const from const");
	static_assert(ecs::traits::is_accessible_resource_v<const ecs::pool<A>::index, ecs::pipeline<A>>, "get const from mut");
	static_assert(ecs::traits::is_accessible_resource_v<ecs::pool<A>::index, ecs::pipeline<ecs::pool<A>::index>>, "get mut from mut");
	static_assert(!ecs::traits::is_accessible_resource_v<ecs::pool<A>::index, ecs::pipeline<const ecs::pool<A>::index>>, "get mut from const");
	static_assert(!ecs::traits::is_accessible_resource_v<const ecs::pool<A>::index, ecs::pipeline<>>, "get const from const");
	static_assert(!ecs::traits::is_accessible_resource_v<ecs::pool<A>::index, ecs::pipeline<>>, "get mut from const");
	*/
}

#endif