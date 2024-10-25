
#pragma once
#include "component.h"
#include "util\tuple_util\tuple_util.h"
#include "util\tuple.h"

#include "pool.h"
#include "view.h"

namespace ecs {
	template<typename ... Ts> class registry;
	
	template<typename ... Ts> 
	struct registry_traits<ecs::registry<Ts...>> 
	{
	private:
		using _arguments = std::tuple<Ts...>;									// argumpent input is a components, resources or event
		using _input_resources = util::filter_t<_arguments, traits::is_resource>;		// get resources
		using _input_components = util::filter_t<_arguments, traits::is_component>; 	// get components 
		using _resource_components = util::eval_t<_input_resources, util::eval_each_<traits::get_resource_component_set>::template type, util::concat>;
	public: // extract components
		using component_set = util::unique_t<util::concat_t<std::tuple<_input_components, _resource_components>>>;
	private: // extract component resources
		using _component_resources = util::eval_t<component_set, util::eval_each_<traits::get_component_resource_set>::template type, util::concat>;
	public: // collect handle set
		using handle_set = util::unique_t<util::eval_each_t<component_set, traits::get_component_handle>>; 					// get component handle
	private: // extract events from resources and component resources
		using _component_handle_generators = util::eval_each_t<handle_set, traits::get_handle_generator>;	// get handle generators
		using _resources = util::concat_t<std::tuple<_input_resources, _component_resources, _component_handle_generators>>; // component resources + input resources	
		using _resource_event_set = util::eval_t<_resources, util::eval_each_<traits::get_resource_event_set>::template type, util::concat>;
		using _input_events = util::filter_t<_arguments, traits::is_event>;			// get events
	public:
		using event_set = util::unique_t<util::concat_t<std::tuple<_input_events, _resource_event_set>>>;
	private:// extract event resources
		using _event_invokers = util::eval_each_t<event_set, traits::get_event_invoker>;					// get event invokers
	public:
		using resource_set = util::unique_t<util::concat_t<std::tuple<_resources, _event_invokers>>>;
	};
	
	template<typename ... Ts>
	class registry
	{
		using map_type = util::rewrap_t<typename registry_traits<registry<Ts...>>::resource_set, util::tuple>;
	public:
		registry() = default;
		registry(const registry&) = delete;
		registry& operator=(const registry&) = delete;
		registry(registry&&) = default;
		registry& operator=(registry&&) = default;

		// on<event_T> -> invoker
		
		template<traits::resource_class T> 
			requires (util::pred::contains_v<typename registry_traits<registry<Ts...>>::resource_set, T>)
		constexpr inline auto get() -> T& {
			return std::get<T>(resource_map);
		}

		template<traits::resource_class T>
			requires (util::pred::contains_v<typename registry_traits<registry<Ts...>>::resource_set, T>)
		constexpr inline auto get() const -> const T& {
			return std::get<std::remove_const_t<T>>(resource_map);
		}
		
		template<traits::event_class T> 
		constexpr inline auto on() -> invoker_t<T>& { 
			return get<invoker_t<T>>();
		}

		template<traits::event_class T>
		constexpr inline auto on() const -> const invoker_t<T>& {
			return get<invoker_t<T>>();
		}

		template<traits::component_class T>
		constexpr inline auto pool() -> ecs::pool<T, ecs::registry<Ts...>> {
			return *this;
		}

		template<traits::component_class T>
		constexpr inline auto pool() const -> ecs::pool<const T, const ecs::registry<Ts...>> {
			return *this;
		}

		// view
		template<typename ... Us, 
			typename from_T=traits::default_from_t<ecs::select<Us...>>, 
			typename where_T=traits::default_where_t<select<Us...>, from_T>>
		constexpr inline auto view(from_T from={}, where_T where={})
		 -> ecs::view<ecs::select<Us...>, from_T, where_T, ecs::registry<Ts...>> {
			return *this;
		}

		template<typename ... Us, 
			typename from_T=traits::default_from_t<ecs::select<Us...>>, 
			typename where_T=traits::default_where_t<select<Us...>, from_T>>
		constexpr inline auto view(from_T from={}, where_T where={}) const
		 -> ecs::view<ecs::select<const Us...>, from_T, where_T, const ecs::registry<Ts...>>{
			return *this;
		 }

		template<traits::component_class T, typename ... arg_Ts>
		constexpr inline auto init(handle_t<T> ent, arg_Ts&& ... args) -> T& {
			return pool<T>().init(ent, std::forward<arg_Ts>()...);
		}

		template<traits::component_class T, typename ... arg_Ts>
		constexpr inline auto get_or_init(handle_t<T> ent, arg_Ts&& ... args) -> T& {
			return pool<T>().get_or_init(ent, std::forward<arg_Ts>()...);
		}

		template<traits::component_class T>
		constexpr inline void terminate(handle_t<T> ent) {
			return pool<T>().terminate(ent);
		}

		template<traits::component_class T>
		constexpr inline auto try_terminate(handle_t<T> ent) -> bool {
			return pool<T>().try_terminate(ent);
		}

		template<traits::component_class T>
		constexpr inline void clear() {
			pool<std::remove_const_t<T>>().clear();
		}

		template<traits::component_class T>
		constexpr inline auto count() const -> size_t {
			return pool<std::remove_const_t<T>>().size();
		}

		template<traits::component_class T>
		constexpr inline auto has(handle_t<T> ent) const -> bool {
			return pool<std::remove_const_t<T>>().contains(ent);
		}

		template<traits::component_class T>
		constexpr inline auto get(handle_t<T> ent) -> T& {
			return pool<std::remove_const_t<T>>().get(ent);
		}

		template<traits::component_class T>
		constexpr inline auto get(handle_t<T> ent) const -> const T& {
			return pool<std::remove_const_t<T>>().get(ent);
		}

		template<traits::handle_class T=ecs::entity>
		constexpr inline auto create() -> T {
			return generator_t<T>::template create(this);
		}
		
		template<traits::handle_class T=ecs::entity>
		constexpr inline void destroy(T ent) {
			generator_t<T>::template destroy(this, ent);
		}

		template<traits::handle_class T=ecs::entity>
		constexpr inline auto alive(T ent) -> bool {
			return generator_t<T>::template alive(this, ent);
		}
	private:
		map_type resource_map;
	};
}

//static_assert(std::random_access_iterator<ecs::pool_iterator<int, ecs::registry<int>>>);
//static_assert(std::ranges::random_access_range<ecs::pool<int, ecs::registry<int>>>);

//static_assert(std::bidirectional_iterator<ecs::view_iterator<ecs::select<int>, ecs::from<int>, ecs::include<int>, ecs::registry<int>>>);
//static_assert(std::ranges::bidirectional_range<ecs::view<ecs::select<int>, ecs::from<int>, ecs::include<int>, ecs::registry<int>>>);