#pragma once
#include <type_traits>
#include <tuple>
#define TRANSFORM template<typename> typename
#define MATCH template<typename> typename
#define COMPARE template<typename,typename> typename
#define TEMPLATE template<typename...> typename

// TODO: add easy pre transform to compare and match operations???
// TODO: naming convention for match, add is to front

// [ ] concat
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

// [ ] set
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

// [ ] negate match
namespace util::mtc::build {
	/// @brief returns true if not matched
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	template<MATCH Match_T> struct negation { template<typename T> using type = std::negation<Match_T<T>>; };
}

// [ ] negate compare
namespace util::cmp::build {
	/// @brief negates a compare predicate
	/// @tparam Cmp_T compare operation
	template<COMPARE Cmp_T> struct negate { 
		template<typename LHS_T, typename RHS_T> using type = std::negation<Cmp_T<LHS_T, RHS_T>>; };
}


// [ ] conditional
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
	template<MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T = std::type_identity> struct conditional { 
		template<typename T> using type = trn::conditional<T, Match_T, If_T, Else_T>; 
		template<typename T> using negated = trn::conditional<T, mtc::build::negation<Match_T>::template type, If_T, Else_T>;
	};

}

// [ ] each
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

// [ ] filter
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

// [ ] compare to
// TODO: swap/explicity say LHS RHS in compare_to operation
namespace util::mtc::build {
	template<typename LHS_T, COMPARE Cmp_T> struct compare_to { 
		template<typename RHS_T> using type = Cmp_T<LHS_T, RHS_T>; 
		template<typename RHS_T> using negated = std::negation<type<RHS_T>>; 
	};
}

// [ ] sort
namespace util::trn {
	/// @brief sorts tuple by Cmp_T func.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuple
	template<typename Tup, COMPARE LT_T> struct sort;
	template<typename Tup, COMPARE LT_T> using sort_t = typename sort<Tup, LT_T>::type;

	template<TEMPLATE Tup, typename T, typename ... Ts, COMPARE Cmp_T>
	struct sort<Tup<T, Ts...>, Cmp_T> : util::trn::concat<std::tuple<
		sort_t<filter_t<Tup<Ts...>, mtc::build::compare_to<T, Cmp_T>::template negated>, Cmp_T>, Tup<T>, // not less than
		sort_t<filter_t<Tup<Ts...>, mtc::build::compare_to<T, Cmp_T>::template type>, Cmp_T>>> // less than
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
// [ ] 
namespace util::trn {
	template<typename Tup, COMPARE Same_T>
	struct unique;
	
	template<typename Tup, COMPARE Same_T>
	using unique_t = typename unique<Tup, Same_T>::type;
	

	template<TEMPLATE Tup, typename T, typename ... Ts, COMPARE Same_T>
	struct unique<Tup<T, Ts...>, Same_T> : 
		concat<std::tuple<Tup<T>, unique_t<filter_t<std::tuple<Ts...>, mtc::build::compare_to<T, Same_T>::template negated>, Same_T>>>
	{ };

	template<TEMPLATE Tup, COMPARE Same_T>
	struct unique<Tup<>, Same_T> { using type = Tup<>; };
}
namespace util::trn::build {
	template<COMPARE Same_T>
	struct unique { template<typename Tup> using type = trn::unique<Tup, Same_T>; };
}

// [ ] set_union
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

// [ ] rewrap
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

// [ ] wrap
namespace util::trn {
	/// @brief wraps passed arg in Tup
	template<typename T, TEMPLATE Tup> struct wrap { using type = Tup<T>; };
	template<typename T, TEMPLATE Tup> using wrap_t = typename wrap<T, Tup>::type;
}
namespace util::trn::build {
	/// @brief wraps passed arg in Tup
	template<TEMPLATE Tup> struct wrap { template<typename T> using type = util::trn::wrap<T, Tup>; };
}

// [ ] match disjunction
namespace util::mtc::build {
	template<MATCH ... Match_Ts>
	struct disjunction { 
		template<typename T> using type = std::disjunction<Match_Ts<T>...>; 
		template<typename T> using negated = std::negation<type<T>>;
	};
}

// [ ] match conjunction
namespace util::mtc::build {
	template<MATCH ... Match_Ts>
	struct conjunction { 
		template<typename T> using type = std::conjunction<Match_Ts<T>...>; 
		template<typename T> using negated = std::negation<type<T>>;
	};
}

// [ ] compare disjunction
namespace util::cmp::build {
	template<COMPARE ... Cmp_Ts>
	struct disjunction { 
		template<typename LHS_T, typename RHS_T> using type = std::disjunction<Cmp_Ts<LHS_T, RHS_T>...>;
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>; 
	};
}

// [ ] compare conjunction
namespace util::cmp::build {
	template<COMPARE ... Cmp_Ts>
	struct conjunction  { 
		template<typename LHS_T, typename RHS_T> using type = std::conjunction<Cmp_Ts<LHS_T, RHS_T>...>; 
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>;
	};
}

// [ ] all of
namespace util::mtc {
	/// @brief returns true if any of types in tuple match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	template<typename Tup, MATCH Match_T> struct allof : util::trn::set_t<Tup,
		trn::build::each<trn::build::wrap<Match_T>::template type>::template type,
		trn::build::rewrap<std::conjunction>::template type> { };
	template<typename Tup, MATCH Match_T> static constexpr bool allof_v = allof<Tup, Match_T>::value;
}
namespace util::mtc::build {
	/// @brief returns true if any of types in tuple match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	template<MATCH Match_T> struct allof { template<typename Tup> using type = mtc::allof<Tup, Match_T>; };
}

static_assert(util::mtc::allof_v<std::tuple<const int, const float>, std::is_const>, "");
static_assert(!util::mtc::allof_v<std::tuple<const int, float>, std::is_const>, "");

// [ ] any of
namespace util::mtc {
	template<typename Tup, MATCH Match_T> struct anyof : util::trn::set_t<Tup,
			trn::build::each<trn::build::wrap<Match_T>::template type>::template type,
			trn::build::rewrap<std::disjunction>::template type> { }; 
	template<typename Tup, MATCH Match_T> static constexpr bool anyof_v = anyof<Tup, Match_T>::value;
}
namespace util::mtc::build {
	/// @brief returns true if all of types in tuple return true in match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	template<MATCH Match_T> struct anyof { 
		template<typename Tup> using type = mtc::anyof<Tup, Match_T>;
	};
}

static_assert(!util::mtc::anyof_v<std::tuple<int, float>, std::is_const>, "");
static_assert(util::mtc::anyof_v<std::tuple<const int, float>, std::is_const>, "");


// [ ] element of/contains
namespace util::mtc {
	template<typename T, typename Tup, COMPARE Cmp_T = std::is_same>
	struct element_of : anyof<Tup, build::compare_to<T, Cmp_T>::template type> { };
	template<typename T, typename Tup, COMPARE Cmp_T = std::is_same>
	static constexpr bool element_of_v = element_of<T, Tup, Cmp_T>::value;
}
namespace util::mtc::build {
	template<typename Tup, COMPARE Cmp_T = std::is_same>
	struct element_of { 
		template<typename T> using type = mtc::element_of<T, Tup, Cmp_T>; 
		template<typename T> using negated = std::negation<type<T>>;
	};

	template<typename T, COMPARE Cmp_T = std::is_same>
	struct contains { 
		template<typename Tup> using type = mtc::element_of<T, Tup>; 
		template<typename Tup> using negated = std::negation<type<Tup>>;
	};
}

static_assert(util::mtc::element_of_v<int, std::tuple<float, int>>, "");
static_assert(!util::mtc::element_of_v<int, std::tuple<float, char>>, "");

// [ ] set_intersect
namespace util::trn {
	/// @brief performs a intersect set operation on the 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuples
	template<typename Tup1, typename Tup2, COMPARE Same_T = std::is_same> struct set_intersect;
	template<typename Tup1, typename Tup2, COMPARE Same_T> struct set_intersect : 
	filter<set_union_t<Tup1, Tup2, Same_T>, mtc::build::conjunction<
		mtc::build::element_of<Tup1, Same_T>::template type, 
		mtc::build::element_of<Tup2, Same_T>::template type
	>::template type> { };
	template<typename Tup, typename Set_T, COMPARE Same_T = std::is_same> using set_intersect_t = typename set_intersect<Tup, Set_T, Same_T>::type;
}
namespace util::trn::build { 
	/// @brief performs a intersect set operation on the 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuples
	template<COMPARE Same_T, typename Set_T> struct set_intersect { template<typename Tup> using type = util::trn::set_intersect<Tup, Set_T, Same_T>; };
}

// [ ] assign
namespace util::trn::build {
	/// @brief transforms type to new type 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam ::type<Tup> the input tuples
	template<typename New_T> struct assign { template<typename T> using type = std::type_identity<New_T>; };
 }


// [ ] compare transformed
namespace util::cmp {
	/// @brief transforms types before comparing them.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM Trans_T> 
	struct transformed : Cmp_T<typename Trans_T<LHS_T>::type, typename Trans_T<RHS_T>::type> { };
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM Trans_T>
	static constexpr bool transformed_v = transformed<LHS_T, RHS_T, Cmp_T, Trans_T>::value;
	
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM LHS_Trans_T, TRANSFORM RHS_Trans_T> 
	struct transformed_individual : Cmp_T<typename LHS_Trans_T<LHS_T>::type, typename RHS_Trans_T<RHS_T>::type> { };
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM LHS_Trans_T, TRANSFORM RHS_Trans_T> 
	static constexpr bool transformed_individual_v = transformed_individual<LHS_T, RHS_T, Cmp_T, LHS_Trans_T, RHS_Trans_T>::value;
	
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM RHS_Trans_T> 
	struct transformed_rhs : transformed_individual<LHS_T, RHS_T, Cmp_T, std::type_identity, RHS_Trans_T> { };
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM RHS_Trans_T>
	static constexpr bool transformed_rhs_v = transformed_rhs<LHS_T, RHS_T, Cmp_T, RHS_Trans_T>::value;
	
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM LHS_Trans_T> 
	struct transformed_lhs : transformed_individual<LHS_T, RHS_T, Cmp_T, LHS_Trans_T, std::type_identity> { };
	template<typename LHS_T, typename RHS_T, COMPARE Cmp_T, TRANSFORM LHS_Trans_T> 
	static constexpr bool transformed_lhs_v = transformed_lhs<LHS_T, RHS_T, Cmp_T, LHS_Trans_T>::value;
	
}
namespace util::cmp::build {
	/// @brief transforms types before comparing them.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, TRANSFORM Trans_T> struct transformed { 
		template<typename LHS_T, typename RHS_T> using type = cmp::transformed<LHS_T, RHS_T, Cmp_T, Trans_T>;
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>;
	};

	template<COMPARE Cmp_T, TRANSFORM LHS_Trans_T, TRANSFORM RHS_Trans_T>
	struct transformed_individual { 
		template<typename LHS_T, typename RHS_T> using type = cmp::transformed_individual<LHS_T, RHS_T, Cmp_T, LHS_Trans_T, RHS_Trans_T>;
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>;
	};

	template<COMPARE Cmp_T, TRANSFORM RHS_Trans_T> struct transformed_rhs : transformed_individual<Cmp_T, std::type_identity, RHS_Trans_T> { };
	template<COMPARE Cmp_T, TRANSFORM LHS_Trans_T> struct transformed_lhs : transformed_individual<Cmp_T, LHS_Trans_T, std::type_identity> { };
}

static_assert(util::cmp::transformed_v<int, const int, std::is_same, std::remove_const>, "");
static_assert(!util::cmp::transformed_v<int, const float, std::is_same, std::remove_const>, "");

static_assert(util::cmp::transformed_lhs_v<int, const int, std::is_same, std::add_const>, "");
static_assert(!util::cmp::transformed_rhs_v<int, const float, std::is_same, std::add_const>, "");



// [ ] prioritize if
namespace util::cmp {
	template<typename LHS_T, typename RHS_T, MATCH Match_T> struct prioritize_if : 
		std::disjunction<std::negation<Match_T<LHS_T>>, Match_T<RHS_T>> { };
	
	template<typename LHS_T, typename RHS_T, MATCH Match_T> 
	static constexpr bool prioritize_if_v = prioritize_if<LHS_T, RHS_T, Match_T>::value;
}
namespace util::cmp::build {
	
	// TODO: finish compare priority list 
	/// @brief constructs a compare template predicate from a match template predicate.
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T> struct prioritize_if {
		template<typename LHS_T, typename RHS_T> using type = cmp::prioritize_if<LHS_T, RHS_T, Match_T>;
		template<typename LHS_T, typename RHS_T> using negated = std::negation<type<LHS_T, RHS_T>>;
	};
}

// [ ] match transformed
namespace util::mtc {
	template<typename T, MATCH Match_T, TRANSFORM Trans_T> struct transformed : Match_T<typename Trans_T<T>::type> { };
	template<typename T, MATCH Match_T, TRANSFORM Trans_T> using transformed_v = transformed<T, Match_T, Trans_T>::value;
}
namespace util::mtc::build {
	/// @brief constructs a match template predicate, transforms input arguments before passing to match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM Trans_T> struct transformed { 
		template<typename T> using type = Match_T<typename util::mtc::transformed<T, Match_T, Trans_T>::type>; 
		template<typename T> using negated = std::negation<type<T>>; };
}

// [ ] post_conditional
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

// [ ] propergate_const
namespace util::trn {
	template<typename T, TRANSFORM Trans_T>
	struct propergate_const : trn::post_conditional<T, Trans_T, std::is_const, std::add_const> { };
	template<typename T, TRANSFORM Trans_T>
	using propergate_const_t = typename propergate_const<T, Trans_T>::type; 

	template<typename T, TRANSFORM Trans_T>
	struct propergate_const_each : util::trn::post_conditional<T, Trans_T, std::is_const, util::trn::build::each<std::add_const>::template type> { };
	template<typename T, TRANSFORM Trans_T>
	using propergate_const_each_t = typename propergate_const_each<T, Trans_T>::type;
	
}
namespace util::trn::build {
	template<TRANSFORM Trans_T>
	struct propergate_const { template<typename T> using type = trn::propergate_const<T, Trans_T>; };
	template<TRANSFORM Trans_T>
	struct propergate_const_each { template<typename T> using type = trn::propergate_const_each<T, Trans_T>; };
}

// [ ] subset
namespace util::mtc::build {
	template<typename SuperSet_T, COMPARE Cmp_T>
	struct subset_of;

	template<typename SuperSet_T, COMPARE Same_T>
	struct subset_of : mtc::build::allof<mtc::build::element_of<SuperSet_T, Same_T>::template type> { };
	
	template<typename SubSet_T, typename SuperSet_T, COMPARE Same_T>
	static constexpr bool subset_of_v = subset_of<SuperSet_T, Same_T>::template type<SubSet_T>::value;
}

#undef TRANSFORM
#undef MATCH
#undef COMPARE
#undef TEMPLATE