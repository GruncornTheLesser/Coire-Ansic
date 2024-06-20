#pragma once
#include <tuple>
#include <type_traits>
#include "util/tuple_util.h"

// ? but it is saving alot of lines, and is more readable?
#define EXPAND(...) __VA_ARGS__
#define DECL_HAS_ATTRIB_TYPE(NAME)\
	template<typename T, typename=std::void_t<>> struct has_##NAME : std::false_type { };\
	template<typename T> struct has_##NAME<T, std::void_t<typename T::NAME>> : std::true_type { };\
	template<typename T> static constexpr bool has_##NAME##_v = has_##NAME<T>::value;

#define DECL_GET_ATTRIB_TYPE(NAME, DEFAULT)\
	template<typename T, typename=std::void_t<>> struct get_##NAME { using type = EXPAND(DEFAULT); };\
	template<typename T> struct get_##NAME<T, std::void_t<typename T::NAME>> { using type = typename T::NAME; };\
	template<typename T> using get_##NAME##_t = get_##NAME<T>::type;

#define DECL_GET_RECURSIVE_ATTRIB_TYPE(NAME)\
	template <typename T, typename=void> struct get_##NAME;\
	template <typename T> struct get_##NAME<T, std::enable_if_t<!has_##NAME##_v<T>>> { using type = std::tuple<T>; };\
	template <typename T> struct get_##NAME<T, std::enable_if_t<has_##NAME##_v<T>>> { using type = typename get_##NAME<typename T::NAME>::type; };\
	template<typename ... Ts> struct get_##NAME<std::tuple<Ts...>, std::enable_if_t<!has_##NAME##_v<std::tuple<Ts...>>>>\
	{ using type = util::concat_t<eval_each_t<std::tuple<Ts...>, get_##NAME>>; };\
	template<typename T> using get_##NAME##_t = typename get_##NAME<T>::type;

#define DECL_HAS_ATTRIB_VALUE(NAME)\
	template<typename T, typename=std::void_t<>> struct has_##NAME : std::false_type { };\
	template<typename T> struct has_##NAME<T, std::void_t<decltype(T::NAME)>> : std::true_type { };\
	template<typename T> static constexpr bool has_##NAME##_v = has_##NAME<T>::value;

#define DECL_GET_ATTRIB_VALUE(NAME, TYPE, DEFAULT)\
	template<typename T, typename=std::void_t<>> struct get_##NAME { static constexpr TYPE value = EXPAND(DEFAULT); };\
	template<typename T> struct get_##NAME<T, std::void_t<decltype(T::NAME)>> { static constexpr TYPE value = T::NAME; };\
	template<typename T> static constexpr TYPE get_##NAME##_v = get_##NAME<T>::value;


/*
namespace ecs {
	template<typename T>
	struct component_traits {
		using execution_policy = traits::get_execution_policy_t<T>;
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

