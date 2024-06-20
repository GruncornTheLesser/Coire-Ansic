#pragma once
#include <tuple>
#include <type_traits>
#include <tuple>
#include "util/tuple_util.h"
#include "traits.h"
#include "component.h"
#include "entity.h"
#include "view_arguments.h"

namespace ecs 
{
	template<typename Select_T, 
		typename From_T = typename traits::view_from_builder<Select_T>::type, 
		typename where_T = typename traits::view_where_builder<Select_T, From_T>::type,
		typename Pip_T = typename traits::view_pipeline_builder<Select_T, From_T, where_T>::type>
	class view;

	template<typename Select_T, typename From_T, typename Pip_T>
	class view_reference;
	
	template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
	class view_iterator;
	
	template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
	class view
	{
	public:
		using resource_set = std::tuple<Select_T, From_T, where_T>;

		using iterator = view_iterator<Select_T, From_T, where_T, Pip_T>;
		using const_iterator = view_iterator<util::eval_each_t<Select_T, std::add_const>, From_T, where_T, Pip_T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		view(Pip_T&& base);

		reverse_iterator begin();
		reverse_iterator end();

		const_reverse_iterator begin() const;
		const_reverse_iterator end() const;
		
		iterator rbegin();
		iterator rend();

		const_iterator rbegin() const;
		const_iterator rend() const;
	private:
		Pip_T pip;
	};

	template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
	class view_iterator 
	{
	public:
		
		using iterator_category = std::random_access_iterator_tag;
		using value_type = view_reference<Select_T, From_T, Pip_T>;
		using reference = view_reference<Select_T, From_T, Pip_T>;
		using const_reference = view_reference<util::eval_each_t<Select_T, std::add_const>, From_T, Pip_T>;
		using difference_type = std::ptrdiff_t;

		view_iterator(Pip_T& pip, int index);

		reference operator*();

		view_iterator& operator++();
		view_iterator operator++(int);
		view_iterator& operator--();
		view_iterator operator--(int);
		
		ptrdiff_t operator-(const view_iterator& other);
		
		bool operator==(const view_iterator& other);
		bool operator!=(const view_iterator& other);
		bool operator<(const view_iterator& other);
		bool operator>(const view_iterator& other);
		bool operator<=(const view_iterator& other);
		bool operator>=(const view_iterator& other);

	private:
		bool valid();

		entity ent;
		Pip_T& pip;
		size_t ind;
	};

	template<typename Select_T, typename From_T, typename Pip_T>
	class view_reference 
	{
	public:
		view_reference(Pip_T& pip, size_t index);
		
		template<typename U> 
		operator U()
		{ 
			return get<0>();
		}

		template<size_t N> 
		std::tuple_element_t<N, view_reference> get();
		
		template<size_t N> 
		std::tuple_element_t<N, const view_reference> get() const;
	private:
		ecs::entity ent;
		Pip_T& pip;
		size_t ind;
	};
}

#pragma region view

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::view(Pip_T&& base) : pip(std::forward<Pip_T>(base))
{ }

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::reverse_iterator 
ecs::view<Select_T, From_T, where_T, Pip_T>::begin()
{
	return std::reverse_iterator { rend() };
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::reverse_iterator 
ecs::view<Select_T, From_T, where_T, Pip_T>::end()
{
	return std::reverse_iterator { rbegin() };
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::const_reverse_iterator 
ecs::view<Select_T, From_T, where_T, Pip_T>::begin() const {
	return ++std::reverse_iterator { rend() };
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::const_reverse_iterator 
ecs::view<Select_T, From_T, where_T, Pip_T>::end() const {
	return std::reverse_iterator { rbegin() };
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::iterator 
ecs::view<Select_T, From_T, where_T, Pip_T>::rbegin()
{
	return ++iterator { pip, -1 };
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::const_iterator 
ecs::view<Select_T, From_T, where_T, Pip_T>::rbegin() const {
	return ++const_iterator { pip, -1 };
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::iterator 
ecs::view<Select_T, From_T, where_T, Pip_T>::rend()
{
	return iterator{ pip, static_cast<int>(pip.template get_resource<typename From_T::type>().size()) };
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view<Select_T, From_T, where_T, Pip_T>::const_iterator 
ecs::view<Select_T, From_T, where_T, Pip_T>::rend() const {
	return const_iterator{ pip, static_cast<int>(pip.template get_resource<typename From_T::type>().size()) };
}
#pragma endregion

#pragma region view iterator 
template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::view_iterator(Pip_T& pip, int index) : pip(pip), ind(index)
{ }

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::reference
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator*()
{
	return reference { pip, static_cast<size_t>(ind) };
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>&
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator++()
{
	while (++ind < pip.template get_resource<typename From_T::type>().size() && !valid());
	return *this;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator++(int)
{
	auto temp = *this;
	++(*this);
	return temp;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>&
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator--()
{
	while (--ind > 0 && !valid());
	return *this;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>
ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator--(int)
{ 
	auto temp = *this;
	--(*this);
	return temp;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
ptrdiff_t ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator-(const view_iterator& other)
{
	return ind - other.ind;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
bool ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator==(const view_iterator& other)
{
	return ind == other.ind;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
bool ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator!=(const view_iterator& other)
{
	return ind != other.ind;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
bool ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator<(const view_iterator& other)
{
	return ind < other.ind;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
bool ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator>(const view_iterator& other)
{
	return ind > other.ind;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
bool ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator<=(const view_iterator& other)
{
	return ind <= other.ind;
}

template<typename Select_T, typename From_T, typename where_T, typename Pip_T>
bool ecs::view_iterator<Select_T, From_T, where_T, Pip_T>::operator>=(const view_iterator& other)
{
	return ind >= other.ind;
}

template<typename Select_T, typename From_T, typename Where_T, typename Pip_T>
bool ecs::view_iterator<Select_T, From_T, Where_T, Pip_T>::valid() 
{
	// !where_T::check(pip, ent = pip.template get_resource<typename From_T::type>()[ind])
	return true;
}

#pragma endregion

#pragma region view reference 

template<typename Select_T, typename From_T, typename Pip_T>
ecs::view_reference<Select_T, From_T, Pip_T>::view_reference(Pip_T& pip, size_t i) : pip(pip), ind(i)
{
	if constexpr (util::anyof_v<Select_T, util::pred::disjunction_<
			util::compare_to_<ecs::entity, util::cmp::is_ignore_const_same>::template type,
			util::compare_to_<typename From_T::type, std::is_same, util::wrap_<ecs::handler>::template type>::template type 
		>::template type>)
	{
		ent = pip.template get_resource<typename From_T::type>()[ind];
	}
}

template<typename Select_T, typename From_T, typename Pip_T>
template<size_t N> 
std::tuple_element_t<N, ecs::view_reference<Select_T, From_T, Pip_T>> 
ecs::view_reference<Select_T, From_T, Pip_T>::get()
{
	using T = std::remove_reference_t<std::tuple_element_t<N, view_reference>>;
	using indexer_t = const ecs::indexer<std::remove_const_t<T>>;
	using storage_t = util::propergate_const_wrap_t<T, storage>;

	if constexpr (std::is_same_v<ecs::entity, std::remove_const_t<T>>)
		return ent;
	else
	{
		if constexpr (traits::is_indexable_v<T, From_T>)
			return pip.template get_resource<storage_t>()[ind];
		else
		{
			size_t i = pip.template get_resource<indexer_t>()[ent];
			return pip.template get_resource<storage_t>()[i];
		}
	}
}

template<typename Select_T, typename From_T, typename Pip_T>
template<size_t N> 
std::tuple_element_t<N, const ecs::view_reference<Select_T, From_T, Pip_T>> 
ecs::view_reference<Select_T, From_T, Pip_T>::get() const {
	using T = std::remove_reference_t<std::tuple_element_t<N, view_reference>>;
	using indexer_t = const ecs::indexer<std::remove_const_t<T>>;
	using storage_t = util::propergate_const_wrap_t<T, storage>;
	
	if constexpr (std::is_same_v<ecs::entity, std::remove_const_t<T>>)
		return ent;
	else
	{
		if constexpr (traits::is_indexable_v<T, From_T>)
			return pip.template get_resource<storage_t>()[ind];
		else
		{
			size_t i = pip.template get_resource<indexer_t>()[ent];
			return pip.template get_resource<storage_t>()[i];
		}
	}
}

// view reference structured binding 
template<typename Select_T, typename From_T, typename Pip_T>
struct std::tuple_size<ecs::view_reference<Select_T, From_T, Pip_T>>
	 : std::tuple_size<typename Select_T::retrieve_set> { };

template<size_t N, typename Select_T, typename From_T, typename Pip_T>
struct std::tuple_element<N, ecs::view_reference<Select_T, From_T, Pip_T>>
	: std::tuple_element<N, typename Select_T::retrieve_set> { };

template<typename Select_T, typename From_T, typename Pip_T>
struct std::tuple_size<const ecs::view_reference<Select_T, From_T, Pip_T>>
	 : std::tuple_size<typename Select_T::retrieve_set> { };

template<size_t N, typename Select_T, typename From_T, typename Pip_T>
struct std::tuple_element<N, const ecs::view_reference<Select_T, From_T, Pip_T>>
	: std::add_const<std::tuple_element<N, typename Select_T::retrieve_set>> { };

#pragma endregion
