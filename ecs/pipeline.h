#pragma once
#include "fwd.h"
#include "registry.h"
#include "tuple_util.h"
namespace ecs {
    template<traits::component_class ... ts>
    class pipeline {
        template<typename u>
        using pool_t = std::conditional_t<std::is_const_v<u>, const pool<u>, pool<u>>*;
        template<typename u>
        static constexpr bool is_accessible = (std::is_same_v<std::remove_const_t<u>, std::remove_const_t<ts>> || ...);
    public:
        pipeline(registry* reg) { 
            std::lock_guard guard(*reg);
            pools = std::tuple{ &reg->pool<ts>()... };
        }

        pipeline(const registry* reg) { 
            std::lock_guard guard(*reg);
            pools = std::tuple{ &reg->pool<ts>() ... };
        }

        void lock() const {
            (std::get<pool_t<ts>>(pools)->lock(), ...);
        }
        
        void unlock() const {
            (std::get<pool_t<ts>>(pools)->unlock(), ...);
        }

        template<traits::component_class u> requires (is_accessible<u>)
        ecs::pool<u>& pool() const {
        }

    private:
        std::tuple<pool_t<ts>...> pools;
    };





}