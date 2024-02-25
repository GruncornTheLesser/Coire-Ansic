#pragma once
#include <shared_mutex>
#include "util/sparse_map.h"
#include "util/next_pow2.h"
#include "entity.h"
#include "traits.h" 

namespace ecs {
	struct resource {
		void acquire() { mtx.lock(); }
		void release() { mtx.unlock(); };
		void acquire() const { mtx.lock_shared(); };
		void release() const { mtx.unlock_shared(); };
		mutable std::shared_mutex mtx;
	};
	
	template<traits::comp_class ... ts>
	struct archetype {
		template<typename u>
		struct comp : public resource {
			using resource_container = archetype<ts...>;
			util::paged_block<u> data;
		};
		
		struct index : public resource {
			using resource_container = archetype<ts...>;
			util::sparse_map<size_t> data;
		};
		
		struct entity : public resource {
			using resource_container = archetype<ts...>;
			util::unpaged_block<ecs::entity> data;
		};

		using resource_storage_set = std::tuple<comp<ts>..., index, entity>;

		template<typename u>
		u& get_resource() {
			return std::get<std::remove_const_t<u>>(data);
		}

	private:
		resource_storage_set data;
	};

}


namespace ecs::traits {
	DECL_GET_ATTRIB_TYPE(pool_tag, ecs::archetype<T>)
}

namespace ecs { 
	template<traits::comp_class t>
	using pool = traits::get_pool_tag_t<t>;
}



#include "pool.tpp"