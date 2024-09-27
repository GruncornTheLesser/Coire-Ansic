#pragma once
#include "paged_vector.h"

// constructors
template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(allocator_type&& elem_alloc, page_allocator_type&& page_alloc)
 : extent(0), alloc(elem_alloc), pages(page_alloc) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::~paged_vector() 
{
	if (pages.data() != nullptr)
	{
		size_t page_n = extent / page_size;
		size_t elem_n = extent % page_size;

		for (int page_i = 0; page_i < page_n; ++page_i)
		{
			std::destroy_n(pages[page_i].data(), page_size);
			std::destroy_at(&pages[page_i]);
		}

		std::destroy_n(pages[page_n].data(), elem_n);
		std::destroy_at(&pages[page_n]);

		// deallocate pages
		for (int page_i = 0; page_i < pages.size(); ++page_i)
			get_allocator().deallocate(pages[page_i].data(), page_size);
	}
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(size_t n, allocator_type&& elem_alloc, page_allocator_type&& page_alloc)
 : extent(n), alloc(elem_alloc), pages(page_alloc)
{
	// allocate pages
	reserve(extent);
	
	for (int page_i = 0; page_i < pages.size() - 1; ++page_i)
		std::uninitialized_default_construct_n(pages[page_i].data(), page_size);
	std::uninitialized_default_construct_n(pages.back().data(), extent % page_size);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(size_t n, const T& value, allocator_type&& elem_alloc, page_allocator_type&& page_alloc)
 : extent(n), alloc(elem_alloc), pages(page_alloc)
{
	// allocate pages
	reserve(extent);

	for (int page_i = 0; page_i < pages.size() - 1; ++page_i) 
		std::uninitialized_fill_n(pages[page_i].data(), page_size, value);
	std::uninitialized_fill_n(pages.back().data(), extent % page_size, value);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(std::initializer_list<T> ilist, allocator_type&& elem_alloc, page_allocator_type&& page_alloc)
 : extent(ilist.size()), alloc(elem_alloc), pages(page_alloc) 
{
	// allocate pages
	reserve(extent);
	
	auto it = ilist.begin();
	for (int page_i = 0; page_i < pages.size() - 1; ++page_i, it += page_size) 
		std::uninitialized_copy_n(it, page_size, pages[page_i].data());
	std::uninitialized_copy_n(it, extent % page_size, pages.back().data());
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator=(std::initializer_list<T> ilist)
{
	reserve(ilist.size());
	
	size_t page_n = extent / page_size;
	size_t elem_n = extent % page_size;
	size_t other_page_n = ilist.size() / page_size;
	size_t other_elem_n = ilist.size() % page_size;

	auto it = ilist.begin();

	if (extent < ilist.size())
	{
		for (int page_i = 0; page_i < page_n; ++page_i, it += page_size)
			std::copy_n(it, page_size, pages[page_i].data());
		std::copy_n(it, elem_n, pages[page_n].data());
		
		it += elem_n;

		if (page_n == other_page_n)
		{
			std::uninitialized_copy_n(it, other_elem_n - elem_n, pages[page_n].data() + elem_n);
		}
		else
		{
			std::uninitialized_copy_n(it, page_size - elem_n, pages[page_n].data() + elem_n);
			it += page_size;

			for (size_t page_i = page_n + 1; page_i < other_page_n; ++page_i, it += page_size)
				std::uninitialized_copy_n(it, page_size, pages[page_i].data());
			
			std::uninitialized_copy_n(it, other_elem_n, pages[other_page_n].data());
		}
	}
	else
	{
		// copy each page from other
		for (int page_i = 0; page_i < other_page_n; ++page_i, it += page_size)
			std::copy_n(it, page_size, pages[page_i].data());
		
		std::copy_n(it, elem_n, pages[other_page_n].data());
		it += elem_n;

		// destroy remaining elements, do not deallocate
		if (page_n == other_page_n)
		{
			std::destroy_n(pages[page_n].data() + elem_n, other_elem_n - elem_n);
		}
		else
		{
			std::destroy_n(pages[other_page_n].data() + other_elem_n, page_size - other_elem_n);
			for (size_t page_i = page_n + 1; page_i < page_n; ++page_i)
				std::destroy_n(pages[page_i].data(), page_size);
			std::destroy_n(pages[page_n].data(), elem_n);
		}
	}

	extent = ilist.size();
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(const paged_vector& other) // copy constructor
 : extent(other.extent), alloc(other.alloc), pages(other.pages.get_allocator())
{
	// allocate pages
	reserve(extent);
	
	// for each page copy element over
	for (int page_i = 0; page_i < pages.size() - 1; ++page_i)
		std::uninitialized_copy_n(other.pages[page_i].data(), page_size, pages[page_i].data());
	std::uninitialized_copy_n(other.pages.back().data(), extent % page_size, pages.back().data());
	
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator=(const paged_vector& other) // copy assignment
{
	reserve(other.extent);
	
	size_t page_n = extent / page_size;
	size_t elem_n = extent % page_size;
	size_t other_page_n = other.extent / page_size;
	size_t other_elem_n = other.extent % page_size;

	if (extent < other.extent)
	{
		for (int page_i = 0; page_i < page_n; ++page_i)
			std::copy_n(other.pages[page_i].data(), page_size, pages[page_i].data());
		std::copy_n(other.pages[page_n].data(), elem_n, pages[page_n].data());

		if (page_n == other_page_n)
		{
			std::uninitialized_copy_n(other.pages[page_n].data() + elem_n, other_elem_n - elem_n, pages[page_n].data() + elem_n);
		}
		else
		{
			std::uninitialized_copy_n(other.pages[page_n].data() + elem_n, page_size - elem_n, pages[page_n].data() + elem_n);
			for (size_t page_i = page_n + 1; page_i < other_page_n; ++page_i)
				std::uninitialized_copy_n(other.pages[page_i].data(), page_size, pages[page_i].data());
			std::uninitialized_copy_n(other.pages[other_page_n].data(), other_elem_n, pages[other_page_n].data());
		}
	}
	else
	{
		// copy each page from other
		for (int page_i = 0; page_i < other_page_n; ++page_i)
			std::copy_n(other.pages[page_i].data(), page_size, pages[page_i].data());
		std::copy_n(other.pages[other_page_n].data(), elem_n, pages[other_page_n].data());

		// destroy remaining elements
		if (page_n == other_page_n)
		{
			std::destroy_n(pages[page_n].data() + elem_n, other_elem_n - elem_n);
		}
		else
		{
			std::destroy_n(pages[other_page_n].data() + other_elem_n, page_size - other_elem_n);
			for (size_t page_i = page_n + 1; page_i < page_n; ++page_i)
				std::destroy_n(pages[page_i].data(), page_size);
			std::destroy_n(pages[page_n].data(), elem_n);
		}
	}

	extent = other.extent;
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(paged_vector&& other) // move constructor
 : extent(other.extent), alloc(std::move(alloc)), pages(std::move(other.pages)) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator=(paged_vector&& other) // move assignment
{
	if (this != &other) 
	{
		std::swap(extent, other.extent);
		std::swap(alloc, other.alloc);
		std::swap(pages, other.pages);
	} 
	return *this;
}



// indexing operators
template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator[](size_t index) 
{ 
	return pages[index / page_size][index % page_size];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator[](size_t index) const 
{ 
	return pages[index / page_size][index % page_size];
}



// element access
template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::at(size_t pos) 
{
	if (pos >= this->size()) throw std::out_of_range("paged vector index out of range");
	return pages[pos / page_size][pos % page_size];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::at(size_t pos) const
{
	if (pos >= this->size()) throw std::out_of_range("paged vector index out of range");
	return pages[pos / page_size][pos % page_size];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::front() 
{ 
	return pages[0][0];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::front() const 
{ 
	return pages[0][0];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::back() 
{ 
	return at(extent - 1);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::back() const 
{ 
	return at(extent - 1);
}



// page access
template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_type& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::get_page(size_t pos) 
{ 
	return pages[pos];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
const util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_type& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::get_page(size_t pos) const 
{ 
	return pages[pos];
}



// iterators
template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::begin() 
{ 
	return { &pages, 0, 0 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::begin() const 
{ 
	return { &pages, 0, 0 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::cbegin() const 
{ 
	return { &pages, 0, 0 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::end() 
{ 
	return { &pages, extent };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::end() const 
{ 
	return const_iterator{ &pages, extent };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::cend() const 
{ 
	return { &pages, extent };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::rbegin() 
{ 
	return iterator{ &pages, extent - 1 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::rbegin() const 
{ 
	return const_iterator{ &pages, extent - 1 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::crbegin() const 
{ 
	return const_iterator{ &pages, extent - 1 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::rend()
{ 
	return iterator{ &pages, -1, page_size - 1 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::rend() const 
{ 
	return const_iterator{ &pages, -1, page_size - 1 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::crend() const 
{ 
	return const_iterator{ &pages, -1, page_size - 1 };
}



// capacity
template<typename T, typename Alloc_T, typename Page_Alloc_T>
bool util::paged_vector<T, Alloc_T, Page_Alloc_T>::empty() const 
{ 
	return extent == 0;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
size_t util::paged_vector<T, Alloc_T, Page_Alloc_T>::size() const 
{ 
	return extent;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
size_t util::paged_vector<T, Alloc_T, Page_Alloc_T>::capacity() const 
{ 
	return pages.size() * page_size;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::reserve(size_t n)
{
	size_t page_count = (n / page_size) + 1;
	if (page_count > pages.size())
	{
		// allocate pages
		pages.reserve(page_count);
		while(pages.size() != page_count)
			pages.emplace_back(get_allocator().allocate(page_size), page_size);
	}
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::shrink_to_fit() 
{
	size_t page_end = (extent / page_size);
	// deallocate pages
	for (int page_i = pages.size() - 1; page_i != page_end; --page_i) 
	{
		get_allocator().deallocate(pages[page_i].data(), page_size);
		pages.pop_back();
	}

	pages.shrink_to_fit();
}



// modifiers
template<typename T, typename Alloc_T, typename Page_Alloc_T>
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::clear()
{
	size_t page_n = extent / page_size;
	size_t elem_n = extent % page_size;
	for (int page_i = 0; page_i < page_n; ++page_i)
		std::destroy_n(pages[page_i].data(), page_size);
	std::destroy_n(pages[page_n].data(), elem_n);
	
	extent = 0;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> template<typename ... Arg_Ts>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::emplace_back(Arg_Ts&&... args)
{ 
	reserve(++extent);
	std::construct_at<T>(&back(), std::forward<Arg_Ts>(args)...);
	return back();
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> template<typename ... Arg_Ts>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::emplace(const_iterator pos, Arg_Ts&&... args)
{ 
	reserve(extent + 1);
	iterator it = begin() + (pos - cbegin());
	std::move_backward(it, end(), end() + 1);
	std::construct_at<T>(&(*it), std::forward<Arg_Ts>(args)...);
	++extent;
	return it;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, const T& value) 
{ 
	return emplace(pos, value);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, T&& value) 
{
	return emplace(pos, std::move(value));
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, size_t n, const T& value)
{ 
	iterator it = begin() + (pos - cbegin());
	reserve(extent + n);
	std::move_backward(it, end(), end() + n);
	std::uninitialized_fill_n(it, n, value);
	extent += n;
	return it;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, size_t n, T& value)
{ 
	return insert(pos, n, static_cast<const T&>(value));
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> template<std::input_iterator It>
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, It first, It last)
{
	iterator it = begin() + (pos - cbegin());
	size_t n = last - first;
	reserve(extent + n);
	std::move_backward(pos, cend(), end() + n);
	std::uninitialized_move(first, last, it);
	extent += n;
	return it;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, std::initializer_list<T> ilist)
{ 
	return insert(pos, ilist.begin(), ilist.end());
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::erase(const_iterator pos, size_t n)
{
	iterator it1 = begin() + (pos - cbegin());
	iterator it2 = it1 + n;

	if (it2 != end()) std::move(it2, end(), it1);
	else std::destroy_at(&*it1);

	extent -= n;
	return it1;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::erase(const_iterator first, const_iterator last)
{
	size_t n = (last - first);
	iterator it1 = begin() + (first - cbegin());
	iterator it2 = it1 + (last - cbegin());

	if (it2 != end()) std::move(it2, end(), it1);
	else std::destroy(it1, it2);
	
	extent -= n;
	return it1;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::push_back(const T& value)
{ 
	size_t index = extent;
	reserve(++extent);
	std::construct_at(&pages[index / extent][index % page_size], value);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::push_back(T&& value)
{ 
	size_t index = extent;
	reserve(++extent);
	std::construct_at(&pages[index / page_size][index % page_size], value);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::pop_back()
{
	std::destroy_at(&pages[extent / page_size][extent % page_size]);
	--extent;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::resize(size_t n)
{
	reserve(n);
	
	size_t page_n = extent / page_size;
	size_t elem_n = extent % page_size;
	size_t new_page_n = n / page_size;
	size_t new_elem_n = n % page_size;

	if (extent < n)
	{
		if (page_n == new_page_n)
		{
			std::uninitialized_default_construct_n(pages[page_n].data() + elem_n, new_elem_n - elem_n);
		}
		else
		{
			std::uninitialized_default_construct_n(pages[page_n].data() + elem_n, page_size - elem_n);
			for (size_t page_i = page_n + 1; page_i < new_page_n; ++page_i)
				std::uninitialized_default_construct_n(pages[page_i].data(), page_size);
			std::uninitialized_default_construct_n(pages[new_page_n].data(), new_elem_n);
		}
	}
	else
	{
		if (page_n == new_page_n)
		{
			std::destroy_n(pages[page_n].data() + elem_n, new_elem_n - elem_n);
		}
		else
		{
			std::destroy_n(pages[new_page_n].data() + new_elem_n, page_size - new_elem_n);
			for (size_t page_i = page_n + 1; page_i < page_n; ++page_i)
				std::destroy_n(pages[page_i].data(), page_size);
			std::destroy_n(pages[page_n].data(), elem_n);
		}
	}

	extent = n;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::swap(paged_vector& other)
{
	std::swap(extent, other.extent);
	std::swap(alloc, other.alloc);
	std::swap(pages, other.pages);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_type* 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::data() 
{
	return pages.data();
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
const util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_type* 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::data() const 
{
	return pages.data();
}


template<typename T, typename Alloc_T, typename Page_Alloc_T> 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::allocator_type& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::get_allocator()
{
	return alloc;
}



template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::paged_vector_iterator()
 : base(nullptr), page_index(0), elem_index(0) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::paged_vector_iterator(container_type* base, size_t index)
 : base(base), page_index(index / page_size), elem_index(index % page_size) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::paged_vector_iterator(container_type* base, size_t page_index, size_t elem_index)
 : base(base), page_index(page_index), elem_index(elem_index) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator 
paged_vector_iterator<const T, Alloc_T, Page_Alloc_T>() const
{
	return paged_vector_iterator<const T, Alloc_T, Page_Alloc_T>{ base, page_index, elem_index };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator*()
{
	return (*base)[page_index][elem_index];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator*() const
{
	return (*base)[page_index][elem_index];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator[](difference_type n)
{
	return *(*this + n);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator[](difference_type n) const
{
	*(*this + n);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator==(const paged_vector_iterator& other) const
{
	return page_index == other.page_index && elem_index == other.elem_index;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator!=(const paged_vector_iterator& other) const
{
	return page_index != other.page_index || elem_index != other.elem_index;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator<(const paged_vector_iterator& other) const 
{
	return page_index < other.page_index || (page_index == other.page_index && elem_index < other.elem_index);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator<=(const paged_vector_iterator& other) const
{
	return page_index < other.page_index || (page_index == other.page_index && elem_index <= other.elem_index);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator>(const paged_vector_iterator& other) const
{
	return page_index > other.page_index || (page_index == other.page_index && elem_index > other.elem_index);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator>=(const paged_vector_iterator& other) const
{
	return page_index > other.page_index || (page_index == other.page_index && elem_index >= other.elem_index);
}



template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> operator+(
	typename util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::difference_type n, 
	const util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& it)
{
	return it + n;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator++()
{
	if (elem_index == page_size - 1)
	{
		elem_index = 0;
		++page_index;
	}
	else
	{
		++elem_index;
	}
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator++(int)
{
	paged_vector_iterator temp = *this; ++(*this); return temp;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator--()
{ 
	if (elem_index == 0)
	{
		elem_index = page_size - 1;
		--page_index;
	}
	else
	{
		--elem_index;
	}
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator--(int)
{
	paged_vector_iterator temp = *this; --(*this); return temp;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator+(difference_type n) const
{
	return { base, (page_index * page_size) + elem_index + n };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator-(difference_type n) const
{
	return { base, (page_index * page_size) + elem_index - n };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator+=(difference_type n)
{
	size_t index = (page_index * page_size) + elem_index + n;
	page_index = index / page_size;
	elem_index = index % page_size;
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator-=(difference_type n)
{
	size_t index = (page_index * page_size) + elem_index - n;
	page_index = index / page_size;
	elem_index = index % page_size;
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::difference_type 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator-(const paged_vector_iterator& other) const
{
	return ((page_index - other.page_index) * page_size) + (elem_index - other.elem_index);
}
