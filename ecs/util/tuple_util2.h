#pragma once
#include <type_traits>
#include <tuple>
#define TRANSFORM template<typename> typename
#define MATCH template<typename> typename
#define COMPARE template<typename,typename> typename
#define TEMPLATE template<typename...> typename

// [X] concat
namespace util::trn {
	/// @brief concatenates each tuple in input tuples together
	/// @tparam Tups the input tuple
	template<typename Tup> struct concat;
	template<typename Tup> using concat_t = typename concat<Tup>::type;

	template<TEMPLATE Container_T, TEMPLATE Tup1, TEMPLATE Tup2, typename ... T1s, typename ... T2s, typename ... Ts> 
	struct concat<Container_T<Tup1<T1s...>, Tup2<T2s...>, Ts...>> : concat<Container_T<Tup1<T1s..., T2s...>, Ts...>> { };
	template<TEMPLATE Container_T, TEMPLATE Tup, typename ... Ts> 
	struct concat<Container_T<Tup<Ts...>>> { using type = Tup<Ts...>; };
	template<TEMPLATE Container_T> 
	struct concat<Container_T<>> { using type = std::tuple<>; };
}
namespace util::trn::build {
	/// @brief concatenates each tuple in input tuples together
	struct concat { template<typename Tup> using type = util::trn::concat<Tup>; };
}

// [X] set
namespace util::trn {
	/// @brief evaluates Transforms from left to right on type argument passed to type<T>
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	/// @tparam ::type<T> the input tuple
	template<typename T, TRANSFORM ... Trans_Ts> struct set;
	template<typename T, TRANSFORM ... Trans_Ts> using set_t = typename set<T, Trans_Ts...>::type;

	template<typename T, TRANSFORM Trans_T, TRANSFORM ... Trans_Ts> 
	struct set<T, Trans_T, Trans_Ts...> { using type = set_t<typename Trans_T<T>::type, Trans_Ts...>; };
	template<typename T> 
	struct set<T> { using type = T; };
}
namespace util::trn::build {
	/// @brief evaluates Transforms from left to right on type argument passed to type<T>
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	/// @tparam ::type<T> the input tuple
	template<TRANSFORM ... Trans_Ts> 
	struct set { template<typename T> using type = util::trn::set<T, Trans_Ts...>; };
}

// [X] conditional
namespace util::trn {
	/// @brief if Match evaulates to true, evaluates transforms left to right
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<typename T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = build::set<>::type, typename=void> struct conditional;
	
	template<typename T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T> struct conditional<T, Match_T, If_T, Else_T, std::enable_if_t<Match_T<T>::value>> : If_T<T> { };
	template<typename T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T> struct conditional<T, Match_T, If_T, Else_T, std::enable_if_t<!Match_T<T>::value>> : Else_T<T> { };
	
	/// @brief if Match evaulates to true, evaluates transforms left to right
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<typename T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = build::set<>::type> using conditional_t = typename conditional<T, Match_T, If_T, Else_T>::type;
}
namespace util::trn::build { 
	/// @brief if Match evaulates to true, evaluates transforms left to right
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = util::trn::build::set<>::type> struct conditional { 
		template<typename T> using type = trn::conditional<T, Match_T, If_T, Else_T>; 
	};

}

// [X] each
namespace util::trn {
	/// @brief transform each type in Tup passed
	/// @tparam Trans_Ts  a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const 
	/// @tparam ::type<Tup> the input tuple
	template<typename Tup, TRANSFORM Trans_T> struct each;
	
	template<TEMPLATE Tup, typename ... Ts, TRANSFORM Trans_T> struct each<Tup<Ts...>, Trans_T> { using type = Tup<typename Trans_T<Ts>::type...>; };
	
	/// @brief if Match evaulates to true, evaluates transforms left to right
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<typename Tup, TRANSFORM Trans_T> using each_t = typename each<Tup, Trans_T>::type;
}
namespace util::trn::build { 
	/// @brief transform each type in Tup passed
	/// @tparam Trans_Ts  a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const 
	/// @tparam ::type<Tup> the input tuple
	template<TRANSFORM Trans_T> struct each { template<typename Tup> using type = util::trn::each<Tup, Trans_T>; };
}

// [X] filter
namespace util::trn {
	/// @brief filters tuple for types that match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam ::type<Tup> the input tuple
	template<typename Tup, MATCH Match_T> struct filter;
	template<typename Tup, MATCH Match_T> using filter_t = typename filter<Tup, Match_T>::type;

	template<template<typename...> typename Tup, typename ... Ts, MATCH Match_T> 
	struct filter<Tup<Ts...>, Match_T> : concat<std::tuple<std::conditional_t<Match_T<Ts>::value, Tup<Ts>, Tup<>>...>> { };
}
namespace util::trn::build { 	
	/// @brief filters tuple for types that match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam ::type<Tup> the input tuple
	template<MATCH Match_T> struct filter { template<typename Tup> using type = util::trn::filter<Tup, Match_T>; };
}

// [X] compare to
namespace util::mtc::build {
	/// @brief constructs a match template predicate from a compare template predicate
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam LHS_T eg int
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, typename LHS_T> struct compare_to { 
		template<typename RHS_T> using type = Cmp_T<LHS_T, RHS_T>; 
		template<typename RHS_T> using negated = std::negation<Cmp_T<LHS_T, RHS_T>>; 
	};
}

// [X] sort
namespace util::trn {
	/// @brief sorts tuple by Cmp_T func.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuple
	template<typename Tup, COMPARE LT_T> struct sort;
	template<typename Tup, COMPARE LT_T> using sort_t = typename sort<Tup, LT_T>::type;

	template<TEMPLATE Tup, typename T, typename ... Ts, COMPARE Cmp_T>
	struct sort<Tup<T, Ts...>, Cmp_T> : util::trn::concat<std::tuple<
		sort_t<filter_t<Tup<Ts...>, mtc::build::compare_to<Cmp_T, T>::template type>, Cmp_T>, Tup<T>, // less than
		sort_t<filter_t<Tup<Ts...>, mtc::build::compare_to<Cmp_T, T>::template negated>, Cmp_T>>> // not less than
	{ };
	
	template<TEMPLATE Tup, COMPARE Cmp_T>
	struct sort<Tup<>, Cmp_T> {
		using type = Tup<>;
	};
}
namespace util::trn::build { 
	/// @brief sorts tuple by Cmp_T func.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuple
	template<COMPARE LT_T> struct sort { template<typename Tup> using type = util::trn::sort<Tup, LT_T>; }; 
}

namespace util::trn {
	template<typename Tup, COMPARE Same_T>
	struct unique;
	
	template<typename Tup, COMPARE Same_T>
	using unique_t = typename unique<Tup, Same_T>::type;
	

	template<TEMPLATE Tup, typename T, typename ... Ts, COMPARE Same_T>
	struct unique<Tup<T, Ts...>, Same_T> : 
		concat<std::tuple<Tup<T>, unique_t<filter_t<std::tuple<Ts...>, mtc::build::compare_to<Same_T, T>::template negated>, Same_T>>>
	{ };

	template<TEMPLATE Tup, COMPARE Same_T>
	struct unique<Tup<>, Same_T> { using type = Tup<>; };
}
namespace util::trn::build {
	template<COMPARE Same_T>
	struct unique { template<typename Tup> using type = trn::unique<Tup, Same_T>; };
}
// [X] set_union
namespace util::trn {
	/// @brief performs a union set operation on the Tups passed. removes duplicates.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuple
	template<typename Tup1, typename Tup2 = std::tuple<>, COMPARE Same_T = std::is_same> struct set_union;
	template<typename Tup, typename Set_T, COMPARE Same_T = std::is_same> using set_union_t = typename set_union<Tup, Set_T, Same_T>::type;
	
	template<typename Tup1, typename Tup2, COMPARE Same_T>
	struct set_union : unique<concat_t<std::tuple<Tup1, Tup2>>, Same_T> { };
}
namespace util::trn::build {
	/// @brief performs a union set operation on the Tups passed. removes duplicates.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuple
	template<typename Set_T, COMPARE Same_T = std::is_same> struct set_union { template<typename Tup> using type = util::trn::set_union<Tup, Set_T, Same_T>;  };
}

// [X] rewrap
namespace util::trn {
	/// @brief unwraps template and wraps in new template 
	template<typename T, TEMPLATE Tup> struct rewrap;
	template<TEMPLATE Tup1, typename ... Ts, TEMPLATE Tup2> struct rewrap<Tup1<Ts...>, Tup2> { using type = Tup2<Ts...>; };
	template<typename T, TEMPLATE Tup> using rewrap_t = typename rewrap<T, Tup>::type;
}
namespace util::trn::build {
	/// @brief unwraps template and wraps in new template 
	template<TEMPLATE Tup> struct rewrap { template<typename T> using type = util::trn::rewrap<T, Tup>; };
}

// [X] wrap
namespace util::trn {
	/// @brief wraps passed arg in Tup
	template<typename T, TEMPLATE Tup> struct wrap { using type = Tup<T>; };
	template<typename T, TEMPLATE Tup> using wrap_t = typename wrap<T, Tup>::type;
}
namespace util::trn::build {
	/// @brief wraps passed arg in Tup
	template<TEMPLATE Tup> struct wrap { template<typename T> using type = util::trn::wrap<T, Tup>; };
}

// [X] match disjunction
namespace util::mtc::build {
	template<MATCH ... Match_Ts>
	struct disjunction { template<typename T> using type = std::disjunction<Match_Ts<T>...>; };
}

// [X] all of
namespace util::mtc::build {
	/// @brief returns true if any of types in tuple match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	template<MATCH Match_T> struct allof { template<typename Tup> using type = util::trn::set_t<Tup,
		trn::build::each<trn::build::wrap<Match_T>::template type>::template type,
		trn::build::rewrap<std::disjunction>::template type>; };
}

// [X] any of
namespace util::mtc::build {
	/// @brief returns true if all of types in tuple return true in match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	template<MATCH Match_T> struct anyof { template<typename Tup> using type = util::trn::set_t<Tup,
		trn::build::each<trn::build::wrap<Match_T>::template type>::template type,
		trn::build::rewrap<std::conjunction>::template type>;  };
}

// [X] match conjunction
namespace util::mtc::build {
	template<MATCH ... Match_Ts>
	struct conjunction { template<typename T> using type = std::conjunction<Match_Ts<T>...>; };
}

// [X] element of/contains
namespace util::mtc::build {
	template<typename Tup, COMPARE Cmp_T = std::is_same>
	struct element_of { 
		template<typename T> using type = anyof<compare_to<Cmp_T, T>::template type>::template type<Tup>; 
		template<typename T> using negated = std::negation<typename anyof<compare_to<Cmp_T, T>::template type>::template type<Tup>>; 
	};

	template<typename T, COMPARE Cmp_T = std::is_same>
	struct contains { 
		template<typename Tup> using type = anyof<compare_to<Cmp_T, T>::template type>::template type<Tup>; 
		template<typename Tup> using negated = std::negation<typename anyof<compare_to<Cmp_T, T>::template type>::template type<Tup>>;
	};
}

// [X] set_intersect
namespace util::trn {
	/// @brief performs a intersect set operation on the 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuples
	template<typename Tup, typename Set_T, COMPARE Same_T = std::is_same> struct set_intersect;
	template<typename Tup1, typename Tup2, COMPARE Same_T> struct set_intersect : 
	filter<set_union_t<Tup1, Tup2, Same_T>, mtc::build::disjunction<
		mtc::build::element_of<Tup1, Same_T>::template type, 
		mtc::build::element_of<Tup1, Same_T>::template type
	>::template type> { };
	template<typename Tup, typename Set_T, COMPARE Same_T = std::is_same> using set_intersect_t = typename set_intersect<Tup, Set_T, Same_T>::type;
}
namespace util::trn::build { 
	/// @brief performs a intersect set operation on the 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuples
	template<COMPARE Same_T, typename Set_T> struct set_intersect { template<typename Tup> using type = util::trn::set_intersect<Tup, Set_T, Same_T>; };
}

// [X] assign
namespace util::trn::build {
	/// @brief transforms type to new type 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuples
	template<typename New_T> struct assign { template<typename T> using type = std::type_identity<New_T>; };
 }

// [X] negate compare
namespace util::cmp::build {
	/// @brief negates a compare predicate
	/// @tparam Cmp_T compare operation
	template<COMPARE Cmp_T> struct negate { 
		template<typename LHS_T, typename RHS_T> using type = std::negation<Cmp_T<LHS_T, RHS_T>>; };
}

// [X] compare transformed
namespace util::cmp::build {
	/// @brief transforms types before comparing them.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, TRANSFORM Trans_T = util::trn::build::set<>::type> struct transformed { 
		template<typename LHS_T, typename RHS_T> using type = Cmp_T<typename Trans_T<LHS_T>::type, typename Trans_T<RHS_T>::type>;
		template<typename LHS_T, typename RHS_T> using negated = std::negation<Cmp_T<typename Trans_T<LHS_T>::type, typename Trans_T<RHS_T>::type>>;
	};
}

// [X] prioritize if
namespace util::cmp::build {

	/// @brief constructs a compare template predicate from a match template predicate.
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM Trans_T = util::trn::build::set<>::type> struct prioritize_if {
		template<typename LHS_T, typename RHS_T> using type = std::conjunction<Match_T<typename Trans_T<LHS_T>::type>, std::negation<Match_T<typename Trans_T<RHS_T>::type>>>;
		template<typename LHS_T, typename RHS_T> using negated = std::disjunction<std::negation<Match_T<typename Trans_T<LHS_T>::type>>, Match_T<typename Trans_T<RHS_T>::type>>;
	};
}

// [X] negate match
namespace util::mtc::build {
	/// @brief returns true if not matched
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	template<MATCH Match_T> struct negate { template<typename T> using type = std::negation<Match_T<T>>; };
}

// [X] match transformed
namespace util::mtc::build {
	/// @brief constructs a match template predicate, transforms input arguments before passing to match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM Trans_T> struct transformed { 
		template<typename T> using type = Match_T<typename Trans_T<T>::type>; 
		template<typename T> using negated = std::negation<Match_T<typename Trans_T<T>::type>>; };
}

// [X] post_conditional
namespace util::trn {

	template<typename T, TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = trn::build::set<>::type, typename=void>
	struct post_conditional;
	template<typename T, TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = trn::build::set<>::type>
	using post_conditional_t = typename post_conditional<T, Trans_T, Match_T, If_T, Else_T>::type;

	template<typename T, TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T>
	struct post_conditional<T, Trans_T, Match_T, If_T, Else_T, std::enable_if_t<Match_T<T>::value>> { using type = typename If_T<typename Trans_T<T>::type>::type; };
	
	template<typename T, TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T>
	struct post_conditional<T, Trans_T, Match_T, If_T, Else_T, std::enable_if_t<!Match_T<T>::value>> { using type = typename Else_T<typename Trans_T<T>::type>::type; };
}

namespace util::trn::build {
	template<TRANSFORM Trans_T, MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = trn::build::set<>::type>
	struct post_conditional { template<typename T> using type = trn::post_conditional<T, Trans_T, Match_T, If_T, Else_T>; };
}






#undef TRANSFORM
#undef MATCH
#undef COMPARE
#undef TEMPLATE