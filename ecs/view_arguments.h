#pragma once
#include "util/tuple_util.h"
#include "handler.h"
#include "resource.h"
#include <tuple>
#include "entity.h"
#include "resource.h"
#include "util/tuple_util.h"

namespace ecs::traits {
	template<typename T>
	concept select_arg_class = ecs::traits::is_component_v<T> || is_entity_v<T>;

	template<typename T>
	concept from_arg_class = ecs::traits::is_component_v<T> || util::is_wrapped_by_v<T, ecs::storage_t> || util::is_wrapped_by_v<T, ecs::handler_t>;

	template<typename T>
	concept where_arg_class = true;
}

namespace ecs {
	template<traits::select_arg_class ... Ts> 
	struct select;

	template<traits::from_arg_class T>
	struct from;

	template<traits::where_arg_class ... Ts> 
	struct where;

	template<traits::component_class ... Ts> 
	struct include;

	template<traits::component_class ... Ts> 
	struct exclude;
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
	using resource_set = util::eval_t<std::tuple<Ts...>, 
		util::filter_<util::pred::disjunction_<std::is_empty, traits::is_entity>::template negated>::template type,
		util::eval_each_<util::propergate_const_<util::wrap_<storage>::template type>::template type>::template type, 
		util::sort_<util::cmp::less_<std::is_const>::type>::type, 
		util::unique_<util::cmp::is_ignore_cvref_same>::type>;

	// the set of values that view_reference evaluates to
	using retrieve_set = util::eval_t<std::tuple<Ts...>, 
		util::filter_<util::pred::negate_<std::is_empty>::template type>::template type, // remove empty types
		util::eval_each_<util::eval_if_<util::compare_to_<ecs::entity, util::cmp::is_ignore_cvref_same>::template type, 
			std::remove_cvref, std::add_lvalue_reference>::template type>::template type>;
};

template<ecs::traits::from_arg_class T> 
struct ecs::from 
{ 
	using type = const util::eval_if_t<T, util::pred::disjunction_<
			util::is_wrapped_by_<ecs::storage_t>::template type, 
			util::is_wrapped_by_<ecs::handler_t>::template type
		>::template negated, util::wrap_<ecs::handler>::type>;
	using resource_set = std::tuple<type>;
};

template<ecs::traits::where_arg_class ... Ts>
struct ecs::where 
{
	using resource_set = util::concat_t<util::eval_each_t<std::tuple<Ts...>, traits::get_resource_set>>;
	// TODO: add retrieve set, pass each to check func -> could change to operator() func check more explicit
	


	template<typename pip_T>
	static bool check(pip_T& pip, ecs::entity ent) { return (Ts::check(pip, ent) && ...); }
};

template<ecs::traits::component_class ... Ts>
struct ecs::include 
{
	using resource_set = std::tuple<const indexer<std::remove_const_t<Ts>>...>;

	template<typename pip_T>
	static bool check(pip_T& pip, ecs::entity ent) { 
		return (pip.template get_resource<const indexer<std::remove_const_t<Ts>>>().contains(ent) && ...); 
	}
};

template<ecs::traits::component_class ... Ts>
struct ecs::exclude 
{
	using resource_set = std::tuple<const indexer<std::remove_const_t<Ts>>...>;

	template<typename pip_T>
	static bool check(pip_T& pip, ecs::entity ent) { return !(pip.template get_resource<const indexer<std::remove_const_t<Ts>>>().contains(ent) || ...); }
};

template<typename select_T>
struct ecs::traits::view_from_builder 
{ 
	using type = util::wrap_t<util::post_eval_if_t<select_T, 
		util::find_<is_component>::template type,
		is_storage_iterable, // select_T is storage iterable
		util::wrap_<ecs::storage>::template type, 
		util::wrap_<ecs::handler>::template type>, ecs::from>;
};

template<typename select_T, typename from_T>
struct ecs::traits::view_where_builder 
{
	using type = std::conditional_t<util::is_wrapped_by_v<from_T, ecs::storage_t>, ecs::where<>, util::eval_t<select_T, 
		util::filter_<util::pred::disjunction_<is_entity, util::add_type_args_<is_indexable, from_T>::template type>::template negated>::template type, 
		util::rewrap_<ecs::include>::template type>>;
};


template<typename T, typename from_t>
struct ecs::traits::is_indexable
 : std::disjunction<std::is_same<typename from_t::type, const handler<T>>, std::is_same<typename from_t::type, const storage<T>>> { };

template<typename select_T>
struct ecs::traits::is_storage_iterable
 : std::conjunction<std::negation<util::anyof<select_T, is_entity>>, util::allof<typename select_T::resource_set, 
	util::compare_to_<util::get_front_t<typename select_T::resource_set>, util::cmp::is_ignore_cvref_same>::template type>> { };
