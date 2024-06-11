#pragma once 
#include <shared_mutex>
#include "util/paged_vector.h"
#include "entity.h"
#include "pool_traits.h"
#include "resource.h"
namespace ecs {
	typedef uint32_t version;
	
	struct version_pair 
	{
		version version;
		version_pair* next;
		entity ent;
	};

	struct version_manager : util::paged_vector<version_pair>
	{
		version current_version;
		version_pair* head;
	};

	template<typename T>
	class version_view_iterator;

	template<typename storage_T>
	class version_view
	{
		using value_type = typename storage_T::value_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		using iterator = version_view_iterator<value_type>;
		using const_iterator = version_view_iterator<const value_type>;

		version_view(storage_T& stor) : storage(stor) { }
		
		iterator begin() { return { storage->head, storage->current_version }; }
		const_iterator begin() const { return { storage->head, storage->current_version }; }
		const_iterator cbegin() const { return { storage->head, storage->current_version }; }

		iterator end() { return { nullptr, storage->current_version }; }
		const_iterator end() const { return { nullptr, storage->current_version }; }
		const_iterator cend() const { return { nullptr, storage->current_version }; }
		
		void update(size_t index) { 
			if (storage[index].version == storage.current_version) return; 
			storage[index].next = storage.head;
			storage.head = &storage[index].next;
			}
		void clear() { }
	private:
		storage_T& storage;
	};

	template<typename T>
	class version_view_iterator {
		using iterator_category = std::forward_iterator_tag;
		using value_type = ecs::entity;
		using pointer = value_type*;
		using reference = value_type&;
		using difference_type = std::ptrdiff_t;
		
		version_view_iterator(version_pair* ptr, version vers) : ptr(ptr), vers(vers) { }
		
		operator version_view_iterator<const T>() const { return { ptr, vers }; }

		reference operator*() { return ptr->ent; }
		
		version_view_iterator& operator++() 
		{ 
			if (ptr != nullptr) {
				if (ptr->version == vers) ptr = ptr->next;
				else ptr = nullptr;
			}
			return *this;
		}
		version_view_iterator operator++(int) { 
			version_view_iterator temp = *this;
			if (ptr != nullptr) {
				if (ptr->version == vers) ptr = ptr->next;
				else ptr = nullptr;
			}
			return temp;
		}
		
		bool operator==(const version_view_iterator& other) { return ptr == other.ptr; }
		bool operator!=(const version_view_iterator& other) { return ptr != other.ptr; }
		
	private:
		version_pair* ptr;
		version vers;
	};
	










	template<typename T>
	struct handler : util::paged_vector<version_pair>, resource { };

}