#pragma once
#include <tuple>
#include "type_name.h"

namespace util {
    template<typename tup, template<typename> typename pred, typename o = std::tuple<>>
	struct tuple_filter;
	
	template<typename t, typename ... ts, template<typename> typename pred, typename ... os>
	struct tuple_filter<std::tuple<t, ts...>, pred, std::tuple<os...>> {
		using type = std::conditional_t<pred<t>::value, 
			tuple_filter<std::tuple<ts...>, pred, std::tuple<os..., t>>,
			tuple_filter<std::tuple<ts...>, pred, std::tuple<os...>>
			>::type;
	};
	
	template<template<typename> typename pred, typename o>
	struct tuple_filter<std::tuple<>, pred, o> {
		using type = o;
	};

	template<typename lhs, typename rhs> 
	struct alphabetical_cmp : std::bool_constant<(util::type_name<lhs>() < util::type_name<rhs>())> { };

	template<typename tup, template<typename, typename> typename cmp = alphabetical_cmp>
	struct tuple_sort;

	template<typename t, typename ... ts, template<typename, typename> typename cmp>
	struct tuple_sort<std::tuple<t, ts...>, cmp> {
		template<typename u> using lt_cmp = cmp<u, t>;
		template<typename u> using gt_cmp = std::negation<cmp<u, t>>;

		using lt = tuple_sort<typename tuple_filter<std::tuple<ts...>, lt_cmp>::type, cmp>::type;
		using gt = tuple_sort<typename tuple_filter<std::tuple<ts...>, gt_cmp>::type, cmp>::type;
	public:
		using type = decltype(std::tuple_cat(std::declval<lt>(), std::declval<std::tuple<t>>(), std::declval<gt>()));
	};
	
	template<template<typename, typename> typename cmp>
	struct tuple_sort<std::tuple<>, cmp> {
		using type = std::tuple<>;
	};

	template<typename tup, template<typename, typename> typename cmp = std::is_same>
	struct tuple_unique;

	template<typename t, typename ... ts, template<typename, typename> typename cmp>
	struct tuple_unique<std::tuple<t, ts...>, cmp> {
		template<typename u> using pred = std::negation<std::is_same<u, t>>;
		using type = decltype(std::tuple_cat(std::declval<std::tuple<t>>(), std::declval<typename tuple_unique<typename tuple_filter<std::tuple<ts...>, pred>::type, cmp>::type>()));
	};

	template<typename t>
	struct not_empty : std::negation<std::is_empty<t>> { };

	template<template<typename, typename> typename cmp>
	struct tuple_unique<std::tuple<>, cmp> {
		using type = std::tuple<>;
	};

    template<typename tup, template<typename ...> typename t>
    struct tuple_extract;

    template<typename ... ts, template<typename...> typename t>
    struct tuple_extract<std::tuple<ts...>, t> { 
        using type = t<ts...>;
	};
}