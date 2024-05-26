#pragma once
#include <tuple>
#include <type_traits>
#include "util/tuple_util.h"

// TODO: I dont think this should be done with macros I want to do to much specific stuff

#define EXPAND(...) __VA_ARGS__
#define DECL_HAS_ATTRIB_TYPE(NAME)\
	template<typename T, typename=std::void_t<>> struct has_##NAME : std::false_type { };\
	template<typename T> struct has_##NAME<T, std::void_t<typename T::NAME>> : std::true_type { };\
	template<typename T> static constexpr bool has_##NAME##_v = has_##NAME<T>::value;

#define DECL_GET_ATTRIB_TYPE(NAME, DEFAULT)\
	template<typename T, typename=std::void_t<>> struct get_##NAME { using type = DEFAULT; };\
	template<typename T> struct get_##NAME<T, std::void_t<typename T::NAME>> { using type = typename T::NAME; };\
	template<typename T> using get_##NAME##_t = get_##NAME<T>::type;

#define DECL_GET_RECURSIVE_ATTRIB_TYPE(NAME)\
	template <typename T, typename=void> struct get_##NAME;\
	template <typename T> struct get_##NAME<T, std::enable_if_t<!has_##NAME##_v<T>>> { using type = std::tuple<T>; };\
	template <typename T> struct get_##NAME<T, std::enable_if_t<has_##NAME##_v<T>>> { using type = typename get_##NAME<typename T::NAME>::type; };\
	template<typename ... Ts> struct get_##NAME<std::tuple<Ts...>, std::enable_if_t<!has_##NAME##_v<std::tuple<Ts...>>>>\
	{ using type = util::concat_t<util::each_t<std::tuple<Ts...>, get_##NAME>>; };\
	template<typename T> using get_##NAME##_t = typename get_##NAME<T>::type;

#define DECL_HAS_ATTRIB_VALUE(NAME)\
	template<typename T, typename=std::void_t<>> struct has_##NAME : std::false_type { };\
	template<typename T> struct has_##NAME<T, std::void_t<decltype(T::NAME)>> : std::true_type { };\
	template<typename T> static constexpr bool has_##NAME##_v = has_##NAME<T>::value;

#define DECL_GET_ATTRIB_VALUE(NAME, TYPE, DEFAULT)\
	template<typename T, typename=std::void_t<>> struct get_##NAME { static constexpr TYPE value = DEFAULT; };\
	template<typename T> struct get_##NAME<T, std::void_t<decltype(T::NAME)>> { static constexpr TYPE value = T::NAME; };\
	template<typename T> static constexpr TYPE get_##NAME##_v = get_##NAME<T>::value;

namespace ecs::traits {
	DECL_HAS_ATTRIB_TYPE(resource_container)
	DECL_GET_ATTRIB_TYPE(resource_container, T)

	DECL_HAS_ATTRIB_TYPE(resource_set)
	DECL_GET_RECURSIVE_ATTRIB_TYPE(resource_set)
	
	DECL_HAS_ATTRIB_TYPE(synchronization_set)
	DECL_GET_ATTRIB_TYPE(synchronization_set, std::tuple<>)
	
	DECL_HAS_ATTRIB_VALUE(lock_priority)
	DECL_GET_ATTRIB_VALUE(lock_priority, float, 0.5f)

	template<typename T, typename=std::void_t<>> struct is_resource : std::false_type { };
	template<typename T> struct is_resource<T, std::void_t<
		decltype(std::declval<T>().acquire()),
		decltype(std::declval<T>().release()),
		decltype(std::declval<const T>().acquire()), 
		decltype(std::declval<const T>().release())>> : std::true_type { };
	template<typename T> concept resource_class = is_resource<T>::value;

	template<typename T, typename Pip_T> struct is_accessible_resource : util::allof<
		util::propergate_const_each_t<T, ecs::traits::get_resource_set>,
			util::build::element_of<ecs::traits::get_resource_set_t<Pip_T>, 
				util::cmp::build::disjunction<std::is_same,
					util::cmp::build::transformed_rhs<std::is_same, std::add_const
				>::template type
			>::template type
		>::template type> { };
	template<typename T, typename Pip_T> static constexpr bool is_accessible_resource_v = is_accessible_resource<T, Pip_T>::value; 
	template<typename T, typename Pip_T> concept accessible_resource_class = is_accessible_resource<T, Pip_T>::value;

	
	// NOTE: this is defined in pool.h, not a huge fan of this. 
	// put here so I could have get_pool in traits.h
	template<typename T> struct default_pool;

	DECL_HAS_ATTRIB_TYPE(pool)
	DECL_GET_ATTRIB_TYPE(pool, typename default_pool<T>::type) // gets T::pool, defaults to archetype<T>

	template<typename T> struct get_pool_index_storage { using type = typename get_pool_t<std::remove_const_t<T>>::index; };
	template<typename T> using index_storage_t = typename get_pool_index_storage<T>::type;

	template<typename T> struct get_pool_entity_storage { using type = typename get_pool_t<std::remove_const_t<T>>::entity; };
	template<typename T> using entity_storage_t = typename get_pool_entity_storage<T>::type;
	
	template<typename T> struct get_pool_component_storage { using type = typename get_pool_t<std::remove_const_t<T>>::template comp<std::remove_const_t<T>>; };
	template<typename T> using component_storage_t = typename get_pool_component_storage<T>::type;


	template<typename T> struct is_component : std::negation<std::disjunction<
		ecs::traits::is_resource<T>, 
		ecs::traits::has_resource_set<T>>> { };
	template<typename T> static constexpr bool is_component_v = is_component<T>::value;
	template<typename T> concept component_class = is_component<T>::value;
}

namespace ecs::traits {
	

	
}

