#pragma once
#include <tuple>
#include "type_name.h"

namespace util {
	template<typename t>
	struct not_empty : std::negation<std::is_empty<t>> { };
	template<typename t1, typename t2>
	struct is_remove_const_same : std::is_same<std::remove_const_t<t1>, std::remove_const_t<t2>> { };
	template<typename lhs, typename rhs> 
	struct alpha_cmp : std::bool_constant<(util::type_name<lhs>() < util::type_name<rhs>())> { };

	template<typename lhs, typename rhs> 
	struct non_const_first_cmp : std::bool_constant<!std::is_const_v<lhs> && std::is_const_v<rhs>> { };


	template<typename lhs, typename rhs>
	struct remove_const_alpha_cmp : alpha_cmp<std::remove_const_t<lhs>, std::remove_const_t<rhs>> { };
	template<typename t>
	struct to_pointer { using type = t*; };

	template<typename t, template<typename ...> typename tup>
	struct is_template : std::false_type { };
	template<typename ... ts, template<typename ...> typename tup>
	struct is_template<tup<ts...>, tup> : std::true_type { };

	template<template<typename, typename...> typename pred>
	struct negate_pred {
		template<typename t, typename ... arg_ts>
		using type = std::bool_constant<!pred<t, arg_ts...>::value>;
	};

	template<template<typename, typename, typename...> typename cmp>
	struct negate_cmp {
		template<typename lhs, typename rhs, typename ... arg_ts>
		using type = std::bool_constant<!cmp<lhs, rhs, arg_ts...>::value>;
	};



	template<typename ... ts>
	struct tuple_expand;

	template<template<typename...> typename t1, typename ... t1s, template<typename...> typename t2, typename ... t2s, typename ... ts>
	struct tuple_expand<t1<t1s...>, t2<t2s...>, ts...> { 
		using type = tuple_expand<t1<t1s..., t2s...>, ts...>::type;
	};

	template<template<typename...> typename t1, typename ... t1s>
	struct tuple_expand<t1<t1s...>> { 
		using type = t1<t1s...>;
	};

	template<>
	struct tuple_expand<> {
		using type = std::tuple<>;
	};

	template<typename ... ts>
	using tuple_expand_t = tuple_expand<ts...>::type;



	template<template<typename> typename pred, typename tup, typename o = std::tuple<>>
	struct tuple_filter;
	
	template<template<typename> typename pred, template<typename ...> typename tup, typename t, typename ... ts, typename ... os>
	struct tuple_filter<pred, tup<t, ts...>, std::tuple<os...>> {
		using type = std::conditional_t<pred<t>::value, 
			tuple_filter<pred, std::tuple<ts...>, std::tuple<os..., t>>,
			tuple_filter<pred, std::tuple<ts...>, std::tuple<os...>>
			>::type;
	};
	
	template<template<typename> typename pred, template<typename ...> typename tup, typename o>
	struct tuple_filter<pred, tup<>, o> {
		using type = o;
	};

	template<template<typename> typename pred, typename tup>
	using tuple_filter_t = tuple_filter<pred, tup>::type;



	template<template<typename, typename> typename cmp, typename tup>
	struct tuple_sort;

	template<template<typename, typename> typename cmp, template<typename ...> typename tup, typename ... ts>
	struct tuple_sort<cmp, tup<ts...>> : tuple_expand<tup<>, typename tuple_sort<cmp, std::tuple<ts...>>::type> { };

	template<template<typename, typename> typename cmp, typename t, typename ... ts>
	struct tuple_sort<cmp, std::tuple<t, ts...>> {
		template<typename u> using lt_cmp = cmp<u, t>;
		template<typename u> using gt_cmp = std::negation<cmp<u, t>>;

		using lt = tuple_sort<cmp, typename tuple_filter<lt_cmp, std::tuple<ts...>>::type>::type;
		using gt = tuple_sort<cmp, typename tuple_filter<gt_cmp, std::tuple<ts...>>::type>::type; // or equal
	public:
		using type = decltype(std::tuple_cat(std::declval<lt>(), std::declval<std::tuple<t>>(), std::declval<gt>()));
	};
	
	template<template<typename, typename> typename cmp>
	struct tuple_sort<cmp, std::tuple<>> {
		using type = std::tuple<>;
	};

	template<template<typename, typename> typename cmp, typename tup>
	using tuple_sort_t = tuple_sort<cmp, tup>::type;

	template<template<typename, typename> typename cmp, typename tup>
	struct tuple_unique;

	template<template<typename, typename> typename cmp, template<typename...> typename tup, typename ... ts>
	struct tuple_unique<cmp, tup<ts...>> : tuple_expand<tup<>, typename tuple_unique<cmp, std::tuple<ts...>>::type> { };

	template<template<typename, typename> typename cmp, typename t, typename ... ts>
	struct tuple_unique<cmp, std::tuple<t, ts...>> {
		template<typename u> using pred = std::negation<std::is_same<u, t>>;
		using type = decltype(std::tuple_cat(std::declval<std::tuple<t>>(), std::declval<typename tuple_unique<cmp, typename tuple_filter<pred, std::tuple<ts...>>::type>::type>()));
	};

	template<template<typename, typename> typename cmp>
	struct tuple_unique<cmp, std::tuple<>> {
		using type = std::tuple<>;
	};

	template<template<typename, typename> typename cmp, typename tup>
	using tuple_unique_t = tuple_unique<cmp, tup>::type;



	template<typename u, typename tup, size_t i = 0>
	struct tuple_index;

	template<typename u, typename t, template<typename...> typename tup, typename ... ts, size_t i>
	struct tuple_index<u, tup<t, ts...>, i> : std::conditional_t<std::is_same_v<t, u>, std::integral_constant<int, i>, tuple_index<u, tup<ts...>,  i + 1>> { };

	template<typename u, template<typename...> typename tup, size_t i>
	struct tuple_index<u, tup<>, i> : std::integral_constant<int, -1> { };

	template<typename u, typename tup>
	static constexpr size_t tuple_index_v = tuple_index<u, tup>::value;

	template<typename u, typename tup>
	static constexpr size_t tuple_contains_v = tuple_index<u, tup>::value != -1;



	template<template<typename, typename...> typename func, typename tup, typename ... args>
	struct tuple_transform;

	template<template<typename, typename ...> typename func, template<typename ...> typename tup, typename ... ts, typename ... args>
	struct tuple_transform<func, tup<ts...>, args...> {
		using type = tup<typename func<ts, args...>::type...>;
	};

	template<template<typename, typename...> typename func, typename tup, typename ... args>
	using tuple_transform_t = tuple_transform<func, tup, args...>::type;
}