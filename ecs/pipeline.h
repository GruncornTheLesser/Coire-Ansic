#include "view.h"
#pragma once
#include "util/tuple_util.h"
#include "util/type_name.h"
#include "traits.h"
#include "pool_traits.h"

namespace ecs::traits {
	template<typename T, typename Pip_T> struct is_accessible_resource;
	template<typename T, typename Pip_T> static constexpr bool is_accessible_resource_v = is_accessible_resource<T, Pip_T>::value; 
	template<typename T, typename Pip_T> concept accessible_resource_class = is_accessible_resource<T, Pip_T>::value;
	template<typename T, typename Pip_T> struct is_accessible_resource : util::allof<
		util::propergate_const_each_t<T, ecs::traits::get_resource_set>,
			util::element_of_<ecs::traits::get_resource_set_t<Pip_T>, 
				util::cmp::disjunction_<std::is_same,
					util::cmp::transformed_rhs_<std::is_same, std::add_const>::template type
				>::template type
		>::template type> { };
}

namespace ecs {
	template<ecs::traits::resource_key_class ... Ts>
	struct pipeline_t 
	{
		// the set of resources locked by this pipeline
		using resource_set = std::tuple<Ts...>;
		template<typename T>
		using pool_set_t = util::filter_t<resource_set, util::match::transformed_<
				util::compare_to_<T, std::is_same>::template type, 
				util::unwrap>::template type>;
		template<typename T>
		using get_resource_t = util::find_t<resource_set, util::match::disjunction_<
			util::compare_to_<T, std::is_same>::template type, // const, const || mut, mut
			util::compare_to_<T, std::is_same, std::add_const>::template type // cpnst, const || const, mut
			>::template type>;
		
		template<typename T> using const_iterator = handler<T>::const_iterator;
		template<typename T> using iterator = handler<T>::iterator;

		// the set of pointers to containers required for all resource set
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

		template<ecs::traits::component_class comp_T,
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>,
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>,
			typename ... Arg_Ts>
		void emplace(const_iterator<comp_T> pos, ecs::entity e, Arg_Ts&& ... args) {
			// get handler
			// exec sequence->emplace
			// queue sync by execution_policy
		}

		template<ecs::traits::component_class comp_T,
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>,
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>,
			std::input_iterator InputIt_T, typename ... Arg_Ts>
		iterator<comp_T> emplace(const_iterator<comp_T> pos, InputIt_T first, InputIt_T last, Arg_Ts&& ... args);

		template<ecs::traits::component_class comp_T, 
			sequence_policy seq = ecs::traits::get_sequence_policy_v<comp_T>,
			execution_policy exec = ecs::traits::get_execution_policy_v<comp_T>,
			typename ... Arg_Ts>
		comp_T& emplace_back(ecs::entity ents, Arg_Ts&& ... args);

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
			typename from_T = ecs::view_from_builder<ecs::select<select_Us...>>::type, 
			typename where_T = ecs::view_where_builder<ecs::select<select_Us...>, from_T>::type>
			requires (traits::is_accessible_resource_v<std::tuple<ecs::select<select_Us...>, from_T, where_T>, pipeline_t<Ts...>>)
		view<select<select_Us...>, from_T, where_T, pipeline_t<Ts...>&> view(from_T from = {}, where_T where = {});
		
		template<traits::accessible_resource_class<pipeline_t<Ts...>> res_T>
		res_T& get_resource();
	private:
		data_set_t set;
	};

	// gathers and manipulates the set and order of required resources, to maintain consistent locking order and prevent deadlocks
	template<typename ... Ts>
	struct pipeline_builder;

	template<typename ... Ts> // resource_class
	using pipeline = typename pipeline_builder<Ts...>::type;
}

template<typename ... Ts>
struct ecs::pipeline_builder
{
	using type = util::eval_t<std::tuple<Ts...>, 
		util::eval_each_< // for each T
			util::propergate_const_each_< // if const, add const to each element in tuple
				util::eval_<
					std::remove_const,
					util::conditional_<ecs::traits::is_component, // // if component, ie not resource 
						ecs::traits::get_pool_resource_set>::template type, // get poolset -> indexer<T>, handler<T>, storage<T>
					ecs::traits::get_resource_set
				>::template type 
			>::template type // end transform post conditional
		>::template type, // end transform each
		util::concat, // concat resource sets together
		util::sort_<
			util::cmp::priority_list_<
				util::cmp::less_<std::is_const>::template type,
				util::cmp::less_<traits::get_lock_priority>::template type,
				util::cmp::less_<util::get_type_name>::template type
			>::template type
		>::template type,
		util::unique_<util::cmp::transformed_<std::is_same, std::remove_const>::template type>::template type, // remove duplicates
		util::rewrap_<ecs::pipeline_t>::template type
	>;
};

//static_assert(std::is_same_v<ecs::pipeline<int>, ecs::pipeline<ecs::pool<int>>>, "pipeline test failed: component to pool conversion");

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

template<ecs::traits::resource_key_class ... Ts>
template<ecs::traits::accessible_resource_class<ecs::pipeline_t<Ts...>> res_T>
res_T& ecs::pipeline_t<Ts...>::get_resource() {
	return std::get<std::remove_cvref_t<res_T>&>(set);
}

template<ecs::traits::resource_key_class ... Ts>
template<typename ... Us, typename from_T, typename where_T>
	requires (ecs::traits::is_accessible_resource_v<std::tuple<ecs::select<Us...>, from_T, where_T>, ecs::pipeline_t<Ts...>>)
ecs::view<ecs::select<Us...>, from_T, where_T, ecs::pipeline_t<Ts...>&> 
ecs::pipeline_t<Ts...>::view(from_T, where_T) {
	return *this;
}