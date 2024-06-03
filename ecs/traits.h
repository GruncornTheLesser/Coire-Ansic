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

	template<typename T> struct default_order_policy;
	DECL_HAS_ATTRIB_TYPE(order_policy)
	DECL_GET_ATTRIB_TYPE(order_policy, default_order_policy<T>::type)
	
	template<typename T> struct default_sequence_policy;
	DECL_HAS_ATTRIB_TYPE(sequence_policy)
	DECL_GET_ATTRIB_TYPE(sequence_policy, default_sequence_policy<T>::type)

	template<typename T> struct default_indexer;
	DECL_HAS_ATTRIB_TYPE(indexer)
	DECL_GET_ATTRIB_TYPE(indexer, default_indexer<T>::type)

	template<typename T> struct default_storage;
	DECL_HAS_ATTRIB_TYPE(storage)
	DECL_GET_ATTRIB_TYPE(storage, default_storage<T>::type)
	
	template<typename T> struct default_handler;
	DECL_HAS_ATTRIB_TYPE(handler)
	DECL_GET_ATTRIB_TYPE(handler, default_handler<T>::type)

	template<typename T> struct default_pool_resource_set;
	DECL_HAS_ATTRIB_TYPE(pool_resource_set)
	DECL_GET_ATTRIB_TYPE(pool_resource_set, default_pool_resource_set<T>::type)
	
	DECL_HAS_ATTRIB_TYPE(resource_set)
	DECL_GET_RECURSIVE_ATTRIB_TYPE(resource_set)

	DECL_HAS_ATTRIB_TYPE(resource_alias)
	DECL_GET_ATTRIB_TYPE(resource_alias, T)
	
	DECL_HAS_ATTRIB_TYPE(resource_type)
	template<typename T, typename=std::void_t<>>  
	struct get_resource_type : std::type_identity<T> { };
	template<typename T> struct get_resource_type<T, std::enable_if_t<has_resource_alias_v<T>>>
	 : get_resource_type<get_resource_alias_t<T>> { };
	template<typename T> struct get_resource_type<T, std::enable_if_t<!has_resource_alias_v<T> && has_resource_type_v<T>>>
	 : std::type_identity<typename T::resource_type> { };
	template<typename T> using get_resource_type_t = get_resource_type<T>::type;
	
	DECL_HAS_ATTRIB_VALUE(lock_priority)
	DECL_GET_ATTRIB_VALUE(lock_priority, float, 0.5f)





	template<typename T, typename=std::void_t<>> struct is_resource;
	template<typename T> concept resource_class = is_resource<T>::value;

	template<typename T> struct is_resource_key;
	template<typename T> concept resource_key_class = is_resource_key<T>::value;
	
	template<typename T, typename Pip_T> struct is_accessible_resource;
	template<typename T, typename Pip_T> static constexpr bool is_accessible_resource_v = is_accessible_resource<T, Pip_T>::value; 
	template<typename T, typename Pip_T> concept accessible_resource_class = is_accessible_resource<T, Pip_T>::value;
	

	template<typename T> struct is_order_policy;
	template<typename T> concept order_policy_class = is_order_policy<T>::value;
	
	template<typename T> struct is_sequence_policy;
	template<typename T> concept sequence_policy_class = is_order_policy<T>::value;

	template<typename T> struct is_component;
	template<typename T> static constexpr bool is_component_v = is_component<T>::value;
	template<typename T> concept component_class = is_component<T>::value;

	template<typename T, typename> struct is_resource : std::false_type { };
	template<typename T> struct is_resource<T, std::void_t<
		decltype(std::declval<T>().acquire()),
		decltype(std::declval<T>().release()),
		decltype(std::declval<const T>().acquire()), 
		decltype(std::declval<const T>().release())>> : std::true_type { };

	template<typename T> struct is_resource_key : is_resource<get_resource_type_t<T>> { }; 



	template<typename T, typename Pip_T> struct is_accessible_resource : util::allof<
		util::propergate_const_each_t<T, ecs::traits::get_resource_set>,
			util::build::element_of<ecs::traits::get_resource_set_t<Pip_T>, 
				util::cmp::build::disjunction<std::is_same,
					util::cmp::build::transformed_rhs<std::is_same, std::add_const
				>::template type
			>::template type
		>::template type> { };

	// resource is anything that does have a resource set/ is itself a resource
	template<typename T> struct is_component : std::negation<std::disjunction<
		ecs::traits::is_resource<T>, 
		ecs::traits::has_resource_set<T>>> { };
	
	template<typename T> struct is_order_policy : std::true_type { };

	template<typename T> struct is_sequence_policy : std::true_type { };

}
/*
namespace ecs {
	template<typename T>
	struct component_traits {
		using order_policy = traits::get_order_policy_t<T>;
		using sequence_policy = traits::get_sequence_policy_t<T>;
		using indexer = indexer<T>;
		using storage = storage<T>;
		using handler = handler<T>;
	};

	template<typename T>
	struct resource_traits {
		using set = traits::get_resource_set_t<T>;
		using alias = traits::get_resource_alias_t<T>;
		using type = traits::get_resource_type_t<T>;
		static constexpr float lock_priority = traits::get_lock_priority_v<T>;
	};

	
}
*/

