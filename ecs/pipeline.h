#pragma once
#include "fwd.h"
#include "registry.h"
#include "util/tuple_util.h"
namespace ecs {
    template<traits::component_class ... ts>
    class pipeline {
        template<typename u>
        static constexpr bool is_accessible = (std::conditional_t<std::is_const_v<ts>, std::is_same<u, ts>, std::is_same<std::remove_const_t<u>, ts>>::value || ...);
    public:
        pipeline(registry* reg) { 
            std::lock_guard guard(*reg);
            pools = std::tuple{ &reg->pool<std::remove_const_t<ts>>()... };
        }

        pipeline(const registry* reg) { 
            std::lock_guard guard(*reg);
            pools = std::tuple{ &reg->pool<std::remove_const_t<ts>>() ... };
        }

        void lock() const { (pool<ts>().lock(), ...); }
        
        void unlock() const { (pool<ts>().unlock(), ...); }

        template<traits::component_class u> requires (is_accessible<u>)
        ecs::traits::pool_builder<u>& pool() const {
            return *std::get<typename ecs::traits::pool_builder<std::remove_const_t<u>>*>(pools);
        }

    private:
        std::tuple<typename ecs::traits::pool_builder<std::remove_const_t<ts>>*...> pools;
    };





}