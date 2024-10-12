#pragma once
#include <stdint.h>
#include <concepts>
#include <iterator>
#include "paged_vector.h"
#include "compressed.h"

//#include "compressed.h"
namespace util
{
	// TODO: increment page versions only on update to current page
	// TODO: on version overflow reset page per page, requires loop through and set to "stale" version
	// TODO: version = index in update list, valid if element index is less than head
	// TODO: could seeding starting version to spread version reset cost
	// TODO: possible traits to implement:
	// * bool inline_metadata = true -> store meta in the same block as the value data.
	// * bool strict_sequence = false -> per element iterator, rather than per page. -> requires different indexing values
	// * bool overflow_protection = false -> for robustness, clear reset versions on overflow. also must queue page updates
	// * uint16_t page_size = 4096


	// TODO: cool shit could be done here -> could do partial list updates, in future could lead to per entity page locking and more granularity of access
	// use case: multiple threads are updating the same paged_vector, each thread can operate on an individual page
	// iterate through page list, try acquire page, if fail push page to back of thread page queue, move to next page
	// update list would supply the page list
	// use case: data is non relational, expensive, low priority or distributed across frames, eg a agent pathfinders
	// can update individual pages and queue update per page.
	
	
	template<typename value_T> 
	class paged_chain_vector_node;
	template<typename value_T, size_t page_size=4096>
	class paged_chain_vector_page;
	template<typename T, typename Alloc_T=std::allocator<T>, size_t page_size=4096u>
	class paged_chain_vector;
	template<typename T, typename Alloc_T=std::allocator<T>, size_t page_size=4096u>
	class paged_chain_vector_view;
	template<typename T, typename Alloc_T=std::allocator<T>, size_t page_size=4096u>
	class paged_chain_vector_iterator;
	class paged_chain_vector_sentinel;
	
	template<typename value_T> 
	class paged_chain_vector_node 
	{
		template<typename,typename,size_t> friend class paged_chain_vector_view;
		template<typename,typename,size_t> friend class paged_chain_vector_iterator;
		friend class paged_chain_vector_sentinel;

		constexpr static uint32_t stale_version = 0;
		constexpr static uint16_t null_index = static_cast<uint16_t>(-1);

	public:
		constexpr paged_chain_vector_node() : value(), next(null_index), version(stale_version) { };
		constexpr paged_chain_vector_node(const paged_chain_vector_node& value) = default;
		constexpr paged_chain_vector_node& operator=(const paged_chain_vector_node& value) = default;
		constexpr paged_chain_vector_node(paged_chain_vector_node&& value) = default;
		constexpr paged_chain_vector_node& operator=(paged_chain_vector_node&& value) = default;

		constexpr paged_chain_vector_node(const value_T& val) : value(val), next(null_index), version(stale_version) { }
		constexpr paged_chain_vector_node& operator=(const value_T& val) { value = val; next = null_index; version=stale_version; return *this; }
		constexpr paged_chain_vector_node(value_T&& val) : value(std::move(val)), next(null_index), version(stale_version) { }
		constexpr paged_chain_vector_node& operator=(value_T&& val) { value = std::move(val); next = null_index; version=stale_version; return *this; }

		constexpr operator value_T&() { return value; }
		constexpr operator const value_T&() const { return value; }
	private:
		value_T value;
		uint16_t next;
		uint32_t version;
	};

	template<> 
	class paged_chain_vector_node<void> 
	{
		template<typename,typename,size_t> friend class paged_chain_vector_view;
		template<typename,typename,size_t> friend class paged_chain_vector_iterator;
		friend class paged_chain_vector_sentinel;

		constexpr static uint32_t stale_version = 0;
		constexpr static uint16_t null_index = static_cast<uint16_t>(-1);

	public:
		constexpr paged_chain_vector_node() : next(null_index), version(stale_version) { };
		constexpr paged_chain_vector_node(const paged_chain_vector_node& value) = default;
		constexpr paged_chain_vector_node& operator=(const paged_chain_vector_node& value) = default;
		constexpr paged_chain_vector_node(paged_chain_vector_node&& value) = default;
		constexpr paged_chain_vector_node& operator=(paged_chain_vector_node&& value) = default;
	private:
		uint16_t next;
		uint32_t version;
	};

	template<typename T, size_t N>
	class paged_chain_vector_page : public std::span<paged_chain_vector_node<T>, N>
	{
		template<typename,typename,size_t> friend class paged_chain_vector_view;
		template<typename,typename,size_t> friend class paged_chain_vector_iterator;
		friend class paged_chain_vector_sentinel;
		
		constexpr static uint32_t stale_version = 0;
		constexpr static uint16_t null_index = static_cast<uint16_t>(-1);
	public:
		constexpr paged_chain_vector_page() noexcept : std::span<paged_chain_vector_node<T>, N>() { }
		constexpr explicit paged_chain_vector_page(paged_chain_vector_node<T>* ptr, size_t count) : std::span<paged_chain_vector_node<T>, N>(ptr, count) { }
		
	private:
		uint16_t head = null_index;
		uint16_t next = null_index; 
		uint16_t prev = null_index;
		uint32_t current = 1;
		uint32_t version = stale_version;
	};

	template<typename T, typename Alloc_T, size_t page_size>
	class paged_chain_vector : public paged_vector<paged_chain_vector_node<T>, typename std::allocator_traits<Alloc_T>::template rebind_alloc<paged_chain_vector_page<T, page_size>>>
	{
		template<typename,typename,size_t> friend class paged_chain_vector_view;
		template<typename,typename,size_t> friend class paged_chain_vector_iterator;
		friend class paged_chain_vector_sentinel;
	public:
		using base_range = paged_vector<paged_chain_vector_node<T>, typename std::allocator_traits<Alloc_T>::template rebind_alloc<paged_chain_vector_page<T, page_size>>>;
		
		using value_type = paged_chain_vector_node<T>;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator = typename base_range::iterator;
		using const_iterator = typename base_range::const_iterator;
		using reverse_iterator = typename base_range::reverse_iterator;
		using const_reverse_iterator = typename base_range::const_reverse_iterator;
		

		constexpr static uint32_t stale_version = 0;
		constexpr static uint16_t null_index = static_cast<uint16_t>(-1);
		 
		constexpr paged_chain_vector() = default;
		constexpr ~paged_chain_vector() = default;

		constexpr paged_chain_vector(const paged_chain_vector& other) = default;
		constexpr paged_chain_vector(const base_range& other) : base_range(other) { }
		constexpr paged_chain_vector(paged_chain_vector&& other) = default;
		constexpr paged_chain_vector(base_range&& other) : base_range(other) { }
		constexpr paged_chain_vector(std::initializer_list<T> ilist) : base_range(ilist.begin(), ilist.end()) { }
		
		constexpr paged_chain_vector& operator=(const paged_chain_vector& other) { base_range::operator=(other); }
		constexpr paged_chain_vector& operator=(paged_chain_vector&& other) { base_range::operator=(other); }
		constexpr paged_chain_vector& operator=(const base_range& other) { base_range::operator=(other); }
		constexpr paged_chain_vector& operator=(base_range&& other)  { base_range::operator=(std::move(other)); }
		constexpr paged_chain_vector& operator=(std::initializer_list<T> ilist) { base_range::operator=(ilist); }

		constexpr paged_chain_vector_view<T, Alloc_T, page_size> chain() { return *this; }
		constexpr paged_chain_vector_view<const T, Alloc_T, page_size> chain() const { return *this; }

	private:
		uint16_t head = null_index;
		uint32_t current = 1;
	};

	class paged_chain_vector_sentinel
	{
	public:
		constexpr bool operator==(const paged_chain_vector_sentinel& rhs) const; // always true
		constexpr bool operator!=(const paged_chain_vector_sentinel& rhs) const; // always false
		
		template<typename T, typename Alloc_T, size_t page_size>
		constexpr bool operator==(const paged_chain_vector_iterator<T, Alloc_T, page_size>& rhs) const;
		template<typename T, typename Alloc_T, size_t page_size>
		constexpr bool operator!=(const paged_chain_vector_iterator<T, Alloc_T, page_size>& rhs) const;
	};

	template<typename T, typename Alloc_T, size_t page_size>
	class paged_chain_vector_iterator
	{
		template<typename, typename, size_t> friend class paged_chain_vector_view;
		friend class paged_chain_vector_sentinel;
		
		using page_type = paged_chain_vector_page<std::remove_const_t<T>, page_size>;

		using base_range = paged_chain_vector<std::remove_const_t<T>, Alloc_T, page_size>;
		using base_iterator = std::conditional_t<std::is_const_v<T>, typename base_range::const_iterator, typename base_range::iterator>;

		constexpr static uint32_t stale_version = 0;
		constexpr static uint16_t null_index = static_cast<uint16_t>(-1);

	public:
		using iterator_category = std::forward_iterator_tag;	
		using value_type = base_iterator::value_type;
		using pointer = base_iterator::pointer;
		using reference = base_iterator::reference;
		using difference_type = base_iterator::difference_type;
		using sentinel_type = paged_chain_vector_sentinel;

		constexpr paged_chain_vector_iterator();
		constexpr operator paged_chain_vector<const T, Alloc_T, page_size>() const;
		constexpr operator base_iterator() const;
		
		constexpr paged_chain_vector_iterator(const paged_chain_vector_iterator& other) = default;
		constexpr paged_chain_vector_iterator& operator=(const paged_chain_vector_iterator& other) = default;
		constexpr paged_chain_vector_iterator(paged_chain_vector_iterator&& other) = default;
		constexpr paged_chain_vector_iterator& operator=(paged_chain_vector_iterator&& other) = default;
	
	private:
		constexpr paged_chain_vector_iterator(base_range* base, uint16_t page_index, uint16_t elem_index);
	
	public:
		constexpr reference operator*();
		constexpr const reference operator*() const;

		constexpr page_type& get_page();
		constexpr const page_type& get_page() const;

		constexpr paged_chain_vector_iterator& operator++();
		constexpr paged_chain_vector_iterator operator++(int);

		reference operator[](difference_type) const = delete;

		constexpr bool operator==(const paged_chain_vector_iterator& rhs) const;
		constexpr bool operator!=(const paged_chain_vector_iterator& rhs) const;
		constexpr bool operator==(const sentinel_type& rhs) const;
		constexpr bool operator!=(const sentinel_type& rhs) const;
	private:
		base_range* base; 
		uint16_t page_index;
		uint16_t elem_index;
	};

	template<typename T, typename Alloc_T, size_t page_size>
	class paged_chain_vector_view
	{
		using paged_chain_vector = std::conditional_t<std::is_const_v<T>, 
			const paged_chain_vector<std::remove_const_t<T>, Alloc_T, page_size>, 
			paged_chain_vector<std::remove_const_t<T>, Alloc_T, page_size>>;
		
		using page_type = paged_chain_vector_page<std::remove_const_t<T>, page_size>;
		
		using base_range = typename paged_chain_vector::base_range;
		using base_iterator = std::conditional_t<std::is_const_v<T>, typename base_range::const_iterator, typename base_range::iterator>;

		constexpr static uint32_t stale_version = 0;
		constexpr static uint16_t null_index = static_cast<uint16_t>(-1);
	public:
		using value_type = paged_chain_vector_node<T>;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator = paged_chain_vector_iterator<T, Alloc_T, page_size>;
		using const_iterator = paged_chain_vector_iterator<const T, Alloc_T, page_size>;
		using sentinel_type = paged_chain_vector_sentinel;

		constexpr paged_chain_vector_view(paged_chain_vector& base);

		constexpr paged_chain_vector_view(const paged_chain_vector_view& other) = default;
		constexpr paged_chain_vector_view& operator=(const paged_chain_vector_view& other) = default;
		constexpr paged_chain_vector_view(paged_chain_vector_view&& other) = default;
		constexpr paged_chain_vector_view& operator=(paged_chain_vector_view&& other) = default;

		constexpr iterator begin();
		constexpr const_iterator begin() const;
		constexpr const_iterator cbegin() const;
		constexpr sentinel_type end();
		constexpr sentinel_type end() const;
		constexpr sentinel_type cend() const;
		
		[[nodiscard]] constexpr bool empty() const noexcept;
		constexpr size_t max_size() const noexcept;

		constexpr reference front();
		constexpr const reference front() const;
		
		constexpr page_type& front_page();
		constexpr const page_type& front_page() const;
	
		bool contains(base_iterator it) const;
		
		constexpr void insert(base_iterator it);
		constexpr void insert(base_iterator first, base_iterator last);
		constexpr void clear();
		constexpr void clear_page(page_type& page);
	private:
		paged_chain_vector& base;
	};
}
	
static_assert(std::ranges::random_access_range<util::paged_chain_vector<int>>);
static_assert(std::random_access_iterator<util::paged_chain_vector<int>::iterator>);
static_assert(std::ranges::forward_range<typename util::paged_chain_vector_view<int>>);
static_assert(std::forward_iterator<typename util::paged_chain_vector_view<int>::iterator>);

#include "paged_chain_vector.tpp"
