#pragma once
#include "fwd.h"
#include "registry.h"
#include "util/tuple_util.h"
namespace ecs {
    template<traits::comp_class ... ts>
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

        void lock() { (pool<ts>().lock(), ...); }
        
        void unlock() { (pool<ts>().unlock(), ...); }

        template<traits::comp_class u> requires (is_accessible<u>)
        traits::pool_builder<u>& pool() const {
            return *std::get<typename traits::pool_builder<std::remove_const_t<u>>*>(pools);
        }

        template<traits::comp_class ... inc_us, traits::comp_class ... exc_us>
        auto view(exc<exc_us...> = exc<>{}) {
            return ecs::view<ecs::pipeline<ts...>, ecs::inc<inc_us...>, ecs::exc<exc_us...>>(this);
        }
    private:
        std::tuple<traits::pool_builder<std::remove_const_t<ts>>*...> pools;
    };





}