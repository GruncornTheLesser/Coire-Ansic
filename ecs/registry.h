#pragma once
#include "fwd.h"
#include "util/any_set.h"
#include "resource.h"
namespace ecs {
    class registry : public resource {
        template<traits::component_class ... ts>
        friend class pipeline;
    public:
        template<traits::component_class u>
        ecs::traits::pool_builder<u>& pool() {
            return pools.get_or_emplace<ecs::pool<std::remove_const_t<u>>>();
        }
        template<traits::component_class u>
        ecs::traits::pool_builder<u>& pool() const {
            return pools.get<ecs::pool<std::remove_const_t<u>>>();
        }

        template<traits::component_class ... us>
        auto pipeline() {
            return typename ecs::traits::pipeline_builder<us...>{ this };
        }

    private:
        util::any_set pools;
    };
}