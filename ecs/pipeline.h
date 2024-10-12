#pragma once
#include <concepts>
#include <span>
#include "../resource/set.h"
#include "traits.h"
#include "entity.h"
#include "policy.h"

/* 
resource acquisition pattern:

+res_A
+on_lock<res_A>
-on_lock<res_A>

+on_unlock<res_A>
-on_unlock<res_A>
-res_B




*/



namespace ecs {
	template<typename ... Ts>
	class pipeline : public resource::set<traits::get_tag_t<Ts>...>
	{
		using set_type = resource::set<traits::get_tag_t<Ts>...>;

	public:
		template<typename ... Us>
		static constexpr bool is_accessible_v =(resource::traits::is_accessible_v<traits::get_tag_t<Us>, set_type> && ...);

		template<typename base_T>
		pipeline(base_T& base) : set_type(base) { }

		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred,
			typename ... arg_Ts>
		void emplace(handle<T> e, arg_Ts&& ... args) requires (is_accessible_v<typename seq_T::template emplace_back<T>>)
		{

			// get_dispatcher<policy::deferred>() ^= typename seq_T::template emplace_back<T, arg_Ts...>{ e, std::forward<arg_Ts>(args)... };
		}

		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred>
		void erase(handle<T> e);

		template<traits::component_class T,
			traits::execution_policy_class exec_T = policy::deferred>
		void clear();

		template<traits::component_class T>
		void sync() requires (is_accessible_v<T>)
		{
			auto& mng = set_type::template get_resource<manager<T>>(); // packed entity array
			auto& ind = set_type::template get_resource<indexer<T>>(); // entity index look_up
			auto& str = set_type::template get_resource<storage<T>>(); // packed component array
		}

		template<traits::component_class T>
		T& get_component(handle<T> e) requires (is_accessible_v<const indexer<T>, storage<T>>)
		{
			auto& str = set_type::template get_resource<storage<T>>();
			auto& ind = set_type::template get_resource<indexer<T>>();
			return str[ind[e]];
		}

		template<traits::component_class T>
		bool has_component(handle<T> e) requires (is_accessible_v<const indexer<T>>)
		{
			return set_type::template get_resource<const indexer<T>>().contains(e);
		}

		template<traits::entity_class handle_T = entity,
			traits::sequence_policy_class seq_T = policy::swap> 
		[[nodiscard]] handle_T create() requires (is_accessible_v<handle_T>)
		{
			return set_type::template get_resource<manager<handle_T>>().create();
		}

		template<traits::entity_class handle_T = entity,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred>
		void destroy(handle_T e) requires (is_accessible_v<handle_T>)
		{
			set_type::template get_resource<manager<handle_T>>().destroy(e);
			// ! must queue component destruction
			// this requires all component pools...
			// uh oh
		}
		template<traits::entity_class handle_T = entity,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred>
		bool alive(handle_T e) requires (is_accessible_v<handle_T>)
		{
			return set_type::template get_resource<manager<handle_T>>().alive(e);
		}
	};
}