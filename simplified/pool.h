#pragma once
#include "traits.h"
namespace ecs {

	template<typename T, typename reg_T>
	class pool { 
	public:
		pool(reg_T& reg);
		constexpr pool& operator=(std::initializer_list<entity<T>> ilist);

		template<std::input_iterator It>
		void assign(It first, It last);
		void assign(size_t n, const T& value);
		void assign(std::initializer_list<T> ilist);

		// iterators
		constexpr iterator begin() noexcept;
		constexpr const_iterator begin() const noexcept;
		constexpr iterator end() noexcept;
		constexpr const_iterator end() const noexcept;
		constexpr reverse_iterator rbegin() noexcept;
		constexpr const_reverse_iterator rbegin() const noexcept;
		constexpr reverse_iterator rend() noexcept;
		constexpr const_reverse_iterator rend() const noexcept;

		constexpr const_iterator cbegin() const noexcept;
		constexpr const_iterator cend() const noexcept;
		constexpr const_reverse_iterator crbegin() const noexcept;
		constexpr const_reverse_iterator crend() const noexcept;

		// capacity
		[[nodiscard]] constexpr bool empty() const noexcept;
		constexpr size_t max_size() const noexcept;
		constexpr size_t size() const;
		constexpr size_t capacity() const;
		constexpr void reserve(size_t n);
		constexpr void shrink_to_fit();

		// element/page/data access
		constexpr reference operator[](size_t index);
		constexpr const reference operator[](size_t index) const;
		constexpr reference at(size_t pos);
		constexpr const reference at(size_t pos) const;
		constexpr reference front();
		constexpr const reference front() const;
		constexpr reference back();
		constexpr const reference back() const;
		constexpr page_type* data() noexcept;
		constexpr const page_type* data() const noexcept;

		// modifiers
		template<typename ... Arg_Ts>
		constexpr reference emplace_back(entity<T> e, Arg_Ts&&... args);
		constexpr void push_back(entity<T> e, const T& value);
		constexpr void push_back(entity<T> e, T&& value);
		constexpr void pop_back();
		template<typename ... Arg_Ts>
		constexpr iterator emplace(const_iterator pos, entity<T> e, Arg_Ts&&... args);
		constexpr iterator insert(const_iterator pos, entity<T> e, const T& value);
		constexpr iterator insert(const_iterator pos, entity<T> e, T&& value);
		template<std::input_iterator It>
		constexpr iterator insert(const_iterator pos, It first, It last);
		constexpr iterator insert(const_iterator pos, std::initializer_list<entity<T>> ilist);
		constexpr iterator erase(const_iterator pos, size_t n=1);
		constexpr iterator erase(const_iterator first, const_iterator last);
		
		constexpr void clear() noexcept;

	private:
		reg_T& reg;
	};}