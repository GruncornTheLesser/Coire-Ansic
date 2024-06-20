#pragma once
#include "traits.h"
#include <shared_mutex>

namespace ecs {
	struct resource {
		void acquire() { mtx.lock(); }
		void release() { mtx.unlock(); };
		void acquire() const { mtx.lock_shared(); };
		void release() const { mtx.unlock_shared(); };
	private:
		mutable std::shared_mutex mtx;
	};
}

namespace ecs::traits {
	// resource alias describes a name for a type
	DECL_HAS_ATTRIB_TYPE(resource_alias)
	DECL_GET_ATTRIB_TYPE(resource_alias, std::remove_const_t<T>)
	
	DECL_HAS_ATTRIB_TYPE(resource_type)
	template<typename T, typename=std::void_t<>>  
	struct get_resource_type : std::type_identity<std::remove_const_t<T>> { };
	template<typename T> struct get_resource_type<T, std::enable_if_t<has_resource_alias_v<T> && !std::is_same_v<get_resource_alias_t<T>, T>>>
	 : get_resource_type<get_resource_alias_t<T>> { };
	template<typename T> struct get_resource_type<T, std::enable_if_t<!has_resource_alias_v<T> && has_resource_type_v<T>>>
	 : std::type_identity<typename T::resource_type> { };
	template<typename T> using get_resource_type_t = get_resource_type<T>::type;
	

	DECL_HAS_ATTRIB_TYPE(resource_set)
	template<typename T, typename callset_T=std::tuple<T>, typename=void>
	struct get_resource_set;
	template<typename T, typename callset>
	struct get_resource_set<T, callset, std::enable_if_t<!util::is_tuple_v<T> && !has_resource_set_v<T>>>
	 : std::type_identity<std::tuple<T>> { };
	template<typename T, typename callset>
	struct get_resource_set<T, callset, std::enable_if_t<!util::is_tuple_v<T> && has_resource_set_v<T>>> 
	 : get_resource_set<typename T::resource_set, callset> { };
	template<typename Tup, typename callset>
	struct get_resource_set<Tup, callset, std::enable_if_t<util::is_tuple_v<Tup>>> 
	 : util::eval<Tup, util::eval_each_<
			util::propergate_const_each_<
				util::eval_if_<util::element_of_<callset>::template type,
					util::wrap_<std::tuple>::template type, // add T
					util::add_type_args_<get_resource_set, 
						util::concat_t<std::tuple<callset, Tup>>
					>::template type // rescursively
				>::template type
			>::template type
		>::template type,
		util::concat> { };
	template<typename T> using get_resource_set_t = typename get_resource_set<T>::type;

	DECL_HAS_ATTRIB_VALUE(lock_priority)
	DECL_GET_ATTRIB_VALUE(lock_priority, int, 0)

	template<typename T, typename=std::void_t<>> struct is_resource;
	template<typename T> concept resource_class = is_resource<T>::value;
	template<typename T, typename> struct is_resource : std::false_type { };
	template<typename T> struct is_resource<T, std::void_t<
		decltype(std::declval<T>().acquire()), decltype(std::declval<const T>().acquire()),
		decltype(std::declval<T>().release()), decltype(std::declval<const T>().release())>>
	 : std::true_type { };

	template<typename T> struct is_resource_key;
	template<typename T> concept resource_key_class = is_resource_key<T>::value;
	template<typename T> struct is_resource_key : is_resource<get_resource_type_t<T>> { }; 
	template<typename T> static constexpr bool is_resource_key_v = is_resource_key<T>::value;


}