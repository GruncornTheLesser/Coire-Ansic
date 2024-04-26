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
	
	template<typename ... Ts>
	struct archetype {
		// TODO: maybe. combine component and index resources?????
		// index is the most useless resource, can be used for contains and index lookups. 
		// operation:
		//		immediate emplace/erase -> index, entity, comp
		//		deferred emplace/erase -> entity
		// 		swap -> index
		//		iterate -> entity, comp 
		//		retrieve -> index, comp

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

		using resource_set = std::tuple<comp<Ts>..., index, entity>;
		using synchronization_set = std::tuple<comp<Ts>..., index, entity>; 
		using pool = archetype<Ts...>; // get pool<archetype<U>>() -> archetype<U>&
		
		template<typename U>
		U& get_resource() {
			return std::get<std::remove_const_t<U>>(data);
		}

		template<typename ... Us>
		void sync() {
			((std::cout << "syncing: " << util::type_name<Us>() << std::endl), ...);
			// TODO: finish me
			// NOTE: can deadlock if needs to acquire new resource oopsies
			// deferred -> 
			
			
			// iterate through entity update list
			// swap component at index 1 with component at index of entity at index 1
			//  
		}
	private:
		resource_set data;
	};



	namespace traits { 
		DECL_GET_ATTRIB_TYPE(pool, ecs::archetype<std::remove_const_t<T>>) // gets T::pool, defaults to archetype<T>
	}


	template<typename T>
	using pool = traits::get_pool_t<T>;
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

