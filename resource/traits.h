#pragma once
#include <type_traits>
#include "../util/tuple_util.h"

namespace resource::traits {
	// if a resource has the alias trait the resource describes an alias for the type defined in resource_alias
	template<typename T, typename=std::void_t<>> 
	struct has_alias : std::false_type { };
	template<typename T> 
	struct has_alias<T, std::void_t<typename T::resource_alias>> : std::true_type { };
	template<typename T> 
	static constexpr bool has_alias_v = has_alias<T>::value;
	
	template<typename T, typename=std::void_t<>> 
	struct get_alias { using type = T; };
	template<typename T> 
	struct get_alias<T, std::enable_if_t<std::is_const_v<T> && has_alias_v<T>>> : std::type_identity<const typename T::resource_alias> { };
	template<typename T> 
	struct get_alias<T, std::enable_if_t<!std::is_const_v<T> && has_alias_v<T>>> : std::type_identity<typename T::resource_alias> { };
	template<typename T> 
	using get_alias_t = get_alias<T>::type;
	

	// instance_count describes the max number of acquirable resources
	// defaults to 1
	template<typename T, typename=std::void_t<>>
	struct has_instance_count : std::false_type { };
	template<typename T>
	struct has_instance_count<T, std::void_t<typename T::resource_instance_count>> : std::true_type { };
	template<typename T>
	static constexpr bool has_instance_count_v = has_instance_count<T>::value;

	template<typename T, typename=std::void_t<>>  
	struct get_instance_count : std::integral_constant<int, 1> { };
	template<typename T>
	struct get_instance_count<T, std::enable_if_t<!std::is_same_v<get_alias_t<T>, T>>>
	 : get_instance_count<get_alias_t<T>> { }; 
	template<typename T>
	struct get_instance_count<T, std::enable_if_t<std::is_same_v<get_alias_t<T>, T> && has_instance_count_v<T>>>
	 : std::integral_constant<int, T::resource_instance_count> { };
	template<typename T> 
	static constexpr int get_instance_count_v = get_instance_count<T>::value;


	// resource type describes the type of value stored. 
	// eg res_A { using resource_type = int; }; int res_type = reg.get_resource<res_A>();  
	template<typename T, typename=std::void_t<>> 
	struct has_type : std::false_type { };
	template<typename T> 
	struct has_type<T, std::void_t<typename T::resource_type>> : std::true_type { };
	template<typename T> 
	static constexpr bool has_type_v = has_type<T>::value;
	
	template<typename T, typename=std::void_t<>>  
	struct get_type : std::type_identity<T> { };
	template<typename T>
	struct get_type<T, std::enable_if_t<!std::is_same_v<get_alias_t<T>, T>>>
	 : util::propagate_const<T, util::eval_<get_alias, get_type>::template type> { };
	template<typename T>
	struct get_type<T, std::enable_if_t<std::is_same_v<get_alias_t<T>, T> && has_type_v<T>>>
	 : std::type_identity<typename T::resource_type> { };
	template<typename T> 
	using get_type_t = get_type<T>::type;
	

	// a resource lockset describes a set of resources that are also locked when locking T. 
	// T does not need to be a resource it could also be an operation. T must appear in T::lockset
	// to be locked. any qualifiers on the types in lockset are propagated through.
	template<typename T, typename=std::void_t<>> 
	struct has_lockset : std::false_type { };
	template<typename T> 
	struct has_lockset<T, std::void_t<typename T::resource_lockset>> : std::true_type { };
	template<typename T> 
	static constexpr bool has_lockset_v = has_lockset<T>::value;
	
	template<typename T, typename callset_T=std::tuple<T>, typename=void>
	struct get_lockset;
	template<typename T, typename callset>
	struct get_lockset<T, callset, std::enable_if_t<!util::is_tuple_v<T> && !has_lockset_v<T>>>
	 : std::type_identity<std::tuple<T>> { };
	template<typename T, typename callset>
	struct get_lockset<T, callset, std::enable_if_t<!util::is_tuple_v<T> && has_lockset_v<T>>> 
	 : get_lockset<typename T::resource_lockset, callset> { };
	template<typename Tup, typename callset>
	struct get_lockset<Tup, callset, std::enable_if_t<util::is_tuple_v<Tup>>> 
	 : util::eval<Tup, 
	 	util::eval_each_< // for each
			util::propagate_const_each_< // if const add const to each
				util::eval_if_<util::element_of_<callset>::template type, // check if looping
					util::wrap_<std::tuple>::template type, // if looping
					util::add_type_args_<get_lockset, // else recursive call
						util::concat_t<std::tuple<callset, Tup>> // add Ts to callset so not called again
					>::template type
				>::template type
			>::template type
		>::template type,
		util::concat> { }; // concat together
	template<typename T> using get_lockset_t = typename get_lockset<T>::type;

	
	// lock level determines the order in which resources are locked, levels are locked in order. // if 2 resources  
	// have equal lock levels then resources lock in unspecified yet consistent order(secretly alphabetical). 
	// lock level defaults to 0.
	template<typename T, typename=std::void_t<>> 
	struct has_lock_level : std::false_type { };
	template<typename T> 
	struct has_lock_level<T, std::void_t<typename T::resource_lock_level>> : std::true_type { };
	template<typename T> 
	static constexpr bool has_lock_level_v = has_lock_level<T>::value;
	
	template<typename T, typename=std::void_t<>> 
	struct get_lock_level { static constexpr int value = 0; };
	template<typename T> 
	struct get_lock_level<T, std::void_t<decltype(T::lock_level)>> { static constexpr int value = T::lock_level; };
	template<typename T> 
	static constexpr int get_lock_level_v = get_lock_level<T>::value;


	template<typename T, typename=std::void_t<>> 
	struct is_resource : std::true_type { }; // TODO update me to a new definition of a resource
	template<typename T> 
	struct is_resource<T, std::void_t<
		decltype(std::declval<T>().acquire()), decltype(std::declval<const T>().acquire()),
		decltype(std::declval<T>().release()), decltype(std::declval<const T>().release())>>
		: std::true_type { };
	template<typename T> 
	static constexpr bool is_v = is_resource<T>::value;
	template<typename T> 
	concept resource_class = is_resource<T>::value;



	template<typename T> 
	struct is_resource_key : is_resource<get_type_t<T>> { }; 
	template<typename T> 
	static constexpr bool is_resource_key_v = is_resource_key<T>::value;
	template<typename T> 
	concept resource_key_class = is_resource_key<T>::value;



	template<typename T, typename lock_T> 
	struct is_accessible : util::is_subset<get_lockset_t<T>, get_lockset_t<lock_T>, util::cmp::is_const_accessible>
	{ };
	template<typename T, typename lock_T> 
	static constexpr bool is_accessible_v = is_accessible<T, lock_T>::value;
	template<typename T, typename lock_T> 
	concept accessible_class = is_accessible<T, lock_T>::value;
}