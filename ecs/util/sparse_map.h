#pragma once
#include <memory>
#include <exception>
#include <cassert>
#include <algorithm>
#include "next_pow2.h"
namespace util {
	template<std::integral val_t>
	class sparse_map {
	public:
		static constexpr val_t tombstone = -1;
		static constexpr size_t page_size = 4096;

		sparse_map(size_t new_page_count = 8) : page_capacity(new_page_count) {
			pages = std::allocator<val_t*>().allocate(page_capacity);
			std::fill(pages, pages + page_capacity, nullptr);
		}
		sparse_map(sparse_map& other);
		sparse_map& operator=(sparse_map& other);
		sparse_map(sparse_map&& other);
		sparse_map& operator=(sparse_map&& other);
		~sparse_map() {
			std::for_each(pages, pages + page_capacity, [](val_t* page) { 
				if (page != nullptr) std::allocator<val_t>().deallocate(page, page_size); 
			});
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
		
		val_t** pages;
		size_t  page_capacity;
	};
}