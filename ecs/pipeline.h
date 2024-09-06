#pragma once
#include <concepts>
#include <span>
#include "../resource/set.h"
#include "traits.h"
#include "entity.h"
#include "policy.h"

namespace ecs {

	template<typename ... Ts>
	class pipeline : public resource::set<traits::get_tag_t<Ts>...>
	{
		using set_T = resource::set<traits::get_tag_t<Ts>...>;
		
	public:
		template<typename base_T>
		pipeline(base_T& base) : set_T(base) { }
		
		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred, 
			typename ... arg_Ts> 
		void emplace(handle<T> entt, arg_Ts&& ... args) requires (
			resource::traits::is_accessible_v<typename seq_T::template emplace_back<T>, set_T> && 
			resource::traits::is_accessible_v<typename exec_T::template syncer<T>, set_T>)
		{ 
			typename seq_T::template emplace_back<T>{ entt }(*this);
			typename exec_T::template syncer<T>{}(*this);
		}

		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred, 
			typename ... arg_Ts>
		void emplace(std::span<handle<T>> entts, arg_Ts&& ... args) requires (
			resource::traits::is_accessible_v<typename seq_T::template emplace_back<T, arg_Ts...>, set_T> && 
			resource::traits::is_accessible_v<typename exec_T::template syncer<T>, set_T>)
		{ 
			typename seq_T::template emplace_back<T>{ entts, args... }(*this);
			typename exec_T::template syncer<T>{}(*this);
		}
		
		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred,
			typename ... arg_Ts> 
		T& emplace_at(size_t index, handle<T> e, arg_Ts&& ... args);

		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred,
			typename ... arg_Ts> 
		T& emplace_at(std::span<handle<T>> entts, arg_Ts&& ... args);

		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred>
		void erase(handle<T> entt);

		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred>
		void erase(std::span<handle<T>> entts);

		template<traits::component_class T,
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred>
		void erase_at(size_t index, size_t count=1);
		
		template<traits::component_class T,
			traits::execution_policy_class exec_T = policy::deferred>
		void clear();

		template<traits::component_class T> 
		void sync() requires (resource::traits::is_accessible_v<policy::immediate::template syncer<T>, set_T>)
		{ 
			typename policy::immediate::template syncer<T>{}(*this);
		}

		template<traits::component_class T> 
		T& get_component(handle<T> entt)
			requires (resource::traits::is_accessible_v<indexer<T>, set_T> && resource::traits::is_accessible_v<storage<T>, set_T>)
		{
			auto& stor = set_T::template get_resource<storage<T>>();
			auto& indx = set_T::template get_resource<indexer<T>>();
			return stor[indx[entt]];
		}
		
		template<traits::component_class T>
		bool has_component(handle<T> entt)
			requires (resource::traits::is_accessible_v<indexer<T>, set_T>)
		{
			return set_T::template get_resource<indexer<T>>().contains(entt);
		}
		
		template<traits::handle_class handle_T = entity, 
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred,
			traits::component_class ... comp_Ts>
		handle_T create(const comp_Ts& ... components)
			requires (resource::traits::is_accessible_v<manager<handle_T>, set_T> && (std::is_same_v<handle_T, handle<comp_Ts>> && ...))
		{
			auto entt = set_T::template get_resource<manager<handle_T>>().create();
			(emplace<comp_Ts, exec_T, seq_T>(entt, components), ...);
			return entt;
		}

		template<traits::handle_class handle_T = entity, 
			traits::sequence_policy_class seq_T = policy::swap,
			traits::execution_policy_class exec_T = policy::deferred>
		void destroy(handle_T entt) 
			requires (resource::traits::is_accessible_v<manager<handle_T>, set_T>)
		{
			// queue component destruction
			return set_T::template get_resource<manager<handle_T>>().destroy(entt);
		}
	};
}

/*
reference emplace<T>(handle<T> e, const T& value);
reference emplace<T>(handle<T> e, std::initializer_list<T> ilist);

reference insert<T>(size_t index, handle<T> e, const T& value);
reference insert<T>(size_t index, handle<T> e, std::initializer_list<T> ilist);

reference insert<T>(size_t index, It first, It last, const T& value);
reference insert<T>(size_t index, It first, It last, std::initializer_list<T> ilist);

void clear<T>();
void erase<T>(entity e);
void erase<T>(It first, It last);
void erase_at<T>(size_t index, size_t n=1);


------
resource_T
on_lock<res_T>
on_unlock<res_T>

lazy_dispatch
on_lock<res_T>
res_A, res_B, res_C




lock
res_A, res_B, res_C

on_lock<res_T>



*/