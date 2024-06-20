
#pragma once
#include "util/tuple_util.h"
#include "util/type_name.h"
#include "view_arguments.h"
#include "view.h"
#include "traits.h"
#include "resource.h"
#include "component.h"

/*
 TODO: add volatile which wont lock resource
 ? wont require type to be resource

*/
namespace ecs::traits 
{
	template<typename T, typename Pip_T> 
	struct is_accessible_resource : util::allof<
		util::propergate_const_each_t<T, ecs::traits::get_resource_set>,
			util::element_of_<ecs::traits::get_resource_set_t<Pip_T>, 
				util::cmp::disjunction_<std::is_same,
					util::cmp::transformed_rhs_<std::is_same, std::add_const>::template type
				>::template type
		>::template type> { };
	
	template<typename T, typename Pip_T> 
	static constexpr bool is_accessible_resource_v = is_accessible_resource<T, Pip_T>::value; 
	
	template<typename T, typename Pip_T> 
	concept accessible_resource_class = is_accessible_resource<T, Pip_T>::value;
	
	template<typename ... Ts>
	struct pipeline_builder;
}

namespace ecs {
	template<typename ... Ts> // resource_class
	using pipeline = typename traits::pipeline_builder<Ts...>::type;


	template<ecs::traits::resource_key_class ... Ts>
	struct pipeline_t
	{
	public:
		// the set of resources locked by this pipeline
		using resource_set = std::tuple<Ts...>;
		template<typename T>
		using pool_set_t = util::filter_t<resource_set, util::pred::transformed_<
				util::compare_to_<T, std::is_same>::template type, 
				util::unwrap>::template type>;

		template<typename T>
		using get_resource_t = util::find_t<resource_set, util::pred::disjunction_<
			util::compare_to_<T, std::is_same>::template type, // const, const || mut, mut
			util::compare_to_<T, std::is_same, std::add_const>::template type // const, const || const, mut
			>::template type>;
				
		template<typename T> using const_iterator = handler<T>::const_iterator;
		template<typename T> using iterator = handler<T>::iterator;

		// the set of references to containers required for all resource set
		using data_set_t = std::tuple<ecs::traits::get_resource_type_t<Ts>&...>;


		template<typename base_T>
		pipeline_t(base_T& base);

		// TODO: a pipeline could be copied if it only has const references???
		pipeline_t(pipeline_t<Ts...>& other) = delete;
		pipeline_t& operator=(pipeline_t<Ts...>& other) = delete;
		
		pipeline_t(pipeline_t<Ts...>&& other);
		pipeline_t& operator=(pipeline_t<Ts...>&& other);

		void lock();
		void unlock();
		void sync();
		
		template<traits::accessible_resource_class<pipeline_t<Ts...>> res_T>
		res_T& get_resource();

		// create/destroy entity
		ecs::entity create() requires(traits::is_accessible_resource_v<ecs::entity_manager, pipeline_t<Ts...>>);
		void destroy(ecs::entity e) requires(traits::is_accessible_resource_v<ecs::entity_manager, pipeline_t<Ts...>>);

		template<ecs::traits::component_class comp_T> 
		comp_T& get_component(ecs::entity e) 
			requires(traits::is_accessible_resource_v<std::tuple<storage<comp_T>, indexer<comp_T>>, pipeline_t<Ts...>>);

		template<ecs::traits::component_class comp_T,
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>,
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>,
			typename ... Arg_Ts>
		void emplace(const_iterator<comp_T> pos, ecs::entity e, Arg_Ts&& ... args);

		template<ecs::traits::component_class comp_T,
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>,
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>,
			std::input_iterator InputIt_T, typename ... Arg_Ts>
		iterator<comp_T> emplace(const_iterator<comp_T> pos, InputIt_T first, InputIt_T last, Arg_Ts&& ... args);

		template<ecs::traits::component_class comp_T, 
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>,
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>,
			typename ... Arg_Ts>
		comp_T& emplace_back(ecs::entity e, Arg_Ts&& ... args);

		template<ecs::traits::component_class comp_T, 
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>,
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>,
			std::input_iterator InputIt_T, typename ... Arg_Ts>
		void emplace_back(InputIt_T first, InputIt_T last, Arg_Ts&& ... args);

		template<ecs::traits::component_class comp_T,
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>,
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>> 
		void erase(ecs::entity e);

		template<ecs::traits::component_class comp_T, 
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>, 
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>, 
			std::input_iterator InputIt_T> 
		void erase(InputIt_T first, InputIt_T last);

		template<typename ... select_Us, 
			typename from_T = ecs::traits::view_from_builder<ecs::select<select_Us...>>::type, 
			typename where_T = ecs::traits::view_where_builder<ecs::select<select_Us...>, from_T>::type>
			requires (traits::is_accessible_resource_v<std::tuple<ecs::select<select_Us...>, from_T, where_T>, pipeline_t<Ts...>>)
		view<select<select_Us...>, from_T, where_T, pipeline_t<Ts...>&> view(from_T from = {}, where_T where = {});
		
	private:
		data_set_t set;
	};
}

template<typename ... Ts>
struct ecs::traits::pipeline_builder
{
	using type = util::eval_t<std::tuple<Ts...>, 
		util::eval_each_<
			util::propergate_const_each_< 
				util::eval_<
					std::remove_const,
					util::eval_if_<is_component, get_component_resource_set>::template type,
					get_resource_set
				>::template type 
			>::template type // end transform post conditional
		>::template type, // end transform each
		util::concat, // concat resource sets together
		util::sort_<util::cmp::greater_<std::is_const>::template type>::template type, // put const first const
		util::unique_<util::cmp::is_ignore_const_same>::template type,
		util::sort_<util::cmp::priority_list_<
			util::cmp::less_<get_lock_priority>::template type,
			util::cmp::less_<util::get_type_ID>::template type
		>::template type>::template type,
		util::rewrap_<ecs::pipeline_t>::template type
	>;
};

template<typename select_T, typename from_T, typename where_T>
struct ecs::traits::view_pipeline_builder 
{
	using type = pipeline<select_T, from_T, where_T>;
};

template<ecs::traits::resource_key_class ... Ts>
template<typename base_T> 
ecs::pipeline_t<Ts...>::pipeline_t(base_T& base)
	: set(base.template get_resource<Ts>()...)
{ }

template<ecs::traits::resource_key_class ... Ts>
ecs::pipeline_t<Ts...>::pipeline_t(pipeline_t<Ts...>&& other)
	 : set(other.set) 
{ }

template<ecs::traits::resource_key_class ... Ts>
ecs::pipeline_t<Ts...>& 
ecs::pipeline_t<Ts...>::operator=(pipeline_t<Ts...>&& other) 
{ set = other.set; return *this; }

template<ecs::traits::resource_key_class ... Ts>
void ecs::pipeline_t<Ts...>::lock() {
	(get_resource<Ts>().acquire(), ...);
}

template<ecs::traits::resource_key_class ... Ts>
void ecs::pipeline_t<Ts...>::unlock() {
	sync();
	(get_resource<Ts>().release(), ...);
}

// sync()

template<ecs::traits::resource_key_class ... Ts>
template<ecs::traits::accessible_resource_class<ecs::pipeline_t<Ts...>> res_T>
res_T& ecs::pipeline_t<Ts...>::get_resource() {
	return std::get<std::remove_const_t<res_T>&>(set);
}

template<ecs::traits::resource_key_class ... Ts>
ecs::entity ecs::pipeline_t<Ts...>::create() requires(traits::is_accessible_resource_v<ecs::entity_manager, pipeline_t<Ts...>>) {
	return get_resource<entity_manager>().create();
}

template<ecs::traits::resource_key_class ... Ts>
void ecs::pipeline_t<Ts...>::destroy(ecs::entity e) requires(traits::is_accessible_resource_v<ecs::entity_manager, pipeline_t<Ts...>>) {
	get_resource<entity_manager>().destroy(e);
}

// emplace/erase/...

template<ecs::traits::resource_key_class ... Ts>
template<typename ... Us, typename from_T, typename where_T>
	requires (ecs::traits::is_accessible_resource_v<std::tuple<ecs::select<Us...>, from_T, where_T>, ecs::pipeline_t<Ts...>>)
ecs::view<ecs::select<Us...>, from_T, where_T, ecs::pipeline_t<Ts...>&> 
ecs::pipeline_t<Ts...>::view(from_T, where_T) {
	return *this;
}