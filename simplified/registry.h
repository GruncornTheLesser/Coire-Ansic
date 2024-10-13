#pragma once
#include <unordered_map>
#include <typeindex>
#include "../util/erased_ptr.h"
#include "../tuple_util/tuple_util.h"

#include "traits.h"
#include "handle.h"
#include "pool.h"

// TODO: be more specific when calling manager, indexer and storage functions 
// * allows for wider variety of resource setups
// manager->entity_at(size_t i)
// indexer->index_of(entity e)
// storage->component_at(size_t i)

namespace ecs {
	class dynamic_registry
	{
		std::unordered_map<std::type_index, util::erased_unique_ptr> data;
	};

	template<typename ... Ts>
	class static_registry : public traits::event_set_dispatcher<Ts...>
	{
		using dispatcher = traits::event_set_dispatcher<Ts...>;
		using resource_set = util::unique_t<std::tuple<
			traits::get_manager_t<entity<Ts>>..., // get all entity managers
			traits::get_manager_t<Ts>...,
			traits::get_indexer_t<Ts>..., 
			traits::get_storage_t<Ts>...
			>>;
	public:
		template<traits::component_class T, typename ... arg_Ts>
		T& init(entity<T> e, arg_Ts&& ... args) {
			return pool<T>().init(e, std::forward<arg_Ts>(args)...);
		}

		template<traits::component_class T>
		void terminate(entity<T> e) {
			return pool<T>().terminate(e);
		}

		template<traits::component_class T, typename ... arg_Ts>
		T* try_init(entity<T> e, arg_Ts&& ... args) {
			return pool<T>().try_init(e, std::forward<arg_Ts>(args)...);
		}

		template<traits::component_class T>
		bool try_terminate(entity<T> e) {
			
			return pool<T>().try_terminate(e);
		}

		template<traits::component_class T>
		T& get_component(entity<T> e) {
			return pool<T>()[e];
		}

		template<traits::component_class T>
		[[nodiscard]] const T& get_component(entity<T> e) const {
			return pool<T>()[e];
		}

		template<traits::component_class T>
		[[nodiscard]] bool has(entity<T> e) const {
			return pool<T>().contains(e);
		}

		template<traits::entity_class entity_T = DEFAULT_HANDLE>
		[[nodiscard]] entity_T create() {
			entity_T e = get_manager<entity_T>().create();
			dispatcher::template on<event::create<entity_T>>().invoke({ e });
			return e;
		}

		template<traits::entity_class entity_T = DEFAULT_HANDLE>
		void destroy(entity_T e) {
			(try_terminate<Ts>(e), ...);
			dispatcher::template on<event::destroy<entity_T>>().invoke({ e });
			get_manager<entity_T>().destroy(e);
		}

		template<traits::entity_class entity_T = DEFAULT_HANDLE>
		bool alive(entity_T e) const {
			return get_manager<entity_T>().alive(e);
		}

		// template<typename ... select_Ts, typename from_T, typename ... where_Ts>
		// view<...> view(from_T from, where_Ts... where) { return *this; }

		template<typename T>
		std::conditional_t<std::is_const_v<T>, const ecs::pool<std::remove_const_t<T>, static_registry<Ts...>>, ecs::pool<T, static_registry<Ts...>>>
		pool() { return this; }

		template<typename T>
		const ecs::pool<std::remove_const_t<T>, static_registry<Ts...>> 
		pool() const { return const_cast<static_registry<Ts...>*>(this); }

		template<typename T> inline manager<T>& get_manager() { return std::get<manager<std::remove_const_t<T>>>(set); }
		template<typename T> inline indexer<T>& get_indexer() { return std::get<indexer<std::remove_const_t<T>>>(set); }
		template<typename T> inline storage<T>& get_storage() { return std::get<storage<std::remove_const_t<T>>>(set); }
		template<typename T> inline manager<const T>& get_manager() const { return std::get<manager<std::remove_const_t<T>>>(set); }
		template<typename T> inline indexer<const T>& get_indexer() const { return std::get<indexer<std::remove_const_t<T>>>(set); }
		template<typename T> inline storage<const T>& get_storage() const { return std::get<storage<std::remove_const_t<T>>>(set); }
	private:
		resource_set set;
	};

	template<typename ... Ts>
	using registry = std::conditional_t<std::tuple_size_v<std::tuple<Ts...>> == 0, dynamic_registry, static_registry<Ts...>>;
}

// example entity
// struct my_entity : entity<> { };