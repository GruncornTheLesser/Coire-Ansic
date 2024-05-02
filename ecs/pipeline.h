#include "view.h" 
// pipeline requires view.h but view.tpp requires pipeline.h
// this is probably bad

#ifndef ECS_PIPELINE_H
#define ECS_PIPELINE_H
#include "util/tuple_util.h"
#include "traits.h"
#include "policy.h"

namespace ecs {
	template<typename ... Ts>
	struct pipeline_t {
		// the set of resources locked by this pipeline
		using resource_set = std::tuple<Ts...>;
		// the set of pointers to containers required for all resource set  
		using pipeline_set = util::trn::set_t<std::tuple<Ts...>, 
			util::trn::build::each<traits::get_resource_container>::template type,
			util::trn::build::wrap<std::tuple>::template type,
			util::trn::build::concat::template type,
			util::trn::build::unique<util::cmp::build::transformed<std::is_same, std::remove_const>::template type>::template type,
			util::trn::build::each<std::add_pointer>::template type
			>;

		template<typename base_T>
		pipeline_t(base_T& base);

		pipeline_t(pipeline_t<Ts...>& other) = delete;
		pipeline_t(pipeline_t<Ts...>&& other);

		pipeline_t& operator=(pipeline_t<Ts...>& other) = delete;
		pipeline_t& operator=(pipeline_t<Ts...>&& other);

		void lock();
		void unlock();
		void sync();

		template<traits::accessible_resource_class<pipeline_t<Ts...>> U>
		U& get_resource();

		template<typename U, template<typename> typename Policy_U = ecs::deferred, typename ... Arg_Us> 
			requires (traits::is_accessible_resource_v<Policy_U<U>, pipeline_t<Ts...>>) && 
			std::is_constructible_v<U, Arg_Us...>
		Policy_U<U>::emplace_return emplace(ecs::entity e, Arg_Us&& ... args);
		
		template<typename U, template<typename> typename Policy_U = ecs::deferred, typename It, typename ... Arg_Us> 
			requires (traits::is_accessible_resource_v<Policy_U<U>, pipeline_t<Ts...>>) && 
			std::is_constructible_v<U, Arg_Us...>
		Policy_U<U>::emplace_return emplace(It first, It last, Arg_Us&& ... args);

		template<typename U, template<typename> typename Policy_U = deferred> 
			requires (traits::is_accessible_resource_v<Policy_U<U>, pipeline_t<Ts...>>)
		void erase(ecs::entity e);

		template<typename U, template<typename> typename Policy_U = deferred, typename It> 
			requires (traits::is_accessible_resource_v<Policy_U<U>, pipeline_t<Ts...>>)
		void erase(It first, It last);

		template<typename ... select_Us, 
			typename from_T = ecs::view_from_builder<ecs::select<select_Us...>>::type, 
			typename where_T = ecs::view_where_builder<ecs::select<select_Us...>, from_T>::type>
			requires (traits::is_accessible_resource_v<std::tuple<ecs::select<select_Us...>, from_T, where_T>, pipeline_t<Ts...>>)
		view<select<select_Us...>, from_T, where_T, pipeline_t<Ts...>&> view(from_T from = {}, where_T where = {});

		

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
