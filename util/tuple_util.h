#pragma once


// TODO: naming convention for pred
// TODO: find a way to remove cmp:: for operations such as negate, disjunction, etc -> they conflict with pred operations of the same name
// TODO: descriptions
// TODO: organize into separate files
// Design: 
// transforms - a transform operation is a template class that with a template parameter and a template member type
// 		eg std::add_const, std::remove_const, std::type_identity
// pred - a pred operation is a template class with 1 template parameter and a static constexpr bool value
//		eg a std::is_const, std::is_reference
// compare - a compare operation is a template class with 2 template parameter and a static constexpr bool value
// 		eg std::is_same, lt_alpha


// Potential Naming Con:
// concat<...>::type
// concat_t<...>
// concat_::template type

#include <type_traits>
#include <tuple>
#define EVALUATE template<typename> typename // describes an attribute of a class. eg std::tuple_size
#define TRANSFORM template<typename> typename // has 'type' eg std::add_const
#define PREDICATE template<typename> typename // has bool 'value' eg std::is_const
#define COMPARE template<typename, typename> typename // has bool 'value' eg std::is_same
#define CONTAINER template<typename...> typename // has n type args... eg std::tuple

// [ ] concat
namespace util {
	template<typename Tup> 
	struct concat;

	template<typename Tup> 
	using concat_t = typename concat<Tup>::type;
	
	template<CONTAINER TupleSet_T, CONTAINER Tup1, CONTAINER Tup2, typename ... T1s, typename ... T2s, typename ... Ts> 
	struct concat<TupleSet_T<Tup1<T1s...>, Tup2<T2s...>, Ts...>>
	 : concat<TupleSet_T<Tup1<T1s..., T2s...>, Ts...>> { };
	
	template<CONTAINER TupleSet_T, CONTAINER Tup, typename ... Ts> 
	struct concat<TupleSet_T<Tup<Ts...>>> 
	{ using type = Tup<Ts...>; };
	
	template<CONTAINER TupleSet_T> 
	struct concat<TupleSet_T<>> 
	{ using type = std::tuple<>; };
}

// [ ] append
namespace util {
	template<typename Tup, typename T> 
	using append = concat<std::tuple<Tup, std::tuple<T>>>;

	template<typename Tup> struct append_ 
	{ template<typename T> using type = append<Tup, T>; };

	template<typename Tup, typename T> 
	using append_t = typename append<Tup, T>::type;
}

// [ ] prepend
namespace util {
	template<typename Tup, typename T> 
	using prepend = concat<std::tuple<std::tuple<T>, Tup>>;

	template<typename Tup> struct prepend_ 
	{ template<typename T> using type = prepend<Tup, T>; };

	template<typename Tup, typename T> 
	using prepend_t = typename prepend<Tup, T>::type;
}


// [ ] pop_front
namespace util {
	template<typename Tup> 
	struct pop_front;
	
	template<CONTAINER Tup, typename T, typename ... Ts>
	struct pop_front<Tup<T, Ts...>> { using type = Tup<Ts...>; };
	
	template<typename Tup> 
	using pop_front_t = typename pop_front<Tup>::type;
}

namespace util {
	template<typename Tup> 
	struct get_front;
	
	template<CONTAINER Tup, typename T, typename ... Ts>
	struct get_front<Tup<T, Ts...>> { using type = T; };
	
	template<typename Tup> 
	using get_front_t = typename get_front<Tup>::type;

	template<typename Tup> 
	struct get_back;
	
	template<CONTAINER Tup, typename ... Ts, typename T>
	struct get_back<Tup<Ts..., T>> { using type = T; };
	
	template<typename Tup> 
	using get_back_t = typename get_front<Tup>::type;
}


// [ ] pop_back
namespace util {
	template<typename Tup> 
	struct pop_back;
	
	template<CONTAINER Tup, typename ... Ts, typename T>
	struct pop_back<Tup<Ts..., T>> { using type = Tup<Ts...>; };
	
	template<typename Tup> 
	using pop_back_t = typename pop_back<Tup>::type;
}


// [ ] eval - transform
namespace util {
	template<typename T, TRANSFORM ... Trans_Ts> 
	struct eval;

	template<TRANSFORM ... Trans_Ts> 
	struct eval_;

	template<typename T, TRANSFORM ... Trans_Ts> 
	using eval_t = typename eval<T, Trans_Ts...>::type;

	template<typename T, TRANSFORM Trans_T, TRANSFORM ... Trans_Ts> 
	struct eval<T, Trans_T, Trans_Ts...>
	 : eval<typename Trans_T<T>::type, Trans_Ts...> { };
	
	template<typename T> 
	struct eval<T> : std::type_identity<T> { };

	template<TRANSFORM ... Trans_Ts> struct eval_ 
	{ template<typename T> using type = eval<T, Trans_Ts...>; };
}
// [ ] negate pred - pred
namespace util::pred {
	// TODO: replace with not evalulator???
	template<PREDICATE Match_T> 
	struct negate_
	 { template<typename T> using type = std::negation<Match_T<T>>; };
}
namespace util::cmp {
	template<COMPARE Cmp_T> 
	struct negate_
	 { template<typename LHS_T, typename RHS_T> using type = std::negation<Cmp_T<LHS_T, RHS_T>>; };
}

// [ ] eval_if - transform
namespace util {
	template<typename T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type, typename=void> 
	struct eval_if;

	template<PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T = std::type_identity> 
	struct eval_if_;

	template<typename T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type> 
	using eval_if_t = typename eval_if<T, Match_T, If_T, Else_T>::type;

	template<typename T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T> 
	struct eval_if<T, Match_T, If_T, Else_T, std::enable_if_t<Match_T<T>::value>>
	 : If_T<T> { };

	template<typename T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T> 
	struct eval_if<T, Match_T, If_T, Else_T, std::enable_if_t<!Match_T<T>::value>>
	 : Else_T<T> { };

	template<PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T> struct eval_if_ 
	{ 
		template<typename T> using type = eval_if<T, Match_T, If_T, Else_T>; 
		template<typename T> using negated = eval_if<T, pred::negate_<Match_T>::template type, If_T, Else_T>;
	};
}

// [ ] on element - transform set
namespace util {
	template<typename Tup, int i, TRANSFORM Trans_T> 
	struct eval_at;
	
	template<int i, TRANSFORM Trans_T> 
	struct eval_at_ 
	{ template<typename Tup> using type = eval_at<Tup, i, Trans_T>; };
	
	template<typename Tup, int i, TRANSFORM Trans_T> 
	using eval_at_t = typename eval_at<Tup, i, Trans_T>::type;

	template<CONTAINER Tup, int i, typename ... Ts, typename T, TRANSFORM Trans_T> 
	struct eval_at<Tup<Ts..., T>, i, Trans_T>
	 : append<typename eval_at<Tup<Ts...>, i-1, Trans_T>::type, T> { };
	
	template<CONTAINER Tup, typename ... Ts, typename T, TRANSFORM Trans_T> 
	struct eval_at<Tup<Ts..., T>, 0, Trans_T>
	 : std::type_identity<Tup<>>{ };
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
	struct eval_each<Tup<Ts...>, Trans_T>
	 : std::type_identity<Tup<typename Trans_T<Ts>::type...>>{ };

	template<TRANSFORM Trans_T>
	struct eval_each_
	 { template<typename Tup> using type = eval_each<Tup, Trans_T>; };
}

// [ ] filter - transform set
namespace util {
	template<typename Tup, PREDICATE Match_T> 
	struct filter;
	
	template<PREDICATE Match_T>
	struct filter_;
	
	template<typename Tup, PREDICATE Match_T>
	using filter_t = typename filter<Tup, Match_T>::type;
	
	template<CONTAINER Tup, typename ... Ts, PREDICATE Match_T> 
	struct filter<Tup<Ts...>, Match_T>
	 : concat<std::tuple<Tup<>, std::conditional_t<Match_T<Ts>::value, Tup<Ts>, Tup<>>...>> { };
	
	template<PREDICATE Match_T> 
	struct filter_ 
	{ template<typename Tup> using type = filter<Tup, Match_T>; };
}

// [ ] find - evaluate/transform set
namespace util {
	template<typename Tup, PREDICATE Match_T, int index=0, typename=void> 
	struct find;
	
	template<PREDICATE Match_T>
	struct find_;

	template<typename Tup, PREDICATE Match_T> 
	using find_t = typename find<Tup, Match_T>::type;

	template<typename Tup, PREDICATE Match_T> 
	static constexpr int find_v = find<Tup, Match_T>::value;

	template<CONTAINER Tup, typename T, typename ... Ts, PREDICATE Match_T, int index>
	struct find<Tup<T, Ts...>, Match_T, index, std::enable_if_t<Match_T<T>::value>>
	 : std::type_identity<T> { static constexpr int value = index; };

	template<CONTAINER Tup, typename T, typename ... Ts, PREDICATE Match_T, int index>
	struct find<Tup<T, Ts...>, Match_T, index, std::enable_if_t<!Match_T<T>::value>>
	 : find<Tup<Ts...>, Match_T, index + 1> { };

	template<CONTAINER Tup, PREDICATE Match_T, int index>
	struct find<Tup<>, Match_T, index, void>
	 : std::type_identity<void> { };

	template<PREDICATE Match_T> 
	struct find_
	{ template<typename Tup> using type = find<Tup, Match_T>; };
}

// [ ] compare to - pred
namespace util 
{
	template<typename LHS_T, COMPARE Cmp_T=std::is_same, TRANSFORM Trans_T=std::type_identity> 
	struct compare_to_ 
	{ 
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
	struct sort<Tup<Pivot_T, Ts...>, Cmp_T> : concat<std::tuple<
		sort_t<filter_t<Tup<Ts...>, compare_to_<Pivot_T, Cmp_T>::template negated>, Cmp_T>, Tup<Pivot_T>, // not less than
		sort_t<filter_t<Tup<Ts...>, compare_to_<Pivot_T, Cmp_T>::template type>, Cmp_T>>> // less than
	{ };
	
	template<CONTAINER Tup, COMPARE Cmp_T>
	struct sort<Tup<>, Cmp_T> {
		using type = Tup<>;
	};

	template<COMPARE LT_T>
	struct sort_ { template<typename Tup> using type = sort<Tup, LT_T>; }; 
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
	struct unique<Tup<T, Ts...>, Same_T> : concat<std::tuple<Tup<T>, typename unique<filter_t<std::tuple<Ts...>, 
		compare_to_<T, Same_T>::template negated>, Same_T>::type>>
	{ };

	template<CONTAINER Tup, COMPARE Same_T>
	struct unique<Tup<>, Same_T> { using type = Tup<>; };

	template<COMPARE Same_T>
	struct unique_ { template<typename Tup> using type = unique<Tup, Same_T>; };

	template<typename Tup, COMPARE Same_T, COMPARE Priority_T>
	struct unique_priority : eval<Tup, sort_<Priority_T>::template type, unique_<Same_T>::template type> { };

	template<COMPARE Same_T, COMPARE Priority_T>
	struct unique_priority_ { template<typename Tup> using type = unique_priority<Tup, Same_T, Priority_T>; };
	
	template<typename Tup, COMPARE Same_T, COMPARE Priority_T>
	using unique_priority_t = typename unique_priority<Tup, Same_T, Priority_T>::type;
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
	struct set_union_ { template<typename Tup> using type = set_union<Tup, Set_T, Same_T>; };
}

// [ ] rewrap - transform
namespace util {
	template<typename T, CONTAINER Tup> 
	struct rewrap;
	
	template<CONTAINER Tup> 
	struct rewrap_ { template<typename T> using type = rewrap<T, Tup>; };
	
	template<typename T, CONTAINER Tup> using rewrap_t = typename rewrap<T, Tup>::type;
	
	template<CONTAINER Tup1, typename ... Ts, CONTAINER Tup2>
	struct rewrap<Tup1<Ts...>, Tup2> { using type = Tup2<Ts...>; };
}

// [ ] wrap - transform
namespace util {
	template<typename T, CONTAINER ... Tups>
	struct wrap;
	
	template<CONTAINER ... Tup>
	struct wrap_ { template<typename T> using type = wrap<T, Tup...>; };
	
	template<typename T, CONTAINER ... Tups>
	struct wrap : eval<T, wrap_<Tups>::template type...> { };
	
	template<typename T, CONTAINER Tup>
	struct wrap<T, Tup> { using type = Tup<T>; };
	
	template<typename T, CONTAINER ... Tups>
	using wrap_t = typename wrap<T, Tups...>::type;
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

// [ ] pred dis/conjunctions - pred/compare
namespace util::pred {
	template<PREDICATE ... Match_Ts>
	struct disjunction_ { 
		template<typename T> using type = std::disjunction<Match_Ts<T>...>; 
		template<typename T> using negated = std::negation<type<T>>;
	};
	template<PREDICATE ... Match_Ts>
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

// [ ] allof - pred set
namespace util {
	template<typename Tup, PREDICATE Match_T> 
	struct allof : eval_t<Tup, eval_each_<wrap_<Match_T>::template type>::template type, rewrap_<std::conjunction>::template type> { };

	template<PREDICATE Match_T> 
	struct allof_ { template<typename Tup> using type = allof<Tup, Match_T>; };
	
	template<typename Tup, PREDICATE Match_T> 
	static constexpr bool allof_v = allof<Tup, Match_T>::value;
	
}

// [ ] anyof - pred set
namespace util {
	template<typename Tup, PREDICATE Match_T> 
	struct anyof : eval_t<Tup, eval_each_<wrap_<Match_T>::template type>::template type, rewrap_<std::disjunction>::template type> { }; 
	
	template<PREDICATE Match_T> 
	struct anyof_ { template<typename Tup> using type = anyof<Tup, Match_T>; };
	
	template<typename Tup, PREDICATE Match_T> 
	static constexpr bool anyof_v = anyof<Tup, Match_T>::value;
}
// [ ] element of/contains - pred set
namespace util {
	template<typename T, typename Tup, COMPARE Same_T = std::is_same>
	struct element_of : anyof<Tup, compare_to_<T, Same_T>::template type> { };
	
	template<typename Tup, COMPARE Cmp_T = std::is_same>
	struct element_of_ {
		template<typename T> using type = element_of<T, Tup, Cmp_T>; 
		template<typename T> using negated = std::negation<type<T>>;
	};
	
	template<typename T, COMPARE Cmp_T = std::is_same>
	struct contains_ {
		template<typename Tup> using type = element_of<T, Tup>; 
		template<typename Tup> using negated = std::negation<type<Tup>>;
	};

	template<typename T, typename Tup, COMPARE Same_T = std::is_same>
	static constexpr bool element_of_v = element_of<T, Tup, Same_T>::value;

	template<typename Tup, typename T, COMPARE Same_T = std::is_same>
	static constexpr bool contains_v = element_of<T, Tup, Same_T>::value;
}

// [ ] set_intersect - transform set
namespace util {
	template<typename Tup1, typename Tup2, COMPARE Same_T = std::is_same> 
	struct set_intersect;

	template<COMPARE Same_T, typename Set_T> 
	struct set_intersect_ { template<typename Tup> using type = set_intersect<Tup, Set_T, Same_T>; };
	
	template<typename Tup, typename Set_T, COMPARE Same_T = std::is_same> 
	using set_intersect_t = typename set_intersect<Tup, Set_T, Same_T>::type;
	
	template<typename Tup1, typename Tup2, COMPARE Same_T>
	struct set_intersect : filter<set_union_t<Tup1, Tup2, Same_T>,	pred::conjunction_<
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
// TODO: convenience multi getter compare ops -> translates to priority list
namespace util::cmp {
	template<typename LHS, typename RHS, EVALUATE getter> 
	struct less : std::bool_constant<(getter<LHS>::value < getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct less_ { template<typename LHS, typename RHS> using type = cmp::less<LHS, RHS, getter>; };
	
	template<typename LHS, typename RHS, EVALUATE getter>
	 struct less_or_equal : std::bool_constant<(getter<LHS>::value <= getter<RHS>::value)> { };
	
	template<EVALUATE getter> 
	struct less_or_equal_ { template<typename LHS, typename RHS> using type = cmp::less_or_equal<LHS, RHS, getter>; };

	template<typename LHS, typename RHS, EVALUATE getter> 
	struct greater : std::bool_constant<(getter<LHS>::value > getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct greater_ { template<typename LHS, typename RHS> using type = cmp::greater<LHS, RHS, getter>; };
	
	template<typename LHS, typename RHS, EVALUATE getter> 
	struct greater_or_equal : std::bool_constant<(getter<LHS>::value >= getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct greater_or_equal_ { template<typename LHS, typename RHS> using type = cmp::greater_or_equal<LHS, RHS, getter>; };
	
	template<typename LHS, typename RHS, EVALUATE getter> 
	struct equal : std::bool_constant<(getter<LHS>::value > getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct equal_ { template<typename LHS, typename RHS> using type = cmp::equal<LHS, RHS, getter>; };
	
	template<typename LHS, typename RHS, EVALUATE getter> 
	struct not_equal : std::bool_constant<(getter<LHS>::value >= getter<RHS>::value)> { };

	template<EVALUATE getter> 
	struct not_equal_ { template<typename LHS, typename RHS> using type = cmp::not_equal<LHS, RHS, getter>; };
}

// [ ] pred transformed - pred
namespace util::pred {
	template<typename T, PREDICATE Match_T, TRANSFORM Trans_T> struct transformed : Match_T<typename Trans_T<T>::type> { };
	template<typename T, PREDICATE Match_T, TRANSFORM Trans_T> using transformed_v = transformed<T, Match_T, Trans_T>::value;
	template<PREDICATE Match_T, TRANSFORM Trans_T> struct transformed_ { 
		template<typename T> using type = Match_T<typename transformed<T, Match_T, Trans_T>::type>; 
		template<typename T> using negated = std::negation<type<T>>; };
}

// [ ] post_eval_if - transform
namespace util {
	// if pred,  with If_T, if true, after 
	template<typename T, TRANSFORM Trans_T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type, typename=void>
	struct post_eval_if;

	template<typename T, TRANSFORM Trans_T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type>
	using post_eval_if_t = typename post_eval_if<T, Trans_T, Match_T, If_T, Else_T>::type;

	template<typename T, TRANSFORM Trans_T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T>
	struct post_eval_if<T, Trans_T, Match_T, If_T, Else_T, std::enable_if_t<Match_T<T>::value>> { using type = typename If_T<typename Trans_T<T>::type>::type; };
	
	template<typename T, TRANSFORM Trans_T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T>
	struct post_eval_if<T, Trans_T, Match_T, If_T, Else_T, std::enable_if_t<!Match_T<T>::value>> { using type = typename Else_T<typename Trans_T<T>::type>::type; };

	template<TRANSFORM Trans_T, PREDICATE Match_T, TRANSFORM If_T, TRANSFORM Else_T = eval_<>::type>
	struct post_eval_if_ { template<typename T> using type = post_eval_if<T, Trans_T, Match_T, If_T, Else_T>; };
}

// [ ] propagate_const - transform, update to post::const_
namespace util {
	template<typename T, TRANSFORM Trans_T>
	struct propagate_const : post_eval_if<T, Trans_T, std::is_const, std::add_const> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_const_ { template<typename T> using type = propagate_const<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_const_t = typename propagate_const<T, Trans_T>::type; 
	
	template<typename T, TRANSFORM Trans_T>
	struct propagate_const_each : post_eval_if<T, Trans_T, std::is_const, eval_each_<std::add_const>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_const_each_ { template<typename T> using type = propagate_const_each<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_const_each_t = typename propagate_const_each<T, Trans_T>::type;
	
	template<typename T, CONTAINER Tup>
	using propagate_const_wrap = propagate_const<T, eval_<std::remove_const, wrap_<Tup>::template type>::template type>;

	template<CONTAINER Tup>
	struct propagate_const_wrap_ { template<typename T> using type = propagate_const_wrap<T, Tup>; };

	template<typename T, CONTAINER Tup>
	using propagate_const_wrap_t = propagate_const_wrap<T, Tup>::type;
}

// [ ] propagate_volatile 
namespace util {
	template<typename T, TRANSFORM Trans_T>
	struct propagate_volatile : post_eval_if<T, Trans_T, std::is_volatile, std::add_volatile> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_volatile_ { template<typename T> using type = propagate_volatile<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_volatile_t = typename propagate_volatile<T, Trans_T>::type; 
	
	template<typename T, TRANSFORM Trans_T>
	struct propagate_volatile_each : post_eval_if<T, Trans_T, std::is_volatile, eval_each_<std::add_volatile>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_volatile_each_ { template<typename T> using type = propagate_volatile_each<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_volatile_each_t = typename propagate_volatile_each<T, Trans_T>::type;
	
	template<typename T, CONTAINER Tup>
	using propagate_volatile_wrap = propagate_volatile<T, eval_<std::remove_volatile, wrap_<Tup>::template type>::template type>;

	template<CONTAINER Tup>
	struct propagate_volatile_wrap_ { template<typename T> using type = propagate_volatile_wrap<T, Tup>; };

	template<typename T, CONTAINER Tup>
	using propagate_volatile_wrap_t = propagate_volatile_wrap<T, Tup>::type;
}

// [ ] propagate_pointer
namespace util {
	template<typename T, TRANSFORM Trans_T>
	struct propagate_pointer : post_eval_if<T, Trans_T, std::is_pointer, std::add_pointer> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_pointer_ { template<typename T> using type = propagate_pointer<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_pointer_t = typename propagate_pointer<T, Trans_T>::type; 
	
	template<typename T, TRANSFORM Trans_T>
	struct propagate_pointer_each : post_eval_if<T, Trans_T, std::is_pointer, eval_each_<std::add_pointer>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_pointer_each_ { template<typename T> using type = propagate_pointer_each<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_pointer_each_t = typename propagate_pointer_each<T, Trans_T>::type;
	
	template<typename T, CONTAINER Tup>
	using propagate_pointer_wrap = propagate_pointer<T, eval_<std::remove_pointer, wrap_<Tup>::template type>::template type>;

	template<CONTAINER Tup>
	struct propagate_pointer_wrap_ { template<typename T> using type = propagate_pointer_wrap<T, Tup>; };

	template<typename T, CONTAINER Tup>
	using propagate_pointer_wrap_t = propagate_pointer_wrap<T, Tup>::type;
}

// [ ] propagate_reference
namespace util {
	template<typename T, TRANSFORM Trans_T>
	struct propagate_rvalue_reference : post_eval_if<T, Trans_T, std::is_rvalue_reference, std::add_rvalue_reference> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_rvalue_reference_ { template<typename T> using type = propagate_rvalue_reference<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_rvalue_reference_t = typename propagate_rvalue_reference<T, Trans_T>::type;

	template<typename T, TRANSFORM Trans_T>
	struct propagate_rvalue_reference_each : post_eval_if<T, Trans_T, std::is_rvalue_reference, eval_each_<std::add_rvalue_reference>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_rvalue_reference_each_ { template<typename T> using type = propagate_rvalue_reference_each<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_rvalue_reference_each_t = typename propagate_rvalue_reference_each<T, Trans_T>::type;
	
	template<typename T, CONTAINER Tup>
	using propagate_rvalue_reference_wrap = propagate_rvalue_reference<T, eval_<std::remove_reference, wrap_<Tup>::template type>::template type>;

	template<CONTAINER Tup>
	struct propagate_rvalue_reference_wrap_ { template<typename T> using type = propagate_rvalue_reference_wrap<T, Tup>; };

	template<typename T, CONTAINER Tup>
	using propagate_rvalue_reference_wrap_t = propagate_rvalue_reference_wrap<T, Tup>::type;



	template<typename T, TRANSFORM Trans_T>
	struct propagate_lvalue_reference : post_eval_if<T, Trans_T, std::is_lvalue_reference, std::add_lvalue_reference> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_lvalue_reference_ { template<typename T> using type = propagate_lvalue_reference<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_lvalue_reference_t = typename propagate_lvalue_reference<T, Trans_T>::type;

	template<typename T, TRANSFORM Trans_T>
	struct propagate_lvalue_reference_each : post_eval_if<T, Trans_T, std::is_lvalue_reference, eval_each_<std::add_lvalue_reference>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_lvalue_reference_each_ { template<typename T> using type = propagate_lvalue_reference_each<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_lvalue_reference_each_t = typename propagate_lvalue_reference_each<T, Trans_T>::type;
	
	template<typename T, CONTAINER Tup>
	using propagate_lvalue_reference_wrap = propagate_lvalue_reference<T, eval_<std::remove_reference, wrap_<Tup>::template type>::template type>;

	template<CONTAINER Tup>
	struct propagate_lvalue_reference_wrap_ { template<typename T> using type = propagate_lvalue_reference_wrap<T, Tup>; };

	template<typename T, CONTAINER Tup>
	using propagate_lvalue_reference_wrap_t = propagate_lvalue_reference_wrap<T, Tup>::type;



	template<typename T, TRANSFORM Trans_T>
	struct propagate_reference
	 : eval_if<T, std::is_rvalue_reference, eval_<Trans_T, std::add_rvalue_reference>::template type, 
		eval_if_<std::is_lvalue_reference, eval_<Trans_T, std::add_lvalue_reference>::template type,
		Trans_T>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_reference_ { template<typename T> using type = propagate_reference<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_reference_t = typename propagate_lvalue_reference<T, Trans_T>::type;

	template<typename T, TRANSFORM Trans_T>
	struct propagate_reference_each
	 : eval_if<T, std::is_rvalue_reference, eval_<Trans_T, eval_each_<std::add_rvalue_reference>::template type>::template type, 
		eval_if_<std::is_lvalue_reference, eval_<Trans_T, eval_each_<std::add_lvalue_reference>::template type>::template type,
		Trans_T>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_reference_each_ { template<typename T> using type = propagate_lvalue_reference_each<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_reference_each_t = typename propagate_lvalue_reference_each<T, Trans_T>::type;
	
	template<typename T, CONTAINER Tup>
	using propagate_reference_wrap = propagate_lvalue_reference<T, eval_<std::remove_reference, wrap_<Tup>::template type>::template type>;

	template<CONTAINER Tup>
	struct propagate_reference_wrap_ { template<typename T> using type = propagate_lvalue_reference_wrap<T, Tup>; };

	template<typename T, CONTAINER Tup>
	using propagate_reference_wrap_t = propagate_lvalue_reference_wrap<T, Tup>::type;
}

// [ ] propagate_cv
namespace util {
	template<typename T, TRANSFORM Trans_T>
	struct propagate_cv
	 : eval_if<T, std::is_const, eval_<Trans_T, std::add_const>::template type, 
		eval_if_<std::is_volatile, eval_<Trans_T, std::add_volatile>::template type,
		Trans_T>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_cv_ { template<typename T> using type = propagate_cv<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_cv_t = typename propagate_cv<T, Trans_T>::type; 
	
	template<typename T, TRANSFORM Trans_T>
	struct propagate_cv_each
	 : eval_if<T, std::is_const, eval_<Trans_T, eval_each_<std::add_const>::template type>::template type, 
		eval_if_<std::is_volatile, eval_<Trans_T, eval_each_<std::add_volatile>::template type>::template type,
		Trans_T>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_cv_each_ { template<typename T> using type = propagate_cv_each<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_cv_each_t = typename propagate_cv_each<T, Trans_T>::type;
	
	template<typename T, CONTAINER Tup>
	using propagate_cv_wrap = propagate_cv<T, eval_<std::remove_cv, wrap_<Tup>::template type>::template type>;

	template<CONTAINER Tup>
	struct propagate_cv_wrap_ { template<typename T> using type = propagate_cv_wrap<T, Tup>; };

	template<typename T, CONTAINER Tup>
	using propagate_cv_wrap_t = propagate_cv_wrap<T, Tup>::type;
}

// [ ] propagate_cvref
namespace util {
	template<typename T, TRANSFORM Trans_T>
	struct propagate_cvref
	 : eval_if<T, std::is_const, eval_<Trans_T, std::add_const>::template type, 
		eval_if_<std::is_volatile, eval_<Trans_T, std::add_volatile>::template type,
		eval_if_<std::is_lvalue_reference, eval_<Trans_T, std::add_lvalue_reference>::template type,
		eval_if_<std::is_rvalue_reference, eval_<Trans_T, std::add_rvalue_reference>::template type,
		Trans_T>::template type>::template type>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_cvref_ { template<typename T> using type = propagate_cvref<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_cvref_t = typename propagate_cvref<T, Trans_T>::type; 
	
	template<typename T, TRANSFORM Trans_T>
	struct propagate_cvref_each
	 : eval_if<T, std::is_const, eval_<Trans_T, eval_each_<std::add_const>::template type>::template type, 
		eval_if_<std::is_volatile, eval_<Trans_T, eval_each_<std::add_volatile>::template type>::template type,
		eval_if_<std::is_lvalue_reference, eval_<Trans_T, eval_each_<std::add_lvalue_reference>::template type>::template type,
		eval_if_<std::is_rvalue_reference, eval_<Trans_T, eval_each_<std::add_rvalue_reference>::template type>::template type,
		Trans_T>::template type>::template type>::template type> { };
	
	template<TRANSFORM Trans_T>
	struct propagate_cvref_each_ { template<typename T> using type = propagate_cvref_each<T, Trans_T>; };
	
	template<typename T, TRANSFORM Trans_T>
	using propagate_cvref_each_t = typename propagate_cvref_each<T, Trans_T>::type;
	
	template<typename T, CONTAINER Tup>
	using propagate_cvref_wrap = propagate_cvref<T, eval_<std::remove_cvref, wrap_<Tup>::template type>::template type>;

	template<CONTAINER Tup>
	struct propagate_cvref_wrap_ { template<typename T> using type = propagate_cvref_wrap<T, Tup>; };

	template<typename T, CONTAINER Tup>
	using propagate_cvref_wrap_t = propagate_cvref_wrap<T, Tup>::type;
}

// [ ] add type args - misc
namespace util {
	template<template<typename ...> typename Func_T, typename ... Arg_Ts>
	struct add_type_args_ { 
		template<typename ... Ts> using type = Func_T<Ts..., Arg_Ts...>;
	};
	// set_t<T, add_type_arg<>>
}

// [ ] subset - pred set
namespace util {
	template<typename SubSet_T, typename SuperSet_T, COMPARE Cmp_T=std::is_same>
	struct is_subset : allof<SubSet_T, element_of_<SuperSet_T, Cmp_T>::template type> { };
	template<typename SubSet_T, typename SuperSet_T, COMPARE Cmp_T=std::is_same>
	static constexpr bool is_subset_v = is_subset<SubSet_T, SuperSet_T, Cmp_T>::value;
	template<typename SuperSet_T, COMPARE Cmp_T=std::is_same>
	struct is_subset_ { template<typename SubSet_T> using type = is_subset<SubSet_T, SuperSet_T, Cmp_T>; };
}

// [ ] is tuple - pred
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
	struct attrib_priority_list_ { template<typename LHS, typename RHS> using type = priority_list<LHS, RHS, cmp::less_<attribs>::template type...>; };
}

// [ ] is_ignore_cvref_same
namespace util::cmp {
	template<typename LHS_T, typename RHS_T>
	struct is_ignore_cvref_same : std::is_same<std::remove_cvref_t<LHS_T>, std::remove_cvref_t<RHS_T>> { };

	template<typename LHS_T, typename RHS_T>
	static constexpr bool is_ignore_cvref_same_v = is_ignore_cvref_same<LHS_T, RHS_T>::value;



	template<typename LHS_T, typename RHS_T>
	struct is_ignore_cv_same : std::is_same<std::remove_cv_t<LHS_T>, std::remove_cv_t<RHS_T>> { };

	template<typename LHS_T, typename RHS_T>
	static constexpr bool is_ignore_cv_same_v = is_ignore_cv_same<LHS_T, RHS_T>::value;



	template<typename LHS_T, typename RHS_T>
	struct is_ignore_reference_same : std::is_same<std::remove_reference_t<LHS_T>, std::remove_reference_t<RHS_T>> { };

	template<typename LHS_T, typename RHS_T>
	static constexpr bool is_ignore_reference_same_v = is_ignore_reference_same<LHS_T, RHS_T>::value;



	template<typename LHS_T, typename RHS_T>
	struct is_ignore_volatile_same : std::is_same<std::remove_volatile_t<LHS_T>, std::remove_volatile_t<RHS_T>> { };

	template<typename LHS_T, typename RHS_T>
	static constexpr bool is_ignore_volatile_same_v = is_ignore_volatile_same<LHS_T, RHS_T>::value;



	template<typename LHS_T, typename RHS_T>
	struct is_ignore_const_same : std::is_same<std::remove_const_t<LHS_T>, std::remove_const_t<RHS_T>> { };

	template<typename LHS_T, typename RHS_T>
	static constexpr bool is_ignore_const_same_v = is_ignore_const_same<LHS_T, RHS_T>::value;



	template<typename LHS_T, typename RHS_T>
	struct is_const_accessible : std::disjunction<std::is_same<LHS_T, RHS_T>, std::is_same<LHS_T, std::add_const_t<RHS_T>>> { };
	
	template<typename LHS_T, typename RHS_T>
	static constexpr bool is_const_accessible_v = is_const_accessible<LHS_T, RHS_T>::value;
}

// [ ] is_wrapped_by
namespace util {
	template<typename T, CONTAINER Tup>
	struct is_wrapped_by : std::false_type { };
	
	template<typename T, CONTAINER Tup>
	struct is_wrapped_by<const T, Tup> : is_wrapped_by<T, Tup> { };
	
	template<typename ... Ts, CONTAINER Tup>
	struct is_wrapped_by<Tup<Ts...>, Tup> : std::true_type { };
	
	template<CONTAINER Tup>
	struct is_wrapped_by_ { template<typename T> using type = is_wrapped_by<T, Tup>; };
	
	template<typename T, CONTAINER Tup>
	static constexpr bool is_wrapped_by_v = is_wrapped_by<T, Tup>::value;

	
}

#undef EVALUATE
#undef TRANSFORM
#undef PREDICATE
#undef COMPARE
#undef CONTAINER