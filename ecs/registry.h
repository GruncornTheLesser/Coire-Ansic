#pragma once
#include "fwd.h"
#include "any_set.h"
namespace ecs {
    class registry {
        template<traits::component_class t>
        ecs::pool<t>& pool() {
            return objects.get_or_emplace<ecs::pool<t>>(this);
        }

        template<traits::component_class t>
        ecs::pool<t>& pool() const {
            return objects.get<ecs::pool<t>>(this);
        }

        template<traits::component_class ... ts>
        ecs::pipeline<ts...>& pipeline() {
            return objects.get_or_emplace<ecs::pipeline<ts...>>(this);
        }

        template<traits::component_class ... ts>
        ecs::pipeline<ts...>& pipeline() const {
            return ecs::pipeline<ts...>(objects.get<traits::pipeline_info>(this);
        }

        template<traits::view_class ... ts>
        view<ts...>& view() {
            return objects.get_or_emplace<ecs::view<ts...>>(this);
        }

        template<traits::view_class ... ts>
        ecs::view<ts...>& view() const {
            return objects.get<ecs::view<ts...>>(this);
        }

    private:
        util::any_set objects;
    };

    template<traits::component_class ... ts>
    class typed_registry {
        template<traits::component_class t>
        pool<t>& pool() {
            return std::get<ecs::pool<t>>(objects);
        }

        const pool<t>& pool() const  {
            return std::get<ecs::pool<t>>(objects);
        }

        template<traits::component_class ... ts>
        pipeline<ts...>& pipeline() {
            return ecs::pipeline<ts...>(*this);
        }

        template<traits::view_class ... ts>
        view<ts...>& view() {
            return ecs::view<ts...>(*this);
        }

    private:
        std::tuple<ts...> objects;
    };


}