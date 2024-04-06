#ifndef ECS_POLICY_H
#define ECS_POLICY_H

namespace ecs {
	template<typename T>
	struct deferred {
		using resource_set = std::tuple<typename ecs::pool<T>::entity>;
		template<typename pip_T>
		static inline T& emplace(pip_T& pip, ecs::entity e)
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

		template<typename pip_T, typename ... arg_Ts>
		static inline T& emplace(pip_T& pip, ecs::entity e, arg_Ts&& ... args)
		{
			auto& entt_arr = pip.template get_resource<typename ecs::pool<T>::entity>();
			auto& index_arr = pip.template get_resource<typename ecs::pool<T>::index>();
			auto& comp_arr = pip.template get_resource<typename ecs::pool<T>::template comp<T>>();

			reserve

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