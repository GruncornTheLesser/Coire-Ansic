#ifndef ECS_TRAITS_H
#define ECS_TRAITS_H
#include <tuple>
#include <type_traits>
#include "util/tuple_util.h"

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
	template <typename T> struct get_##NAME<T, std::enable_if_t<!has_##NAME##_v<T>>> { using type = std::tuple<std::remove_cvref_t<T>>; };\
	template <typename T> struct get_##NAME<T, std::enable_if_t<has_##NAME##_v<T>>> { using type = typename get_##NAME<typename T::NAME>::type; };\
	template<typename ... Ts> struct get_##NAME<std::tuple<Ts...>, std::enable_if_t<!has_##NAME##_v<std::tuple<Ts...>>>>\
	{ using type = util::trn::concat_t<util::trn::each_t<std::tuple<Ts...>, get_##NAME>>; };\
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

	template<typename T, typename=std::void_t<>> struct is_resource;
	template<typename T, typename resource_set_T, typename=std::void_t<>> struct is_storage;
	template<typename T, typename resource_set_T> struct is_acquirable;

	template<typename T> concept resource_class = is_resource<T>::value;
	template<typename T, typename resource_set_T> concept storage_class = is_storage<T, get_resource_set_t<resource_set_T>>::value;
	template<typename T, typename resource_set_T> concept acquirable_class = is_acquirable<T, resource_set_T>::value;

	template<typename T, typename Pip_T> struct is_pipeline_accessible;
	template<typename T, typename Pip_T> static constexpr bool is_pipeline_accessible_v = is_pipeline_accessible<T, Pip_T>::value; 
	template<typename T, typename Pip_T> concept pipeline_accessible_class = is_pipeline_accessible<T, Pip_T>::value;
}
#include "traits.tpp"
#endif