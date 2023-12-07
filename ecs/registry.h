#pragma once
#include "fwd.h"
#include "util/any_set.h"
#include <shared_mutex>
namespace ecs {
    class registry {
        template<traits::comp_class ... ts>
        friend class pipeline;
    public:
        template<traits::comp_class u>
        ecs::traits::pool_builder<u>& pool() {
            // get non const/non-const pool reference and cast on return
            return pools.get_or_emplace<ecs::pool<std::remove_const_t<u>>>();
        }
        template<traits::comp_class u>
        ecs::traits::pool_builder<u>& pool() const {
            // get non const/non-const pool reference and cast on return
            return pools.get<ecs::pool<std::remove_const_t<u>>>(); 
        }

        template<traits::comp_class ... us>
        traits::pipeline_builder<us...> pipeline() {
            return typename ecs::traits::pipeline_builder<us...>{ this };
        }

        template<traits::comp_class ... us>
        traits::pipeline_builder<us...> pipeline() const {
            return typename ecs::traits::pipeline_builder<us...>{ this };
        }

        void lock() { mtx.lock(); }
        void lock() const { mtx.lock_shared(); }
        
        void unlock() { mtx.unlock(); }
        void unlock() const { mtx.unlock_shared(); }

    private:
        mutable std::shared_mutex mtx;
        util::any_set     pools;
    };
}