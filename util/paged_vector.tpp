#pragma once
#include "paged_vector.h"

// constructors
template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(const allocator_type& elem_alloc, const page_allocator_type& page_alloc) noexcept
 : extent(0), alloc(elem_alloc), pages(page_alloc) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(size_t n, const allocator_type& elem_alloc, const page_allocator_type& page_alloc)
 : extent(n), alloc(elem_alloc), pages(page_alloc)
{
	reserve(extent);
	
	for (int page_i = 0; page_i < pages.size() - 1; ++page_i)
		std::uninitialized_default_construct_n(pages[page_i].data(), page_size);
	std::uninitialized_default_construct_n(pages.back().data(), extent % page_size);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(size_t n, const T& value, const allocator_type& elem_alloc, const page_allocator_type& page_alloc)
 : extent(n), alloc(elem_alloc), pages(page_alloc)
{
	reserve(extent);

	for (int page_i = 0; page_i < pages.size() - 1; ++page_i) 
		std::uninitialized_fill_n(pages[page_i].data(), page_size, value);
	std::uninitialized_fill_n(pages.back().data(), extent % page_size, value);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> template<std::input_iterator It>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(It first, It last, const allocator_type& elem_alloc, const page_allocator_type& page_alloc)
 : extent(last - first), alloc(elem_alloc), pages(page_alloc) 
{
	reserve(extent);

	for (int page_i = 0; page_i < pages.size() - 1; ++page_i, first += page_size) 
		std::uninitialized_copy_n(first, page_size, pages[page_i].data());
	std::uninitialized_copy_n(first, extent % page_size, pages.back().data());
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(const paged_vector& other) // copy constructor
 : extent(other.extent), alloc(other.alloc), pages(other.pages.get_allocator())
{
	reserve(extent);
	
	for (int page_i = 0; page_i < pages.size() - 1; ++page_i)
		std::uninitialized_copy_n(other.pages[page_i].data(), page_size, pages[page_i].data());
	std::uninitialized_copy_n(other.pages.back().data(), extent % page_size, pages.back().data());
	
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(paged_vector&& other) // move constructor
 : extent(other.extent), alloc(std::move(alloc)), pages(std::move(other.pages)) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::paged_vector(std::initializer_list<T> ilist, const allocator_type& elem_alloc, const page_allocator_type& page_alloc)
 : paged_vector(ilist.begin(), ilist.end(), elem_alloc, page_alloc) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::~paged_vector() 
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
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator=(const paged_vector& other) // copy assignment
{
	assign(other.begin(), other.end());
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>&
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

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator=(std::initializer_list<T> ilist)
{
	assign(ilist);
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
template<std::input_iterator It>
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::assign(It first, It last)
{
	size_t count = last - first;
	reserve(count);
		
	size_t page_n = extent / page_size;
	size_t elem_n = extent % page_size;
	size_t other_page_n = count / page_size;
	size_t other_elem_n = count % page_size;

	if (extent < count)
	{
		for (int page_i = 0; page_i < page_n; ++page_i, first += page_size)
			std::copy_n(first, page_size, pages[page_i].data());
		std::copy_n(first, elem_n, pages[page_n].data());
		
		first += elem_n;

		if (page_n == other_page_n)
		{
			std::uninitialized_copy_n(first, other_elem_n - elem_n, pages[page_n].data() + elem_n);
		}
		else
		{
			std::uninitialized_copy_n(first, page_size - elem_n, pages[page_n].data() + elem_n);
			first += page_size;

			for (size_t page_i = page_n + 1; page_i < other_page_n; ++page_i, first += page_size)
				std::uninitialized_copy_n(first, page_size, pages[page_i].data());
			std::uninitialized_copy_n(first, other_elem_n, pages[other_page_n].data());
		}
	}
	else
	{
		// copy each page from other
		for (int page_i = 0; page_i < other_page_n; ++page_i, first += page_size)
			std::copy_n(first, page_size, pages[page_i].data());
		
		std::copy_n(first, elem_n, pages[other_page_n].data());
		first += elem_n;

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

	extent = count;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::assign(size_t count, const T& value)
{
	reserve(count);
	
	size_t page_n = extent / page_size;
	size_t elem_n = extent % page_size;
	size_t other_page_n = count / page_size;
	size_t other_elem_n = count % page_size;

	if (extent < count)
	{
		for (int page_i = 0; page_i < page_n; ++page_i)
			std::fill_n(pages[page_i].data(), page_size, value);
		std::fill_n(pages[page_n].data(), elem_n, value);

		if (page_n == other_page_n)
		{
			std::uninitialized_fill_n(pages[page_n].data() + elem_n, other_elem_n - elem_n, value);
		}
		else
		{
			std::uninitialized_fill_n(pages[page_n].data() + elem_n, page_size - elem_n, value);
			for (size_t page_i = page_n + 1; page_i < other_page_n; ++page_i)
				std::uninitialized_fill_n(pages[page_i].data(), page_size, value);
			std::uninitialized_fill_n(pages[other_page_n].data(), other_elem_n, value);
		}
	}
	else
	{
		// copy each page from other
		for (int page_i = 0; page_i < other_page_n; ++page_i)
			std::fill_n(pages[page_i].data(), page_size, value);
		std::fill_n(pages[other_page_n].data(), elem_n, value);

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

	extent = count;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
void util::paged_vector<T, Alloc_T, Page_Alloc_T>::assign(std::initializer_list<T> ilist)
{
	assign(ilist.begin(), ilist.end());
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::allocator_type
util::paged_vector<T, Alloc_T, Page_Alloc_T>::get_allocator() const noexcept
{
	return alloc;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_allocator_type
util::paged_vector<T, Alloc_T, Page_Alloc_T>::get_page_allocator() const noexcept
{
	return pages.get_allocator();
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::begin() noexcept
{ 
	return { &pages, 0, 0 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::begin() const noexcept
{ 
	return cbegin();
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::end() noexcept
{
	return { &pages, extent };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::end() const noexcept
{ 
	return cend();
}


template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::rbegin() noexcept
{ 
	return iterator{ &pages, extent - 1 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::rbegin() const noexcept
{ 
	return crbegin();
}



template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::rend() noexcept
{ 
	return iterator{ &pages, -1, page_size - 1 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::rend() const noexcept
{ 
	return crend();
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::cbegin() const noexcept
{ 
	return { &pages, 0, 0 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::cend() const noexcept
{ 
	return { &pages, extent };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::crbegin() const noexcept
{ 
	return const_iterator{ &pages, extent - 1 };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reverse_iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::crend() const noexcept
{ 
	return const_iterator{ &pages, -1, page_size - 1 }; // last element of page -1
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr bool util::paged_vector<T, Alloc_T, Page_Alloc_T>::empty() const noexcept
{ 
	return extent == 0;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr size_t util::paged_vector<T, Alloc_T, Page_Alloc_T>::size() const 
{ 
	return extent;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr size_t util::paged_vector<T, Alloc_T, Page_Alloc_T>::capacity() const 
{ 
	return pages.size() * page_size;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::resize(size_t n)
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
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::resize(size_t n, const T& value)
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
			std::uninitialized_fill_n(pages[page_n].data() + elem_n, new_elem_n - elem_n, value);
		}
		else
		{
			std::uninitialized_fill_n(pages[page_n].data() + elem_n, page_size - elem_n, value);
			for (size_t page_i = page_n + 1; page_i < new_page_n; ++page_i)
				std::uninitialized_fill_n(pages[page_i].data(), page_size, value);
			std::uninitialized_fill_n(pages[new_page_n].data(), new_elem_n, value);
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
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::reserve(size_t n)
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
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::shrink_to_fit() 
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

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator[](size_t index) 
{ 
	return pages[index / page_size][index % page_size];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::operator[](size_t index) const 
{ 
	return pages[index / page_size][index % page_size];
}

// element access
template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::at(size_t pos) 
{
	if (pos >= this->size()) throw std::out_of_range("paged vector index out of range");
	return pages[pos / page_size][pos % page_size];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::at(size_t pos) const
{
	if (pos >= this->size()) throw std::out_of_range("paged vector index out of range");
	return pages[pos / page_size][pos % page_size];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::front() 
{ 
	return pages[0][0];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::front() const 
{ 
	return pages[0][0];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::back() 
{ 
	return at(extent - 1);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::back() const 
{ 
	return at(extent - 1);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_type& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::get_page(size_t pos) 
{ 
	return pages[pos];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr const util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_type& 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::get_page(size_t pos) const 
{ 
	return pages[pos];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_type* 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::data() noexcept
{
	return pages.data();
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr const util::paged_vector<T, Alloc_T, Page_Alloc_T>::page_type* 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::data() const noexcept
{
	return pages.data();
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> template<typename ... Arg_Ts>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::emplace_back(Arg_Ts&&... args)
{ 
	reserve(++extent);
	std::construct_at<T>(&back(), std::forward<Arg_Ts>(args)...);
	return back();
}


template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::push_back(const T& value)
{ 
	size_t index = extent;
	reserve(++extent);
	std::construct_at(&pages[index / extent][index % page_size], value);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::push_back(T&& value)
{ 
	size_t index = extent;
	reserve(++extent);
	std::construct_at(&pages[index / page_size][index % page_size], value);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::pop_back()
{
	std::destroy_at(&pages[extent / page_size][extent % page_size]);
	--extent;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> template<typename ... Arg_Ts>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
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
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, const T& value) 
{ 
	return emplace(pos, value);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, T&& value) 
{
	return emplace(pos, std::move(value));
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, size_t n, const T& value)
{ 
	iterator it = begin() + (pos - cbegin());
	reserve(extent + n);
	std::move_backward(it, end(), end() + n);
	std::uninitialized_fill_n(it, n, value);
	extent += n;
	return it;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> template<std::input_iterator It>
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
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
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
util::paged_vector<T, Alloc_T, Page_Alloc_T>::insert(const_iterator pos, std::initializer_list<T> ilist)
{ 
	return insert(pos, ilist.begin(), ilist.end());
}

template<typename T, typename Alloc_T, typename Page_Alloc_T> 
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
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
constexpr util::paged_vector<T, Alloc_T, Page_Alloc_T>::iterator 
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
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::swap(paged_vector& other)
{
	std::swap(extent, other.extent);
	std::swap(alloc, other.alloc);
	std::swap(pages, other.pages);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr void util::paged_vector<T, Alloc_T, Page_Alloc_T>::clear() noexcept
{
	size_t page_n = extent / page_size;
	size_t elem_n = extent % page_size;
	for (int page_i = 0; page_i < page_n; ++page_i)
		std::destroy_n(pages[page_i].data(), page_size);
	std::destroy_n(pages[page_n].data(), elem_n);
	
	extent = 0;
}



template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::paged_vector_iterator()
 : base(nullptr), page_index(0), elem_index(0) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::paged_vector_iterator(container_type* base, size_t index)
 : base(base), page_index(index / page_size), elem_index(index % page_size) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::paged_vector_iterator(container_type* base, size_t page_index, size_t elem_index)
 : base(base), page_index(page_index), elem_index(elem_index) { }

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator 
paged_vector_iterator<const T, Alloc_T, Page_Alloc_T>() const
{
	return paged_vector_iterator<const T, Alloc_T, Page_Alloc_T>{ base, page_index, elem_index };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator*()
{
	return (*base)[page_index][elem_index];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator*() const
{
	return (*base)[page_index][elem_index];
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::reference 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator[](difference_type n)
{
	return *(*this + n);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::const_reference 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator[](difference_type n) const
{
	*(*this + n);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator==(const paged_vector_iterator& other) const
{
	return page_index == other.page_index && elem_index == other.elem_index;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator!=(const paged_vector_iterator& other) const
{
	return page_index != other.page_index || elem_index != other.elem_index;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator<(const paged_vector_iterator& other) const 
{
	return page_index < other.page_index || (page_index == other.page_index && elem_index < other.elem_index);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator<=(const paged_vector_iterator& other) const
{
	return page_index < other.page_index || (page_index == other.page_index && elem_index <= other.elem_index);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator>(const paged_vector_iterator& other) const
{
	return page_index > other.page_index || (page_index == other.page_index && elem_index > other.elem_index);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr bool util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator>=(const paged_vector_iterator& other) const
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
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& 
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
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator++(int)
{
	paged_vector_iterator temp = *this; ++(*this); return temp;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& 
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
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator--(int)
{
	paged_vector_iterator temp = *this; --(*this); return temp;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator+(difference_type n) const
{
	return { base, (page_index * page_size) + elem_index + n };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T> 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator-(difference_type n) const
{
	return { base, (page_index * page_size) + elem_index - n };
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator+=(difference_type n)
{
	size_t index = (page_index * page_size) + elem_index + n;
	page_index = index / page_size;
	elem_index = index % page_size;
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>& 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator-=(difference_type n)
{
	size_t index = (page_index * page_size) + elem_index - n;
	page_index = index / page_size;
	elem_index = index % page_size;
	return *this;
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::difference_type 
util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::operator-(const paged_vector_iterator& other) const
{
	return ((page_index - other.page_index) * page_size) + (elem_index - other.elem_index);
}

template<typename T, typename Alloc_T, typename Page_Alloc_T>
constexpr size_t util::paged_vector_iterator<T, Alloc_T, Page_Alloc_T>::index() const
{
	return (page_index * page_size) + elem_index;
}
