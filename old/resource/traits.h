#pragma once
#include <type_traits>
#include "../tuple_util/tuple_util.h"
#include "../util/attribute.h"

namespace resource::traits {
	DECL_ATTRIB_NAMESPACE
	DECL_TYPE_ATTRIB(resource_alias, T)
	DECL_TYPE_ATTRIB(resource_type, std::remove_cv_t<T>)
	DECL_TYPE_ATTRIB(resource_lockset, std::tuple<T>)
	DECL_VALUE_ATTRIB(int, resource_inst_count, 1)
	DECL_VALUE_ATTRIB(int, resource_lock_level, 0)


	/// @brief the resource alias is a type used to identify the resource, a alias can be shared between multiple type
	/// @example res_A { using resource_alias = int; }; res_B { using resource_alias = int; }; &reg.get_resource<res_A>() == &reg.get_resource<res_B>()
	template<typename T> struct get_alias : util::copy_cv<get_attribute_t<T, attribute::resource_alias>, T> { };
	template<typename T> using get_alias_t = get_alias<T>::type;

	/// @brief resource type describes the type of value stored.
	/// eg res_A { using resource_type = int; }; int res = reg.get_resource<res_A>();
	template<typename T> struct get_type : get_attribute<get_alias_t<T>, attribute::resource_type> { };
	template<typename T> using get_type_t = get_type<T>::type;



	/// @brief instance count describes the max number of acquirable resources, defaults to 1.
	/// @example res_A { static constexpr size_t inst_count = 3; }; auto lock_ref_1 = reg.get_resource<res_A>();
	template<typename T> struct get_inst_count : get_attribute<get_alias_t<T>, attribute::resource_inst_count> { };
	template<typename T> static constexpr size_t get_inst_count_v = get_inst_count<T>::value;
	
	/// @brief lock level determines the order in which resources are locked, levels are locked in order.
	/// if 2 resources have equal lock levels then resources lock in a consistent alphabetical order.
	/// lock level defaults to 0.
	template<typename T> struct get_lock_level : get_attribute<get_alias_t<T>, attribute::resource_lock_level> { };
	template<typename T> static constexpr int get_lock_level_v = get_lock_level<T>::value;



	/// @brief a resource lockset describes a set of resources that are also locked when locking T in a resource set.
	/// lockset resources are collected recursively, any qualifiers are propagated down to the types in T::lockset.
	/// T does not itself need to be a resource. T can represent a collection of resources T::lockset.
	template<typename T, typename callset_T=std::tuple<>, typename=std::void_t<>>
	struct get_lockset;

	template<typename T, typename callset_T>
	struct get_lockset<T, callset_T, std::enable_if_t<has_attribute_v<T, attribute::resource_lockset> && !util::pred::element_of_v<T, callset_T, util::cmp::is_ignore_cv_same>>>
	 : util::propagate_cv_each<T, util::add_arg_<get_attribute, attribute::resource_lockset>::template type, // get resource set
		util::eval_each_<util::add_arg_<get_lockset, util::append_t<callset_T, T>>::template type>::template type, util::concat> { };

	template<typename T, typename callset_T> 
	struct get_lockset<T, callset_T, std::enable_if_t<!has_attribute_v<T, attribute::resource_lockset> || util::pred::element_of_v<T, callset_T, util::cmp::is_ignore_cv_same>>>
	 : std::type_identity<std::tuple<T>> { };

	template<typename T>
	using get_lockset_t = typename get_lockset<T>::type;



	template<typename T, typename=void>
	struct is_resource : std::negation<std::is_same<get_alias_t<T>, void>> { };
	template<typename T>
	static constexpr bool is_resource_v = is_resource<T>::value;
	template<typename T>
	concept resource_class = is_resource<T>::value;


	template<typename T, typename lock_T>
	struct is_accessible : util::pred::is_subset<get_lockset_t<T>, get_lockset_t<lock_T>, util::cmp::is_const_accessible> { };
	template<typename T, typename lock_T>
	static constexpr bool is_accessible_v = is_accessible<T, lock_T>::value;
	template<typename T, typename lock_T>
	concept accessible_class = is_accessible<T, lock_T>::value;
}

namespace resource
{
	static constexpr size_t dynamic_instance_count = 0;
} 


//#if _DEBUG
namespace resource::test
{
	struct res_A { };
	using res_A_type = resource::traits::get_type_t<res_A>;
	using res_A_alias = resource::traits::get_alias_t<res_A>;
	using res_A_lockset = resource::traits::get_lockset_t<res_A>;
	static constexpr int res_A_inst_count = resource::traits::get_inst_count_v<res_A>;
	static constexpr int res_A_lock_level = resource::traits::get_lock_level_v<res_A>;
	struct res_B
	{
		using resource_type = int;
		using resource_lockset = std::tuple<int, float>;
		static constexpr int resource_lock_level = 1;
		static constexpr int resource_inst_count = 2;
	};

	using res_B_type = resource::traits::get_type_t<res_B>;
	using res_B_alias = resource::traits::get_alias_t<res_B>;
	using res_B_lockset = resource::traits::get_lockset_t<res_B>;
	static constexpr int res_B_inst_count = resource::traits::get_inst_count_v<res_B>;
	static constexpr int res_B_lock_level = resource::traits::get_lock_level_v<res_B>;

	struct res_C
	{
		using resource_alias = res_B;
		using resource_lockset = std::tuple<res_C, res_B>;
	};
	using res_C_type = resource::traits::get_type_t<res_C>;
	using res_C_alias = resource::traits::get_alias_t<res_C>;
	using res_C_lockset = typename resource::traits::get_lockset_t<res_C>;
	static constexpr int res_C_inst_count = resource::traits::get_inst_count_v<res_C>;
	static constexpr int res_C_lock_level = resource::traits::get_lock_level_v<res_C>;

	struct res_D
	{
		using resource_lockset = std::tuple<const res_B>;
	};

	using res_D_lockset = typename resource::traits::get_lockset_t<res_D>;


	static_assert(std::is_same_v<resource::traits::get_type_t<res_A>, res_A>, "failed default resource type");
	static_assert(std::is_same_v<resource::traits::get_alias_t<res_A>, res_A>, "failed default resource alias");
	static_assert(std::is_same_v<resource::traits::get_lockset_t<res_A>, std::tuple<res_A>>, "failed default resource lockset");
	static_assert(resource::traits::get_inst_count_v<res_A> == 1, "failed default resource instance count");
	static_assert(resource::traits::get_lock_level_v<res_A> == 0, "failed default resource lock level");

	static_assert(std::is_same_v<resource::traits::get_type_t<res_B>, int>, "failed assigned for resource type");
	static_assert(std::is_same_v<resource::traits::get_lockset_t<res_B>, std::tuple<int, float>>, "failed assigned resource lockset");
	static_assert(resource::traits::get_lock_level_v<res_B> == 1, "failed assigned resource lock level");
	static_assert(resource::traits::get_inst_count_v<res_B> == 2, "failed assigned resource instance count");

	static_assert(std::is_same_v<resource::traits::get_type_t<res_C>, int>, "failed alias pass through for resource type");
	static_assert(resource::traits::get_lock_level_v<res_C> == 1, "failed alias pass through resource lock level");
	static_assert(resource::traits::get_inst_count_v<res_C> == 2, "failed alias pass through resource instance count");
	static_assert(std::is_same_v<resource::traits::get_lockset_t<res_C>, std::tuple<res_C, int, float>>, "failed self inclusive resource lockset");
}
//#endif