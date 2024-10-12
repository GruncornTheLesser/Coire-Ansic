#pragma once
#include "util/tuple_util.h"
#include "traits.h"
#include "entity.h"
namespace ecs::traits {
	template<typename T>
	concept select_arg_class = is_component_v<T> || util::cmp::is_ignore_cv_same_v<T, entity>;

	template<typename T>
	concept from_arg_class = is_component_v<T> || is_storage_v<T> || is_manager_v<T>; // or storage or manager

	template<typename T>
	concept where_arg_class = std::is_invocable_v<T>; // is callable, and returns bool
}

namespace ecs {
	template<traits::select_arg_class ... Ts> struct select;
	template<traits::from_arg_class T> struct from;
	template<traits::where_arg_class ... Ts> struct where;
	template<traits::component_class ... Ts> struct include;
	template<traits::component_class ... Ts> struct exclude;
	// just a functor
}

// builders
namespace ecs::traits {
	template<typename select_T>
	struct view_from_builder;

	template<typename select_T>
	using view_from_builder_t = typename view_from_builder<select_T>::type;

	template<typename select_T,
		typename from_T = typename view_from_builder<select_T>::type>
	struct view_where_builder;

	template<typename select_T,
		typename from_T = typename view_from_builder<select_T>::type>
	using view_where_builder_t = typename view_where_builder<select_T, from_T>::type;

	template<typename select_T,
		typename from_T = typename view_from_builder<select_T>::type,
		typename where_T = typename view_where_builder<select_T, from_T>::type>
	struct view_pipeline_builder;

	template<typename select_T,
		typename from_T = typename view_from_builder<select_T>::type,
		typename where_T = typename view_where_builder<select_T, from_T>::type>
	using view_pipeline_builder_t = typename view_pipeline_builder<select_T, from_T, where_T>::type;
}

namespace ecs::traits {
	template<typename T, typename from_t>
	struct is_indexable;

	template<typename T, typename from_T>
	static constexpr bool is_indexable_v = is_indexable<T, from_T>::value;

	template<typename select_T>
	struct is_storage_iterable;

	template<typename select_T>
	static constexpr bool is_storage_iterable_v = is_storage_iterable<select_T>::value;
}


template<ecs::traits::select_arg_class ... Ts>
struct ecs::select
{
public:
	using resource_lockset = util::eval_t<std::tuple<Ts...>,
		util::filter_<util::pred::disjunction_<std::is_empty, traits::is_entity>::template negated>::template type,
		util::eval_each_<util::propagate_const_<util::wrap_<storage>::template type>::template type>::template type,
		util::unique_priority_<util::cmp::is_ignore_cv_same, util::cmp::less_<std::is_const>::template type>::template type>;

	// the set of values that view_reference evaluates to
	using retrieve_set = util::eval_t<std::tuple<Ts...>,
		util::filter_<util::pred::negate_<std::is_empty>::template type>::template type, // remove empty types
		util::eval_each_<util::eval_if_<util::compare_to_<ecs::entity, util::cmp::is_ignore_cv_same>::template type,
			std::remove_cv, std::add_lvalue_reference>::template type>::template type>;
};

template<ecs::traits::from_arg_class T>
struct ecs::from
{
	using resource_alias = const util::eval_if_t<T, traits::is_component, util::wrap_<ecs::manager>::type>;
};

template<ecs::traits::where_arg_class ... Ts>
struct ecs::where
{
	using resource_lockset = std::tuple<Ts...>;

	template<typename pip_T>
	static bool valid(pip_T& pip, ecs::entity ent) { return (Ts::valid(pip, ent) && ...); }
};

template<ecs::traits::component_class ... Ts>
struct ecs::include
{
	using resource_lockset = std::tuple<const indexer<Ts>>...>;

	template<typename base_T>
	static bool valid(base_T& base, ecs::entity e) {
		return (base.template get_resource<const indexer<Ts>>>().contains(e) && ...);
	}
};

template<ecs::traits::component_class ... Ts>
struct ecs::exclude
{
	using resource_lockset = std::tuple<const indexer<Ts>>...>;

	template<typename base_T>
	static bool valid(base_T& base, ecs::entity e) {
		return !(base.template get_resource<const indexer<Ts>>().contains(e) || ...);
	}
};

template<typename select_T>
struct ecs::traits::view_from_builder
{
	using type = util::wrap_t<util::post_eval_if_t<select_T,
		util::find_first_<is_component>::template type,
		is_storage_iterable, // select_T is storage iterable
		util::wrap_<ecs::storage>::template type,
		util::wrap_<ecs::manager>::template type>, ecs::from>;
};

template<typename select_T, typename from_T>
struct ecs::traits::view_where_builder
{
	using type = std::conditional_t<util::is_wrapped_by_v<from_T, ecs::storage_t>, ecs::where<>, util::eval_t<select_T,
		util::filter_<util::pred::disjunction_<is_entity, util::add_arg_<is_indexable, from_T>::template type>::template negated>::template type,
		util::rewrap_<ecs::include>::template type>>;
};


template<typename T, typename from_t>
struct ecs::traits::is_indexable
 : std::disjunction<std::is_same<typename from_t::type, const manager<T>>, std::is_same<typename from_t::type, const storage<T>>> { };

template<typename select_T>
struct ecs::traits::is_storage_iterable
 : std::conjunction<std::negation<util::anyof<select_T, is_entity>>, util::allof<typename select_T::resource_lockset,
	util::compare_to_<util::get_front_t<typename select_T::resource_lockset>, util::cmp::is_ignore_cv_same>::template type>> { };
