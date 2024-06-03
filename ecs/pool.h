#pragma once

#include "util/sparse_map.h"
#include "util/paged_vector.h"
#include "util/next_pow2.h"
#include "entity.h"
#include "traits.h"
#include "util/type_name.h"
#include <shared_mutex>
#include <iostream>




namespace ecs {
	struct resource {
		void acquire() { mtx.lock(); }
		void release() { mtx.unlock(); };
		void acquire() const { mtx.lock_shared(); };
		void release() const { mtx.unlock_shared(); };
	private:
		mutable std::shared_mutex mtx;
	};
	
	using version = uint32_t;

	template<typename T>
	class version_storage_iterator;

	template<typename T>
	struct data_version_pair {
		T value;
		version version;
		data_version_pair* next;
	};

	template<typename T, template<typename> typename container_template = std::vector>
	struct versioned_storage : public container_template<data_version_pair<T>> {
		struct version_view_t {
			using reference = T&;
			using const_reference = const T&;
			using iterator = version_storage_iterator<T>;
			using const_iterator = version_storage_iterator<const T>;

			version_view_t(versioned_storage& base) : base(base) { }
			
			iterator begin() { return { base.next, base.version }; }
			const_iterator begin() const { return iterator{ base.next, base.version }; }
			const_iterator cbegin() const { return iterator{ base.next, base.version }; }

			iterator end() { return { nullptr, base.version }; }
			const_iterator end() const { return iterator{ nullptr, base.version }; }
			const_iterator cend() const { return iterator{ nullptr, base.version }; }
			
		private:
			versioned_storage& base;
		};
		using countainer = container_template<data_version_pair<T>>;
		void get_version(uint32_t index) {
			return countainer::at(index).version;
		}
		void update(uint32_t index) 
		{
			T& elem = countainer::at(index);
			if (elem.version == current_version) return;
			elem.next = next;
			next = elem;
		}
		void clear_updates() 
		{ 
			++current_version;
			next = nullptr;
		}
		version_view_t version_view()
		{ 
			return version_view(*this); 
		}
		
	private:
		version current_version;
		data_version_pair<entity>* next;
	};

	template<typename T>
	class versioned_storage_iterator {
		
		using container_type = util::paged_vector<std::remove_const_t<T>>;
		using iterator_category = std::forward_iterator_tag;
		using data_type = std::remove_const_t<T>**;
		using value_type = T;
		using pointer = T*;
		using reference = T&;
		using difference_type = std::ptrdiff_t;
		
		friend class container_type::iterator;
		friend class container_type::const_iterator;

		versioned_storage_iterator(data_version_pair<T>* curr, version version) : curr(curr), version(version) { }
		
		operator versioned_storage_iterator<const T>() const { return versioned_storage_iterator<const T>{ curr, version }; }

		reference operator*() { return curr->value; }
		
		versioned_storage_iterator<T>& operator++() 
		{ 
			curr = curr->version == version ? curr -> next : nullptr; 
			return *this;
		}
		versioned_storage_iterator<T> operator++(int) { 
			versioned_storage_iterator<T> temp = *this;
			curr = curr->version == version ? curr -> next : nullptr; 
			return temp;
		}
		
		bool operator==(const versioned_storage_iterator<T>& other) { return curr == other.curr; }
		bool operator!=(const versioned_storage_iterator<T>& other) { return curr != other.curr; }
		
	private:
		data_version_pair<T>* curr;
		version version;
	};
	
	template<typename T> struct indexer : traits::get_indexer_t<T>, resource { };
	template<typename T> struct storage : traits::get_storage_t<T>, resource { };
	template<typename T> struct handler : traits::get_handler_t<T>, resource { };

	template<typename T> struct traits::default_indexer { using type = util::sparse_map<ecs::entity>; };
	template<typename T> struct traits::default_storage { using type = util::paged_vector<T>; };
	template<typename T> struct traits::default_handler { using type = versioned_storage<ecs::entity, util::paged_vector>; };
	template<typename T> struct traits::default_sequence_policy { using type = int; };
	template<typename T> struct traits::default_order_policy { using type = int; };
	template<typename ... Ts> struct resource_set_t { using resource_set = std::tuple<Ts...>; }; 
	template<typename T> struct traits::default_pool_resource_set { using type = resource_set_t<indexer<T>, storage<T>, handler<T>>; };
	
	
	
	
	// TODO: pool view
	template<typename T, typename Pip_T> // handler, indexer, storage
	struct pool_view {

	};
}


			// entity pair
			// entity | next = 0000FFFF | FFFF0000
			// narrowing to 4 bytes converts to entity 
			// uses upper 4 bytes to store index to next update
			// defaults to FFFF to mean negative 1 so no next

			// no

			// just us std::pair<entity, uint32_t>

			// index -1 -> back

			// update list -> how:
			//	when entity changes must point to push to back of update list
			// 	iterate through update list perform operations
			//

			// [entities..., update list]???
			// create, destroy, move
			// create, create, list

			//	for i in entity index update list: 
			//		curr = i
			// 		prev = index[entity[curr]]; 
			//		while curr != prev:
			//			if prev == -1: // created new entity
			//				index[entity[curr]] = curr
			//				break
			//			swap(components[curr], components[prev]);
			//			swap(index[entity[i]], curr);

