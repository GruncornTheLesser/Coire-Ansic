#pragma once
#include <tuple>

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

	template<typename tup, template<typename, typename> typename cmp>
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

    template<typename tup, template<typename ...> typename t>
    struct tuple_extract;

    template<typename ... ts, template<typename...> typename t>
    struct tuple_extract<std::tuple<ts...>, t> { 
        using type = t<ts...>;
	};
	
	template<typename tup, template<typename...> typename pred, typename ... args>
	struct tuple_find;

	template<typename t, typename ... ts, template<typename...> typename pred, typename ... args>
	struct tuple_find<std::tuple<t, ts...>, pred, args...> {
		using type = std::conditional_t<pred<t, args...>::value, t, typename tuple_find<std::tuple<ts...>, pred, args...>::type>;
	};

	template<template<typename...> typename pred, typename ... args>
	struct tuple_find<std::tuple<>, pred, args...> {
		using type = std::exception;
	};
}