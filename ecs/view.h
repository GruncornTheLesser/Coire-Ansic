#ifndef ECS_VIEW_H
#define ECS_VIEW_H
#include <tuple>
#include "entity.h"

namespace ecs {
	template<typename ... Ts> 
	struct select;
	template<typename T> 
	struct from;
	template<typename ... Ts> 
	struct where;

	template<typename ... Ts> 
	struct include;
	template<typename ... Ts> 
	struct exclude;

	// TODO: move view_builders to traits

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

	template<typename select_T, 
		typename from_T = typename view_from_builder<select_T>::type, 
		typename where_T = typename view_where_builder<select_T, from_T>::type,
		typename Pip_T = typename view_pipeline_builder<select_T, from_T, where_T>::type>
	class view;
	
	template<typename select_T, typename from_T, typename where_T, typename pip_T>
	class view_iterator;
	
	template<typename select_T, typename from_T, typename pip_T>
	class view_reference;

	template<typename select_T, typename from_T, typename where_T, typename pip_T>
	class view {
	public:
		using resource_set = std::tuple<select_T, from_T, where_T>;

		using iterator = view_iterator<select_T, from_T, where_T, pip_T>;
		using const_iterator = view_iterator<util::trn::each_t<select_T, std::add_const>, from_T, where_T, pip_T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		view(pip_T&& base);

		reverse_iterator begin();
		reverse_iterator end();

		const_reverse_iterator begin() const;
		const_reverse_iterator end() const;
		
		iterator rbegin();
		iterator rend();

		const_iterator rbegin() const;
		const_iterator rend() const;
	private:
		pip_T pip;
	};

	template<typename select_T, typename from_T, typename where_T, typename pip_T>
	class view_iterator {
	public:
		
		using iterator_category = std::random_access_iterator_tag;
		using value_type = view_reference<select_T, from_T, pip_T>;
        using reference = view_reference<select_T, from_T, pip_T>;
		using const_reference = view_reference<util::trn::each_t<select_T, std::add_const>, from_T, pip_T>;
		//using pointer = T*;
		//using const_pointer = const T*;
		using difference_type = std::ptrdiff_t;

		view_iterator(pip_T& pip, int index);

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
		entity ent;
		pip_T& pip;
		ptrdiff_t ind;
	};

	template<typename select_T, typename from_T, typename pip_T>
	class view_reference {
	public:
		view_reference(pip_T& pip, size_t index);

		template<size_t N> 
		std::tuple_element_t<N, view_reference> get();
		
		template<size_t N> 
		std::tuple_element_t<N, const view_reference> get() const;
	private:
		ecs::entity ent;
		pip_T& pip;
		size_t ind;
	};
}

// structured binding
template<typename select_T, typename from_T, typename pip_T>
struct std::tuple_size<ecs::view_reference<select_T, from_T, pip_T>>
	 : std::tuple_size<typename select_T::retrieve_set> { };

template<size_t N, typename select_T, typename from_T, typename pip_T>
struct std::tuple_element<N, ecs::view_reference<select_T, from_T, pip_T>>
	: std::tuple_element<N, typename select_T::retrieve_set> { };

template<typename select_T, typename from_T, typename pip_T>
struct std::tuple_size<const ecs::view_reference<select_T, from_T, pip_T>>
	 : std::tuple_size<typename select_T::retrieve_set> { };

template<size_t N, typename select_T, typename from_T, typename pip_T>
struct std::tuple_element<N, const ecs::view_reference<select_T, from_T, pip_T>>
	: std::add_const<std::tuple_element<N, typename select_T::retrieve_set>> { };

#include "view.tpp"
#endif