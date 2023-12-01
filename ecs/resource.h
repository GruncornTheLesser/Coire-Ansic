#pragma once
#include <shared_mutex>
namespace ecs {
    struct resource {
    public:
        void lock() { mtx.lock(); }
        void lock() const { mtx.lock_shared(); }
        
        void unlock() { mtx.unlock(); }
        void unlock() const { mtx.unlock_shared(); }
        
        bool try_lock() { return mtx.try_lock(); }
        bool try_lock() const { return mtx.try_lock_shared(); }
    private:
        mutable std::shared_mutex mtx; 
    };
}