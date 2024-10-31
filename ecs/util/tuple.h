#pragma once
#include <stdint.h>
#include <type_traits>
#include <utility>
namespace util {
	template<typename ... Ts> struct tuple;

	namespace details {
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
	}

	template<typename ... Ts> struct tuple : private details::tuple_elem<Ts>... {
		
		constexpr tuple() = default;
		constexpr tuple(const Ts& ... args) : details::tuple_elem<Ts>(args)... { };

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
		return static_cast<util::details::tuple_elem<tuple_element_t<I, util::tuple<Ts...>>>&>(tup).value; 
	}
	template<size_t I, typename ... Ts> 
	constexpr inline const auto& get(const util::tuple<Ts...>& tup) {
		return static_cast<const util::details::tuple_elem<tuple_element_t<I, util::tuple<Ts...>>>&>(tup).value; 
	}
	template<typename T, typename ... Ts> 
	constexpr inline auto& get(util::tuple<Ts...>& tup) {
		return static_cast<util::details::tuple_elem<T>&>(tup).value;
	}
	template<typename T, typename ... Ts> 
	constexpr inline const auto& get(const util::tuple<Ts...>& tup) {
		return static_cast<const util::details::tuple_elem<T>&>(tup).value;
	}
}

namespace util {
	namespace details {
		template<typename Tup, typename Func_T, size_t ... Is>
		constexpr inline void apply(Func_T&& func, std::index_sequence<Is...>) {
			((func.template operator()<std::tuple_element_t<Is, Tup>>()), ...);
		}
	}
	template<typename Tup, typename Func_T>
	constexpr inline void apply(Func_T&& func) {
		details::apply<Tup>(std::forward<Func_T>(func), std::make_index_sequence<std::tuple_size_v<Tup>>{});
	}

	template<typename Tup, typename T> struct contains;
	template<template<typename ...> typename Tup, typename ... Ts, typename T> struct contains<Tup<Ts...>, T>
	 : std::disjunction<std::is_same<T, Ts>...> { };
	template<typename Tup, typename T> static constexpr bool contains_v = contains<Tup, T>::value;
	
	
}