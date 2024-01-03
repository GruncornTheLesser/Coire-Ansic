#pragma once
#include "fwd.h"
#include "util/any_set.h"
#include <shared_mutex>
namespace ecs {
    class registry {
        template<type_traits::pool_class ... ts> friend class pipeline_t;
    public:        
        template<type_traits::comp_class ... us>
        ecs::pipeline<us...> pipeline() {
            return ecs::pipeline<us...>{ *this };
        }
        template<type_traits::comp_class ... us>
        ecs::pipeline<us...> pipeline() const {
            return ecs::pipeline<us...>{ *this };
        }
        void lock() { mtx.lock(); }
        void lock() const { mtx.lock_shared(); }
        
        void unlock() { mtx.unlock(); }
        void unlock() const { mtx.unlock_shared(); }

    private:
        mutable std::shared_mutex mtx;
        ::util::any_set poolset;
    };
}