#pragma once
#include "util/paged_vector.h"
#include "entity.h"
namespace ecs {
	
	// TODO: make custom type
	typedef uint32_t version;
	
	template<typename T>
	struct version_pair
	{
		template<typename storage_T>
		friend class version_view;

	public:
		template<typename ... Arg_Ts>
		version_pair(Arg_Ts ... args) : value(std::forward<Arg_Ts>(args)...), version(0), next(nullptr) { }
		// ? maybe this should be an explicit get func. 
		// ? would maybe get syntax like: so storage[i].get(), handler[i].get(), indexer[i].get()
		operator T&() { return value; } 
		operator const T&() const { return value; }
		//operator T() const { return value; }
	private:
		T value;
		version version;
		version_pair<T>* next;
	};

	template<>
	struct version_pair<void>
	{
		version version = 0;
		version_pair* next = nullptr;
	};
}

namespace ecs {
	template<typename T>
	class version_view_iterator;

	template<typename T>
	class version_view
	{
	public:
		using value_type = version_pair<T>;
		using reference = value_type&;
		using pointer = value_type*;
		using const_reference = const value_type&;
		using iterator = version_view_iterator<value_type>;
		using const_iterator = version_view_iterator<const value_type>;

		version_view() : current_version(0), head(nullptr) { }
		
		iterator begin() { return { head, current_version }; }
		const_iterator begin() const { return { head, current_version }; }
		const_iterator cbegin() const { return { head, current_version }; }

		iterator end() { return { nullptr, current_version }; }
		const_iterator end() const { return { nullptr, current_version }; }
		const_iterator cend() const { return { nullptr, current_version }; }
		
		reference pop() 
		{ 
			--head->version;
			pointer temp = head;
			head = head->next;
			return *temp;
		}
		
		void push_back(reference elem)
		{ 
			if (elem.version == current_version) return; 

			elem.next = head;
			head = &elem;
			elem.version = current_version;
		}

		bool active(reference elem) 
		{
			return elem.version == current_version; 
		}
		
		void clear()
		{
			++current_version;
			head = nullptr;
		}

		bool empty() const 
		{ 
			return head == nullptr;
		}

	private:
		version current_version;
		version_pair<T>* head;
	};

	template<typename T>
	class version_view_iterator 
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;
		using difference_type = std::ptrdiff_t;
		
		version_view_iterator(pointer ptr, version vers) : ptr(ptr), vers(vers) { }
		
		operator version_view_iterator<const T>() const { return { ptr, vers }; }

		reference operator*() { return *ptr; }
		
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
		pointer ptr;
		version vers;
	};
}