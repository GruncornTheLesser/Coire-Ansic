#pragma once
#include <stdint.h>
#include <concepts>
#include <iterator>
#include <vector>

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
	// use case: multiple threads are updating the same vector, each thread can operate on an individual page
	// iterate through page list, try acquire page, if fail push page to back of thread page queue, move to next page
	// update list would supply the page list
	// use case: data is non relational, expensive, low priority or distributed across frames, eg a agent pathfinders
	// can update individual pages and queue update per page.
	
	
	template<typename value_T> 
	class chain_vector_node;

	template<typename T, typename Alloc_T=std::allocator<T>>
	class chain_vector;
	
	template<typename T, typename Alloc_T=std::allocator<T>>
	class chain_vector_view;
	
	template<typename T, typename Alloc_T=std::allocator<T>>
	class chain_vector_iterator;
	
	template<typename value_T> 
	class chain_vector_node 
	{
		template<typename,typename> friend class chain_vector_view;
		template<typename,typename> friend class chain_vector_iterator;

		constexpr static uint32_t stale_version = 0;
		constexpr static int32_t null_index = -1;

	public:
		constexpr chain_vector_node() : value(), next(null_index), version(stale_version) { }
		constexpr chain_vector_node(const chain_vector_node& value) = default;
		constexpr chain_vector_node& operator=(const chain_vector_node& value) = default;
		constexpr chain_vector_node(chain_vector_node&& value) = default;
		constexpr chain_vector_node& operator=(chain_vector_node&& value) = default;

		constexpr chain_vector_node(const value_T& val) : value(val), next(null_index), version(stale_version) { }
		constexpr chain_vector_node& operator=(const value_T& val) { value = val; next = null_index; version=stale_version; return *this; }
		constexpr chain_vector_node(value_T&& val) : value(std::move(val)), next(null_index), version(stale_version) { }
		constexpr chain_vector_node& operator=(value_T&& val) { value = std::move(val); next = null_index; version=stale_version; return *this; }

		constexpr operator value_T&() { return value; }
		constexpr operator const value_T&() const { return value; }
	
	private:
		value_T value;
		int32_t next;
		uint32_t version;
	};

	template<> 
	class chain_vector_node<void> 
	{
		template<typename,typename> friend class chain_vector_view;
		template<typename,typename> friend class chain_vector_iterator;

		constexpr static uint32_t stale_version = 0;
		constexpr static int32_t null_index = 0;

	public:
		constexpr chain_vector_node() : next(null_index), version(stale_version) { }
		constexpr chain_vector_node(const chain_vector_node& value) = default;
		constexpr chain_vector_node& operator=(const chain_vector_node& value) = default;
		constexpr chain_vector_node(chain_vector_node&& value) = default;
		constexpr chain_vector_node& operator=(chain_vector_node&& value) = default;
	private:
		int32_t next;
		uint32_t version;
	};

	template<typename T, typename Alloc_T>
	class chain_vector : public std::vector<chain_vector_node<T>, typename std::allocator_traits<Alloc_T>::template rebind_alloc<chain_vector_node<T>>>
	{
		template<typename,typename> friend class chain_vector_view;
		template<typename,typename> friend class chain_vector_iterator;
		
	public:
		using base_range = std::vector<chain_vector_node<T>, typename std::allocator_traits<Alloc_T>::template rebind_alloc<chain_vector_node<T>>>;
		
		using value_type = chain_vector_node<T>;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator = typename base_range::iterator;
		using const_iterator = typename base_range::const_iterator;
		using reverse_iterator = typename base_range::reverse_iterator;
		using const_reverse_iterator = typename base_range::const_reverse_iterator;
		

		constexpr static uint32_t stale_version = 0;
		constexpr static int32_t null_index = -1;
		 
		constexpr chain_vector() = default;
		constexpr ~chain_vector() = default;

		constexpr chain_vector(const chain_vector& other) = default;
		constexpr chain_vector(const base_range& other) : base_range(other) { }
		constexpr chain_vector(chain_vector&& other) = default;
		constexpr chain_vector(base_range&& other) : base_range(other) { }
		constexpr chain_vector(std::initializer_list<T> ilist) : base_range(ilist.begin(), ilist.end()) { }
		
		constexpr chain_vector& operator=(const chain_vector& other) { base_range::operator=(other); }
		constexpr chain_vector& operator=(chain_vector&& other) { base_range::operator=(other); }
		constexpr chain_vector& operator=(const base_range& other) { base_range::operator=(other); }
		constexpr chain_vector& operator=(base_range&& other)  { base_range::operator=(std::move(other)); }
		constexpr chain_vector& operator=(std::initializer_list<T> ilist) { base_range::operator=(ilist); }

		constexpr chain_vector_view<T, Alloc_T> chain() { return *this; }
		constexpr chain_vector_view<const T, Alloc_T> chain() const { return *this; }

	private:
		uint32_t head = null_index;
		uint32_t current = 1;
	};

	template<typename T, typename Alloc_T>
	class chain_vector_iterator
	{
		template<typename, typename> friend class chain_vector_view;
		
		using base_range = chain_vector<std::remove_const_t<T>, Alloc_T>;
		using base_iterator = std::conditional_t<std::is_const_v<T>, typename base_range::const_iterator, typename base_range::iterator>;

		constexpr static uint32_t stale_version = 0;
		constexpr static uint32_t null_index = static_cast<uint32_t>(-1);

	public:
		using iterator_category = std::forward_iterator_tag;	
		using value_type = base_iterator::value_type;
		using pointer = base_iterator::pointer;
		using reference = base_iterator::reference;
		using difference_type = base_iterator::difference_type;


		constexpr chain_vector_iterator() : ptr(nullptr) { }
		constexpr chain_vector_iterator(value_type* ptr) : ptr(ptr) { }
		constexpr operator chain_vector_iterator<const T, Alloc_T>() const { return ptr; }
		constexpr operator base_iterator() const { return ptr; }
			
		constexpr reference operator*() { return *ptr; }
		constexpr const reference operator*() const { return *ptr; }

		constexpr chain_vector_iterator& operator++() { ptr += *ptr.next; return *this; }
		constexpr chain_vector_iterator operator++(int) { auto tmp = *this; ++(*this); return *this; }

		constexpr bool operator==(const chain_vector_iterator& rhs) const { return ptr == rhs.ptr; }
		constexpr bool operator!=(const chain_vector_iterator& rhs) const { return ptr != rhs.ptr; }
		
		constexpr difference_type operator-(const base_iterator& rhs) const { return ptr - &(*rhs); }
		
	private:
		value_type* ptr;
	};

	template<typename T, typename Alloc_T>
	class chain_vector_view
	{
		using base_vector = std::conditional_t<std::is_const_v<T>, const chain_vector<std::remove_const_t<T>, Alloc_T>, chain_vector<T, Alloc_T>>;
		using base_iterator = std::conditional_t<std::is_const_v<T>, typename base_vector::base_range::const_iterator, typename base_vector::base_range::iterator>;

		constexpr static uint32_t stale_version = 0;
		constexpr static int32_t null_index = -1;
	public:
		using value_type = chain_vector_node<T>;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator = chain_vector_iterator<T, Alloc_T>;
		using const_iterator = chain_vector_iterator<const T, Alloc_T>;

		constexpr chain_vector_view(base_vector& base) : base(base) { }

		constexpr iterator begin() { return base.data() + base.head; }
		constexpr const_iterator begin() const { return cbegin(); }
		constexpr const_iterator cbegin() const { return base.data() + base.head; }
		constexpr iterator end() { return nullptr; }
		constexpr const_iterator end() const { return cend(); }
		constexpr const_iterator cend() const { return nullptr; }
		
		[[nodiscard]] constexpr bool empty() const noexcept { return base.head == -1; }

		constexpr reference front() { return base[base.head]; }
		constexpr const reference front() const { return base[base.head]; }
	
		bool contains(base_iterator it) const { return (*it).version == base.current; }
		
		constexpr void pop_front() 
		{
			base[base.head].version = stale_version;
			base.head += base[base.head].next;
		}

		constexpr void insert(base_iterator it)
		{ 
			if (contains(it)) return;
			(*it).version = base.current;
			size_t index = (it - base.begin());
			(*it).next = base.head - index;
			base.head = index;
		}
		constexpr void clear() 
		{
			if (++base.current == 0) 
			{
				++base.current;
			}
			base.head = -1;
		}
	private:
		base_vector& base;
	};
}
	
static_assert(std::ranges::random_access_range<util::chain_vector<int>>);
static_assert(std::random_access_iterator<util::chain_vector<int>::iterator>);
static_assert(std::ranges::forward_range<typename util::chain_vector_view<int>>);
static_assert(std::forward_iterator<typename util::chain_vector_view<int>::iterator>);

