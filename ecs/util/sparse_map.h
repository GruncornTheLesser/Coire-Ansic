#pragma once
#include <memory>
#include <exception>

namespace util {
	template<typename val_t, val_t tombstone = static_cast<val_t>(-1)>
	class sparse_map {
	public:
		sparse_map(size_t new_page_count = 8) : page_count(new_page_count) {
			pages = std::allocator<size_t*>().allocate(page_count);
			std::fill(pages, pages + page_count, nullptr);
		}

		~sparse_map() {
			for (size_t **cur = pages, **end = pages + page_count; cur != end; ++cur) {
				if (*cur != nullptr) std::allocator<size_t>().deallocate(*cur, page_size);
			}
			std::allocator<size_t*>().deallocate(pages, page_size);
		}
		
		size_t& operator[](int key) {
			size_t page_index = key / page_size;			
			size_t elem_index = key % page_size; 
			return assure_page(page_index)[elem_index];
		}

		const size_t& operator[](int key) const {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			
			if (page_index > page_count || pages[page_index] == nullptr) 
				throw std::out_of_range("");
			
			return pages[page_index][elem_index];
		} 
		
		size_t& at(int key) {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			if (page_index > page_count || pages[page_index] == nullptr) 
				throw std::out_of_range("");
			return pages[page_index][elem_index];
		}

		size_t at(int key) const {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			if (page_index > page_count || pages[page_index] == nullptr) 
				tombstone;
			return pages[page_index][elem_index];
		}

		void reserve(int key) {
			assure_page(key / page_size);
		}

		size_t size() const { 
			return page_count * page_size;
		}
		
		bool contains(int key) const {
			size_t page_index = key / page_size;
			size_t elem_index = key % page_size;
			return page_index <= page_count && pages[page_index] != nullptr && pages[page_index][elem_index] != tombstone;
		}
		
	private:
		size_t* assure_page(size_t page_index) {
			if (page_index >= page_count) 
			{	
				size_t new_page_count = page_index + 1;
				size_t** new_pages = std::allocator<size_t*>().allocate(new_page_count);
				
				std::copy(pages, pages + page_count, new_pages);
				std::fill(new_pages + page_count, new_pages + new_page_count, nullptr);

				std::allocator<size_t*>().deallocate(pages, page_count);

				pages = new_pages;
				page_count = new_page_count;
			}

			size_t* page = pages[page_index];
		
			if (page != nullptr) return page;
			
			page = std::allocator<size_t>().allocate(page_size);
			std::fill(page, page + page_size, tombstone);
			pages[page_index] = page;
			
			return page;
		}
	
		static constexpr size_t page_size = 4096;
	
		size_t** pages;
		size_t   page_count;
	};
}