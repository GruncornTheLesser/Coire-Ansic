#pragma once
#include <tuple>
#include "util/tuple_util.h"
#include <type_traits>
#include <concepts>
#include <stdint.h>

namespace ecs {
    using entity = unsigned long int;
	entity tombstone = -1;
	using version = unsigned long int;
}

namespace ecs::traits {
	template<typename t> struct is_comp;
	template<typename t> struct is_pool;
	template<typename t> struct is_pipeline;
	template<typename t> struct is_view;
	template<typename t> struct is_group;
	
	template<typename t, typename comp> struct is_container;
	template<typename t, typename comp> struct is_component_interface;

	template<typename t> concept comp_class = traits::is_comp<t>::value;

	template<typename t> concept pipeline_class = traits::is_pipeline<t>::value;
	template<typename t> concept pool_class = traits::is_pool<t>::value;
	template<typename t> concept view_class = traits::is_view<t>::value;
	template<typename t> concept group_class = traits::is_group<t>::value;

    template<typename t> struct comp_id;

    template<typename t, typename u> struct comp_cmp_lt;
    template<typename t, typename u> struct comp_cmp_eq;
}

namespace ecs::pool_policy {
    struct swap_policy;
    struct inplace_policy;
    struct where;
    struct back_policy;
    struct front_policy;
    struct swap_policy;
    struct inplace_policy;
    struct immediate_policy;
    //struct deferred_policy;
}
    
namespace ecs {
    struct back { };
    struct front { };
    struct at { size_t value; };   // wrapper for index
    struct elem { entity value; }; // wrapper for entity
}

namespace ecs {
    /**
     * @brief a registry controls the lifetime of all ecs objects and the synchronization of pipelines.
     */
    class registry;

    /**
     * @brief controls entity id and version control, version increments on a synchronization
     */
    class handle_manager;

    /**
     * @brief a pool is comp container which stores a comp array array, entity index mapping and version control
     * @tparam t the comp stored in the pool 
     */
    template<traits::comp_class t> class pool;

    /**
     * @brief a pipeline controls the pool access mutex locks to allow multithreading
     * @tparam ...ts the comps accessible by this pipeline
     */
    template<traits::comp_class ... ts> class pipeline;

    /**
     * @brief a container for the view class. list of comps that must be included
     * @tparam ...ts comps included in this view
     */
    template<traits::comp_class ... ts> struct inc { };
    /**
     * @brief a container for the view class. list of comps that must be excluded
     * @tparam ...ts comps excluded in this view 
     */
    template<traits::comp_class ... ts> struct exc { };
    /**
     * @brief a view allows for iteration of a set of comps. 
     * @tparam inc comps included in this view
     * @tparam exc comps excluded in this view
     */
    template<traits::pipeline_class pip, typename inc_t, typename exc_t> class view;

    /**
     * @brief
    */
    template<typename base> class iterator;
}

namespace ecs::traits {
    template<typename t> struct is_comp : std::bool_constant<!std::is_void_v<comp_id<t>>> { };
    
    template<typename t> struct is_comp<const t> : is_comp<t> { };
    
    template<typename t> struct is_pool : std::false_type { };
    template<typename t> struct is_pool<pool<t>> : std::true_type { };

    template<typename t>      struct is_pipeline : std::false_type { };
    template<typename ... ts> struct is_pipeline<pipeline<ts...>> : std::true_type { };

    template<typename t, typename comp_t> struct is_pool_policy {
        static constexpr bool value = 
            std::is_same_v<decltype(t::allocate_at(std::declval<ecs::pool<comp_t>&>(), 0, 0)), void> &&
            std::is_same_v<decltype(t::deallocate_at(std::declval<ecs::pool<comp_t>&>(), 0, 0)), void>;
    };
    
    template<typename t> struct comp_id  { static constexpr int value = t::comp_id; };

    template<typename t, typename u> struct comp_cmp_lt : std::bool_constant<(comp_id<t>::value < comp_id<u>::value)> { };
    template<typename t, typename u> struct comp_cmp_eq : std::bool_constant<(comp_id<t>::value == comp_id<u>::value)> { };

    template<typename t>      using pool_builder = std::conditional_t<std::is_const_v<t>, const pool<std::remove_const_t<t>>, pool<t>>;
    template<typename ... ts> using pipeline_builder = util::tuple_extract<typename util::tuple_sort<std::tuple<ts...>, comp_cmp_lt>::type, pipeline>::type;
}