#pragma once
#include <unordered_map>
#include <typeindex>
#include "../util/erased_ptr.h"
#include "../tuple_util/tuple_util.h"

#include "traits.h"
#include "entity.h"
#include "pool.h"
#include "view.h"
#include "macros.h"
// TODO: be more specific when calling manager, indexer and storage functions 
// * allows for wider variety of resource setups
// manager->entity_at(size_t i)
// indexer->index_of(entity e)
// storage->component_at(size_t i)



namespace ecs {
	//template<typename handle_T, typename ... Ts>
	//struct table { };

	namespace traits {
		//template<typename handle_T, typename compset_T>
		//struct get_table : util::eval<compset_T, 
		//	util::filter_<util::cmp::to_<handle_T, std::is_same, traits::get_handle>::template type>::template type, 
		//	util::prepend_<handle_T>::template type, util::rewrap_<table>::template type> { }; 
		//template<typename handle_T, typename compset_T>
		//using get_table_t = typename get_table<handle_T, compset_T>::type;

		//template<typename compset_T>
		//struct get_handle_set : util::eval<compset_T, 
		//	util::eval_each_<traits::get_handle>::template type, 
		//	util::unique_<std::is_same>::template type> { };
		//template<typename compset_T>
		//using get_handle_set_t = typename get_handle_set<compset_T>::type;
		
		//template<typename handleset_T, typename compset_T>
		//struct get_table_set : util::eval_each<handleset_T, util::add_arg_<get_table, compset_T>::template type> { };
		//template<typename handleset_T, typename compset_T>
		//using get_table_set_t = typename get_table_set<handleset_T, compset_T>::type;

		template<typename compset_T>
		struct get_dispatcher : util::eval<compset_T, util::eval_each_<
			util::eval_split_<get_events, util::eval_<get_handle, get_events>::template type>::template type,
			util::concat>::template type, util::concat, util::unique_<>::template type, util::rewrap_<dispatcher>::template type>
		{ };
		template<typename compset_T>
		using get_dispatcher_t = typename get_dispatcher<compset_T>::type;

	}
	
	template<typename ... Ts>
	class registry : public traits::get_dispatcher_t<std::tuple<Ts...>>
	{
		using component_set = std::tuple<Ts...>;
		using dispatcher = traits::get_dispatcher_t<component_set>;
		//using handle_set = traits::get_handle_set_t<component_set>;
		//using table_set = traits::get_table_set_t<handle_set, component_set>;

		using resource_set = util::unique_t<std::tuple<
			traits::get_manager_t<handle<Ts>>..., // get all entity managers
			traits::get_manager_t<Ts>...,
			traits::get_indexer_t<Ts>..., 
			traits::get_storage_t<Ts>...
			>>;
	public:
		registry() = default;
		registry(const registry&) = delete;
		registry& operator=(const registry&) = delete;
		registry(registry&&) = default;
		registry& operator=(registry&&) = default;


		~registry() {
			(pool<Ts>().clear(), ...);
			//(.clear())
			
		}


		template<traits::component_class T, typename ... arg_Ts>
		T& init(handle<T> e, arg_Ts&& ... args) {
			return pool<T>().init(e, std::forward<arg_Ts>(args)...);
		}

		template<traits::component_class T>
		void terminate(handle<T> e) {
			return pool<T>().terminate(e);
		}

		template<traits::component_class T, typename ... arg_Ts>
		T* try_init(handle<T> e, arg_Ts&& ... args) {
			return pool<T>().try_init(e, std::forward<arg_Ts>(args)...);
		}

		template<traits::component_class T>
		bool try_terminate(handle<T> e) {
			return pool<T>().try_terminate(e);
		}

		template<traits::component_class T>
		T& get(handle<T> e) {
			return pool<T>()[e];
		}

		template<traits::component_class T>
		[[nodiscard]] const T& get(handle<T> e) const {
			return pool<T>()[e];
		}

		template<traits::component_class T>
		[[nodiscard]] bool has(handle<T> e) const {
			return pool<T>().contains(e);
		}

		template<traits::component_class T>
		[[nodiscard]] FORCE_INLINE size_t count() const {
			return pool<T>().size();
		}

		template<traits::handle_class handle_T = DEFAULT_HANDLE>
		[[nodiscard]] handle_T create() {
			handle_T e = get_manager<handle_T>().create();
			dispatcher::template on<event::create<handle_T>>().invoke({ e });
			return e;
		}

		template<traits::handle_class handle_T = DEFAULT_HANDLE>
		[[nodiscard]] handle_T create(size_t n) {
			handle_T e = get_manager<handle_T>().create();
			dispatcher::template on<event::create<handle_T>>().invoke({ e });
			return e;
		}

		template<traits::handle_class handle_T = DEFAULT_HANDLE>
		void destroy(handle_T e) {
			(try_terminate<Ts>(e), ...); // filter for types with matching handle try terminate
			dispatcher::template on<event::destroy<handle_T>>().invoke({ e });
			get_manager<handle_T>().destroy(e);
		}

		template<traits::handle_class handle_T = DEFAULT_HANDLE>
		void destroy_all(); 
		/*{
			dispatcher::template on<event::destroy<handle_T>>().invoke({ e });
			get_manager<handle_T>().destroy(e);
		}*/

		template<traits::handle_class handle_T = DEFAULT_HANDLE>
		bool alive(handle_T e) const {
			return get_manager<handle_T>().alive(e);
		}

		template<typename ... select_Ts, 
			typename from_T=ecs::default_from<ecs::select<select_Ts...>>, 
			typename where_T=ecs::default_where<ecs::select<select_Ts...>, ecs::from<from_T>>>
		FORCE_INLINE ecs::view<ecs::select<select_Ts...>, from_T, where_T, registry<Ts...>>
		view(from_T from=from_T{}, where_T where=where_T{}) { return this; }

		template<typename ... select_Ts, 
			typename from_T=ecs::default_from<ecs::select<select_Ts...>>, 
			typename where_T=ecs::default_where<ecs::select<select_Ts...>, ecs::from<from_T>>>
		FORCE_INLINE const ecs::view<ecs::select<select_Ts...>, from_T, where_T, registry<Ts...>>
		view(from_T from=from_T{}, where_T where=where_T{}) const { return this; }

		template<typename T> FORCE_INLINE
		std::conditional_t<std::is_const_v<T>, const ecs::pool<T, registry<Ts...>>, ecs::pool<T, registry<Ts...>>> 
		pool() { return { this }; }

		template<typename T> FORCE_INLINE 
		const ecs::pool<std::remove_const_t<T>, registry<Ts...>> 
		pool() const { return const_cast<registry<Ts...>*>(this); }

		template<typename T> FORCE_INLINE manager<T>& get_manager() { return std::get<manager<std::remove_const_t<T>>>(set); }
		template<typename T> FORCE_INLINE indexer<T>& get_indexer() { return std::get<indexer<std::remove_const_t<T>>>(set); }
		template<typename T> FORCE_INLINE storage<T>& get_storage() { return std::get<storage<std::remove_const_t<T>>>(set); }
		template<typename T> FORCE_INLINE manager<const T>& get_manager() const { return std::get<manager<std::remove_const_t<T>>>(set); }
		template<typename T> FORCE_INLINE indexer<const T>& get_indexer() const { return std::get<indexer<std::remove_const_t<T>>>(set); }
		template<typename T> FORCE_INLINE storage<const T>& get_storage() const { return std::get<storage<std::remove_const_t<T>>>(set); }
	private:
		resource_set set;
	};
}





static_assert(std::random_access_iterator<ecs::pool_iterator<int, ecs::registry<int>>>);
static_assert(std::ranges::random_access_range<ecs::pool<int, ecs::registry<int>>>);

static_assert(std::bidirectional_iterator<ecs::view_iterator<ecs::select<int>, ecs::from<int>, ecs::include<int>, ecs::registry<int>>>);
static_assert(std::ranges::bidirectional_range<ecs::view<ecs::select<int>, ecs::from<int>, ecs::include<int>, ecs::registry<int>>>);