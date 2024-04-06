#ifndef ECS_POOL_H
#define ECS_POOL_H

#include <shared_mutex>
#include "util/sparse_map.h"
#include "util/next_pow2.h"
#include "entity.h"
#include "traits.h" 

#include <iostream>

namespace ecs {
	struct resource {
		void acquire() { mtx.lock(); }
		void release() { mtx.unlock(); };
		void acquire() const { mtx.lock_shared(); };
		void release() const { mtx.unlock_shared(); };
		mutable std::shared_mutex mtx;
	};
	
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
		}
	private:
		resource_set data;
	};



	namespace traits { 
		DECL_GET_ATTRIB_TYPE(pool, ecs::archetype<T>)

		template<typename LHS_T, typename RHS_T>
		struct same_pool : std::is_same<get_pool_t<std::remove_const_t<LHS_T>>, get_pool_t<std::remove_const_t<RHS_T>>> { };
	}


	template<typename T>
	using pool = traits::get_pool_t<std::remove_const_t<T>>;
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

