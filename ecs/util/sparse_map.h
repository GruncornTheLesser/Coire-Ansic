#pragma once
#include <memory>
#include <exception>
#include <cassert>
#include <algorithm>
#include "next_pow2.h"
namespace util {
	template<typename val_t, val_t tombstone = static_cast<val_t>(-1)>
	class sparse_map {
	public:
		sparse_map(size_t new_page_count = 8) : page_capacity(new_page_count) {
			pages = std::allocator<val_t*>().allocate(page_capacity);
			std::fill(pages, pages + page_capacity, nullptr);
		}

		~sparse_map() {
			for (val_t **cur = pages, **end = pages + page_capacity; cur != end; ++cur) {
				if (*cur != nullptr) std::allocator<val_t>().deallocate(*cur, page_size);
			}
			std::allocator<val_t*>().deallocate(pages, page_size);
		}
		
		val_t& operator[](int key) {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			return assure_page(page_index)[elem_index];
		}

		const val_t& operator[](int key) const {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			
			if (page_index > page_capacity || pages[page_index] == nullptr) 
				throw std::out_of_range("");
			
			return pages[page_index][elem_index];
		} 
		
		val_t& at(int key) {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			if (page_index > page_capacity || pages[page_index] == nullptr) 
				throw std::out_of_range("");
			return pages[page_index][elem_index];
		}

		const val_t& at(int key) const {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			if (page_index > page_capacity || pages[page_index] == nullptr) 
				tombstone;
			return pages[page_index][elem_index];
		}

		void reserve(int key) {
			assure_page(key / page_size);
		}

		size_t size() const { 
			return page_capacity * page_size;
		}
		
		bool contains(size_t key) const {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			return page_index <= page_capacity && pages[page_index] != nullptr && pages[page_index][elem_index] != tombstone;
		}
		
	private:
		val_t* assure_page(size_t page_index) {
			if (page_index >= page_capacity) 
			{	
				size_t new_page_count = page_index + 1;
				val_t** new_pages = std::allocator<val_t*>().allocate(new_page_count);
				
				std::copy(pages, pages + page_capacity, new_pages);
				std::fill(new_pages + page_capacity, new_pages + new_page_count, nullptr);

				std::allocator<val_t*>().deallocate(pages, page_capacity);

				pages = new_pages;
				page_capacity = new_page_count;
			}

			val_t* page = pages[page_index];
		
			if (page != nullptr) return page;
			
			page = std::allocator<val_t>().allocate(page_size);
			std::fill(page, page + page_size, tombstone);
			pages[page_index] = page;
			
			return page;
		}
	
		static constexpr size_t page_size = 4096;
	
		val_t** pages;
		size_t  page_capacity;
	};

	template<typename T> class paged_block;
	template<typename T> class paged_block_iterator;

	template<typename T>
	class paged_block {
	public:
		static constexpr size_t page_size = 4096;
	private:
		using page = T*;
		
		using reference = T&;
		using const_reference = const T&;
		
		using iterator = paged_block_iterator<T>;
		using const_iterator = paged_block_iterator<const T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

	public:
		paged_block(size_t page_capacity = 8, size_t page_count = 1) : page_capacity(page_capacity), page_count(page_count), pages(std::allocator<page>().allocate(page_capacity)) { 
			std::for_each(pages, pages + page_count, [](page& page) { page = std::allocator<T>().allocate(page_size); });
		}
		
		~paged_block() {
			std::for_each(pages, pages + page_count, [](page p) { std::allocator<T>().deallocate(p, page_size); });
			std::allocator<page>().deallocate(pages, page_capacity);
		}

		iterator begin() { return { 0, pages }; }
		iterator end() { return { page_count * page_size, pages }; }
		const_iterator begin() const { return iterator{ 0, pages }; }
		const_iterator end() const { return iterator{ page_count * page_size, pages }; }
		
		reverse_iterator rbegin() { return iterator{ page_count * page_size - 1, pages }; }
		reverse_iterator rend() { return iterator{ -1, pages }; }
		
		const_reverse_iterator rbegin() const { return iterator{ page_count * page_size - 1, pages }; }
		const_reverse_iterator rend() const { return iterator{ -1, pages }; }
		
		const_reverse_iterator crbegin() const { return iterator{ page_count * page_size - 1, pages }; }
		const_reverse_iterator crend() const { return iterator{ -1, pages }; }
		
		size_t capacity() const { return page_count * page_size; }
		
		reference operator[](size_t index) { return pages[index / page_size][index % page_size]; }
		const_reference operator[](size_t index) const { return pages[index / page_size][index % page_size]; }

		void reserve(size_t n)
		{
			size_t new_page_count = n / page_size + 1;

			if (new_page_count > page_count)
			{
				if (new_page_count > page_capacity) {
					size_t new_page_capacity = util::next_pow2(new_page_count);

					page* new_pages = std::allocator<page>().allocate(new_page_capacity);
					std::move(pages, pages + page_count, new_pages);
					
					std::allocator<page>().deallocate(pages, page_capacity);

					page_capacity = new_page_capacity;
					pages = new_pages;
				}

				std::for_each(pages + page_count, pages + new_page_count, [](page& page) { page = std::allocator<T>().allocate(page_size); });
			}
			else if (new_page_count + 1 < page_count)
			{
				new_page_count += 1;
				std::for_each(pages + new_page_count, pages + page_count, [](page page) { std::allocator<T>().deallocate(page, page_size); });
			
			}

			page_count = new_page_count;
		}

		size_t size() const { return page_size * page_count; }

	private:
		size_t page_capacity;
		size_t page_count;
		page* pages;
	};

	template<typename T>
	class paged_block_iterator {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
        using pointer = T*;
        using reference = T&;
		using difference_type = std::ptrdiff_t;
		
		paged_block_iterator(size_t index, T** pages) : index(index), pages(pages) { }
		
		reference operator*() { return pages[index / paged_block<T>::page_size][index % paged_block<T>::page_size]; }
		pointer operator->() { return this->operator*(); }
		
		paged_block_iterator<T> operator++() { ++index; return *this; }
		paged_block_iterator<T> operator++(int) { paged_block_iterator<T> temp = *this; ++index; return temp; }
		paged_block_iterator<T> operator--() { --index; return *this; }
		paged_block_iterator<T> operator--(int) { paged_block_iterator<T> temp = *this; --index; return temp; }
		paged_block_iterator<T> operator+(int n) { return { index + n, pages }; }
		paged_block_iterator<T> operator-(int n) { return { index - n, pages }; }
		
		difference_type operator-(const paged_block_iterator<T>& other) { return index - other.index; }
		
		bool operator==(const paged_block_iterator<T>& other) { return index == other.index; }
		bool operator!=(const paged_block_iterator<T>& other) { return index != other.index; }
		bool operator<(const paged_block_iterator<T>& other) { return index < other.index; }
		bool operator>(const paged_block_iterator<T>& other) { return index > other.index; }
		bool operator<=(const paged_block_iterator<T>& other) { return index <= other.index; }
		bool operator>=(const paged_block_iterator<T>& other) { return index >= other.index; }
		
	private:
		size_t index;
		T** pages;
	};

	template<typename T> class unpaged_block;
	template<typename T> class unpaged_block_iterator;

	template<typename T> 
	class unpaged_block {
	public:
		using reference = T&;
		using const_reference = const T&;
		
		using iterator = unpaged_block_iterator<T>;
		using const_iterator = unpaged_block_iterator<const T>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;

		unpaged_block(size_t cap = 4096) : cap(cap), data(std::allocator<T>().allocate(cap)) { }
		
		~unpaged_block() {
			std::allocator<T>().deallocate(data, cap);
		}

		iterator begin() { return { data }; }
		iterator end() { return { data + cap }; }
		const_iterator begin() const { return { data }; }
		const_iterator end() const { return { data + cap }; }
		
		reverse_iterator rbegin() { return iterator{ data + cap }; }
		reverse_iterator rend() { return iterator{ data - 1 }; }
		const_reverse_iterator rbegin() const { return const_iterator { data + cap - 1 }; }
		const_reverse_iterator rend() const { return const_iterator { data - 1 }; }
		
		reference operator[](size_t index) { return data[index]; }
		const_reference operator[](size_t index) const { return data[index]; }

		void reserve(size_t n, size_t prev_size_hint = 0)
		{
			if (n > cap) {
				size_t new_cap = util::next_pow2(n);
				T* new_data = std::allocator<T>().allocate(new_cap);
				std::move(data, data + prev_size_hint, new_data);
				std::allocator<T>().deallocate(data, cap);
				data = new_data;
				cap = new_cap;
			}
		}

		size_t capacity() const { return cap; }
	private:
		size_t cap;
		T* data;
	};

	template<typename T>
	class unpaged_block_iterator {
	public:
		using iterator_category = std::random_access_iterator_tag;
		using value_type = T;
        using pointer = T*;
        using reference = T&;
		using difference_type = std::ptrdiff_t;

		unpaged_block_iterator(T* ptr) : ptr(ptr) { }

		reference operator*() { return *ptr; }
		pointer operator->() { return ptr; }
		
		unpaged_block_iterator<T> operator++() { ++ptr; return *this; }
		unpaged_block_iterator<T> operator++(int) { unpaged_block_iterator<T> temp = *this; ++ptr; return temp; }
		unpaged_block_iterator<T> operator--() { --ptr; return *this; }
		unpaged_block_iterator<T> operator--(int) { unpaged_block_iterator<T> temp = *this; --ptr; return temp; }
		unpaged_block_iterator<T> operator+(int n) { return { ptr + n }; }
		unpaged_block_iterator<T> operator-(int n) { return { ptr - n }; }
		
		difference_type operator-(const unpaged_block_iterator& other) { return ptr - other.ptr; }
		
		bool operator==(const unpaged_block_iterator<T>& other) { return ptr == other.ptr; }
		bool operator!=(const unpaged_block_iterator<T>& other) { return ptr != other.ptr; }
		bool operator<(const unpaged_block_iterator<T>& other) { return ptr < other.ptr; }
		bool operator>(const unpaged_block_iterator<T>& other) { return ptr > other.ptr; }
		bool operator<=(const unpaged_block_iterator<T>& other) { return ptr <= other.ptr; }
		bool operator>=(const unpaged_block_iterator<T>& other) { return ptr >= other.ptr; }
		
	private:
		T* ptr;
	};

}