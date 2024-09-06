#pragma once
#include <memory>
#include <exception>
#include <cassert>
#include <algorithm>

// TODO: could also try hierarchal paging
// std::map has functions for iterating through the set of key hashes, it would fit quite nicely to align these with the pages
// could then apply a different algorithm for acquiring the value from the page
// every time I look at this file I have these impure thoughts - sparse mapping works because it is simple - I forbid you from over complicating it. again.

// TODO: tidy up sparse map and align with std::map interface
// template<typename key_t, typename val_t, val_t tombstone=val_t{}>

/*
(constructor) - constructs the map
(destructor) - destroy the map
operator= - values to the container
at - access specified element with bounds checking
operator[] - access or insert specified element

Capacity: // NOTE: currently it doesnt count capacity
empty - checks whether the container is empty
max_size - returns the maximum possible number of elements

Modifiers: // NOTE: a lot of these are unsuitable too, 
clear - clears the contents
insert - inserts elements or nodes
insert_range - inserts a range of elements
insert_or_assign - inserts an element or assigns to the current element if the key already exists
emplace - constructs element in-place
emplace_hint - constructs elements in-place using a hint
try_emplace - inserts in-place if the key does not exist, does nothing if the key exists
erase - erases elements
swap - swaps the contents
extract - extracts nodes from the container
merge - splices nodes from another container

Lookup:
count - returns the number of elements preding specific key
find - finds element with specific key
contains - checks if the container contains element with specific key

// NOTE: doesnt apply unless exposing page
equal_range - returns range of elements preding a specific key
lower_bound - returns an iterator to the first element not less than the given key
upper_bound - returns an iterator to the first element greater than the given key
Observers:
key_comp - returns the function that compares keys

value_comp - returns the function that compares keys in objects of type value_type

*/
namespace util {
	template<typename key_t, typename val_t, val_t tombstone=val_t{}> 
	class sparse_map 
	{
	public:
		static constexpr size_t page_size = 4096;

		sparse_map(size_t new_page_count = 8);
		~sparse_map();
		
		sparse_map(sparse_map<key_t, val_t, tombstone>& other);
		sparse_map<key_t, val_t, tombstone>& operator=(sparse_map<key_t, val_t, tombstone>& other);
		sparse_map(sparse_map<key_t, val_t, tombstone>&& other);
		sparse_map<key_t, val_t, tombstone>& operator=(sparse_map<key_t, val_t, tombstone>&& other);
		
		val_t& operator[](key_t key);
		const val_t& operator[](key_t key) const;
		
		val_t& at(key_t key);
		const val_t& at(key_t key) const;

		void reserve(key_t key);

		size_t capacity() const;
		
		bool contains(size_t key) const;

	private:
		val_t* assure_page(size_t page_index);

		val_t** pages;
		size_t  page_capacity;
	};
}

template<typename key_t, typename val_t, val_t tombstone>
util::sparse_map<key_t, val_t, tombstone>::sparse_map(size_t new_page_count) : page_capacity(new_page_count) {
	pages = std::allocator<val_t*>().allocate(page_capacity);
	std::fill(pages, pages + page_capacity, nullptr);
}


template<typename key_t, typename val_t, val_t tombstone>
util::sparse_map<key_t, val_t, tombstone>::~sparse_map() 
{
	if (pages != nullptr)
	{
		std::for_each(pages, pages + page_capacity, [](val_t* page) { 
			if (page != nullptr) std::allocator<val_t>().deallocate(page, page_size); 
		});
		std::allocator<val_t*>().deallocate(pages, page_size);
	}
}
		

////template<typename K, typename V, V T>
////util::sparse_map<K, V, T>::sparse_map(sparse_map<K, V, T>& other);

////template<typename K, typename V, V T>
////util::sparse_map<K, V, T>& util::sparse_map<K, V, T>::operator=(sparse_map<K, V, T>& other);

template<typename key_t, typename val_t, val_t tombstone>
util::sparse_map<key_t, val_t, tombstone>::sparse_map(sparse_map&& other) 
{
	pages = other.pages;
	page_capacity = other.page_capacity;
	other.pages = nullptr;
	other.page_capacity = 0;
}

template<typename key_t, typename val_t, val_t tombstone>
util::sparse_map<key_t, val_t, tombstone>& util::sparse_map<key_t, val_t, tombstone>::operator=(sparse_map<key_t, val_t, tombstone>&& other) 
{ 
	std::swap(pages, other.pages);
	std::swap(page_capacity, other.page_capacity);
}




template<typename key_t, typename val_t, val_t tombstone>
val_t& util::sparse_map<key_t, val_t, tombstone>::operator[](key_t key) 
{
	size_t page_index = key / page_size;
	size_t elem_index = key % page_size;
	return assure_page(page_index)[elem_index];
}


template<typename key_t, typename val_t, val_t tombstone>
const val_t& util::sparse_map<key_t, val_t, tombstone>::operator[](key_t key) const {
	size_t page_index = key / page_size;
	size_t elem_index = key % page_size;
	
	//if (page_index > page_capacity || pages[page_index] == nullptr) 
	//	throw std::out_of_range("");
	
	return pages[page_index][elem_index];
}
		
template<typename key_t, typename val_t, val_t tombstone>
val_t& util::sparse_map<key_t, val_t, tombstone>::at(key_t key) 
{
	size_t page_index = key / page_size;
	size_t elem_index = key % page_size;
	if (page_index > page_capacity || pages[page_index] == nullptr) 
		throw std::out_of_range("");
	return pages[page_index][elem_index];
}

template<typename key_t, typename val_t, val_t tombstone>
const val_t& util::sparse_map<key_t, val_t, tombstone>::at(key_t key) const 
{
	size_t page_index = key / page_size;
	size_t elem_index = key % page_size;
	// ? error handling?? -> 
	//if (page_index > page_capacity || pages[page_index] == nullptr) 
	// 		throw error???
	return pages[page_index][elem_index];
}

template<typename key_t, typename val_t, val_t tombstone>
void util::sparse_map<key_t, val_t, tombstone>::reserve(key_t key) 
{
	assure_page(key / page_size);
}

template<typename key_t, typename val_t, val_t tombstone>
size_t util::sparse_map<key_t, val_t, tombstone>::capacity() const { 
	return page_capacity * page_size;
}

template<typename key_t, typename val_t, val_t tombstone>	
bool util::sparse_map<key_t, val_t, tombstone>::contains(size_t key) const 
{
	size_t page_index = key / page_size;
	size_t elem_index = key % page_size;
	return page_index <= page_capacity && pages[page_index] != nullptr && pages[page_index][elem_index] != tombstone;
}

template<typename key_t, typename val_t, val_t tombstone>		
val_t* util::sparse_map<key_t, val_t, tombstone>::assure_page(size_t page_index)
{
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