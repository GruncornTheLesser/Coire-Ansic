#pragma once
#include "util/tuple_util.h"
#include "pool.h"
#include "traits.h"

namespace ecs {
	template<ecs::traits::resource_class ... ts>
	struct pipeline_t {
		using resource_set_t = util::tuple_unique_t<util::is_remove_const_same, util::tuple_transform_t<util::to_pointer, util::tuple_transform_t<traits::get_resource_container, std::tuple<ts...>>>>;
		
		template<typename u> 
		static constexpr bool is_accessible = util::tuple_contains_v<u, std::tuple<const ts...>> || util::tuple_contains_v<u, std::tuple<ts...>>;
		
		template<typename base_t>
		pipeline_t(base_t& base);

		void lock();
		void unlock();

		template<ecs::traits::resource_class u>
		u& get_resource() requires (is_accessible<u>);

	private:
		resource_set_t resource_set;
	};
 
	// A horrors beyond human comprehension
	// stare into the abyss, dive head and fall for all eternity. The light fades and sight is swallowed. 
	// Drown in an ocean of black, a neverending nothing, The vastness of unknown washes over you, through you. 
	// You are all that remains, the last bastion of something-anything: you, your self, your sanity. Until even 
	// that diminishes piece by piece, stripped back, all that is you, all that encompasses your thoughts, 
	// your feelings, emotions slip between your fingers the broken shards of a shattered mind. leaving only the 
	// twisting corruption of a maddened mind, twisting under the gaze of that unbearable emptyness. 
	// Then, and only then could you begin to understand this code.
	// honestly its not that bad. just convenluted

	
	template<traits::acquireable ... ts>
	using pipeline = 
		util::tuple_unique_t<util::is_remove_const_same, 
			util::tuple_sort_t<util::remove_const_alpha_cmp, 
			util::tuple_sort_t<util::non_const_first_cmp,
			util::tuple_expand_t<pipeline_t<>,									// casts to pipeline, fills with resources
			util::tuple_filter_t<												// filter all without storage set
			util::negate_pred<ecs::traits::has_resource_storage_set>::type,
				std::tuple<ts...>>,
			typename util::tuple_expand_t<util::tuple_expand<>,					// casts to tuple_expand, and fills with resource sets
				util::tuple_transform_t<traits::get_resource_storage_set,		// get all storage resources
				util::tuple_filter_t<ecs::traits::has_resource_storage_set, 	// filter all with storage set
					std::tuple<ts...>>>>::type
			>>>>;
}

#include "pipeline.tpp"