#pragma once
#include "chain_vector.h"

template<typename T, typename Alloc_T, size_t page_size>
constexpr util::chain_vector_iterator<T, Alloc_T, page_size>::chain_vector_iterator() { }

template<typename T, typename Alloc_T, size_t page_size>
constexpr util::chain_vector_iterator<T, Alloc_T, page_size>::chain_vector_iterator(base_range* base, uint16_t page_index, uint16_t elem_index)
 : base(base), page_index(page_index), elem_index(elem_index) { }

template<typename T, typename Alloc_T, size_t page_size>
constexpr util::chain_vector_iterator<T, Alloc_T, page_size>::reference 
util::chain_vector_iterator<T, Alloc_T, page_size>::operator*()
{
	return base->get_page(page_index)[elem_index];
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr const util::chain_vector_iterator<T, Alloc_T, page_size>::reference 
util::chain_vector_iterator<T, Alloc_T, page_size>::operator*() const
{
	return base->get_page(page_index)[elem_index];
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr util::chain_vector_iterator<T, Alloc_T, page_size>::page_type& 
util::chain_vector_iterator<T, Alloc_T, page_size>::get_page()
{
	return base->get_page(page_index);
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr const util::chain_vector_iterator<T, Alloc_T, page_size>::page_type& 
util::chain_vector_iterator<T, Alloc_T, page_size>::get_page() const
{
	return base->get_page(page_index);
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr util::chain_vector_iterator<T, Alloc_T, page_size>& 
util::chain_vector_iterator<T, Alloc_T, page_size>::operator++() 
{
	elem_index = (**this).next;
	if (elem_index == null_index || (**this).version != get_page().current)
	{
		page_index = get_page().next;

		if (page_index != null_index && get_page().version == base->current)
			elem_index = base->get_page(page_index).head;
		else 
			elem_index = null_index;
	}
	return *this;
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr util::chain_vector_iterator<T, Alloc_T, page_size>
util::chain_vector_iterator<T, Alloc_T, page_size>::operator++(int) 
{ 
	auto tmp = *this; ++(*this); return tmp; 
}


template<typename T, typename Alloc_T, size_t page_size>
constexpr bool util::chain_vector_iterator<T, Alloc_T, page_size>::operator==(const chain_vector_iterator& other) const 
{ 
	return page_index == other.page_index && elem_index == other.elem_index;
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr bool util::chain_vector_iterator<T, Alloc_T, page_size>::operator!=(const chain_vector_iterator& other) const 
{
	return page_index != other.page_index || elem_index != other.elem_index;
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr bool util::chain_vector_iterator<T, Alloc_T, page_size>::operator==(const sentinel_type& other) const 
{ 
	return elem_index == null_index; 
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr bool util::chain_vector_iterator<T, Alloc_T, page_size>::operator!=(const sentinel_type& other) const 
{
	return elem_index != null_index;
}

template<typename T, typename Alloc_T, size_t page_size>
constexpr util::chain_vector_view<T, Alloc_T, page_size>::chain_vector_view(chain_vector& base) : base(base) { }

template<typename T, typename Alloc_T, size_t page_size> 
constexpr util::chain_vector_view<T, Alloc_T, page_size>::iterator 
util::chain_vector_view<T, Alloc_T, page_size>::begin() 
{ 
	if (empty()) return { &base, null_index, null_index };
	else return { &base, base.head, front_page().head };
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr util::chain_vector_view<T, Alloc_T, page_size>::const_iterator 
util::chain_vector_view<T, Alloc_T, page_size>::begin() const 
{ 
	return cbegin(); 
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr util::chain_vector_view<T, Alloc_T, page_size>::const_iterator 
util::chain_vector_view<T, Alloc_T, page_size>::cbegin() const 
{ 
	if (empty()) return { &base, null_index, null_index };
	else return { &base, base.head, front_page().head };
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr util::chain_vector_view<T, Alloc_T, page_size>::sentinel_type 
util::chain_vector_view<T, Alloc_T, page_size>::end() 
{ 
	return { }; 
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr util::chain_vector_view<T, Alloc_T, page_size>::sentinel_type 
util::chain_vector_view<T, Alloc_T, page_size>::end() const 
{ 
	return cend(); 
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr util::chain_vector_view<T, Alloc_T, page_size>::sentinel_type 
util::chain_vector_view<T, Alloc_T, page_size>::cend() const 
{ 
	return { }; 
}

template<typename T, typename Alloc_T, size_t page_size> 
[[nodiscard]] constexpr bool util::chain_vector_view<T, Alloc_T, page_size>::empty() const noexcept { return base.head == null_index; }

template<typename T, typename Alloc_T, size_t page_size> 
constexpr size_t util::chain_vector_view<T, Alloc_T, page_size>::max_size() const noexcept 
{
	return page_size * null_index; 
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr util::chain_vector_view<T, Alloc_T, page_size>::reference 
util::chain_vector_view<T, Alloc_T, page_size>::front() 
{
	return *begin();
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr const util::chain_vector_view<T, Alloc_T, page_size>::reference 
util::chain_vector_view<T, Alloc_T, page_size>::front() const 
{
	return *begin(); 
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr util::chain_vector_view<T, Alloc_T, page_size>::page_type& 
util::chain_vector_view<T, Alloc_T, page_size>::front_page() 
{ 
	return base.get_page(base.head); 
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr const util::chain_vector_view<T, Alloc_T, page_size>::page_type& 
util::chain_vector_view<T, Alloc_T, page_size>::front_page() const 
{ 
	return base.get_page(base.head); 
}

template<typename T, typename Alloc_T, size_t page_size> 
bool util::chain_vector_view<T, Alloc_T, page_size>::contains(base_iterator it) const 
{ 
	return it.get_page().current == (*it).version;
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr void util::chain_vector_view<T, Alloc_T, page_size>::insert(base_iterator it) 
{
	auto [page_i, elem_i] = it.get_indices();

	page_type& page = base.get_page(page_i);
	value_type& elem = page[elem_i];
	
	if (page.version != base.current)			// page not in chain list
	{
		page.version = base.current;			// set page version to current
		page.next = base.head;					// insert page into list
		page.prev = null_index;					// insert page into list

		if (base.head != null_index) front_page().prev = page_i;
		base.head = page_i;
	}
	else if (elem.version == page.current) return; 	// already in list
	
	elem.version = page.current;
	elem.next = page.head;
	page.head = elem_i;
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr void util::chain_vector_view<T, Alloc_T, page_size>::clear() 
{ 
	base.head = -1; 
	if (++base.current == stale_version) 
	{ 
		++base.current; 
		// TODO: overflow version protection clear all element versions 
	} 
}

template<typename T, typename Alloc_T, size_t page_size> 
constexpr void util::chain_vector_view<T, Alloc_T, page_size>::clear_page(page_type& page) 
{
	if (base.current != page.version) return; // not in chain list

	if (++page.current == stale_version) // increment page version
	{ 
		++page.current; 
		// TODO: overflow version protection clear page element versions
	} 
	
	if (page.prev != null_index) base.get_page(page.prev).next = page.next;
	if (page.next != null_index) base.get_page(page.next).prev = page.prev;

	page.head = null_index;
	page.prev = null_index;
	page.next = null_index;
}
