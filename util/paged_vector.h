#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <span>

// TODO: resize change size_t for page and element indexing

namespace util {
	template<typename T, typename Alloc_T = std::allocator<T>, typename Page_Alloc_T=std::allocator<std::span<T, 4096>>>
	class paged_vector;
	
	template<typename T, typename Alloc_T = std::allocator<T>, typename Page_Alloc_T=std::allocator<std::span<T, 4096>>> 
	class paged_vector_iterator;

	template<typename T, typename Alloc_T, typename Page_Alloc_T>
	class paged_vector 
	{
	public:
		using allocator_type = Alloc_T;
		using page_allocator_type = Page_Alloc_T;
	
		using value_type = allocator_type::value_type;
		using page_type = page_allocator_type::value_type;
		using reference = T&;
		using const_reference = const T&;
		using iterator = paged_vector_iterator<T, allocator_type, page_allocator_type>;
		using const_iterator = paged_vector_iterator<const T, allocator_type, page_allocator_type>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		static constexpr size_t page_size = page_type::extent;
	public:
		paged_vector(allocator_type&& elem_alloc={}, page_allocator_type&& page_alloc={});
		~paged_vector();
		
		paged_vector(size_t n, allocator_type&& elem_alloc={}, page_allocator_type&& page_alloc={});
		paged_vector(size_t n, const T& value, allocator_type&& elem_alloc={}, page_allocator_type&& page_alloc={});
		
		paged_vector(std::initializer_list<T> ilist, allocator_type&& elem_alloc={}, page_allocator_type&& page_alloc={});
		paged_vector& operator=(std::initializer_list<T> ilist);

		paged_vector(const paged_vector& other);
		paged_vector& operator=(const paged_vector& other);
		
		paged_vector(paged_vector&& other);
		paged_vector& operator=(paged_vector&& other);

		reference operator[](size_t index);
		const_reference operator[](size_t index) const;

		// element access
		reference at(size_t pos);
		const_reference at(size_t pos) const;
		reference front();
		const_reference front() const;
		reference back();
		const_reference back() const;
		
		// page access
		page_type& get_page(size_t pos);
		const page_type& get_page(size_t pos) const;

		// iterators
		iterator begin();
		const_iterator begin() const;
		const_iterator cbegin() const;
		iterator end();
		const_iterator end() const;
		const_iterator cend() const;
		reverse_iterator rbegin();
		const_reverse_iterator rbegin() const;
		const_reverse_iterator crbegin() const;
		reverse_iterator rend();
		const_reverse_iterator rend() const;
		const_reverse_iterator crend() const;
		
		// capacity
		bool empty() const;
		size_t size() const;
		size_t capacity() const;
		void reserve(size_t n);
		void shrink_to_fit();

		// modifiers
		void clear();
		
		template<typename ... Arg_Ts>
		reference emplace_back(Arg_Ts&&... args);
		template<typename ... Arg_Ts>
		iterator emplace(const_iterator pos, Arg_Ts&&... args);
		iterator insert(const_iterator pos, const T& value);
		iterator insert(const_iterator pos, T&& value);
		iterator insert(const_iterator pos, size_t n, const T& value);
		iterator insert(const_iterator pos, size_t n, T& value);
		template<std::input_iterator It>
		iterator insert(const_iterator pos, It first, It last);
		iterator insert(const_iterator pos, std::initializer_list<T> ilist);
		iterator erase(const_iterator pos, size_t n=1);
		iterator erase(const_iterator first, const_iterator last);
		void push_back(const T& value);
		void push_back(T&& value);
		void pop_back();
		void resize(size_t n);
		void swap(paged_vector& other);
		
		page_type* data();
		const page_type* data() const;
		allocator_type& get_allocator();
	private:
		size_t extent;
		allocator_type alloc;
		std::vector<page_type, page_allocator_type> pages;
	};

	template<typename T, typename Alloc_T, typename Page_Alloc_T>
	class paged_vector_iterator 
	{
	public:
		using allocator_type = Alloc_T;
		using page_allocator_type = Page_Alloc_T;
		
		using iterator_category = std::random_access_iterator_tag;
		using value_type = allocator_type::value_type;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;
		using const_pointer = const pointer;
		using const_reference = const reference;
		
		using page_type = page_allocator_type::value_type;
		static constexpr size_t page_size = page_type::extent;
		
		using container_type = std::conditional_t<std::is_const_v<T>, 
			const std::vector<page_type, page_allocator_type>, 
			std::vector<page_type, page_allocator_type>>;
	public:
		paged_vector_iterator();
		paged_vector_iterator(container_type* base, size_t index);
		paged_vector_iterator(container_type* base, size_t page_index, size_t elem_index);
		
		operator paged_vector_iterator<const T, Alloc_T, Page_Alloc_T>() const;
		
		reference operator*();
		const_reference operator*() const;

		reference operator[](difference_type n);
		const_reference operator[](difference_type n) const;
		
		bool operator==(const paged_vector_iterator& other) const;
		bool operator!=(const paged_vector_iterator& other) const;
		bool operator<(const paged_vector_iterator& other) const;
		bool operator<=(const paged_vector_iterator& other) const;
		bool operator>(const paged_vector_iterator& other) const;
		bool operator>=(const paged_vector_iterator& other) const;
		
		paged_vector_iterator& operator++(); // pre increment
		paged_vector_iterator operator++(int); // post increment
		paged_vector_iterator& operator--(); // pre increment
		paged_vector_iterator operator--(int); // post increment
		paged_vector_iterator operator+(difference_type n) const; 
		paged_vector_iterator operator-(difference_type n) const;
		paged_vector_iterator& operator+=(difference_type n);
		paged_vector_iterator& operator-=(difference_type n);

		difference_type operator-(const paged_vector_iterator& other) const;

	private:
		container_type* base;
		size_t page_index;
		size_t elem_index;
	};

	template<typename T, typename Alloc_T, typename Page_Alloc_T> 
	paged_vector_iterator<T, Alloc_T, Page_Alloc_T> operator+(
		typename paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::difference_type n, 
		const paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& it);

}

#include "paged_vector.tpp"

static_assert(std::ranges::random_access_range<util::paged_vector<int>>);
static_assert(std::random_access_iterator<util::paged_vector<int>::iterator>);