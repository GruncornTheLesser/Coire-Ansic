#include "view.h" 
// pipeline requires view.h but view.tpp requires pipeline.h
// this is probably bad

#pragma once
#include "util/tuple_util.h"
#include "util/type_name.h"
#include "traits.h"

namespace ecs {
	template<ecs::traits::resource_key_class ... Ts>
	struct pipeline_t 
	{
		// the set of resources locked by this pipeline
		using resource_set = std::tuple<Ts...>;
		// the set of pointers to containers required for all resource set
		using data_set = std::tuple<ecs::traits::get_resource_type_t<Ts>&...>;

		template<typename base_T>
		pipeline_t(base_T& base);

		pipeline_t(pipeline_t<Ts...>& other) = delete;
		pipeline_t(pipeline_t<Ts...>&& other);

		pipeline_t& operator=(pipeline_t<Ts...>& other) = delete;
		pipeline_t& operator=(pipeline_t<Ts...>&& other);

		void lock();
		void unlock();
		void sync();

		template<typename T>
		using const_iterator = uint32_t; // TODO: set up pipeline const iterators -> probably make this a template arg for handler<T>::iterator

		template<ecs::traits::component_class T,
			ecs::traits::sequence_policy_class seq_T = ecs::traits::get_sequence_policy_t<T>,
			ecs::traits::order_policy_class ord_T = ecs::traits::get_order_policy_t<T>,
			typename ... Arg_Ts> 
		//requires (is_accessible_resource_v<template ord_T::emplacer<T>, pipeline_t<Ts...>> && is_accessible_resource_v<Seq_T, pipeline_t<Ts...>>)
		void emplace(const_iterator<T> pos, ecs::entity e, Arg_Ts&& ... args);

		template<ecs::traits::component_class T,
			ecs::traits::sequence_policy_class seq_T = ecs::traits::get_sequence_policy_t<T>,
			ecs::traits::order_policy_class ord_T = ecs::traits::get_order_policy_t<T>,
			std::input_iterator InputIt_T, typename ... Arg_Ts>
		void emplace(const_iterator<T> pos, InputIt_T first, InputIt_T last, Arg_Ts&& ... args);

		template<ecs::traits::component_class T, 
			ecs::traits::sequence_policy_class seq_T = ecs::traits::get_sequence_policy_t<T>,
			ecs::traits::order_policy_class ord_T = ecs::traits::get_order_policy_t<T>,
			typename ... Arg_Ts>
		void emplace_back(ecs::entity ents, Arg_Ts&& ... args);

		template<ecs::traits::component_class T, 
			ecs::traits::sequence_policy_class seq_T = ecs::traits::get_sequence_policy_t<T>,
			ecs::traits::order_policy_class ord_T = ecs::traits::get_order_policy_t<T>,
			std::input_iterator InputIt_T, typename ... Arg_Ts>
		void emplace_back(InputIt_T first, InputIt_T last, Arg_Ts&& ... args);

		template<ecs::traits::component_class T,
			ecs::traits::sequence_policy_class seq_T = ecs::traits::get_sequence_policy_t<T>,
			ecs::traits::order_policy_class ord_T = ecs::traits::get_order_policy_t<T>> 
		void erase(ecs::entity e);

		template<ecs::traits::component_class T, 
			ecs::traits::sequence_policy_class seq_T = ecs::traits::get_sequence_policy_t<T>, 
			ecs::traits::order_policy_class ord_T = ecs::traits::get_order_policy_t<T>, 
			std::input_iterator InputIt_T> 
		void erase(InputIt_T first, InputIt_T last);

		template<typename ... select_Us, 
			typename from_T = ecs::view_from_builder<ecs::select<select_Us...>>::type, 
			typename where_T = ecs::view_where_builder<ecs::select<select_Us...>, from_T>::type>
			requires (traits::is_accessible_resource_v<std::tuple<ecs::select<select_Us...>, from_T, where_T>, pipeline_t<Ts...>>)
		view<select<select_Us...>, from_T, where_T, pipeline_t<Ts...>&> view(from_T from = {}, where_T where = {});
		
		template<traits::accessible_resource_class<pipeline_t<Ts...>> U>
		U& get_resource();
	private:
		data_set set;
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
	using type = util::set_t<std::tuple<Ts...>, 
		util::build::each< // for each T
			util::build::propergate_const_each< // if const, add const to each element in tuple
				util::build::set<
					std::remove_const,
					util::build::conditional<ecs::traits::is_component, ecs::traits::get_pool_resource_set>::template type,// if component, ie not resource get poolset -> indexer<T>, handler<T>, storage<T>
					ecs::traits::get_resource_set
				>::template type 
			>::template type // end transform post conditional
		>::template type, // end transform each
		util::build::concat::template type, // concat resource sets together
		util::build::sort<util::alpha_lt>::template type, // sort types by name
		util::build::sort<util::cmp::build::prioritize_if<std::is_const>::template negated>::template type, // sort type by const
		util::build::unique<util::cmp::build::transformed<std::is_same, std::remove_const>::template type>::template type, // remove duplicates
		util::build::rewrap<ecs::pipeline_t>::template type
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
template<ecs::traits::component_class T,
	ecs::traits::sequence_policy_class seq_T,
	ecs::traits::order_policy_class ord_T,
	std::input_iterator InputIt_T, typename ... Arg_Ts>
void ecs::pipeline_t<Ts...>::emplace(const_iterator<T> pos, InputIt_T first, InputIt_T last, Arg_Ts&& ... args) {

}

template<ecs::traits::resource_key_class ... Ts>
template<ecs::traits::component_class T, 
	ecs::traits::sequence_policy_class seq_T,
	ecs::traits::order_policy_class ord_T,
	typename ... Arg_Ts>
void ecs::pipeline_t<Ts...>::emplace_back(ecs::entity ents, Arg_Ts&& ... args) {

}

template<ecs::traits::resource_key_class ... Ts>
template<ecs::traits::component_class T, 
	ecs::traits::sequence_policy_class seq_T,
	ecs::traits::order_policy_class ord_T,
	std::input_iterator InputIt_T, typename ... Arg_Ts>
void ecs::pipeline_t<Ts...>::emplace_back(InputIt_T first, InputIt_T last, Arg_Ts&& ... args) {
	
}

template<ecs::traits::resource_key_class ... Ts>
template<ecs::traits::component_class T,
			ecs::traits::sequence_policy_class seq_T,
			ecs::traits::order_policy_class ord_T>
void ecs::pipeline_t<Ts...>::erase(ecs::entity e) { }

template<ecs::traits::resource_key_class ... Ts>
template<ecs::traits::component_class T, 
	ecs::traits::sequence_policy_class seq_T, 
	ecs::traits::order_policy_class ord_T, 
	std::input_iterator InputIt_T> 
void ecs::pipeline_t<Ts...>::erase(InputIt_T first, InputIt_T last) { }

template<ecs::traits::resource_key_class ... Ts>
template<ecs::traits::accessible_resource_class<ecs::pipeline_t<Ts...>> U>
U& ecs::pipeline_t<Ts...>::get_resource() {
		return *std::get<U*>(set);
}

template<ecs::traits::resource_key_class ... Ts>
template<typename ... Us, typename from_T, typename where_T>
	requires (ecs::traits::is_accessible_resource_v<std::tuple<ecs::select<Us...>, from_T, where_T>, ecs::pipeline_t<Ts...>>)
ecs::view<ecs::select<Us...>, from_T, where_T, ecs::pipeline_t<Ts...>&> 
ecs::pipeline_t<Ts...>::view(from_T, where_T) {
	return *this;
}