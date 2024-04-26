#include "view.h" 
// pipeline requires view.h but view.tpp requires pipeline.h
// this is probably bad

#ifndef ECS_PIPELINE_H
#define ECS_PIPELINE_H
#include "util/tuple_util.h"
#include "traits.h"
#include "policy.h"

// TODO: isolate template members eg resource set to traits class -> allow for defaulting for eg component 

namespace ecs {
	template<typename ... Ts>
	struct pipeline_t {
		using resource_container_set = std::tuple<Ts...>;
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

		template<traits::pipeline_accessible_class<pipeline_t<Ts...>> U>
		U& get_resource();

		template<typename U, template<typename> typename Policy_U = ecs::deferred, typename ... Arg_Us> 
			requires (traits::is_pipeline_accessible_v<Policy_U<U>, pipeline_t<Ts...>>) && 
			std::is_constructible_v<U, Arg_Us...>
		U& emplace(ecs::entity e, Arg_Us&& ... args);
		
		template<typename U, template<typename> typename Policy_U = ecs::deferred, typename It, typename ... Arg_Us> 
			requires (traits::is_pipeline_accessible_v<Policy_U<U>, pipeline_t<Ts...>>) && 
			std::is_constructible_v<U, Arg_Us...>
		U& emplace(It first, It last, Arg_Us&& ... args);

		template<typename U, typename Policy_U, typename ... Arg_Us> 
			requires (traits::is_pipeline_accessible_v<Policy_U, pipeline_t<Ts...>>) && 
			std::is_constructible_v<U, Arg_Us...>
		U& emplace(Policy_U pol, ecs::entity e, Arg_Us&& ... args);
		
		template<typename U, typename Policy_U, typename It, typename ... Arg_Us>
			requires (traits::is_pipeline_accessible_v<Policy_U, pipeline_t<Ts...>>)
		U& emplace(Policy_U pol, It first, It last, Arg_Us&& ... args);

		template<typename U, template<typename> typename Policy_U = deferred> 
			requires (traits::is_pipeline_accessible_v<Policy_U<U>, pipeline_t<Ts...>>)
		void erase(ecs::entity e);

		template<typename U, template<typename> typename Policy_U = deferred, typename It> 
			requires (traits::is_pipeline_accessible_v<Policy_U<U>, pipeline_t<Ts...>>)
		void erase(It first, It last);

		template<traits::pipeline_accessible_class<pipeline_t<Ts...>> ... Select_Us, 
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
