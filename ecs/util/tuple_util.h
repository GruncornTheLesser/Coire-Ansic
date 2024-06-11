#pragma once


// TODO: naming convention for match, add is to front or alternatively
// TODO: find a way to remove cmp:: for operations such as negate, disjunction, etc -> they conflict with match operations of the same name
// TODO: descriptions
// TODO: compare/match/transform/evaluate traits class -> eg symmetrical_compare ... I'm sure there are others

// Design: 
// transforms - eg std::add_const, std::remove_const, std::type_identity
// 		a transform operation is a template class that with a template parameter and a template member type
// match - eg a std::is_const, std::is_reference
// 		a match operation is a template class with a template parameter and a static constexpr bool value
// compare - eg std::is_same, util::lt_alpha
// 		a match operation is a template class with 2 template parameter and a static constexpr bool value


// Potential Naming Con:
// util::concat<...>::type
// util::concat_t<...>
// util::concat_::template type

#include <type_traits>
#include <tuple>
#define EVALUATE template<typename> typename // describes an attribute of a class. eg std::tuple_size
#define TRANSFORM template<typename> typename // has 'type' eg std::add_const
#define MATCH template<typename> typename // has bool 'value' eg std::is_const
#define COMPARE template<typename, typename> typename // has bool 'value' eg std::is_same
#define CONTAINER template<typename...> typename // has n type args... eg std::tuple

// [ ] concat
namespace util {
	template<typename Tup> 
	struct concat;
	template<typename Tup> 
	using concat_t = typename concat<Tup>::type;
	
	template<CONTAINER TupleSet_T, CONTAINER Tup1, CONTAINER Tup2, typename ... T1s, typename ... T2s, typename ... Ts> 
	struct concat<TupleSet_T<Tup1<T1s...>, Tup2<T2s...>, Ts...>> : concat<TupleSet_T<Tup1<T1s..., T2s...>, Ts...>> { };
	
	template<CONTAINER TupleSet_T, CONTAINER Tup, typename ... Ts> 
	struct concat<TupleSet_T<Tup<Ts...>>> { using type = Tup<Ts...>; };
	
	template<CONTAINER TupleSet_T> 
	struct concat<TupleSet_T<>> { using type = std::tuple<>; };
	
	template<typename Tup, typename T> using append = concat<std::tuple<Tup, std::tuple<T>>>;
	template<typename Tup> struct append_ { template<typename T> using type = append<Tup, T>; };
	template<typename Tup, typename T> using append_t = typename append<Tup, T>::type;
}

// [ ] set - transform
namespace util {
	template<typename T, TRANSFORM ... Trans_Ts> 
	struct eval;

	template<TRANSFORM ... Trans_Ts> 
	struct eval_;

	template<typename T, TRANSFORM ... Trans_Ts> 
	using eval_t = typename eval<T, Trans_Ts...>::type;

	template<typename T, TRANSFORM Trans_T, TRANSFORM ... Trans_Ts> 
	struct eval<T, Trans_T, Trans_Ts...> : eval<typename Trans_T<T>::type, Trans_Ts...> { };
	
	template<typename T> 
	struct eval<T> : std::type_identity<T> { };

	template<TRANSFORM ... Trans_Ts> struct eval_ 
	{ template<typename T> using type = eval<T, Trans_Ts...>; };
}
// [ ] negate match - match
namespace util::match {
	// TODO: replace with not evalulator???
	template<MATCH Match_T> 
	struct negate_ { template<typename T> using type = std::negation<Match_T<T>>; };
}
namespace util::cmp {
	template<COMPARE Cmp_T> 
	struct negate_ { template<typename LHS_T, typename RHS_T> using type = std::negation<Cmp_T<LHS_T, RHS_T>>; };
}

// [ ] conditional - transform
namespace util {
	template<typename T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type, typename=void> 
	struct conditional;

	template<MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = std::type_identity> 
	struct conditional_;

	template<typename T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type> 
	using conditional_t = typename conditional<T, Match_T, If_T, Else_T>::type;

	template<typename T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T> 
	struct conditional<T, Match_T, If_T, Else_T, std::enable_if_t<Match_T<T>::value>> : If_T<T> { };

	template<typename T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T> 
	struct conditional<T, Match_T, If_T, Else_T, std::enable_if_t<!Match_T<T>::value>> : Else_T<T> { };

	template<MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T> struct conditional_ { 
		template<typename T> using type = conditional<T, Match_T, If_T, Else_T>; 
		template<typename T> using negated = conditional<T, match::negate_<Match_T>::template type, If_T, Else_T>;
	};
}

// [ ] on element - transform set
namespace util {
	template<typename Tup, size_t i, TRANSFORM Trans_T> 
	struct eval_at;
	
	template<size_t i, TRANSFORM Trans_T> 
	struct eval_at_ { template<typename Tup> using type = eval_at<Tup, i, Trans_T>; };
	
	template<typename Tup, size_t i, TRANSFORM Trans_T> 
	using eval_at_t = typename eval_at<Tup, i, Trans_T>::type;

	template<CONTAINER Tup, size_t i, typename ... Ts, typename T, TRANSFORM Trans_T> 
	struct eval_at<Tup<Ts..., T>, i, Trans_T> : util::append<typename eval_at<Tup<Ts...>, i-1, Trans_T>::type, T> { };
	
	template<CONTAINER Tup, typename ... Ts, typename T, TRANSFORM Trans_T> 
	struct eval_at<Tup<Ts..., T>, 0, Trans_T> : std::type_identity<Tup<>>{ };
}

// [ ] each - transform set
namespace util {
	template<typename Tup, TRANSFORM Trans_T> 
	struct eval_each;
	
	template<TRANSFORM Trans_T> 
	struct eval_each_;

	template<typename Tup, TRANSFORM Trans_T> 
	using eval_each_t = typename eval_each<Tup, Trans_T>::type;
	
	template<CONTAINER Tup, typename ... Ts, TRANSFORM Trans_T> 
	struct eval_each<Tup<Ts...>, Trans_T> : std::type_identity<Tup<typename Trans_T<Ts>::type...>>{ };

	template<TRANSFORM Trans_T>
	struct eval_each_ { template<typename Tup> using type = eval_each<Tup, Trans_T>; };
}

// [ ] filter - transform set
namespace util {
	template<typename Tup, MATCH Match_T> 
	struct filter;
	
	template<MATCH Match_T>
	struct filter_;
	
	template<typename Tup, MATCH Match_T>
	using filter_t = typename filter<Tup, Match_T>::type;
	
	template<CONTAINER Tup, typename ... Ts, MATCH Match_T> 
	struct filter<Tup<Ts...>, Match_T> : concat<std::tuple<std::conditional_t<Match_T<Ts>::value, Tup<Ts>, Tup<>>...>> { };
	
	template<MATCH Match_T> struct filter_ { template<typename Tup> using type = util::filter<Tup, Match_T>; };
}

// [ ] find - evaluate/transform set
namespace util {
	template<typename Tup, MATCH Match_T, int index=0, typename=void> 
	struct find;
	
	template<MATCH Match_T>
	struct find_;

	template<typename Tup, MATCH Match_T> 
	using find_t = typename find<Tup, Match_T>::type;

	template<CONTAINER Tup, typename T, typename ... Ts, MATCH Match_T, int index>
	struct find<Tup<T, Ts...>, Match_T, index, std::enable_if<Match_T<T>::value>>
	 : std::integral_constant<int, index>, std::type_identity<T> { };

	template<CONTAINER Tup, typename T, typename ... Ts, MATCH Match_T, int index>
	struct find<Tup<T, Ts...>, Match_T, index, std::enable_if<!Match_T<T>::value>>
	 : find<Tup<Ts...>, Match_T, index+1> { };

	template<CONTAINER Tup, MATCH Match_T, int index>
	struct find<Tup<>, Match_T, index, void>
	 : std::integral_constant<int, -1>, std::type_identity<void> { };

	template<MATCH Match_T> 
	struct find_ { template<typename Tup> using type = find<Tup, Match_T>; };
}

// [ ] compare to - match
namespace util {
	template<typename LHS_T, COMPARE Cmp_T=std::is_same, TRANSFORM Trans_T=std::type_identity> 
	struct compare_to_ { 
		template<typename RHS_T> using type = Cmp_T<LHS_T, typename Trans_T<RHS_T>::type>;
		template<typename RHS_T> using negated = std::negation<type<RHS_T>>;
	};
}

// [ ] sort - transform set
namespace util {
	template<typename Tup, COMPARE LT_T> 
	struct sort;
	
	template<COMPARE LT_T> 
	struct sort_;
	
	template<typename Tup, COMPARE LT_T> 
	using sort_t = typename sort<Tup, LT_T>::type;

	template<CONTAINER Tup, typename Pivot_T, typename ... Ts, COMPARE Cmp_T>
	struct sort<Tup<Pivot_T, Ts...>, Cmp_T> : util::concat<std::tuple<
		sort_t<filter_t<Tup<Ts...>, compare_to_<Pivot_T, Cmp_T>::template negated>, Cmp_T>, Tup<Pivot_T>, // not less than
		sort_t<filter_t<Tup<Ts...>, compare_to_<Pivot_T, Cmp_T>::template type>, Cmp_T>>> // less than
	{ };
	
	template<CONTAINER Tup, COMPARE Cmp_T>
	struct sort<Tup<>, Cmp_T> {
		using type = Tup<>;
	};

	template<COMPARE LT_T>
	struct sort_ { template<typename Tup> using type = util::sort<Tup, LT_T>; }; 
}

// [ ] unique - transform set
namespace util {
	template<typename Tup, COMPARE Same_T>
	struct unique;
	
	template<COMPARE Same_T> 
	struct unique_;

	template<typename Tup, COMPARE Same_T>
	using unique_t = typename unique<Tup, Same_T>::type;
	

	template<CONTAINER Tup, typename T, typename ... Ts, COMPARE Same_T>
	struct unique<Tup<T, Ts...>, Same_T> : 
		concat<std::tuple<Tup<T>, unique_t<filter_t<std::tuple<Ts...>, compare_to_<T, Same_T>::template negated>, Same_T>>>
	{ };

	template<CONTAINER Tup, COMPARE Same_T>
	struct unique<Tup<>, Same_T> { using type = Tup<>; };

	template<COMPARE Same_T>
	struct unique_ { template<typename Tup> using type = util::unique<Tup, Same_T>; };
}

// [ ] set_union - transform set
namespace util {
	template<typename Tup1, typename Tup2 = std::tuple<>, COMPARE Same_T = std::is_same> 
	struct set_union;
	
	template<typename Tup1, COMPARE Same_T>
	struct set_union_;

	template<typename Tup, typename Set_T, COMPARE Same_T = std::is_same> 
	using set_union_t = typename set_union<Tup, Set_T, Same_T>::type;
	
	template<typename Tup1, typename Tup2, COMPARE Same_T>
	struct set_union : unique<concat_t<std::tuple<Tup1, Tup2>>, Same_T> { };
	
	template<typename Set_T, COMPARE Same_T = std::is_same> 
	struct set_union_ { template<typename Tup> using type = util::set_union<Tup, Set_T, Same_T>; };
}

// [ ] rewrap - transform
namespace util {
	template<typename T, CONTAINER Tup> 
	struct rewrap;
	
	template<CONTAINER Tup> 
	struct rewrap_ { template<typename T> using type = util::rewrap<T, Tup>; };
	
	template<typename T, CONTAINER Tup> using rewrap_t = typename rewrap<T, Tup>::type;
	
	template<CONTAINER Tup1, typename ... Ts, CONTAINER Tup2>
	struct rewrap<Tup1<Ts...>, Tup2> { using type = Tup2<Ts...>; };
}

// [ ] wrap - transform
namespace util {
	template<typename T, CONTAINER Tup>
	struct wrap { using type = Tup<T>; };
	
	template<CONTAINER Tup>
	struct wrap_ { template<typename T> using type = util::wrap<T, Tup>; };
	
	template<typename T, CONTAINER Tup>
	using wrap_t = typename wrap<T, Tup>::type;
}

// [ ] unwrap - transform
namespace util {
	template<typename T> 
	struct unwrap;

	template<CONTAINER Tup, typename T> 
	struct unwrap<Tup<T>> : std::type_identity<T> { };

	template<typename T> 
	using unwrap_t = typename unwrap<T>::type;
}

// [ ] match dis/conjunctions - match/compare
namespace util::match {
	template<MATCH ... Match_Ts>
	struct disjunction_ { 
		template<typename T> using type = std::disjunction<Match_Ts<T>...>; 
		template<typename T> using negated = std::negation<type<T>>;
	};
	template<MATCH ... Match_Ts>
	struct conjunction_ { 
		template<typename T> using type = std::conjunction<Match_Ts<T>...>; 
		template<typename T> using negated = std::negation<type<T>>;
	};
}
namespace util::cmp {
	template<COMPARE ... Cmp_Ts>
	struct disjunction_ { 
		template<typename LHS_T, typename RHS_T> using type = std::disjunction<Cmp_Ts<LHS_T, RHS_T>...>;
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>; 
	};
	template<COMPARE ... Cmp_Ts>
	struct conjunction_ {
		template<typename LHS_T, typename RHS_T> using type = std::conjunction<Cmp_Ts<LHS_T, RHS_T>...>; 
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>;
	};
}

// [ ] allof - match set
namespace util {
	template<typename Tup, MATCH Match_T> 
	struct allof : eval_t<Tup, eval_each_<wrap_<Match_T>::template type>::template type, rewrap_<std::conjunction>::template type> { };

	template<MATCH Match_T> 
	struct allof_ { template<typename Tup> using type = util::allof<Tup, Match_T>; };
	
	template<typename Tup, MATCH Match_T> 
	static constexpr bool allof_v = allof<Tup, Match_T>::value;
	
}

// [ ] anyof - match set
namespace util {
	template<typename Tup, MATCH Match_T> 
	struct anyof : eval_t<Tup, eval_each_<wrap_<Match_T>::template type>::template type, rewrap_<std::disjunction>::template type> { }; 
	
	template<MATCH Match_T> 
	struct anyof_ { template<typename Tup> using type = util::anyof<Tup, Match_T>; };
	
	template<typename Tup, MATCH Match_T> 
	static constexpr bool anyof_v = anyof<Tup, Match_T>::value;
}
// [ ] element of/contains - match set
namespace util {
	template<typename T, typename Tup, COMPARE Same_T = std::is_same>
	struct element_of : anyof<Tup, compare_to_<T, Same_T>::template type> { };
	
	template<typename Tup, COMPARE Cmp_T = std::is_same>
	struct element_of_ {
		template<typename T> using type = util::element_of<T, Tup, Cmp_T>; 
		template<typename T> using negated = std::negation<type<T>>;
	};
	
	template<typename T, COMPARE Cmp_T = std::is_same>
	struct contains_ {
		template<typename Tup> using type = util::element_of<T, Tup>; 
		template<typename Tup> using negated = std::negation<type<Tup>>;
	};

	template<typename T, typename Tup, COMPARE Same_T = std::is_same>
	static constexpr bool element_of_v = element_of<T, Tup, Same_T>::value;
}

// [ ] set_intersect - transform set
namespace util {
	template<typename Tup1, typename Tup2, COMPARE Same_T = std::is_same> 
	struct set_intersect;

	template<COMPARE Same_T, typename Set_T> 
	struct set_intersect_ { template<typename Tup> using type = util::set_intersect<Tup, Set_T, Same_T>; };
	
	template<typename Tup, typename Set_T, COMPARE Same_T = std::is_same> 
	using set_intersect_t = typename set_intersect<Tup, Set_T, Same_T>::type;
	
	template<typename Tup1, typename Tup2, COMPARE Same_T>
	struct set_intersect : filter<set_union_t<Tup1, Tup2, Same_T>,	match::conjunction_<
		element_of_<Tup1, Same_T>::template type, 
		element_of_<Tup2, Same_T>::template type
	>::template type> { };
}

// [ ] compare transformed - compare
namespace util::cmp {
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM Trans_T> 
	struct transformed : Cmp_T<typename Trans_T<LHS_T>::type, typename Trans_T<RHS_T>::type> { };

	template<COMPARE Cmp_T, TRANSFORM Trans_T> struct transformed_ { 
		template<typename LHS_T, typename RHS_T> using type = cmp::transformed<LHS_T, RHS_T, Cmp_T, Trans_T>;
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>;
	};

	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM Trans_T>
	static constexpr bool transformed_v = transformed<LHS_T, RHS_T, Cmp_T, Trans_T>::value;



	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM LHS_Trans_T, TRANSFORM RHS_Trans_T> 
	struct transformed_individual : Cmp_T<typename LHS_Trans_T<LHS_T>::type, typename RHS_Trans_T<RHS_T>::type> { };
	
	template<COMPARE Cmp_T, TRANSFORM LHS_Trans_T, TRANSFORM RHS_Trans_T>
	struct transformed_individual_ { 
		template<typename LHS_T, typename RHS_T> using type = cmp::transformed_individual<LHS_T, RHS_T, Cmp_T, LHS_Trans_T, RHS_Trans_T>;
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>;
	};

	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM LHS_Trans_T, TRANSFORM RHS_Trans_T> 
	static constexpr bool transformed_individual_v = transformed_individual<LHS_T, RHS_T, Cmp_T, LHS_Trans_T, RHS_Trans_T>::value;
	


	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM RHS_Trans_T> 
	struct transformed_rhs : transformed_individual<LHS_T, RHS_T, Cmp_T, std::type_identity, RHS_Trans_T> { };

	template<COMPARE Cmp_T, TRANSFORM RHS_Trans_T> 
	struct transformed_rhs_ : transformed_individual_<Cmp_T, std::type_identity, RHS_Trans_T> { };
	
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM RHS_Trans_T>
	static constexpr bool transformed_rhs_v = transformed_rhs<LHS_T, RHS_T, Cmp_T, RHS_Trans_T>::value;
	


	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM LHS_Trans_T> 
	struct transformed_lhs : transformed_individual<LHS_T, RHS_T, Cmp_T, LHS_Trans_T, std::type_identity> { };

	template<COMPARE Cmp_T, TRANSFORM LHS_Trans_T> 
	struct transformed_lhs_ : transformed_individual_<Cmp_T, LHS_Trans_T, std::type_identity> { };
	
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM LHS_Trans_T> 
	static constexpr bool transformed_lhs_v = transformed_lhs<LHS_T, RHS_T, Cmp_T, LHS_Trans_T>::value;
}

// [ ] compare operations - compare
namespace util::cmp {
	template<typename LHS, typename RHS, EVALUATE getter> 
	struct less : std::bool_constant<(getter<LHS>::value < getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct less_ { template<typename LHS, typename RHS> using type = util::cmp::less<LHS, RHS, getter>; };
	
	template<typename LHS, typename RHS, EVALUATE getter>
	 struct less_or_equal : std::bool_constant<(getter<LHS>::value <= getter<RHS>::value)> { };
	
	template<EVALUATE getter> 
	struct less_or_equal_ { template<typename LHS, typename RHS> using type = util::cmp::less_or_equal<LHS, RHS, getter>; };

	template<typename LHS, typename RHS, EVALUATE getter> 
	struct greater : std::bool_constant<(getter<LHS>::value > getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct greater_ { template<typename LHS, typename RHS> using type = util::cmp::greater<LHS, RHS, getter>; };
	
	template<typename LHS, typename RHS, EVALUATE getter> 
	struct greater_or_equal : std::bool_constant<(getter<LHS>::value >= getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct greater_or_equal_ { template<typename LHS, typename RHS> using type = util::cmp::greater_or_equal<LHS, RHS, getter>; };
	
	template<typename LHS, typename RHS, EVALUATE getter> 
	struct equal : std::bool_constant<(getter<LHS>::value > getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct equal_ { template<typename LHS, typename RHS> using type = util::cmp::equal<LHS, RHS, getter>; };
	
	template<typename LHS, typename RHS, EVALUATE getter> 
	struct not_equal : std::bool_constant<(getter<LHS>::value >= getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct not_equal_ { template<typename LHS, typename RHS> using type = util::cmp::not_equal<LHS, RHS, getter>; };
}

// [ ] match transformed - match
namespace util::match {
	template<typename T, MATCH Match_T, TRANSFORM Trans_T> struct transformed : Match_T<typename Trans_T<T>::type> { };
	template<typename T, MATCH Match_T, TRANSFORM Trans_T> using transformed_v = transformed<T, Match_T, Trans_T>::value;
	template<MATCH Match_T, TRANSFORM Trans_T> struct transformed_ { 
		template<typename T> using type = Match_T<typename transformed<T, Match_T, Trans_T>::type>; 
		template<typename T> using negated = std::negation<type<T>>; };
}

// [ ] post_conditional - transform
namespace util {
	template<typename T, TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type, typename=void>
	struct post_conditional;
	template<typename T, TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type>
	using post_conditional_t = typename post_conditional<T, Trans_T, Match_T, If_T, Else_T>::type;

	template<typename T, TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T>
	struct post_conditional<T, Trans_T, Match_T, If_T, Else_T, std::enable_if_t<Match_T<T>::value>> { using type = typename If_T<typename Trans_T<T>::type>::type; };
	
	template<typename T, TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T>
	struct post_conditional<T, Trans_T, Match_T, If_T, Else_T, std::enable_if_t<!Match_T<T>::value>> { using type = typename Else_T<typename Trans_T<T>::type>::type; };

	template<TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type>
	struct post_conditional_ { template<typename T> using type = util::post_conditional<T, Trans_T, Match_T, If_T, Else_T>; };
}

// [ ] propergate_const - transform
namespace util {
	template<typename T, TRANSFORM Trans_T>
	struct propergate_const : post_conditional<T, Trans_T, std::is_const, std::add_const> { };
	template<typename T, TRANSFORM Trans_T>
	using propergate_const_t = typename propergate_const<T, Trans_T>::type; 

	template<typename T, TRANSFORM Trans_T>
	struct propergate_const_each : util::post_conditional<T, Trans_T, std::is_const, eval_each_<std::add_const>::template type> { };
	template<typename T, TRANSFORM Trans_T>
	using propergate_const_each_t = typename propergate_const_each<T, Trans_T>::type;
	
	template<TRANSFORM Trans_T>
	struct propergate_const_ { template<typename T> using type = util::propergate_const<T, Trans_T>; };
	template<TRANSFORM Trans_T>
	struct propergate_const_each_ { template<typename T> using type = util::propergate_const_each<T, Trans_T>; };
}

// [ ] add type args - misc
namespace util {
	template<template<typename ...> typename Func_T, typename ... Arg_Ts>
	struct add_type_args_ { 
		template<typename ... Ts> using type = Func_T<Ts..., Arg_Ts...>;
	};
	// set_t<T, add_type_arg<>>
}

// [ ] subset - match set
namespace util {
	template<typename SubSet_T, typename SuperSet_T, COMPARE Cmp_T>
	struct subset_of : allof<SubSet_T, element_of_<SuperSet_T, Cmp_T>::template type> { };
	template<typename SubSet_T, typename SuperSet_T, COMPARE Cmp_T>
	static constexpr bool subset_of_v = subset_of<SubSet_T, SuperSet_T, Cmp_T>::value;
	template<typename SuperSet_T, COMPARE Cmp_T>
	struct subset_of_ { template<typename SubSet_T> using type = util::subset_of<SubSet_T, SuperSet_T, Cmp_T>; };
}

// [ ] is tuple - match
namespace util {
	template<typename T> struct is_tuple : std::false_type { };
	template<typename ... Ts> struct is_tuple<std::tuple<Ts...>> : std::true_type { };
	template<typename T> static constexpr bool is_tuple_v = is_tuple<T>::value;
}

// [ ] priority list - compare
namespace util::cmp {
	template<typename LHS, typename RHS, COMPARE Cmp_T>
	struct is_symmetrical : std::bool_constant<Cmp_T<LHS, RHS>::value == Cmp_T<RHS, LHS>::value> { };

	template<typename LHS, typename RHS, COMPARE ... Cmp_Ts>
	struct priority_list;

	template<typename LHS, typename RHS, COMPARE Cmp_T, COMPARE ... Cmp_Ts>
	struct priority_list<LHS, RHS, Cmp_T, Cmp_Ts...>
	 : std::disjunction<Cmp_T<LHS, RHS>, std::conjunction<is_symmetrical<LHS, RHS, Cmp_T>, priority_list<LHS, RHS, Cmp_Ts...>>> { };
	
	template<typename LHS, typename RHS, COMPARE Cmp_T>
	struct priority_list<LHS, RHS, Cmp_T>
	 : Cmp_T<LHS, RHS> { };
	
	template<typename LHS, typename RHS, COMPARE ... Cmp_Ts>
	static constexpr bool priority_list_v = priority_list<LHS, RHS, Cmp_Ts...>::value;

	template<COMPARE ... Cmp_Ts>
	struct priority_list_ { template<typename LHS, typename RHS> using type = priority_list<LHS, RHS, Cmp_Ts...>; };
	
	template<EVALUATE ... attribs>
	struct attrib_priority_list_ { template<typename LHS, typename RHS> using type = priority_list<LHS, RHS, util::cmp::less_<attribs>::template type...>; };
};

#undef EVALUATE
#undef TRANSFORM
#undef MATCH
#undef COMPARE
#undef CONTAINER