#ifndef ECS_VIEW_TPP
#define ECS_VIEW_TPP
#include <type_traits>
#include <tuple>
#include "util/tuple_util.h"
#include "traits.h"
#include "pool.h"
#include "pipeline.h"

#pragma region decorators

template<typename ... Ts>
struct ecs::select {
	using resource_set = util::trn::each_t<util::trn::filter_t<std::tuple<Ts...>,
		util::mtc::build::compare_to<ecs::entity, util::cmp::build::transformed<std::is_same, std::remove_const>::template type>::template negated>, 
		util::trn::build::propergate_const<ecs::traits::get_pool_component_storage>::template type>;

	// for each type in filtered for set of not empty
	using retrieve_set = util::trn::each_t<util::trn::filter_t<std::tuple<Ts...>,
			util::mtc::build::negation<std::is_empty>::type>, 
			util::trn::build::conditional<
				util::mtc::build::compare_to<
					ecs::entity, 
					util::cmp::build::transformed<std::is_same, std::remove_cvref>::type
				>::negated, // not equal to entity
				std::add_lvalue_reference 
			>::type>;
};
template<typename T>
struct ecs::from {
	using pool = traits::get_pool_t<T>;
	using resource_set = std::tuple<const typename traits::get_pool_t<T>::entity>;
};
template<typename ... Ts>
struct ecs::where {
	using resource_set = util::trn::concat_t<util::trn::each_t<std::tuple<Ts...>, traits::get_resource_set>>;

	template<typename pip_T>
	static bool check(pip_T& pip, ecs::entity ent) {
		return (Ts::check(pip, ent) && ...);
	}
};

template<typename ... Ts>
struct ecs::include {
	using resource_set = std::tuple<typename pool<Ts>::index...>;

	template<typename pip_T>
	static bool check(pip_T& pip, ecs::entity ent) {
		return (pip.template get_resource<typename pool<Ts>::index>().contains(ent) && ...);
	}
};
template<typename ... Ts>
struct ecs::exclude {
	using resource_set = std::tuple<typename pool<Ts>::index...>;

	template<typename pip_T>
	static bool check(pip_T& pip, ecs::entity ent) {
		return !(pip.template get_resource<pool<Ts>::index>().contains(ent) || ...);
	}
};


#pragma endregion

#pragma region builders

template<typename select_T>
struct ecs::view_from_builder { 
	using type = ecs::from<std::tuple_element_t<0, 
		util::trn::filter_t<util::trn::rewrap_t<select_T, std::tuple>, 
		util::mtc::build::compare_to<ecs::entity, std::is_same>::negated>>>;
};
template<typename select_T, typename from_T>
struct ecs::view_where_builder<select_T, ecs::from<from_T>> {
	using type = ecs::where<util::trn::filter_t<util::trn::rewrap_t<select_T, ecs::include>, util::mtc::build::disjunction<
		util::mtc::build::compare_to<from_T, util::cmp::build::transformed<std::is_same, std::remove_const>::template type>::template type,
		util::mtc::build::compare_to<ecs::entity, util::cmp::build::transformed<std::is_same, std::remove_const>::template type>::template type
		>::template negated>>;
};
template<typename ... select_Ts, typename from_T, typename ... where_Ts>
struct ecs::view_pipeline_builder<ecs::select<select_Ts...>, ecs::from<from_T>, ecs::where<where_Ts...>> {
	using type = pipeline<select<select_Ts...>, from<from_T>, where<where_Ts...>>;
};

#pragma endregion 

#pragma region view

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::view(pip_T&& base) : pip(std::forward<pip_T>(base)) { }

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::reverse_iterator 
ecs::view<select_T, from_T, where_T, pip_T>::begin() {
	return std::reverse_iterator { rend() };
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::reverse_iterator 
ecs::view<select_T, from_T, where_T, pip_T>::end() {
	return std::reverse_iterator { rbegin() };
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::const_reverse_iterator 
ecs::view<select_T, from_T, where_T, pip_T>::begin() const {
	return ++std::reverse_iterator { rend() };
}


template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::const_reverse_iterator 
ecs::view<select_T, from_T, where_T, pip_T>::end() const {
	return std::reverse_iterator { rbegin() };
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::iterator 
ecs::view<select_T, from_T, where_T, pip_T>::rbegin() {
	return ++iterator { pip, -1 };
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::const_iterator 
ecs::view<select_T, from_T, where_T, pip_T>::rbegin() const {
	return ++const_iterator { pip, -1 };
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::iterator 
ecs::view<select_T, from_T, where_T, pip_T>::rend() {
	return iterator{ pip, static_cast<int>(pip.template get_resource<const typename from_T::pool::entity>().size) };
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view<select_T, from_T, where_T, pip_T>::const_iterator 
ecs::view<select_T, from_T, where_T, pip_T>::rend() const {
	return const_iterator{ pip, static_cast<int>(pip.template get_resource<const typename from_T::pool::entity>().size) };
}
#pragma endregion

#pragma region view iterator 
template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view_iterator<select_T, from_T, where_T, pip_T>::view_iterator(pip_T& pip, int index) : pip(pip), ind(index)
{ }

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view_iterator<select_T, from_T, where_T, pip_T>::reference
ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator*() { 
	return reference { pip, static_cast<size_t>(ind) };
}
template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view_iterator<select_T, from_T, where_T, pip_T>&
ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator++() {
	// hehe hoo hoo I lika one line
	while (++ind < pip.template get_resource<const typename from_T::pool::entity>().size && 
		!where_T::check(pip, ent = pip.template get_resource<const typename from_T::pool::entity>()[static_cast<size_t>(ind)].first));
	return *this;
}


template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view_iterator<select_T, from_T, where_T, pip_T>
ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator++(int) {
	auto temp = *this;
	++(*this);
	return temp;
}


template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view_iterator<select_T, from_T, where_T, pip_T>&
ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator--() {
	while (--ind > 0 && !where_T::check(pip, ent = pip.template get_resource<const typename from_T::pool::entity>()[static_cast<size_t>(ind)].first));
	return *this;
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ecs::view_iterator<select_T, from_T, where_T, pip_T>
ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator--(int) { 
	auto temp = *this;
	--(*this);
	return temp;
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
ptrdiff_t ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator-(const view_iterator& other) {
	return ind - other.ind;
}

template<typename select_T, typename from_T, typename where_T, typename pip_T>
bool ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator==(const view_iterator& other) {
	return ind == other.ind;
}


template<typename select_T, typename from_T, typename where_T, typename pip_T>
bool ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator!=(const view_iterator& other) {
	return ind != other.ind;
}


template<typename select_T, typename from_T, typename where_T, typename pip_T>
bool ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator<(const view_iterator& other) {
	return ind < other.ind;
}


template<typename select_T, typename from_T, typename where_T, typename pip_T>
bool ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator>(const view_iterator& other) {
	return ind > other.ind;
}


template<typename select_T, typename from_T, typename where_T, typename pip_T>
bool ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator<=(const view_iterator& other) {
	return ind <= other.ind;
}


template<typename select_T, typename from_T, typename where_T, typename pip_T>
bool ecs::view_iterator<select_T, from_T, where_T, pip_T>::operator>=(const view_iterator& other) {
	return ind >= other.ind;
}

#pragma endregion

#pragma region view reference 

template<typename select_T, typename from_T, typename pip_T>
ecs::view_reference<select_T, from_T, pip_T>::view_reference(pip_T& base, size_t i) : pip(base), ind(i) {
	//if constexpr (util::tuple_anyof_v<util::cmp_to<std::is_same, ecs::entity, std::remove_cvref>::type, select_T>)
	// if has entity selected or more than one comp pool being iterated
	// util::tuple_anyof_v<util::cmp_to<std::is_same, ecs::entity, std::remove_cvref>::type, select_T>
	// util::tuple_size_v<util::tuple_union<util::transform<>>>>
	ent = pip.template get_resource<const typename from_T::pool::entity>()[ind].first;
}

template<typename select_T, typename from_T, typename pip_T>
template<size_t N> 
std::tuple_element_t<N, ecs::view_reference<select_T, from_T, pip_T>> 
ecs::view_reference<select_T, from_T, pip_T>::get() {
	using type = std::remove_reference_t<std::tuple_element_t<N, view_reference>>; 
	
	if constexpr (std::is_same_v<ecs::entity, std::remove_const_t<type>>)
		return ent;
	else
	{
		if constexpr (std::is_same_v<traits::get_pool_t<type>, std::remove_cvref_t<typename from_T::pool>>)
			return pip.template get_resource<util::trn::propergate_const_t<type, ecs::traits::get_pool_component_storage>>()[ind];
		else 
		{
			size_t i = pip.template get_resource<ecs::traits::entity_index_t<type>>()[ent];
			return pip.template get_resource<util::trn::propergate_const_t<type, ecs::traits::get_pool_component_storage>>()[i];
		}
	}
}

template<typename select_T, typename from_T, typename pip_T>
template<size_t N> 
std::tuple_element_t<N, const ecs::view_reference<select_T, from_T, pip_T>> 
ecs::view_reference<select_T, from_T, pip_T>::get() const {
	using type = std::remove_cvref_t<std::tuple_element_t<N, view_reference>>; 
	
	if constexpr (std::is_same_v<ecs::entity, type>)
		return ent;
	else
	{
		using index_lookup_t = typename traits::get_pool_t<type>::index;
		using comp_array_t = traits::get_pool_t<type>::template comp<type>;

		if constexpr (std::is_same_v<traits::get_pool_t<type>, std::remove_cvref_t<typename from_T::pool>>)
			return pip.template get_resource<comp_array_t>()[ind];
		else 
		{
			size_t i = pip.template get_resource<index_lookup_t>()[ent];
			return pip.template get_resource<comp_array_t>()[i];
		}
	}
}

#endif