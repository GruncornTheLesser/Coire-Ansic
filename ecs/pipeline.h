#include "view.h" 
// pipeline requires view.h but view.tpp requires pipeline.h
// this is probably bad

#ifndef ECS_PIPELINE_H
#define ECS_PIPELINE_H
#include "util/tuple_util.h"
#include "traits.h"
#include "policy.h"

namespace ecs {
	template<typename Resource_Set_T, typename Storage_T, typename=void>
	struct storage_syncer;

	template<typename ... Ts>
	struct pipeline_t {
		using resource_container_set = std::tuple<Ts...>;
		using pipeline_set = 
			util::tuple_transform_t<std::add_pointer, 								// add pointer
			util::tuple_union_t<util::cmp<std::is_same, std::remove_const>::type, 	// erase repeats
			util::tuple_transform_t<traits::get_resource_container, resource_container_set>>>;// get resource containers
		
		template<typename U>
		struct is_accessible : util::tuple_subset<util::cmp_disjunction< // match const with const, mut with const or mut
				util::cmp_transform_rhs<std::is_same, std::remove_const>::type, 
				util::cmp_transform_rhs<std::is_same, std::add_const>::type>::type,
				traits::get_resource_set_t<U>, std::tuple<Ts...>>
		{ };

		template<typename U>
		static constexpr bool is_accessible_v = is_accessible<U>::value;

		template<typename base_T>
		pipeline_t(base_T& base);

		pipeline_t(pipeline_t<Ts...>& other) = delete;
		pipeline_t(pipeline_t<Ts...>&& other);

		pipeline_t& operator=(pipeline_t<Ts...>& other) = delete;
		pipeline_t& operator=(pipeline_t<Ts...>&& other);

		void lock();
		void unlock();
		void sync();

		template<typename U>
		U& get_resource() requires (is_accessible_v<U>);

		template<typename U, template<typename> typename Policy_U = ecs::deferred, typename ... Arg_Us>
		U& emplace(ecs::entity e, Arg_Us&& ... args) requires (util::tuple_allof_v<is_accessible, traits::get_resource_set_t<Policy_U<U>>>);

		template<typename U, template<typename> typename Policy_U = ecs::deferred, typename ... Arg_Us>
		U& emplace(ecs::entity e, Arg_Us&& ... args) requires (util::tuple_allof_v<is_accessible, traits::get_resource_set_t<Policy_U<U>>>);

		template<typename U, template<typename> typename Policy_U = deferred>
		void erase(ecs::entity e) requires (util::tuple_allof_v<is_accessible, traits::get_resource_set_t<Policy_U<U>>>);

		template<typename ... Select_Us, 
			typename from_T = ecs::view_from_builder<ecs::select<Select_Us...>>::type, 
			typename where_T = ecs::view_where_builder<ecs::select<Select_Us...>, from_T>::type>
		view<select<Select_Us...>, from_T, where_T, pipeline_t<Ts...>&> view(from_T from = {}, where_T where = {});

		

	private:
		pipeline_set set;
	};

	// gathers and manipulates the set and order of required resources, to maintain consistent locking order and prevent deadlocks
	template<typename ... Ts>
	struct pipeline_builder;

	template<typename ... Ts> // resource_class
	using pipeline = typename pipeline_builder<Ts...>::type;
}

#include "pipeline.tpp"
#endif
