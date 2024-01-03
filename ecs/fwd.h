#pragma once
#include <tuple>
#include <type_traits>
#include <concepts>
#include <stdint.h>
#include "util/tuple_util.h"

namespace ecs::type_traits {
    template<typename t> struct is_pipeline;
    template<typename t> struct is_comp;
    template<typename t> struct is_pool;
    template<typename t> struct is_view;

    template<typename t> struct is_archetype;
    template<typename t> struct is_uniontype;
    
    template<typename t> concept pipeline_class = is_pipeline<t>::value;
    template<typename t> concept comp_class = is_comp<t>::value;
    template<typename t> concept pool_class = is_pool<t>::value;
    template<typename t> concept view_class = is_view<t>::value;
}

namespace ecs::policy {
    enum class order { swap, inplace };
    enum class execution { immediate, deferred };    
}

namespace ecs::allocator {
    template<policy::order,     typename elem_t> struct pool_allocator;
    template<policy::execution, typename elem_t> struct pool_sequencer;
}

namespace ecs {
    /** @brief an entity is a unique handle that corresponds to a set of comp */
    class entity;

    /** @brief a registry controls the lifetime of all ecs objects and the synchronization of pipelines */
    class registry;

    /** @brief a handle manager controls entity id and version control, version increments on a synchronization */
    class handle_manager;

    /** @brief a pool stores the components and the entity lookup */
    template<typename t> class pool_t;

    /** @brief a pipeline limits pool access to streamline multithreading */
    template<type_traits::pool_class ... ts> class pipeline_t;

    /** @brief a container for the view class. list of comps that must be included */
    template<type_traits::comp_class ... ts> struct inc { };
    
    /** @brief a container for the view class. list of comps that must be excluded */
    template<type_traits::comp_class ... ts> struct exc { };
    
    /** @brief a view allows for iteration of a set of comps */
    template<type_traits::pipeline_class pip, typename inc_t, typename exc_t> class view;
}

namespace ecs::container {
    template<type_traits::comp_class ...> struct archetype;
    template<type_traits::comp_class ...> struct uniontype;
    template<type_traits::comp_class>     struct basictype;
    template<type_traits::pipeline_class, type_traits::comp_class...>  struct view_element;
    //using emptytype = ecs::entity;
}

namespace ecs::type_traits {
    // Primary template for has_member_type
    template <typename t, typename default_t = ecs::container::basictype<t>, typename = std::void_t<>>
    struct container { using type = default_t; };

    // Specialization for types that do have a nested type 'container'
    template <typename t, typename default_t>
    struct container<t, default_t, std::void_t<typename t::container>> { using type = typename t::container; };
}

namespace ecs::type_traits {
    template<typename t>      struct is_pipeline : std::false_type { };
    template<typename ... ts> struct is_pipeline<pipeline_t<ts...>> : std::true_type { };

    template<typename t> struct is_comp : std::true_type { };
    
    template<typename t> struct is_pool : std::false_type { };
    template<typename t> struct is_pool<pool_t<t>> : std::true_type { };

    template<typename t>      struct is_view : std::false_type { };
    template<typename ... ts> struct is_view<view<ts...>> : std::true_type { };
}

namespace ecs {
    /** @brief a pool type builder */
    template<typename t> using pool = std::conditional_t<std::is_const_v<t>, const pool_t<typename type_traits::container<std::remove_const_t<t>>::type>, pool_t<typename type_traits::container<t>::type>>;

    /** @brief a pipeline builder */
    template<type_traits::comp_class ... ts> using pipeline = util::tuple_extract<typename util::tuple_unique<typename util::tuple_sort<std::tuple<pool<ts>...>>::type>::type, pipeline_t>::type;
}