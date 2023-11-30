#pragma once
#include "fwd.h"
#include "registry.h"
#include "tuple_util.h"
namespace ecs {
    namespace utility {
        
    }
    template<traits::component_class ... ts>
    class pipeline {
        template<typename t>
        using pool_t = std::conditional_t<std::is_const_v<t>, const pool<t>, pool<t>>*;
    public:
        pipeline(registry* reg) { 
            reg->lock();
            pools = std::tuple{ &reg->pool<ts>()... };            
            reg->unlock();
        }

        pipeline(const registry* reg) { 
            reg->lock();
            std::lock_guard<const resource> guard{ reg };
            pools = std::tuple{ &reg->pool<ts>() ... };
            reg->unlock();
        }

        void lock() {
            (std::get<pool_t<ts>>(pools)->lock(), ...);
        }
        
        void unlock() {
            (std::get<pool_t<ts>>(pools)->unlock(), ...);
        }
    private:
        std::tuple<pool_t<ts>...> pools;
    };





}