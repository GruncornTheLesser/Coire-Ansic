
#pragma once
#include "util\tuple.h"
#include "traits.h"
#include "generator.h"
#include "invoker.h"
#include "pool.h"
#include "view.h"

namespace ecs::traits {
	template<typename ... Ts> 
	struct resource_map_builder : resource_map_builder<std::tuple<>, traits::get_dependency_set_t<Ts>...> { };
	
	template<typename ... Ts, typename U, typename ... Us, typename ... Tups> requires (!(std::is_same_v<Ts, U> || ...)) 
	struct resource_map_builder<std::tuple<Ts...>, std::tuple<U, Us...>, Tups...>
	 : resource_map_builder<std::tuple<Ts..., U>, std::tuple<Us...>, Tups...> { };
	
	template<typename ... Ts, typename U, typename ... Us, typename ... Tups> requires ((std::is_same_v<Ts, U> || ...))
	struct resource_map_builder<std::tuple<Ts...>, std::tuple<U, Us...>, Tups...>
	 : resource_map_builder<std::tuple<Ts...>, std::tuple<Us...>, Tups...> { };

	template<typename ... Ts, typename ... Us, typename ... Tups>
	struct resource_map_builder<std::tuple<Ts...>, std::tuple<void, Us...>, Tups...>
	 : resource_map_builder<std::tuple<Ts...>, std::tuple<Us...>, Tups...> { };
	
	template<typename ... Ts, typename ... Tups>
	struct resource_map_builder<std::tuple<Ts...>, std::tuple<>, Tups...>
	 : resource_map_builder<std::tuple<Ts...>, Tups...> { };
	
	template<typename ... Ts> 
	struct resource_map_builder<std::tuple<Ts...>> { 
		using type = util::tuple<Ts...>;
	};
}



namespace ecs {	
	template<traits::contract_class ... Ts>
	class registry
	{
		// take components from storage resources
		// take handles from generator resources
		using map_type = typename traits::resource_map_builder<Ts...>::type;
		using component_set = util::filter_t<std::tuple<Ts...>, traits::is_component>;

	public:
		registry() = default;
		~registry() {
			util::apply<component_set>([&]<typename T>(){ pool<T>().clear(); });
			// util::apply<handle_set>([&]<typename T>(){ generator<T>().clear(); }); // 
		}
		registry(const registry&) = delete;
		registry& operator=(const registry&) = delete;
		registry(registry&&) = default;
		registry& operator=(registry&&) = default;

		template<traits::resource_class T> requires (util::contains_v<map_type, T>)
		constexpr inline T& get() {
			return std::get<T>(resource_map);
		}

		template<traits::resource_class T> requires (util::contains_v<map_type, T>)
		constexpr inline const T& get() const {
			return std::get<std::remove_const_t<T>>(resource_map);
		}
		
		template<traits::event_class T>
			requires (util::contains_v<map_type, get_invoker_t<T>>)
		constexpr inline get_invoker_t<T>& on() { 
			return get<get_invoker_t<T>>();
		}

		template<traits::event_class T>
			requires (util::contains_v<map_type, get_invoker_t<T>>)
		constexpr inline const get_invoker_t<T>& on() const {
			return get<get_invoker_t<T>>();
		}

		template<traits::component_class T>
		constexpr inline ecs::pool<T, registry<Ts...>> pool() {
			return this;
		}

		template<traits::component_class T>
		constexpr inline ecs::pool<const T, const registry<Ts...>> pool() const {
			return this;
		}

		// view
		template<typename ... Us, 
			typename from_T=traits::default_from_t<ecs::select<Us...>>, 
			typename where_T=traits::default_where_t<select<Us...>, from_T>>
		constexpr inline ecs::view<ecs::select<Us...>, from_T, where_T, ecs::registry<Ts...>>  
		view(from_T from={}, where_T where={})
		{
			return this;
		}

		template<typename ... Us, 
			typename from_T=traits::default_from_t<ecs::select<Us...>>, 
			typename where_T=traits::default_where_t<select<Us...>, from_T>>
		constexpr inline ecs::view<ecs::select<const Us...>, from_T, where_T, const ecs::registry<Ts...>>
		view(from_T from={}, where_T where={}) const
		{
			return this;
		}

		template<traits::component_class T, typename ... arg_Ts>
		constexpr inline T& init(get_handle_t<T> ent, arg_Ts&& ... args) {
			return pool<T>().init(ent, std::forward<arg_Ts>()...);
		}

		template<traits::component_class T, typename ... arg_Ts>
		constexpr inline T& get_or_init(get_handle_t<T> ent, arg_Ts&& ... args) {
			return pool<T>().get_or_init(ent, std::forward<arg_Ts>()...);
		}

		template<traits::component_class T>
		constexpr inline void terminate(get_handle_t<T> ent) {
			return pool<T>().terminate(ent);
		}

		template<traits::component_class T>
		constexpr inline bool try_terminate(get_handle_t<T> ent) {
			return pool<T>().try_terminate(ent);
		}

		template<traits::component_class T>
		constexpr inline void clear() {
			pool<std::remove_const_t<T>>().clear();
		}

		template<traits::component_class T>
		constexpr inline size_t count() const {
			return pool<std::remove_const_t<T>>().size();
		}

		template<traits::component_class T>
		constexpr inline bool has(get_handle_t<T> ent) const {
			return pool<std::remove_const_t<T>>().contains(ent);
		}

		template<traits::component_class T>
		constexpr inline T& get(get_handle_t<T> ent) {
			return pool<std::remove_const_t<T>>().get(ent);
		}

		template<traits::component_class T>
		constexpr inline const T& get(get_handle_t<T> ent) const {
			return pool<std::remove_const_t<T>>().get(ent);
		}

		template<traits::handle_class T=ecs::entity>
		constexpr inline T create() {
			T ent = get<get_generator_t<T>>().template create();
			on<ecs::create<T>>().invoke({ent});
			return ent;
		}
		
		template<traits::handle_class T=ecs::entity>
		constexpr inline void destroy(T ent) {
			on<ecs::destroy<T>>().invoke({ent});
			get<get_generator_t<T>>().template destroy(ent);
		}

		template<traits::handle_class T=ecs::entity>
		constexpr inline bool alive(T ent) {
			return get<get_generator_t<T>>().template alive(ent);
		}

		template<traits::contract_class ... Us>
		void acquire(priority p);

		template<traits::contract_class ... Us>
		void release();

	private:
		map_type resource_map;
	};
}
