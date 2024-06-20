#pragma once
#include "util/tuple_util.h"
#include "traits.h"
#include "entity.h"
#include "resource.h"
#include <tuple>

namespace ecs {
	// execution policy informs the pipeline when to synchronize the acquired resources:
	// deferred - wait until resource release(DEFAULT)
	// lazy - wait until resource reacquired
	// immediate - execute operation immediately
	enum execution_policy { IMMEDIATE = 4, DEFERRED = 2, LAZY = 1 };
	// sequence policy informs the pipeline how to re-order the components for an operation
	// strict - maintains original order
	// swap - swaps elements for least amount of move operations(DEFAULT)
	enum sequence_policy { STRICT = 2, SWAP = 1 };

	template<typename T> struct handler_t;
	
	template<typename T> struct indexer_t;
	
	template<typename T> struct storage_t;
}

namespace ecs::traits {	
	template<typename T> 
	struct is_component;
	
	template<typename T> 
	concept component_class = is_component<T>::value;
	
	template<typename T> 
	struct is_component
	 : std::negation<std::disjunction<is_resource<T>, has_resource_set<T>, is_entity<T>>> { };
	
	template<typename T>
	 static constexpr bool is_component_v = is_component<T>::value;

	// default execution policy to be used when no argument given
	DECL_HAS_ATTRIB_VALUE(execution_policy)
	DECL_GET_ATTRIB_VALUE(execution_policy, ecs::execution_policy, DEFERRED)
	// default sequence policy to be used when no argument given
	DECL_HAS_ATTRIB_VALUE(sequence_policy)
	DECL_GET_ATTRIB_VALUE(sequence_policy, ecs::sequence_policy, SWAP)

	DECL_HAS_ATTRIB_TYPE(component_type)
	DECL_GET_ATTRIB_TYPE(component_type, T)

	DECL_HAS_ATTRIB_TYPE(component_resource_set)
	DECL_GET_ATTRIB_TYPE(component_resource_set, EXPAND(std::tuple<
		handler_t<get_component_type_t<T>>,
		indexer_t<get_component_type_t<T>>,
		storage_t<get_component_type_t<T>>>))
}

namespace ecs {
	template<typename T>
	using handler = util::propergate_const_t<T, util::eval_<ecs::traits::get_component_type, util::wrap_<handler_t>::template type>::template type>;
	template<typename T>
	using indexer = util::propergate_const_t<T, util::eval_<ecs::traits::get_component_type, util::wrap_<indexer_t>::template type>::template type>;
	template<typename T> 
	using storage = util::propergate_const_t<T, util::eval_<ecs::traits::get_component_type, util::wrap_<storage_t>::template type>::template type>;
}