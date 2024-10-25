#pragma once
#include <stdint.h>
#include <type_traits>
namespace util {
	template<typename ... Ts> struct tuple;
	template<typename T> struct tuple_elem
	{
		template<size_t, typename ... Us> constexpr friend auto& std::get(util::tuple<Us...>&);
		template<size_t, typename ... Us> constexpr friend const auto& std::get(const util::tuple<Us...>&);
		template<typename, typename ... Us> constexpr friend auto& std::get(util::tuple<Us...>&);
		template<typename, typename ... Us> constexpr friend const auto& std::get(const util::tuple<Us...>&);

		constexpr tuple_elem() = default;
		constexpr tuple_elem(const tuple_elem&) = default;
		constexpr tuple_elem& operator=(const tuple_elem&) = default;
		constexpr tuple_elem(tuple_elem&&) = default;
		constexpr tuple_elem& operator=(tuple_elem&&) = default;
		
		constexpr tuple_elem(const T& val) : value(val) { }
		constexpr tuple_elem& operator=(const T& val) { value = val; }
	private: 
		[[no_unique_address]] T value;
	};


	template<typename ... Ts> struct tuple : private tuple_elem<Ts>... {
		
		constexpr tuple() = default;
		constexpr tuple(const Ts& ... args) : tuple_elem<Ts>(args)... { };

		template<size_t, typename ... Us> constexpr friend auto& std::get(util::tuple<Us...>&);
		template<size_t, typename ... Us> constexpr friend const auto& std::get(const util::tuple<Us...>&);
		template<typename, typename ... Us> constexpr friend auto& std::get(util::tuple<Us...>&);
		template<typename, typename ... Us> constexpr friend const auto& std::get(const util::tuple<Us...>&);
		
	};
	
}

namespace std {
	template<typename ... Ts> class tuple_size<util::tuple<Ts...>> : public std::integral_constant<size_t, sizeof...(Ts)> { };
	
	template<size_t I, typename T, typename ... Ts> class tuple_element<I, util::tuple<T, Ts...>> : public tuple_element<I - 1, util::tuple<Ts...>> { };
	template<typename T, typename ... Ts> class tuple_element<0, util::tuple<T, Ts...>> : public std::type_identity<T> { };
		
	template<size_t I, typename ... Ts> 
	constexpr inline auto& get(util::tuple<Ts...>& tup) { 
		return static_cast<util::tuple_elem<tuple_element_t<I, util::tuple<Ts...>>>&>(tup).value; 
	}
	template<size_t I, typename ... Ts> 
	constexpr inline const auto& get(const util::tuple<Ts...>& tup) {
		return static_cast<const util::tuple_elem<tuple_element_t<I, util::tuple<Ts...>>>&>(tup).value; 
	}
	template<typename T, typename ... Ts> 
	constexpr inline auto& get(util::tuple<Ts...>& tup) {
		return static_cast<util::tuple_elem<T>&>(tup).value;
	}
	template<typename T, typename ... Ts> 
	constexpr inline const auto& get(const util::tuple<Ts...>& tup) {
		return static_cast<const util::tuple_elem<T>&>(tup).value;
	}
}