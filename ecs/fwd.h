#pragma once
#include <tuple>
#include "tuple_util.h"

namespace ecs {
    using entity = unsigned long int;
	entity tombstone = -1;
	using version = unsigned long int;
}

ecs::entity operator""_entity(unsigned long long int value) {
    return static_cast<ecs::entity>(value);
}

namespace ecs::traits {
	template<typename t> struct is_component;
	template<typename t> struct is_pool;
	template<typename t> struct is_pipeline;
	template<typename t> struct is_view;
	template<typename t> struct is_group;
	
	template<typename t, typename comp> struct is_container;
	template<typename t, typename comp> struct is_component_interface;
	template<typename t, typename comp> struct is_pool_policy;

	template<typename t> concept component_class = traits::is_component<t>::value;
	template<typename t> concept pipeline_class = traits::is_pipeline<t>::value;
	template<typename t> concept pool_class = traits::is_pool<t>::value;
	template<typename t> concept view_class = traits::is_view<t>::value;
	template<typename t> concept group_class = traits::is_group<t>::value;
	
	template<typename t, typename comp> concept component_interface = is_component_interface<t, comp>::value;
	template<typename t, typename comp> concept policy_class = is_pool_policy<t, comp>::value;

    template<typename t> struct component_id;

    template<typename t, typename u> struct component_sort;
	template<typename ... ts> struct pipeline_builder;

}
    
namespace ecs::pool_policy {
	struct back;

	struct immediate_swap;
	struct immediate_inplace;

	struct deferred_swap;
	struct deferred_inplace;

	using immediate = immediate_swap;
	using defer = deferred_swap;
	using inplace = immediate_inplace;
	using swap = immediate_swap;
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
     * @brief a pool is component container which stores a component array array, entity index mapping and version control
     * @tparam t the component stored in the pool 
     */
    template<traits::component_class t> class pool;

    /**
     * @brief a pipeline controls the pool access mutex locks to allow multithreading
     * @tparam ...ts the components accessible by this pipeline
     */
    template<traits::component_class ... ts> class pipeline;

    /**
     * @brief a container for the group class. list of components to be retrieved but not reordered by this group
     * @tparam ...ts components retrieved but not owned by a group 
     */
    template<traits::component_class ... ts> using get = std::tuple<ts...>;
    /**
     * @brief a container for the group class. list of components to be reordered for faster access by this group
     * @tparam ...ts components owned by a group
     */
    template<traits::component_class ... ts> using own = std::tuple<ts...>;
    /**
     * @brief a group stores an entity index mapping, and allows for faster iteration of a set of components
     * @tparam own components owned by this group
     * @tparam get components retrieved by this group
     */
    template<typename own, typename get> class group;

    /**
     * @brief a container for the view class. list of components that must be included
     * @tparam ...ts components included in this view
     */
    template<traits::component_class ... ts> using inc = std::tuple<ts...>;
    /**
     * @brief a container for the view class. list of components that must be excluded
     * @tparam ...ts components excluded in this view 
     */
    template<traits::component_class ... ts> using exc = std::tuple<ts...>;
    /**
     * @brief a view allows for iteration of a set of components. 
     * @tparam inc components included in this view
     * @tparam exc components excluded in this view
     */
    template<typename inc, typename exc> class view;
}

namespace ecs::traits {
    template<typename t> struct is_component : std::bool_constant<!std::is_void_v<component_id<t>>> { }; 
    template<> struct is_component<entity> : std::false_type { };
    
    template<typename t> struct is_component<const t> : is_component<t> { };
    
    template<typename t> struct is_pool : std::false_type { };
    template<typename t> struct is_pool<pool<t>> : std::true_type { };

    template<typename t>             struct is_pipeline : std::false_type { };
    template<pool_class ... pool_ts> struct is_pipeline<pipeline<pool_ts...>> : std::true_type { };

    template<typename t, typename comp_t> struct is_pool_policy {
        static constexpr bool value = 
            std::is_same_v<decltype(t::allocate_at(std::declval<ecs::pool<comp_t>&>(), 0, 0)), void> &&
            std::is_same_v<decltype(t::deallocate_at(std::declval<ecs::pool<comp_t>&>(), 0, 0)), void>;
    };

    template<typename t, typename comp_t> struct is_component_interface : std::false_type { };    
    template<typename t, typename comp_t> struct is_component_interface<t&, comp_t> : is_component_interface<t, comp_t> { };

    template<typename comp_t> struct is_component_interface<const comp_t, const comp_t> : std::true_type { };
    template<typename comp_t> struct is_component_interface<      comp_t, const comp_t> : std::true_type { };
    template<typename comp_t> struct is_component_interface<      comp_t,       comp_t> : std::true_type { };
    
    template<typename ... ts, typename comp_t> struct is_component_interface<pool<ts...>,     comp_t> : std::bool_constant<(is_component_interface<ts, comp_t>::value || ...)> { };
    template<typename ... ts, typename comp_t> struct is_component_interface<pipeline<ts...>, comp_t> : std::bool_constant<(is_component_interface<ts, comp_t>::value || ...)> { };

    template<typename t> struct component_id  { static constexpr int value = t::component_id; };

    template<typename t, typename u> struct component_sort { static constexpr bool value = component_id<t>::value < component_id<t>::value; };
	template<typename ... ts> struct pipeline_builder : utility::tuple_extract<typename utility::tuple_sort<std::tuple<ts...>, component_sort>::type, pipeline> { };
}