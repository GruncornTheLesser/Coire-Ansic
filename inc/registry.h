
#pragma once
#include "component.h"
#include "..\tuple_util\tuple_util.h"
#include <tuple>
#include <utility>
#include <stdint.h>

//#include "view.h"

// TODO: be more specific when calling manager, indexer and storage functions 
// * allows for wider variety of resource setups
// manager->entity_at(size_t i)
// indexer->index_of(entity e)
// storage->component_at(size_t i)

namespace util {
	template<typename T> 
	struct tup_set_elem;
	
	template<typename ... Ts> 
	struct tup_set : tup_set_elem<Ts>... { };

	template<typename T> 
	struct tup_set_elem {
		template<size_t, typename... Ts> constexpr FORCE_INLINE friend auto& std::get(util::tup_set<Ts...>&);
		template<size_t, typename... Ts> constexpr FORCE_INLINE friend const auto& std::get(const util::tup_set<Ts...>&);
		template<typename, typename... Ts> constexpr FORCE_INLINE friend auto& std::get(util::tup_set<Ts...>&);
		template<typename, typename... Ts> constexpr FORCE_INLINE friend const auto& std::get(const util::tup_set<Ts...>&);
		private: [[no_unique_address]] T value;
	};
}

namespace std {
	template<typename ... Ts> class tuple_size<util::tup_set<Ts...>> : std::integral_constant<size_t, sizeof...(Ts)> { };
	
	template<size_t I, typename T, typename ... Ts> class tuple_element<I, util::tup_set<T, Ts...>> : tuple_element<I - 1, util::tup_set<Ts...>> { };
	template<typename T, typename ... Ts> class tuple_element<0, util::tup_set<T, Ts...>> : std::type_identity<T> { };
		
	template<size_t I, typename ... Ts>
	constexpr FORCE_INLINE auto& get(util::tup_set<Ts...>& tup) {
		return static_cast<util::tup_set_elem<tuple_element_t<I, util::tup_set<Ts...>>>&>(tup).value;
	}
	
	template<size_t I, typename ... Ts> 
	constexpr FORCE_INLINE const auto& get(const util::tup_set<Ts...>& tup) {
		return static_cast<util::tup_set_elem<tuple_element_t<I, util::tup_set<Ts...>>>&>(tup).value;
	}

	template<typename T, typename ... Ts>
	constexpr FORCE_INLINE auto& get(util::tup_set<Ts...>& tup) {
		return static_cast<util::tup_set_elem<T>&>(tup).value;
	}
	
	template<typename T, typename ... Ts>
	constexpr FORCE_INLINE const auto& get(const util::tup_set<Ts...>& tup) {
		return static_cast<util::tup_set_elem<T>&>(tup).value;
	}
}


#include "pool.h"

namespace ecs {
	template<typename ... Ts> class registry;
	
	template<typename ... Ts> 
	struct registry_traits<ecs::registry<Ts...>> 
	{
		using _arguments = std::tuple<Ts...>;									// argumpent input is a components, resources or event
		using _input_resources = util::filter_t<_arguments, traits::is_resource>;		// get resources
		using _input_components = util::filter_t<_arguments, traits::is_component>; 	// get components 
		using _resource_components = util::eval_t<_input_resources, util::eval_each_<traits::get_resource_component_set>::template type, util::concat>;
		using component_set = util::unique_t<util::concat_t<std::tuple<_input_components, _resource_components>>>;
		using _component_resources = util::eval_t<component_set, util::eval_each_<traits::get_component_resource_set>::template type, util::concat>;
		using handle_set = util::unique_t<util::eval_each_t<component_set, traits::get_component_handle>>; 					// get component handle
		using _component_handle_generators = util::eval_each_t<handle_set, traits::get_handle_generator>;	// get handle generators
		using _resources = util::concat_t<std::tuple<_input_resources, _component_resources, _component_handle_generators>>; // component resources + input resources
		
		
		using _resource_event_set = util::eval_t<_resources, util::eval_each_<traits::get_resource_event_set>::template type, util::concat>;
		using _input_events = util::filter_t<_arguments, traits::is_event>;			// get events
		using event_set = util::unique_t<util::concat_t<std::tuple<_input_events, _resource_event_set>>>;
		using _event_invokers = util::eval_each_t<event_set, traits::get_event_invoker>;					// get event invokers
		using resource_set = util::unique_t<util::concat_t<std::tuple<_resources, _event_invokers>>>;
	//private:
		// extract components
	//public:
	//private: // extract component resources
	//public:
	
	//private: // extract events from resources and component resources
	//public:
	
	//private:// extract event resources
	//public:
	};
	
	template<typename ... Ts>
	class registry
	{
		using map_type = util::rewrap_t<typename registry_traits<registry<Ts...>>::resource_set, util::tup_set>;
	public:
		registry() = default;
		registry(const registry&) = delete;
		registry& operator=(const registry&) = delete;
		registry(registry&&) = default;
		registry& operator=(registry&&) = default;

		// on<event_T> -> invoker
		
		template<traits::resource_class T>
		constexpr FORCE_INLINE auto get() -> T& {
			return std::get<T>(resource_map);
		}

		template<traits::resource_class T>
		constexpr FORCE_INLINE auto get() const -> const T& {
			return std::get<T>(resource_map);
		}
		
		template<traits::event_class T> 
		constexpr FORCE_INLINE auto on() -> invoker_t<T>& { 
			return get<invoker_t<T>>();
		}

		template<traits::event_class T>
		constexpr FORCE_INLINE auto on() const -> const invoker_t<T>& {
			return get<invoker_t<T>>();
		}

		template<traits::component_class T>
		constexpr FORCE_INLINE auto pool() -> ecs::pool<T, ecs::registry<Ts...>> {
			return this;
		}

		template<traits::component_class T>
		constexpr FORCE_INLINE auto pool() const -> ecs::pool<const T, ecs::registry<Ts...>> {
			return this;
		}

		// view
		template<traits::component_class ... Us, typename from_T, typename where_T>
		constexpr FORCE_INLINE auto view(from_T from={}, where_T where={})
		 -> ecs::view<select<Us...>, from_T, where_T, ecs::registry<Ts...>>;

		template<traits::component_class ... Us, typename from_T, typename where_T>
		constexpr FORCE_INLINE auto view(from_T from={}, where_T where={})
		 -> ecs::view<select<const Us...>, from_T, where_T, ecs::registry<Ts...>>;




		template<traits::component_class T, typename ... arg_Ts>
		constexpr FORCE_INLINE auto init(handle_t<T> e, arg_Ts&& ... args) -> T& {
			return pool<T>().init(e, std::forward<arg_Ts>()...);
		}

		template<traits::component_class T, typename ... arg_Ts>
		constexpr FORCE_INLINE auto try_init(handle_t<T> e, arg_Ts&& ... args) -> T* {
			return pool<T>().try_init(e, std::forward<arg_Ts>()...);
		}

		template<traits::component_class T, typename ... arg_Ts>
		constexpr FORCE_INLINE void terminate(handle_t<T> e) {
			return pool<T>().terminate(e);
		}

		template<traits::component_class T, typename ... arg_Ts>
		constexpr FORCE_INLINE auto try_terminate(handle_t<T> e) -> bool {
			return pool<T>().try_terminate(e);
		}

		template<traits::component_class T>
		constexpr FORCE_INLINE auto count() const -> size_t {
			return pool<T>().count();
		}

		template<traits::component_class T>
		constexpr FORCE_INLINE auto has(handle_t<T> e) const -> bool {
			return pool<T>().contains(e);
		}

		template<traits::component_class T>
		constexpr FORCE_INLINE auto get(handle_t<T> e) -> T& {
			return pool<T>().get(e);
		}

		template<traits::component_class T>
		constexpr FORCE_INLINE auto get(handle_t<T> e) const -> const T& {
			return pool<T>().get(e);
		}

		template<traits::handle_class T=ecs::entity>
		constexpr FORCE_INLINE auto create() -> T {
			return get<generator_t<T>>().create();
		}
		
		template<traits::handle_class T=ecs::entity>
		constexpr FORCE_INLINE void destroy(T e) {
			get<generator_t<T>>().destroy(e);
		}

		template<traits::handle_class T=ecs::entity>
		constexpr FORCE_INLINE auto alive(T e) -> bool {
			return get<generator_t<T>>().alive(e);
		}

	private:
		map_type resource_map;
	};
}

//static_assert(std::random_access_iterator<ecs::pool_iterator<int, ecs::registry<int>>>);
//static_assert(std::ranges::random_access_range<ecs::pool<int, ecs::registry<int>>>);

//static_assert(std::bidirectional_iterator<ecs::view_iterator<ecs::select<int>, ecs::from<int>, ecs::include<int>, ecs::registry<int>>>);
//static_assert(std::ranges::bidirectional_range<ecs::view<ecs::select<int>, ecs::from<int>, ecs::include<int>, ecs::registry<int>>>);