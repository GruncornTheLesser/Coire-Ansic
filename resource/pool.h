#pragma once
#include <mutex>
#include <condition_variable>
#include <bitset>
#include <iostream>
#include <cassert>
#include "traits.h"
#include "../tuple_util/tuple_util.h"

namespace resource 
{
	enum class priority { NONE=-1, LOW=0, MEDIUM=1, HIGH=2 };

	template<typename T, size_t N>
	class locked_ptr_t;

	template<typename T>
	using locked_ptr = locked_ptr_t<traits::get_type_t<T>, traits::get_inst_count_v<T>>;

	template<typename T, size_t N>
	class pool_t;

	template<typename T>
	using pool = util::copy_cv_t<pool_t<std::remove_cv_t<traits::get_type_t<T>>, traits::get_inst_count_v<T>>, T>;

	template<typename T, size_t N>
	class pool_t 
	{
		friend class locked_ptr_t<T, N>;
		friend class locked_ptr_t<const T, N>;
		friend class locked_ptr_t<volatile T, N>;
		friend class locked_ptr_t<const volatile T, N>;

		void notify_next() 
		{
			for (int i=2; i >= 0; --i)
			{ 
				if (exclusive_wait_count[i])
				{
					exclusive_cv[i].notify_one();
					return;
				}
				
				if (constant_wait_count[i])
				{
					constant_cv.notify_all();
					return;
				}

				if (volatile_wait_count[i])
				{
					constant_cv.notify_all();
					return;
				}
			}
		}

		uint32_t find_inactive() 
		{
			for (int i=0; i<N; ++i)
				if (ref_count[i]==0)
					return i;
			return static_cast<uint32_t>(-1);
		}
		
		std::mutex mtx;

		uint32_t ref_count[N];
		uint32_t exclusive_wait_count[3];
		uint32_t constant_wait_count[3];
		uint32_t volatile_wait_count[3];

		std::condition_variable exclusive_cv[3];
		std::condition_variable constant_cv;
		std::condition_variable volatile_cv;

		uint32_t last_released = static_cast<int>(-1);
		uint32_t constant_index = static_cast<int>(-1);
		uint32_t volatile_index = static_cast<int>(-1);

		T data[N];
	};

	template<typename T, size_t N>
	class locked_ptr_t
	{
		using pool_type = pool_t<std::remove_cv_t<T>, N>;

		
		static constexpr enum { VOLATILE, CONSTANT, EXCLUSIVE } access = std::is_volatile_v<T> ? VOLATILE : (std::is_const_v<T> ? CONSTANT : EXCLUSIVE);
		
		template<typename U>
		static constexpr bool is_copyable = (std::is_const_v<T> && std::is_const_v<U>) || 
			(access==VOLATILE && util::cmp::is_const_accessible_v<T, U>);

		template<typename U>
		static constexpr bool is_movable = std::is_same_v<T, U> || 
			(access==VOLATILE && util::cmp::is_const_accessible_v<T, U>);

	public:
		locked_ptr_t(pool_type* pool, priority p = priority::NONE);
		~locked_ptr_t();

		template<typename U>
		locked_ptr_t(locked_ptr_t<U, N>& other) requires (is_copyable<U>);
		
		template<typename U>
		locked_ptr_t<T, N>& operator=(locked_ptr_t<U, N>& other) requires (is_copyable<U>);

		template<typename U>
		locked_ptr_t(locked_ptr_t<U, N>&& other) requires (is_movable<U>);
		
		template<typename U>
		locked_ptr_t<T, N>& operator=(locked_ptr_t<U, N>&& other) requires (is_movable<U>);

		T& operator*();
		const T& operator*() const;

		T* operator->();
		const T* operator->() const;

		void lock(priority p = priority::MEDIUM);
		bool try_lock(priority p = priority::MEDIUM);
		void unlock();
	private:
		pool_type* pl = nullptr;
		T* ptr = nullptr;
			
	};
}

template<typename T, size_t N>
resource::locked_ptr_t<T, N>::locked_ptr_t(pool_type* pl, resource::priority p)
 : pl(pl)
{
	if (p != priority::NONE) lock(p);
}

template<typename T, size_t N>
resource::locked_ptr_t<T, N>::~locked_ptr_t()
{
	if (ptr != nullptr) unlock();
}

template<typename T, size_t N>
template<typename U> 
resource::locked_ptr_t<T, N>::locked_ptr_t(resource::locked_ptr_t<U, N>& other) requires (is_copyable<U>)
 : pl(other.pl), ptr(other.ptr)
{ 
	ptr = other.ptr;
	pl = other.pl;

	if (ptr != nullptr)
		++(pl->ref_count[ptr - pl->data]);
}

template<typename T, size_t N>
template<typename U>
resource::locked_ptr_t<T, N>& resource::locked_ptr_t<T, N>::operator=(locked_ptr_t<U, N>& other) requires (is_copyable<U>)
{
	if (ptr == nullptr)
		unlock();
	
	ptr = other.ptr;
	pl = other.pl;

	if (ptr != nullptr)
		++(pl->ref_count[ptr - pl->data]);
	
	return *this;
}

template<typename T, size_t N>
template<typename U> 
resource::locked_ptr_t<T, N>::locked_ptr_t(resource::locked_ptr_t<U, N>&& other) requires (is_movable<U>)
{ 
	std::swap(pl, other.pl);
	std::swap(ptr, other.ptr);
}

template<typename T, size_t N>
template<typename U>
resource::locked_ptr_t<T, N>& resource::locked_ptr_t<T, N>::operator=(locked_ptr_t<U, N>&& other) requires (is_movable<U>)
{
	std::swap(pl, other.pl);
	std::swap(ptr, other.ptr);

	return *this;
}

template<typename T, size_t N>
T& resource::locked_ptr_t<T, N>::operator*()
{
	return *ptr;
}

template<typename T, size_t N>
const T& resource::locked_ptr_t<T, N>::operator*() const
{
	return *ptr;
}

template<typename T, size_t N>
T* resource::locked_ptr_t<T, N>::operator->()
{
	return ptr;
}

template<typename T, size_t N>
const T* resource::locked_ptr_t<T, N>::operator->() const
{
	return ptr;
}

template<typename T, size_t N>
void resource::locked_ptr_t<T, N>::lock(priority p) 
{
	std::unique_lock lk(pl->mtx);

	uint32_t i = -1;
	if constexpr (access == VOLATILE) i = pl->volatile_index;
	if constexpr (access == CONSTANT) i = pl->constant_index;
	
	if (i == static_cast<uint32_t>(-1) && (i = pl->find_inactive()) == static_cast<uint32_t>(-1))
	{
		if constexpr (access == EXCLUSIVE) 
		{ 
			++pl->exclusive_wait_count[static_cast<int>(p)]; 
			pl->exclusive_cv[static_cast<int>(p)].wait(lk);
			--pl->exclusive_wait_count[static_cast<int>(p)]; 

			i = pl->last_released;
		}
		if constexpr (access == CONSTANT)  
		{ 
			++pl->constant_wait_count[static_cast<int>(p)]; 
			pl->constant_cv.wait(lk);
			--pl->constant_wait_count[static_cast<int>(p)]; 

			i = pl->constant_index = pl->last_released;
		}
		if constexpr (access == VOLATILE)  
		{ 
			++pl->volatile_wait_count[static_cast<int>(p)]; 
			pl->volatile_cv.wait(lk);
			--pl->volatile_wait_count[static_cast<int>(p)]; 

			i = pl->volatile_index = pl->last_released;
		}
	}

	++pl->ref_count[i];
	ptr = pl->data + i;
}

template<typename T, size_t N>
bool resource::locked_ptr_t<T, N>::try_lock(priority p) 
{
	std::unique_lock lk(pl->mtx);

	uint32_t i = -1;
	if constexpr (access == VOLATILE) i = pl->volatile_index;
	if constexpr (access == CONSTANT) i = pl->constant_index;
	
	if (i == static_cast<uint32_t>(-1) && (i = pl->find_inactive()) == static_cast<uint32_t>(-1))
	{
		return false;
	}

	++pl->ref_count[i];
	ptr = pl->data + i;

	return true;
}

template<typename T, size_t N>
void resource::locked_ptr_t<T, N>::unlock()
{
	std::lock_guard lk(pl->mtx);
	
	uint32_t i = ptr - pl->data;

	if (--(pl->ref_count[i]) == 0)
	{
		if constexpr (access == VOLATILE) pl->volatile_index = static_cast<uint32_t>(-1);
		if constexpr (access == CONSTANT) pl->constant_index = static_cast<uint32_t>(-1);

		pl->last_released = i;
		pl->notify_next();
	}
}