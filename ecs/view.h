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
		using pip_storage = std::conditional_t<std::is_reference_v<pip_T>, pip_T, pip_T&>;
		using iterator = view_iterator<select_T, from_T, where_T, pip_T>;
		using const_iterator = view_iterator<util::tuple_for_each_t<std::add_const, select_T>, from_T, where_T, pip_T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		view(pip_T&& base);

		iterator begin();
		iterator end();

		const_iterator begin() const;
		const_iterator end() const;
		
		reverse_iterator rbegin();
		reverse_iterator rend();

		const_reverse_iterator rbegin() const;
		const_reverse_iterator rend() const;
	private:
		pip_storage pip;
	};

	template<typename select_T, typename from_T, typename where_T, typename pip_T>
	class view_iterator {
	public:
		using reference = view_reference<select_T, from<from_T>, pip_T>;
		using const_reference = view_reference<util::tuple_for_each_t<std::add_const, select_T>, from_T, pip_T>;
		
		view_iterator(pip_T& pip, size_t index);

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
		size_t ind;
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