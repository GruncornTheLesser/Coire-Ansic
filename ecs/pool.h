#ifndef ECS_POOL_H
#define ECS_POOL_H

#include <shared_mutex>
#include "util/sparse_map.h"
#include "util/next_pow2.h"
#include "entity.h"
#include "traits.h" 
#include "util/type_name.h"
#include <iostream>

namespace ecs {
	struct resource {
		void acquire() { mtx.lock(); }
		void release() { mtx.unlock(); };
		void acquire() const { mtx.lock_shared(); };
		void release() const { mtx.unlock_shared(); };
	private:
		mutable std::shared_mutex mtx;
	};
	
	// TODO: remove nested resources in archetype and instead infer container from component???
	// TODO: maybe. combine component and index resources?????
	// index is the most useless resource, can be used for contains and index lookups. 
	// operation:
	//		immediate emplace/erase -> index, entity, comp
	//		deferred emplace/erase -> entity
	// 		swap -> index
	//		iterate -> entity, comp 
	//		retrieve -> index, comp
	// TODO: traits class -> with default values etc

	template<typename ... Ts>
	struct archetype {
		
		// resource attributes
		template<typename U>
		struct comp : public resource, util::paged_block<U> {
			using resource_container = archetype<Ts...>;
		};

		struct index : public resource, util::sparse_map<size_t> {
			using resource_container = archetype<Ts...>;
		};

		struct entity : public resource, util::unpaged_block<std::pair<ecs::entity, uint32_t>> {
			using resource_container = archetype<Ts...>;
		
			size_t& size() { return n; }
			size_t size() const { return n; }
		private:
			size_t n = 0;
		};

		// the set of resources associated with this contianer
		using resource_set = std::tuple<comp<Ts>..., index, entity>;

		// the set of resources that are requested when sync is called on this resource container object
		using synchronization_set = std::tuple<comp<Ts>..., index, entity>;

		// is a pool of itself in case get_pool<pool<Ts...>> is called;
		using pool = archetype<Ts...>; 
		
		template<typename U>
		U& get_resource() {
			return std::get<std::remove_const_t<U>>(data);
		}

		template<typename ... Us>
		void sync() {
			std::cout << "syncing container: " << util::type_name<archetype<Ts...>>() << " = {";
			((std::cout << std::string(util::type_name<Us>()).substr(util::type_name<archetype<Ts...>>().size()) << ", "  ), ...);
			std::cout << "\b\b}" << std::endl;
			// TODO: finish me
			// NOTE: can deadlock if needs to acquire new resource oopsies
			// deferred -> 
			
			
			// iterate through entity update list
			// swap component at index 1 with component at index of entity at index 1
		}
	private:
		resource_set data;
	};

	namespace traits { 
		DECL_GET_ATTRIB_TYPE(pool, ecs::archetype<std::remove_const_t<T>>) // gets T::pool, defaults to archetype<T>

		template<typename T> struct get_pool_index_storage { using type = typename get_pool_t<T>::index; };
		template<typename T> using get_pool_index_storage_t = get_pool_index_storage<T>;
		template<typename T> struct get_pool_entity_storage { using type = typename get_pool_t<T>::entity; };
		template<typename T> using get_pool_entity_storage_t = get_pool_entity_storage<T>;
		template<typename T> struct get_pool_component_storage { using type = typename get_pool_t<T>::template comp<T>; };
		template<typename T> using get_pool_component_storage_t = get_pool_component_storage<T>;
	}
	template<typename T>
	using pool = util::trn::propergate_const_t<T, traits::get_pool>;
}
#endif


			// entity pair
			// entity | next = 0000FFFF | FFFF0000
			// narrowing to 4 bytes converts to entity 
			// uses upper 4 bytes to store index to next update
			// defaults to FFFF to mean negative 1 so no next

			// no

			// just us std::pair<entity, uint32_t>

			// index -1 -> back

			// update list -> how:
			//	when entity changes must point to push to back of update list
			// 	iterate through update list perform operations
			//

			// [entities..., update list]???
			// create, destroy, move
			// create, create, list

			//	for i in entity index update list: 
			//		curr = i
			// 		prev = index[entity[curr]]; 
			//		while curr != prev:
			//			if prev == -1: // created new entity
			//				index[entity[curr]] = curr
			//				break
			//			swap(components[curr], components[prev]);
			//			swap(index[entity[i]], curr);

