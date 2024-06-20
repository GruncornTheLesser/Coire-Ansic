#pragma once 
#include <shared_mutex>
#include "util/paged_vector.h"
#include "util/sparse_map.h"
#include "entity.h"
#include "versioner.h"
#include "resource.h"
#include "component.h"

namespace ecs {
	
	template<typename T>
	struct handler_t : util::paged_vector<version_pair<ecs::entity>>, resource { }; // requires more or 

	template<typename T> 
	struct indexer_t : util::sparse_map<size_t, size_t, static_cast<size_t>(-1)>, resource { };

	template<typename T> 
	struct storage_t : util::paged_vector<T>, resource { };
}