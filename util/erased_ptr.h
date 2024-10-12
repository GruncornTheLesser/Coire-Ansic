#pragma once
#include <memory>
#include <functional>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdelete-incomplete"


namespace util {
	template<typename T> struct default_delete { void operator()(void* ptr) const { return std::default_delete<T>{}(reinterpret_cast<T*>(ptr)); }};

	class erased_unique_ptr
	{
	public:
		template<typename T>
		erased_unique_ptr(T* ptr);
		template<typename T> T& get();
	private:
		std::unique_ptr<void, std::function<void(void*)>> ptr;
	};

	template<typename T>
	erased_unique_ptr::erased_unique_ptr(T* ptr)
		: ptr(ptr, util::default_delete<T>{})
	{ }

	template<typename T>
	T& erased_unique_ptr::get() {
		return *reinterpret_cast<T*>(ptr.get());
	}


	class erased_shared_ptr
	{
	public:
		template<typename T>
		erased_shared_ptr(T* ptr);
		template<typename T> T& get();
	private:
		std::shared_ptr<void> ptr;
	};

	template<typename T>
	erased_shared_ptr::erased_shared_ptr(T* ptr)
		: ptr(ptr, util::default_delete<T>{})
	{ }

	template<typename T>
	T& erased_shared_ptr::get() {
		return *reinterpret_cast<T*>(ptr.get());
	}
}

#pragma GCC diagnostic pop
