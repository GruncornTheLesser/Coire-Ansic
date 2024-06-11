#pragma once

#include "util/sparse_map.h"
#include "util/paged_vector.h"
#include "entity.h"
#include "traits.h"
#include "util/type_name.h"
#include "resource.h"
#include <shared_mutex>
#include <iostream>



namespace ecs {
	// execution policy informs the pipeline when to synchronize with the other resources of a pool set:
	// deferred - wait until resource release
	// lazy - wait until resource acquired
	// immediate - execute operation immediately
	enum execution_policy { IMMEDIATE = 4, DEFERRED = 2, LAZY = 1 };
	// sequence policy informs the pipeline how to re-order the components for an operation
	enum sequence_policy { STRICT = 2, SWAP = 1 };
}
namespace ecs::traits {
	// default execution policy to be used when no argument given
	DECL_HAS_ATTRIB_VALUE(execution_policy)
	DECL_GET_ATTRIB_VALUE(execution_policy, ecs::execution_policy, DEFERRED)
	// default sequence policy to be used when no argument given
	DECL_HAS_ATTRIB_VALUE(sequence_policy)
	DECL_GET_ATTRIB_VALUE(sequence_policy, ecs::sequence_policy, SWAP)
}

namespace ecs {
	template<typename T> struct handler; // versioned_storage<paged_vector>
	template<typename T> struct indexer : util::sparse_map<ecs::entity>, resource { };
	template<typename T> struct storage : util::paged_vector<T>, resource { };
}

namespace ecs::traits {
	// the set of resources acquired when component used as key
	DECL_HAS_ATTRIB_TYPE(pool_resource_set)
	DECL_GET_ATTRIB_TYPE(pool_resource_set, EXPAND(std::tuple<ecs::handler<T>,ecs::indexer<T>,ecs::storage<T>>))
}





// TODO: pool view
template<typename T, typename Pip_T> // handler, indexer, storage
struct pool_view {

};



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

