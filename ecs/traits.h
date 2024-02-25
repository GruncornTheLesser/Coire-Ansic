#pragma once
#define VA_ARGS_COMMA(...), ##__VA_ARGS__
#define DECL_HAS_ATTRIB_TYPE(NAME, ATTRIB)\
	template<typename T, typename=std::void_t<>> struct has_##NAME : std::false_type { };\
	template<typename T> struct has_##NAME<T, std::void_t<typename T::ATTRIB>> : std::true_type { };\
	template<typename T> static constexpr bool has_##NAME##_v = has_##NAME<T>::value;

#define DECL_GET_ATTRIB_TYPE(NAME, DEFAULT)\
	template<typename T, typename=std::void_t<>> struct get_##NAME { using type = DEFAULT; };\
	template<typename T> struct get_##NAME<T, std::void_t<typename T::NAME>> { using type = typename T::NAME; };\
	template<typename T> using get_##NAME##_t = get_##NAME<T>::type;

#define DECL_HAS_ATTRIB_VALUE(NAME, ATTRIB)\
	template<typename T, typename=std::void_t<>> struct has_##NAME : std::false_type { };\
	template<typename T> struct has_##NAME<T, std::void_t<decltype(std::declval<T>().ATTRIB)>> : std::true_type { };\
	template<typename T> static constexpr bool has_##NAME##_v = has_##NAME<T>::value;

#define DECL_HAS_STATIC_ATTRIB_VALUE(NAME, ATTRIB)\
	template<typename T, typename=std::void_t<>> struct has_##NAME : std::false_type { };\
	template<typename T> struct has_##NAME<T, std::void_t<decltype(T::##ATTRIB)>> : std::true_type { };\
	template<typename T> static constexpr bool has_##NAME##_v = has_##NAME<T>::value;

namespace ecs::traits {
	DECL_HAS_ATTRIB_VALUE(resource_acquire_func, acquire())
	DECL_HAS_ATTRIB_VALUE(resource_release_func, release())
	
	DECL_HAS_ATTRIB_TYPE(resource_container, resource_container)
	DECL_GET_ATTRIB_TYPE(resource_container, T)
	DECL_HAS_ATTRIB_TYPE(resource_storage_set, resource_storage_set)
	DECL_GET_ATTRIB_TYPE(resource_storage_set, std::tuple<>)



	template<typename t, typename=std::void_t<>> struct is_pipeline;
	template<typename t, typename=std::void_t<>> struct is_comp;
	template<typename t, typename=std::void_t<>> struct is_pool;
	template<typename t, typename=std::void_t<>> struct is_view;
	template<typename t, typename=std::void_t<>> struct is_resource;
	template<typename t, typename=std::void_t<>> struct is_resource_storage;

	template<typename t> concept pipeline_class = is_pipeline<t>::value;
	template<typename t> concept comp_class = is_comp<t>::value;
	template<typename t> concept pool_class = is_pool<t>::value;
	template<typename t> concept view_class = is_view<t>::value;
	template<typename t> concept resource_class = is_resource<t>::value;
	template<typename t> concept resource_storage_class = is_resource_storage<t>::value;
	template<typename t> concept acquireable = traits::resource_class<t> || traits::resource_storage_class<t>;
}

#include "traits.tpp"