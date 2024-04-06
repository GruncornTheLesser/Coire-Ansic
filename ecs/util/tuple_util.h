#ifndef ECS_TUPLE_UTIL_H
#define ECS_TUPLE_UTIL_H

#include <tuple>
#include <type_traits>
#include "type_name.h"

#define TRANSFORM template<typename> typename
#define MATCH template<typename> typename
#define COMPARE template<typename,typename> typename
#define TEMPLATE template<typename...> typename

namespace util {
	/// @brief a template compare predicate builder, transforms arguments before comparison
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts> struct cmp;
	/// @brief an inverse compare predicate constructor, an inverse predicate inverse the "value" of the Compare template predicate
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts> struct inv_cmp;
	/// @brief constructs a compare template predicate from a match template predicate.
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct cmp_if;
	/// @brief constructs an inverse match from a template compare predicate, ie if A and not B
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct inv_cmp_if;
	/// @brief constructs a match template predicate, transforms input arguments before passing to match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct match;
	/// @brief constructs an inverse match template predicate, transforms input arguments before passing to match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct inv_match;
	/// @brief constructs a match template predicate from a compare template predicate
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam LHS_T eg int
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, typename LHS_T, TRANSFORM ... Trans_Ts> struct cmp_to;
	/// @brief constructs an inverse match template predicate from a compare template predicate
	/// @tparam LHS_T 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, typename LHS_T, TRANSFORM ... Trans_Ts> struct inv_cmp_to;
	/// @brief constructs an inverse match template predicate from a compare template predicate
	/// @tparam MATCH_T 
	/// @tparam Trans_Ts a template taking 1 typename arg, with member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct conditional_transform;

		

	/// @brief concatenates each tuple in input tuples together
	/// @tparam Tups the input tuple
	template<typename ... Tups> struct tuple_concat;
	template<typename ... Ts> using tuple_concat_t = tuple_concat<Ts...>::type;

	/// @brief filters tuple for types that match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Tup the input tuple
	template<MATCH Match_T, typename Tup, typename=void> struct tuple_filter;
	template<MATCH Match_T, typename Tup> using tuple_filter_t = tuple_filter<Match_T, Tup>::type;
	
	/// @brief returns true if any of types in tuple match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Tup the input tuple
	template<MATCH Match_T, typename Tup> struct tuple_anyof;
	template<MATCH Match_T, typename Tup> static constexpr bool tuple_anyof_v = tuple_anyof<Match_T, Tup>::value;

	/// @brief returns true if all of types in tuple return true in match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Tup the input tuple
	template<MATCH Match_T, typename Tup> struct tuple_allof;
	template<MATCH Match_T, typename Tup> static constexpr bool tuple_allof_v = tuple_allof<Match_T, Tup>::value;
	
	/// @brief sorts tuple according to Cmp_T sort func
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Tup the input tuple
	template<COMPARE Cmp_T, typename Tup> struct tuple_sort;
	template<COMPARE Cmp_T, typename Tup> using tuple_sort_t = tuple_sort<Cmp_T, Tup>::type;
	
	/// @brief performs a union set operation on the 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Tup the input tuple
	template<COMPARE Cmp_T, typename ... Tups> struct tuple_union;
	template<COMPARE Cmp_T, typename ... Tups> using tuple_union_t = typename tuple_union<Cmp_T, Tups...>::type;
	
	/// @brief performs a intersect set operation on the 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Tups the input tuples
	template<COMPARE Cmp_T, typename ... Tups> struct tuple_intersect;
	template<COMPARE Cmp_T, typename ... Tups> using tuple_intersect_t = typename tuple_intersect<Cmp_T, Tups...>::type;
	
	/// @brief transforms each type in input tuple 
	/// @tparam Trans_T a template taking 1 typename arg, with member alias "type", eg std::remove_const
	/// @tparam Tups the input tuple
	template<TRANSFORM Trans_T, typename Tup> struct tuple_transform;
	template<TRANSFORM Trans_T, typename Tup> using tuple_transform_t = typename tuple_transform<Trans_T, Tup>::type;

	template<typename T, TRANSFORM ... Trans_Ts> struct type_transform;
	template<typename T, TRANSFORM ... Trans_Ts> using type_transform_t = typename type_transform<T, Trans_Ts...>::type;

	








	template<MATCH Match_T, TRANSFORM ... Trans_Ts>
	struct match {
		template<typename T>
		using type = Match_T<typename type_transform<T, Trans_Ts...>::type>;
	};

	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts>
	struct cmp {
		template<typename LHS_T, typename RHS_T>
		using type = Cmp_T<typename type_transform<LHS_T, Trans_Ts...>::type, typename type_transform<RHS_T, Trans_Ts...>::type>;
	};

	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts>
	struct cmp_transform_lhs {
		template<typename LHS_T, typename RHS_T>
		using type = Cmp_T<typename type_transform<LHS_T, Trans_Ts...>::type, RHS_T>;
	};

	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts>
	struct cmp_transform_rhs {
		template<typename LHS_T, typename RHS_T>
		using type = Cmp_T<LHS_T, typename type_transform<RHS_T, Trans_Ts...>::type>;
	};

	template<COMPARE Cmp_T, COMPARE ... Cmp_Ts>
	struct cmp_conjunction {
		template<typename LHS_T, typename RHS_T>
		using type = std::conjunction<Cmp_T<LHS_T, RHS_T>, Cmp_Ts<LHS_T, RHS_T>...>;
	};

	template<COMPARE Cmp_T, COMPARE ... Cmp_Ts>
	struct cmp_disjunction {
		template<typename LHS_T, typename RHS_T>
		using type = std::disjunction<Cmp_T<LHS_T, RHS_T>, Cmp_Ts<LHS_T, RHS_T>...>;
	};

	template<MATCH Match_T, TRANSFORM ... Trans_Ts>
	struct inv_match {
		template<typename T>
		using type = std::negation<Match_T<typename type_transform<T, Trans_Ts...>::type>>;
	};

	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts>
	struct inv_cmp {
		template<typename LHS_T, typename RHS_T>
		using type = std::negation<Cmp_T<typename type_transform<LHS_T, Trans_Ts...>::type, typename type_transform<RHS_T, Trans_Ts...>::type>>;
	};
	

	template<COMPARE Cmp_T, typename LHS_T, TRANSFORM ... Trans_Ts>
	struct cmp_to {
		template<typename RHS_T>
		using type = cmp<Cmp_T, Trans_Ts...>::template type<LHS_T, RHS_T>;
	};

	template<MATCH Match_T, TRANSFORM ... Trans_Ts>
	struct cmp_if {
		template<typename LHS_T, typename RHS_T>
		using type = std::conjunction<Match_T<typename type_transform<LHS_T, Trans_Ts...>::type>, 
			            std::negation<Match_T<typename type_transform<RHS_T, Trans_Ts...>::type>>>;
	};

	template<COMPARE Cmp_T, typename LHS_T, TRANSFORM ... Trans_Ts> 
	struct inv_cmp_to {
		template<typename RHS_T>
		using type = std::negation<Cmp_T<
			typename type_transform<LHS_T, Trans_Ts...>::type, 
			typename type_transform<RHS_T, Trans_Ts...>::type>>;
	};

	template<MATCH Match_T, TRANSFORM ... Trans_Ts> 
	struct inv_cmp_if {
		template<typename LHS_T, typename RHS_T>
		using type = std::disjunction<Match_T<typename type_transform<RHS_T, Trans_Ts...>::type>, 
			            std::negation<Match_T<typename type_transform<LHS_T, Trans_Ts...>::type>>>;
	};

	template<MATCH Match_T, TRANSFORM ... Trans_Ts>
	struct conditional_transform {
		template<typename T>
		using type = std::conditional<Match_T<T>::value, type_transform_t<T, Trans_Ts...>, T>;
	};











	template<TEMPLATE Tup1, typename ... T1s, TEMPLATE Tup2, typename ... T2s, typename ... Tups>
	struct tuple_concat<Tup1<T1s...>, Tup2<T2s...>, Tups...> { 
		using type = tuple_concat<Tup1<T1s..., T2s...>, Tups...>::type;
	};


	template<TEMPLATE Tup, typename ... Ts>
	struct tuple_concat<Tup<Ts...>> {
		using type = Tup<Ts...>;
	};

	template<>
	struct tuple_concat<> {
		using type = std::tuple<>;
	};

	
	template<MATCH Match_T, TEMPLATE Tup, typename T, typename ... Ts>
	struct tuple_filter<Match_T, Tup<T, Ts...>, std::enable_if_t<Match_T<T>::value>> : 
		tuple_concat<Tup<T>, typename tuple_filter<Match_T, Tup<Ts...>>::type>
	{ };

	template<MATCH Match_T, TEMPLATE Tup, typename T, typename ... Ts>
	struct tuple_filter<Match_T, Tup<T, Ts...>, std::enable_if_t<!Match_T<T>::value>> : 
		tuple_filter<Match_T, Tup<Ts...>>
	{ };

	template<MATCH Match_T, TEMPLATE Tup>
	struct tuple_filter<Match_T, Tup<>, void> { using type = Tup<>; };


	

	template<MATCH Match_T, TEMPLATE Tup, typename ... Ts>
	struct tuple_anyof<Match_T, Tup<Ts...>> : std::disjunction<Match_T<Ts>...> { };
	

	template<MATCH Match_T, TEMPLATE Tup, typename ... Ts>
	struct tuple_allof<Match_T, Tup<Ts...>> : std::conjunction<Match_T<Ts>...> { };
	
	template<COMPARE Cmp_T, typename T, typename Tup>
	struct tuple_contains : tuple_anyof<cmp_to<Cmp_T, T>::template type, Tup> { };

	template<COMPARE Cmp_T, typename T, typename Tup>
	static constexpr bool tuple_contains_v = tuple_contains<Cmp_T, T, Tup>::value;



	template<COMPARE Cmp_T, TEMPLATE Tup, typename ... Ts>
	struct tuple_sort<Cmp_T, Tup<Ts...>> : tuple_concat<Tup<>, typename tuple_sort<Cmp_T, std::tuple<Ts...>>::type> { };

	template<COMPARE Cmp_T, typename T, typename ... Ts>
	struct tuple_sort<Cmp_T, std::tuple<T, Ts...>> : util::tuple_concat<
		typename tuple_sort<Cmp_T, tuple_filter_t<cmp_to<Cmp_T, T>::template type, std::tuple<Ts...>>>::type, 
		std::tuple<T>,
		typename tuple_sort<Cmp_T, tuple_filter_t<inv_match<cmp_to<Cmp_T, T>::template type>::template type, 
		std::tuple<Ts...>>>::type>
	{ };
	
	template<COMPARE Cmp_T>
	struct tuple_sort<Cmp_T, std::tuple<>> {
		using type = std::tuple<>;
	};

	



	template<COMPARE Cmp_T, typename ... Tups>
	struct tuple_union : tuple_union<Cmp_T, util::tuple_concat_t<Tups...>>  { };
	
	template<COMPARE Cmp_T, TEMPLATE Tup, typename T, typename ... Ts>
	struct tuple_union<Cmp_T, Tup<T, Ts...>> : tuple_concat<Tup<T>, 
		typename tuple_union<Cmp_T, tuple_filter_t<
		inv_match<cmp_to<Cmp_T, T>::template type>::template type, 
		std::tuple<Ts...>>>::type>
	{ };

	template<COMPARE Cmp_T, TEMPLATE Tup>
	struct tuple_union<Cmp_T, Tup<>> {
		using type = Tup<>;
	};

	



	template<COMPARE Cmp_T, typename ... Tups>
	struct tuple_intersect {
		// T is contained in all Tups...
		template<typename T>
		using Pred = std::conjunction<tuple_anyof<cmp_to<Cmp_T, T>::template type, Tups>...>;
		using type = tuple_filter_t<Pred, tuple_union_t<Cmp_T, Tups...>>;
	};


	template<COMPARE Cmp_T> 
	struct tuple_intersect<Cmp_T> {
		using type = std::tuple<>;
	};

	template<COMPARE Cmp_T, typename ... Tups>
	using tuple_intersect_t = typename tuple_intersect<Cmp_T, Tups...>::type;




	template<COMPARE Cmp_T, typename SubSet_T, typename SuperSet_T>
	struct tuple_subset;

	template<COMPARE Cmp_T, TEMPLATE Sub, typename ... Ts, typename SuperSet_T>
	struct tuple_subset<Cmp_T, Sub<Ts...>, SuperSet_T> : std::conjunction<tuple_anyof<cmp_to<Cmp_T, Ts>::template type, SuperSet_T>...> { };

	template<COMPARE Cmp_T, typename SubSet_T, typename SuperSet_T>
	static constexpr bool tuple_subset_v = tuple_subset<Cmp_T, SubSet_T, SuperSet_T>::value;


	template<TRANSFORM Trans_T, TEMPLATE Tup, typename ... Ts>
	struct tuple_transform<Trans_T, Tup<Ts...>> {
		using type = Tup<typename Trans_T<Ts>::type...>;
	};
	template<TRANSFORM Trans_T, typename Tup>
	using tuple_transform_t = typename tuple_transform<Trans_T, Tup>::type;

		template<typename T, TRANSFORM Trans_T, TRANSFORM ... Trans_Ts>
	struct type_transform<T, Trans_T, Trans_Ts...> {
		using type = type_transform<typename Trans_T<T>::type, Trans_Ts...>::type;
	};

	template<typename T>
	struct type_transform<T> {
		using type = T;
	};
}

#undef TRANSFORM
#undef MATCH
#undef COMPARE
#undef TEMPLATE

#endif