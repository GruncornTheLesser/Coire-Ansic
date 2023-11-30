#pragma once
#include "fwd.h"
#include "any_set.h"
#include "resource.h"
namespace ecs {
    class registry : public resource {
        template<traits::component_class ... ts>
        friend class pipeline;
    public:
        template<traits::component_class u>
        ecs::pool<u>& pool() {
            return pools.get_or_emplace<ecs::pool<u>>();
        }
        template<traits::component_class u>
        const ecs::pool<u>& pool() const {
            return pools.get<ecs::pool<u>>();
        }

        template<traits::component_class ... us>
        auto pipeline() {
            return typename ecs::traits::pipeline_builder<us...>::type{ this };
        }

        template<traits::view_class ... us>
        ecs::view<us...>& view() {
            return ecs::view<us...>(this);
        }

    private:
        util::any_set pools;
    };
}