#ifndef ECS_TUPLE_UTIL_H
#define ECS_TUPLE_UTIL_H

#include <tuple>
#include <type_traits>
#include "type_name.h"

#define TRANSFORM template<typename> typename
#define MATCH template<typename> typename
#define COMPARE template<typename,typename> typename
#define TEMPLATE template<typename...> typename

// operations
// tuple/type/ -> arguments passed
// need to distinguish between -> operating on type vs on type in tuple, type as immediate operation vs transform or match class
// prefix type/tuple -> tuple_for_each, type_transform, tuple_contains<Cmp_T, T, Tup>::value, contains<Cmp_T, T>::type<Tup>::value
namespace util {

	#pragma region compare builders
	/// @brief a template compare predicate builder, transforms arguments before comparison
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts> struct cmp;
	/// @brief an inverse compare predicate constructor, an inverse predicate inverse the "value" of the Compare template predicate
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts> struct inv_cmp;
	/// @brief constructs a compare template predicate from a match template predicate.
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct cmp_from_if;
	/// @brief constructs an inverse match from a template compare predicate, ie if A and not B
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct inv_cmp_from_if;
	#pragma endregion
	#pragma region match builders
	/// @brief constructs a match template predicate, transforms input arguments before passing to match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct match;
	/// @brief constructs an inverse match template predicate, transforms input arguments before passing to match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<MATCH Match_T, TRANSFORM ... Trans_Ts> struct inv_match;
	/// @brief constructs a match template predicate from a compare template predicate
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam LHS_T eg int
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, typename LHS_T, TRANSFORM ... Trans_Ts> struct cmp_to;
	/// @brief constructs an inverse match template predicate from a compare template predicate
	/// @tparam LHS_T 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<COMPARE Cmp_T, typename LHS_T, TRANSFORM ... Trans_Ts> struct inv_cmp_to;
	#pragma endregion
	#pragma region transform builder
	/// @brief creates new transform from collection of transform operations executed left to right
	/// @tparam Trans_Ts a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	template<TRANSFORM ... Trans_Ts> struct type_transform; // 
	template<MATCH Match_T, TRANSFORM Trans_T> struct transform_if;
	template<MATCH Match_T, TRANSFORM Trans_T> struct transform_if_not;
	template<MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T> struct transform_if_else;
	template<TRANSFORM ... Trans_Ts> struct transform_each;
	
	

	//TODO: remove transform -> replace with type_transform
	//TODO: maybe?? remove trailing arg transform operations -> replace with type_transform
	/// @brief given a single type, transforms type by each transform passed 
	/// @tparam Trans_T a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	/// @tparam T the input type
	template<typename T, TRANSFORM ... Trans_Ts> struct transform;
	template<typename T, TRANSFORM ... Trans_Ts> using transform_t = typename transform<T, Trans_Ts...>::type;
	#pragma endregion
	#pragma region tuple set operations
	/// @brief concatenates each tuple in input tuples together
	/// @tparam Tups the input tuple
	template<typename ... Tups> struct tuple_concat;
	template<typename ... Ts> using tuple_concat_t = tuple_concat<Ts...>::type;

	/// @brief filters tuple for types that match
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Tup the input tuple
	template<MATCH Match_T, typename Tup, typename=void> struct tuple_filter;
	template<MATCH Match_T, typename Tup> using tuple_filter_t = tuple_filter<Match_T, Tup>::type;
	
	/// @brief sorts tuple by Cmp_T func.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Tup the input tuple
	template<COMPARE LT_T, typename Tup> struct tuple_sort;
	template<COMPARE LT_T, typename Tup> using tuple_sort_t = tuple_sort<LT_T, Tup>::type;
	
	/// @brief performs a union set operation on the Tups passed. removes duplicates.
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Tup the input tuple
	template<COMPARE Same_T, typename ... Tups> struct tuple_union;
	template<COMPARE Same_T, typename ... Tups> using tuple_union_t = typename tuple_union<Same_T, Tups...>::type;
	
	/// @brief performs a intersect set operation on the 
	/// @tparam Cmp_T a template predicate taking 2 typename args, with member bool "value", eg std::is_same
	/// @tparam Tups the input tuples
	template<COMPARE Same_T, typename ... Tups> struct tuple_intersect;
	template<COMPARE Same_T, typename ... Tups> using tuple_intersect_t = typename tuple_intersect<Same_T, Tups...>::type;
	
	#pragma endregion
	#pragma region boolean set operations
	//TODO: maybe?? configure to work as a match operation
	// template<MATCH Match_T> tuple_any_of { template<typename Tup> type = ; };

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
	
	template<COMPARE Same_T, typename T, typename Tup> struct tuple_contains;
	template<COMPARE Same_T, typename T, typename Tup> static constexpr bool tuple_contains_v = tuple_contains<Same_T, T, Tup>::value;
	#pragma endregion
	#pragma region for each operations 
	/// @brief transforms each type in input tuple 
	/// @tparam Trans_T a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	/// @tparam Tups the input tuple
	template<TRANSFORM Trans_T, typename Tup> struct tuple_for_each;
	template<TRANSFORM Trans_T, typename Tup> using tuple_for_each_t = typename tuple_for_each<Trans_T, Tup>::type;

	/// @brief transforms each type in input tuple if type matches
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_T a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	/// @tparam Tups the input tuple
	template<MATCH Match_T, TRANSFORM Trans_T, typename Tup> struct tuple_for_each_if;
	template<MATCH Match_T, TRANSFORM Trans_T, typename Tup> using tuple_for_each_if_t = typename tuple_for_each_if<Match_T, Trans_T, Tup>::type;

	/// @brief transforms each type in input tuple if type matches
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam Trans_T a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	/// @tparam Tups the input tuple
	template<MATCH Match_T, TRANSFORM Trans_T, typename Tup> struct tuple_for_each_if_not;
	template<MATCH Match_T, TRANSFORM Trans_T, typename Tup> using tuple_for_each_if_not_t = typename tuple_for_each_if_not<Match_T, Trans_T, Tup>::type;

	/// @brief transforms each type in input tuple if type matches
	/// @tparam Match_T a template taking 1 argument with a static constexpr boolean value, eg std::is_const
	/// @tparam If_T a template taking 1 typename arg, with a typename member alias "type", eg std::remove_const
	/// @tparam Else_T a template taking 1 typename arg, with type member alias "type", eg std::remove_const
	/// @tparam Tups the input tuple
	template<MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T, typename Tup> struct tuple_for_each_if_else;
	template<MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T, typename Tup> using tuple_for_each_if_else_t = typename tuple_for_each_if_else<Match_T, If_T, Else_T, Tup>::type;

	#pragma endregion


	template<MATCH Match_T, TRANSFORM ... Trans_Ts>
	struct match {
		template<typename T>
		using type = Match_T<transform_t<T, Trans_Ts...>>;
	};

	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts>
	struct cmp {
		template<typename LHS_T, typename RHS_T>
		using type = Cmp_T<transform_t<LHS_T, Trans_Ts...>, transform_t<RHS_T, Trans_Ts...>>;
	};

	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts>
	struct cmp_transform_lhs {
		template<typename LHS_T, typename RHS_T>
		using type = Cmp_T<transform_t<LHS_T, Trans_Ts...>, RHS_T>;
	};

	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts>
	struct cmp_transform_rhs {
		template<typename LHS_T, typename RHS_T>
		using type = Cmp_T<LHS_T, transform_t<RHS_T, Trans_Ts...>>;
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
		using type = std::negation<Match_T<transform_t<T, Trans_Ts...>>>;
	};

	template<COMPARE Cmp_T, TRANSFORM ... Trans_Ts>
	struct inv_cmp {
		template<typename LHS_T, typename RHS_T>
		using type = std::negation<Cmp_T<transform_t<LHS_T, Trans_Ts...>, transform_t<RHS_T, Trans_Ts...>>>;
	};
	

	template<COMPARE Cmp_T, typename LHS_T, TRANSFORM ... Trans_Ts>
	struct cmp_to {
		template<typename RHS_T>
		using type = cmp<Cmp_T, Trans_Ts...>::template type<LHS_T, RHS_T>;
	};

	template<MATCH Match_T, TRANSFORM ... Trans_Ts>
	struct cmp_from_if {
		template<typename LHS_T, typename RHS_T>
		using type = std::conjunction<Match_T<transform_t<LHS_T, Trans_Ts...>>, 
			            std::negation<Match_T<transform_t<RHS_T, Trans_Ts...>>>>;
	};

	template<COMPARE Cmp_T, typename LHS_T, TRANSFORM ... Trans_Ts> 
	struct inv_cmp_to {
		template<typename RHS_T>
		using type = std::negation<Cmp_T<
			transform_t<LHS_T, Trans_Ts...>, 
			transform_t<RHS_T, Trans_Ts...>>>;
	};

	template<MATCH Match_T, TRANSFORM ... Trans_Ts> 
	struct inv_cmp_from_if {
		template<typename LHS_T, typename RHS_T>
		using type = std::disjunction<Match_T<transform_t<RHS_T, Trans_Ts...>>, 
			            std::negation<Match_T<transform_t<LHS_T, Trans_Ts...>>>>;
	};

	
	

	template<TRANSFORM Trans_T, TRANSFORM ... Trans_Ts>
	struct type_transform<Trans_T, Trans_Ts...> {
		template<typename T>
		using type = typename type_transform<Trans_Ts...>::template type<typename Trans_T<T>::type>;
	};

	template<>
	struct type_transform<> {
		template<typename T>
		using type = T;
	};










	template<TEMPLATE Tup1, typename ... T1s, TEMPLATE Tup2, typename ... T2s, typename ... Tups>
	struct tuple_concat<Tup1<T1s...>, Tup2<T2s...>, Tups...> { 
		using type = tuple_concat_t<Tup1<T1s..., T2s...>, Tups...>;
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
		tuple_concat<Tup<T>, tuple_filter_t<Match_T, Tup<Ts...>>>
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



	template<COMPARE Cmp_T, TEMPLATE Tup, typename ... Ts>
	struct tuple_sort<Cmp_T, Tup<Ts...>> : tuple_concat<Tup<>, tuple_sort_t<Cmp_T, std::tuple<Ts...>>> { };

	template<COMPARE Cmp_T, typename T, typename ... Ts>
	struct tuple_sort<Cmp_T, std::tuple<T, Ts...>> : util::tuple_concat<
		tuple_sort_t<Cmp_T, tuple_filter_t<cmp_to<Cmp_T, T>::template type, std::tuple<Ts...>>>, 
		std::tuple<T>,
		tuple_sort_t<Cmp_T, tuple_filter_t<inv_cmp_to<Cmp_T, T>::template type, std::tuple<Ts...>>>>
	{ };
	
	template<COMPARE Cmp_T>
	struct tuple_sort<Cmp_T, std::tuple<>> {
		using type = std::tuple<>;
	};

	



	template<COMPARE Cmp_T, typename ... Tups>
	struct tuple_union : tuple_union<Cmp_T, util::tuple_concat_t<Tups...>>  { };
	
	template<COMPARE Cmp_T, TEMPLATE Tup, typename T, typename ... Ts>
	struct tuple_union<Cmp_T, Tup<T, Ts...>> : tuple_concat<Tup<T>, tuple_union_t<Cmp_T, tuple_filter_t<inv_cmp_to<Cmp_T, T>::template type, std::tuple<Ts...>>>>
	{ };

	template<COMPARE Cmp_T, TEMPLATE Tup>
	struct tuple_union<Cmp_T, Tup<>> {
		using type = Tup<>;
	};

	



	template<COMPARE Cmp_T, typename ... Tups>
	struct tuple_intersect {
	private:
		// T is contained in all Tups...
		template<typename T>
		using Pred = std::conjunction<tuple_anyof<cmp_to<Cmp_T, T>::template type, Tups>...>;
	public:
		using type = tuple_filter_t<Pred, tuple_union_t<Cmp_T, Tups...>>;

	};

	template<COMPARE Cmp_T> 
	struct tuple_intersect<Cmp_T> {
		using type = std::tuple<>;
	};

	template<TRANSFORM Trans_T, TEMPLATE Tup, typename ... Ts>
	struct tuple_for_each<Trans_T, Tup<Ts...>> {
		using type = Tup<typename Trans_T<Ts>::type...>;
	};
	template<TRANSFORM Trans_T, typename Tup>
	using tuple_for_each_t = typename tuple_for_each<Trans_T, Tup>::type;

	template<typename T, TRANSFORM Trans_T, TRANSFORM ... Trans_Ts>
	struct transform<T, Trans_T, Trans_Ts...> {
		using type = transform<typename Trans_T<T>::type, Trans_Ts...>::type;
	};

	template<typename T>
	struct transform<T> {
		using type = T;
	};

	template<MATCH Match_T, TRANSFORM Trans_T>
	struct transform_if {
		template<typename T> 
		using type = std::conditional<Match_T<T>::value, typename Trans_T<T>::type, T>; 
	};
	
	template<MATCH Match_T, TRANSFORM Trans_T> 
	struct transform_if_not {
		template<typename T> 
		using type = std::conditional<Match_T<T>::value, T, typename Trans_T<T>::type>; 
	};

	template<MATCH Match_T, TRANSFORM If_T, TRANSFORM Else_T> 
	struct transform_if_else {
		template<typename T> 
		using type = std::conditional<Match_T<T>::value, typename If_T<T>::type, typename Else_T<T>::type>; 
	};


	template<MATCH Match_T, TRANSFORM Trans_T, typename Tup>
	struct tuple_for_each_if : tuple_for_each<transform_if<Match_T, Trans_T>::template type, Tup> { };

	template<MATCH Match_T, TRANSFORM Trans_T, typename Tup>
	struct tuple_for_each_if_not : tuple_for_each<transform_if_not<Match_T, Trans_T>::template type, Tup> { };

	template<MATCH Match_T, TRANSFORM If_T,TRANSFORM Else_T, typename Tup>
	struct tuple_for_each_if_else : tuple_for_each<transform_if_else<Match_T, If_T, Else_T>::template type, Tup> { };
	
	// TRANSFORM Operation
	template<TRANSFORM ... Trans_Ts>
	struct transform_each {
		template<typename Tup>
		using type = tuple_for_each<type_transform<Trans_Ts...>::template type, Tup>;
	};

	template<COMPARE Cmp_T, typename SubSet_T, typename SuperSet_T>
	struct is_subset;

	template<COMPARE Cmp_T, TEMPLATE Sub, typename ... Ts, typename SuperSet_T>
	struct is_subset<Cmp_T, Sub<Ts...>, SuperSet_T> : std::conjunction<tuple_anyof<cmp_to<Cmp_T, Ts>::template type, SuperSet_T>...> { };

	template<COMPARE Cmp_T, typename SubSet_T, typename SuperSet_T>
	static constexpr bool is_subset_v = is_subset<Cmp_T, SubSet_T, SuperSet_T>::value;




}

#undef TRANSFORM
#undef MATCH
#undef COMPARE
#undef TEMPLATE

#endif