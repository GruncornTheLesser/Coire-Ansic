#pragma once
#include <memory>
#include <algorithm>
#include <limits>
#include <utility>
#include "next_pow2.h"


namespace util {
	// TODO: allocator parameter for paged_vector
	template<typename T, typename Alloc_T=std::allocator<T>, typename Alloc_Page_T=std::allocator<T*>> 
	class paged_vector;
	
	template<typename T> 
	class paged_vector_iterator;

	template<typename T, typename Alloc_T, typename Page_Alloc_T>
	class paged_vector 
	{
		static constexpr size_t page_size = 4096;
		using page = T*;
	public:
		using value_type = T;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = paged_vector_iterator<value_type>;
		using const_iterator = paged_vector_iterator<const value_type>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	public:
		paged_vector() : count(0), page_count(1), page_capacity(8)
		{
			pages = get_page_allocator().allocate(page_capacity);
			*pages = get_allocator().allocate(page_size);
		}
		paged_vector(size_t n, const T& value)
		{
			count = n;
			page_count = (n / page_size) + 1;
			page_capacity = (8 < page_count) ? page_count : 8;
			pages = get_page_allocator().allocate(page_capacity);
			std::for_each(pages, pages + page_count, [&](page& page)
			{ 
				page = get_allocator().allocate(page_size);
				std::for_each(page, page + page_size, [&value](T& v)
				{ 
					std::construct_at(&v, value);
				});
			});
		}
		paged_vector(std::initializer_list<T> ilist) 
		{
			count = ilist.size();
			page_count = (count / page_size) + 1;
			page_capacity = (8 < page_count) ? page_count : 8;

			pages = get_page_allocator().allocate(page_capacity);
			std::for_each(pages, pages + page_count, [](page& page) 
			{ 
				page = get_allocator().allocate(page_size); 
			});
			std::move(ilist.begin(), ilist.end(), begin());
		}
		paged_vector& operator=(std::initializer_list<T> ilist)
		{
			clear();
			reserve(ilist.size());
			std::move(ilist.begin(), ilist.end(), begin());
			auto new_end = begin() + ilist.size();
			auto old_end = end();
			if (new_end < old_end) std::destroy(new_end, old_end);
			std::move(ilist.begin(), ilist.end(), begin());
			count = ilist.size();
			return *this;
		}
		
		~paged_vector() 
		{
			if (pages != nullptr)
 			{
				std::for_each(pages, pages + page_count, [&](page p) 
				{ 
					get_allocator().deallocate(p, page_size); 
				});
				get_page_allocator().deallocate(pages, page_capacity);
			}
		}

		paged_vector(const paged_vector& other) 
		{ 
			count = other.count;
			page_count = other.page_count;
			page_capacity = other.page_capacity;
			pages = get_page_allocator().allocate(page_capacity);

			int back = page_count - 1;
			for (int i = 0; i < back; ++i) 
			{
				pages[i] = get_allocator().allocate(page_size); 
				std::copy(other.pages[i], other.pages[i] + page_size, pages[i]);
			}
			pages[back] = get_allocator().allocate(page_size); 
			std::copy(other.pages[back], other.pages[back] + count % page_size, pages[back]);
		}
		paged_vector& operator=(const paged_vector& other) 
		{
			if (pages != other.pages) 
			{
				count = other.count;
				page_count = other.page_count;
				page_capacity = other.page_capacity;
				pages = get_page_allocator().allocate(page_capacity);

				int back = page_count - 1;
				for (int i = 0; i < back; ++i) 
				{
					pages[i] = get_allocator().allocate(page_size); 
					std::copy(other.pages[i], other.pages[i] + page_size, pages[i]);
				}
				
				pages[back] = get_allocator().allocate(page_size); 
				std::copy(other.pages[back], other.pages[back] + count % page_size, pages[back]);
			}

			return *this;
		}
		paged_vector(paged_vector&& other)
		{
			count = other.count;
			page_count = other.page_count;
			page_capacity = other.page_capacity;
			pages = other.pages;
			other.pages = nullptr;
		}
		paged_vector& operator=(paged_vector&& other) 
		{
			if (pages != other.pages)
			{
				std::swap(other);
			}
			return *this;
		}

		reference operator[](size_t index) 
		{ 
			return pages[index / page_size][index % page_size]; 
		}
		const_reference operator[](size_t index) const 
		{ 
			return pages[index / page_size][index % page_size]; 
		}

		// element access
		reference at(size_t pos) 
		{ 
			return pages[pos / page_size][pos % page_size]; 
		}
		const_reference at(size_t pos) const
		{ 
			return pages[pos / page_size][pos % page_size]; 
		}
		reference front() 
		{ 
			return pages[0][0]; 
		}
		const_reference front() const 
		{ 
			return pages[0][0]; 
		}
		reference back() 
		{ 
			return at(count - 1); 
		}
		const_reference back() const 
		{ 
			return at(count - 1);
		}
		
		// iterators
		iterator begin() 
		{ 
			return { 0, pages }; 
		}
		const_iterator begin() const 
		{ 
			return iterator{ 0, pages }; 
		}
		const_iterator cbegin() const 
		{ 
			return iterator{ 0, pages }; 
		}
		iterator end() 
		{ 
			return { count, pages };
		}
		const_iterator end() const 
		{ 
			return iterator{ count, pages }; 
		}
		const_iterator cend() const 
		{ 
			return iterator{ count, pages }; 
		}
		reverse_iterator rbegin() 
		{ 
			return iterator{ count - 1, pages }; 
		}
		const_reverse_iterator rbegin() const 
		{ 
			return const_iterator{ count - 1, pages }; 
		}
		const_reverse_iterator crbegin() const 
		{ 
			return const_iterator{ count - 1, pages }; 
		}
		reverse_iterator rend() 
		{ 
			return { -1, pages }; 
		}
		const_reverse_iterator rend() const 
		{ 
			return const_iterator{ -1, pages }; 
		}
		const_reverse_iterator crend() const 
		{ 
			return const_iterator{ -1, pages }; 
		}
		
		// capacity
		bool empty() const 
		{ 
			return count == 0; 
		}
		size_t size() const 
		{ 
			return count; 
		}
		size_t capacity() const 
		{ 
			return page_count * page_size; 
		}
		size_t max_size() const 
		{ 
			return std::numeric_limits<size_t>::max(); 
		}
		void reserve(size_t n)
		{
			size_t new_page_count = n / page_size + 1;
			if (new_page_count > page_count)
			{
				if (new_page_count > page_capacity)
				{
					size_t new_page_capacity = util::next_pow2(new_page_count);

					page* new_pages = get_page_allocator().allocate(new_page_capacity);
					std::move(pages, pages + page_count, new_pages);
					
					get_page_allocator().deallocate(pages, page_capacity);

					page_capacity = new_page_capacity;
					pages = new_pages;
				}

				std::for_each(pages + page_count, pages + new_page_count, [&](page& page)
				{ 
					page = get_allocator().allocate(page_size); 
				});
				page_count = new_page_count;
			}
		}
		void shrink_to_fit() 
		{
			size_t new_page_count = count / page_size + 1;

			std::for_each(pages + new_page_count, pages + page_count, [](page p)
			{ 
				get_allocator().deallocate(p, page_size); 
			});
			page_count = new_page_count;
		}

		// modifiers
		void clear() 
		{
			std::destroy_n(begin(), count);
			count = 0;
		}
		template<typename ... Arg_Ts>
		iterator emplace(const_iterator pos, Arg_Ts&&... args) 
		{ 
			reserve(++count);
			size_t offset = pos - cbegin();
			iterator it = { offset , pages };
			std::move_backward(it, end(), end() + 1);
			std::construct_at<T>(&*it, std::forward<Arg_Ts>(args)...);
			return it;
		}
		template<typename ... Arg_Ts>
		reference emplace_back(Arg_Ts&&... args) 
		{ 
			reserve(++count);
			std::construct_at<T>(&back(), std::forward<Arg_Ts>(args)...);
			return back();
		}
		
		iterator insert(const_iterator pos, const T& value) 
		{ 
			return emplace(pos, value);
		}
		iterator insert(const_iterator pos, T&& value) 
		{
			return emplace(pos, std::move(value));
		}
		iterator insert(const_iterator pos, size_t n, const T& value)
		{ 
			size_t offset = pos - cbegin();
			reserve(count += n);
			std::move_backward(pos, cend(), end() + n);
			std::fill_n(begin() + offset, n, value);
			return begin() + offset;
		}
		iterator insert(const_iterator pos, size_t n, T&& value)
		{ 
			size_t offset = pos - cbegin();
			size_t new_size = count + n;
			reserve(new_size);
			std::move_backward(pos, cend(), end() + n);
			std::fill_n(begin() + offset, n, value);
			count = new_size;
			return begin() + offset;
		}
		template<typename It>
		iterator insert(const_iterator pos, It first, It last)
		{ 
			size_t n = last - first;
			size_t offset = pos - cbegin();
			size_t new_size = count + n;
			reserve(new_size);
			std::move_backward(pos, cend(), end() + n);
			std::move_backward(first, last, begin() + offset);
			count = new_size;
			return begin() + offset;
		}
		iterator insert(const_iterator pos, std::initializer_list<T> ilist)
		{ 
			return insert(pos, ilist.begin(), ilist.end());
		}
		
		iterator erase(const_iterator pos)
		{
			iterator it1 = begin() + (pos - cbegin());
			iterator it2 = it1 + 1;

			if (it2 != end()) std::move(it2, end(), it1);
			else std::destroy_at<T>(&*it1);

			--count;
			return it1;
		}
		iterator erase(const_iterator first, const_iterator last)
		{
			size_t n = (last - first);
			iterator it1 = begin() + (first - cbegin());
			iterator it2 = it1 + (last - cbegin());

			if (it2 != end()) std::move(it2, end(), it1);
			else std::for_each(it1, it2, [](T& elem) { std::destroy_at<T>(&elem); });
			
			count -= n;
			return it1;
		}
		
		void push_back(const T& value)
		{ 
			size_t index = count; 
			reserve(++count);
			size_t page_index = page_count - 1;
			pages[page_index][index - page_index * page_size] = value;
		}
		void push_back(T&& value)
		{ 
			size_t index = count; 
			reserve(++count); 
			size_t page_index = page_count - 1;
			pages[index / page_size][index % page_size] = std::move(value);
		}
		void pop_back()
		{
			std::destroy_at(&back());
			--count;
		}
		void resize(size_t n) 
		{
			reserve(n);
			
			if (n < count)
				std::destroy_n(begin(), n);
			else
				std::for_each_n(begin() + count, n, [](T& value) { std::construct_at<T>(&value); });
			
			count = n;
		}
		void swap(paged_vector<T>& other)
		{
			size_t temp_count = count;
			size_t temp_page_count = page_count;
			size_t temp_page_capacity = page_capacity;
			page* temp_pages = pages;

			count = other.count;
			page_count = other.page_count;
			page_capacity = other.page_capacity;
			pages = other.pages;

			other.count = temp_count;
			other.page_count = temp_page_count;
			other.page_capacity = temp_page_capacity;
			other.pages = temp_pages;
		}
		
		Alloc_T& get_allocator() 
		{
			return alloc;
		}
		Page_Alloc_T& get_page_allocator() 
		{
			return page_alloc;
		}
	private:
		Alloc_T alloc;
		Page_Alloc_T page_alloc;

		size_t count;
		size_t page_count;
		size_t page_capacity;
		page* pages;
	};

	template<typename T>
	class paged_vector_iterator 
	{
		static constexpr size_t page_size = 4096;
	public:
		using container_type = paged_vector<std::remove_const_t<T>>;
		using iterator_category = std::random_access_iterator_tag;
		using data_type = std::remove_const_t<T>**;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using difference_type = std::ptrdiff_t;
		
		friend class container_type::iterator;
		friend class container_type::const_iterator;

		paged_vector_iterator(size_t index, data_type pages) : index(index), pages(pages) { }
		
		operator paged_vector_iterator<const T>() const 
		{ 
			return paged_vector_iterator<const T>{ index, pages }; 
		}

		reference operator*()
		{ 
			return pages[index / page_size][index % page_size]; 
		}
		
		paged_vector_iterator<T>& operator++()
		{
			++index; return *this; 
		}
		paged_vector_iterator<T> operator++(int)
		{
			paged_vector_iterator<T> temp = *this; ++index; return temp; 
		}
		paged_vector_iterator<T>& operator--()
		{ 
			--index; return *this; 
		}
		paged_vector_iterator<T> operator--(int)
		{
			paged_vector_iterator<T> temp = *this; --index; return temp; 
		}
		paged_vector_iterator<T> operator+(int n)
		{
			return { index + n, pages }; 
		}
		paged_vector_iterator<T> operator-(int n)
		{
			return { index - n, pages }; 
		}
		paged_vector_iterator<T>& operator+=(int n)
		{
			index += n; return *this; 
		}
		paged_vector_iterator<T>& operator-=(int n)
		{
			index -= n; return *this; 
		}
		
		difference_type operator-(const paged_vector_iterator<T>& other)
		{
			return index - other.index; 
		}
		
		bool operator==(const paged_vector_iterator<T>& other)
		{ 
			return index == other.index; 
		}
		bool operator!=(const paged_vector_iterator<T>& other)
		{ 
			return index != other.index; 
		}
		bool operator<(const paged_vector_iterator<T>& other)
		{ 
			return index < other.index; 
		}
		bool operator>(const paged_vector_iterator<T>& other)
		{ 
			return index > other.index; 
		}
		bool operator<=(const paged_vector_iterator<T>& other)
		{ 
			return index <= other.index; 
		}
		bool operator>=(const paged_vector_iterator<T>& other)
		{ 
			return index >= other.index; 
		}
		
	private:
		size_t index;
		data_type pages;
	};
}