#pragma once
#include "tuple_util\tuple_util.h"

namespace util {
	template <typename T>
	class proxy_ref {
	public:
		using value_type = T;

		constexpr explicit proxy_ref(const T& val) : value(val) { }
		constexpr proxy_ref(T&& val) : value(std::forward<T>(val)) { }
		constexpr proxy_ref& operator=(const T& val) { value = val; return *this; }
		
		constexpr operator T&() const { return const_cast<T&>(value); }
		constexpr T& operator*() const { return const_cast<T&>(value); }
		constexpr T* operator->() const { return const_cast<T*>(&value); }

		template<size_t I> auto& get() const { return std::get<I>(const_cast<T&>(value)); }
	private:
		T value;
	};
}

namespace std {
	template<typename T> class tuple_size<util::proxy_ref<T>> : public tuple_size<T> { };
	template<size_t I, typename T> class tuple_element<I, util::proxy_ref<T>> : public util::copy_indirect_cv<tuple_element_t<I, T>, T> { };
	
	template<size_t I, typename T> constexpr decltype(auto) get(util::proxy_ref<T>& ref) { return std::get<I>(*ref); }
	template<size_t I, typename T> constexpr decltype(auto) get(const util::proxy_ref<T>& ref) { return std::get<I>(*ref); }
	template<size_t I, typename T> constexpr decltype(auto) get(util::proxy_ref<T>&& ref) { return std::get<I>(std::move(*ref)); }
	
	template<typename U, typename T> constexpr decltype(auto) get(util::proxy_ref<T>& ref) { return std::get<U>(*ref); }
	template<typename U, typename T> constexpr decltype(auto) get(const util::proxy_ref<T>& ref) { return std::get<U>(*ref); }
	template<typename U, typename T> constexpr decltype(auto) get(util::proxy_ref<T>&& ref) { return std::get<U>(std::move(*ref)); }
	
}