#ifndef ECS_POLICY_H
#define ECS_POLICY_H

namespace ecs {
	template<typename T>
	struct deferred {
		using resource_set = std::tuple<typename ecs::pool<T>::entity>;
		using emplace_return = void;
		template<typename pip_T>
		static inline void emplace(pip_T& pip, ecs::entity e)
		{
			auto& entt_arr = pip.template get_resource<typename ecs::pool<T>::entity>();
		}

		template<typename pip_T>
		static inline void erase(pip_T& pip, ecs::entity e)
		{
			auto& entt_arr = pip.template get_resource<typename ecs::pool<T>::entity>();
			
			//entt_arr.erase(entt_arr.begin() + index_arr[e]);
		}
	};

	template<typename T>
	struct immediate {
		using resource_set = typename ecs::pool<T>::resource_set;
		using emplace_return = std::conditional_t<std::is_empty_v<T>, void, T&>;
		template<typename pip_T, typename ... Arg_Ts>
		static inline emplace_return emplace(pip_T& pip, ecs::entity e, Arg_Ts&& ... args)
		{
			auto& entt_arr = pip.template get_resource<traits::entity_storage_t<T>>();
			auto& index_arr = pip.template get_resource<traits::entity_index_t<T>>();
			auto& comp_arr = pip.template get_resource<traits::entity_component_t<T>>();

			size_t n = ++entt_arr.size;
			
			index_arr.reserve(e);
			entt_arr.reserve(n);
			comp_arr.reserve(n);

			index_arr[e] = --n;
			entt_arr[n] = std::pair{e, 0ull};

			return *std::construct_at(&comp_arr[n], std::forward<Arg_Ts>(args)...);
		}

		template<typename pip_T, typename It, typename ... arg_Ts>
		static inline T& emplace(pip_T& pip, It first, It last, arg_Ts&& ... args)
		{
			auto& entt_arr = pip.template get_resource<typename ecs::pool<T>::entity>();
			auto& index_arr = pip.template get_resource<typename ecs::pool<T>::index>();
			auto& comp_arr = pip.template get_resource<typename ecs::pool<T>::template comp<T>>();
		}

		template<typename pip_T>
		static inline void erase(pip_T& pip, ecs::entity e)
		{
			auto& entt_arr = pip.template get_resource<typename ecs::pool<T>::entity>();
			auto& indx_arr = pip.template get_resource<typename ecs::pool<T>::index>();
			auto& comp_arr = pip.template get_resource<typename ecs::pool<T>::template comp<T>>();
		}
	};
}

#endif 