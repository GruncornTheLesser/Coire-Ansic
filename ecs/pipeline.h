#pragma once
#include "fwd.h"
#include "registry.h"
namespace ecs {
    template<type_traits::pool_class ... ts>
    class pipeline_t {
        using poolset_t = std::tuple<std::remove_const_t<ts>&...>;
        template<typename u>
        static constexpr bool is_accessible = (std::conditional_t<std::is_const_v<ts>, std::is_same<ecs::pool<u>, ts>, std::is_same<std::remove_const_t<ecs::pool<u>>, ts>>::value || ...);
    public:
        pipeline_t(registry& reg) : poolset(poolset_t{ reg.poolset.get<std::remove_const_t<ts>>()... }) { }

        pipeline_t(const registry& reg) : poolset(poolset_t{ reg.poolset.get<std::remove_const_t<ts>>()... }) { }

        void lock() { (pool<ts>().lock(), ...); }
        void unlock() { (pool<ts>().unlock(), ...); }

        template<type_traits::comp_class u> requires (is_accessible<u>)
        ecs::pool<u>& pool() {
            return std::get<ecs::pool<std::remove_const_t<u>>&>(poolset);
        }

        template<type_traits::comp_class u> requires (is_accessible<u>)
        ecs::pool<const u>& pool() const {
            return std::get<ecs::pool<std::remove_const_t<u>>&>(poolset);
        }

        template<type_traits::comp_class ... inc_us, type_traits::comp_class ... exc_us>
        auto view(exc<exc_us...> = exc<>{}) {
            return ecs::view<ecs::pipeline_t<ts...>, ecs::inc<inc_us...>, ecs::exc<exc_us...>>(*this);
        }
    private:
        poolset_t poolset;
    };
}