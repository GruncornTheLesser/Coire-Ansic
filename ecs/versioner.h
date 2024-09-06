#pragma once
#include <stdint.h>

/*
!!! MUST EXTRACT index easily from  
TODO: 

*/

namespace ecs 
{
	using version = uint32_t;
	
	template<typename T>
	struct versioned
	{
		template<typename> friend class versioner;
		template<typename> friend class versioner_iterator;

	public:
		template<typename ... Arg_Ts>
		versioned(Arg_Ts ... args) : value(std::forward<Arg_Ts>(args)...), next(nullptr) { }

		operator T&() { return value; } 
		operator const T&() const { return value; }
	private:
		T value;
		versioned<T>* next;
	};

	template<>
	struct versioned<void>
	{
		template<typename> friend class versioner;
		template<typename> friend class versioner_iterator;

	private:
		version version = 0;
		versioned* next = nullptr;
	};

	template<typename T>
	class versioner_iterator;

	template<typename T>
	class versioner final
	{
	public:
		using value_type = versioned<T>;
		using pointer = value_type*;
		using reference = value_type&;
		using const_pointer = const pointer;
		using const_reference = const reference;
		using iterator = versioner_iterator<versioned<T>>;
		using const_iterator = versioner_iterator<const versioned<T>>;

		versioner() : current_version(1), head(head) { }
		
		iterator begin() { return { head, current_version }; }
		const_iterator begin() const { return { head, current_version }; }
		const_iterator cbegin() const { return { head, current_version }; }

		iterator end() { return { nullptr, current_version }; }
		const_iterator end() const { return { nullptr, current_version }; }
		const_iterator cend() const { return { nullptr, current_version }; }
		
		reference pop()
		{
			// --head->version; // version assigned to null_version
			pointer temp = head;
			head = head->next;
			return *temp;
		}
		
		void update(reference elem)
		{
			if (elem.version == current_version) return;

			elem.next = head;
			head = &elem;
		}

		void clear()
		{
			++current_version;
			head = nullptr;
		}
		
		bool contains(const_reference elem) const
		{
			return elem.version == current_version;
		}
		
		bool empty() const 
		{ 
			return head == nullptr;
		}

	private:
		version current_version;
		versioned<T>* head;
	};

	template<typename T>
	class versioner_iterator
	{
		template<typename> friend class versioner;

		using iterator_category = std::forward_iterator_tag;
		using value_type = T;
		using pointer = value_type*;
		using reference = value_type&;
		using difference_type = std::ptrdiff_t;
		
		versioner_iterator(pointer ptr, version vers) : ptr(ptr), vers(vers) { }
	
	public:
		operator versioner_iterator<const T>() const { return { ptr, vers }; }

		reference operator*() { return *ptr; }
		
		versioner_iterator& operator++() 
		{ 
			if (ptr != nullptr) {
				if (ptr->version == vers) ptr = ptr->next;
				else ptr = nullptr;
			}
			return *this;
		}
		versioner_iterator operator++(int) { 
			versioner_iterator temp = *this;
			if (ptr != nullptr) {
				if (ptr->version == vers) ptr = ptr->next;
				else ptr = nullptr;
			}
			return temp;
		}
		
		bool operator==(const versioner_iterator& other) { return ptr == other.ptr; }
		bool operator!=(const versioner_iterator& other) { return ptr != other.ptr; }
		
	private:
		pointer ptr;
		version vers;
	};

}