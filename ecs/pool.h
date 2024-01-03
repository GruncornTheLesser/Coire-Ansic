#pragma once
#include <algorithm>
#include <functional>
#include <shared_mutex>
#include "fwd.h"
#include "entity.h"
#include "util/sparse_map.h"
#include "util/next_pow2.h"
//#include "allocator.h"
namespace ecs {
    template<typename elem_t>
    class pool_t {
    private:
        using sparse_t = util::sparse_map<unsigned long int>;
        using packed_t = elem_t*;
        using iterator = elem_t*;
        using const_iterator = const elem_t*;
        using pointer = elem_t*;
        using const_pointer = const elem_t*;
        using reference = elem_t&;
        using const_reference = const elem_t&;
        static constexpr policy::order default_order_policy = policy::order::swap;
        static constexpr policy::execution default_exec_policy = policy::execution::immediate;
    public:
        pool_t(size_t cap = 8) : packed(std::allocator<elem_t>().allocate(cap)), extent(0), capacity(cap) { }
        
        __attribute__ ((warning("copy constructing pool")))
        pool_t(const pool_t<elem_t>&) = default;

        __attribute__ ((warning("copy assigning pool")))
        pool_t<elem_t>& operator=(const pool_t<elem_t>&) = default;
        
        ~pool_t() { 
            std::for_each(begin(), end(), [](elem_t& elem){ std::destroy_at(&elem); });
            std::allocator<elem_t>().deallocate(capacity);
        }

        bool contains(entity e) const { return sparse.at(e); }

        reference operator[](entity e) { return packed[sparse.at(e)]; }
        reference operator[](size_t i) { return packed[i]; }
        
        const_reference operator[](entity e) const { return packed[sparse.at(e)]; }
        const_reference operator[](size_t i) const { return packed[i]; }

        size_t size() const { return extent; }

        iterator begin() { return packed; }
        iterator end() { return packed + extent; }

        const_iterator begin() const { return packed; }
        const_iterator end() const { return packed + extent; }

        void lock() { mtx.lock(); }
        void lock() const { mtx.lock_shared(); }

        void unlock() { mtx.unlock(); }
        void unlock() const { mtx.unlock_shared(); }

        template<policy::order order_pol = default_order_policy, policy::execution exec_pol = default_exec_policy, typename ... arg_us>
        elem_t& emplace(entity e, arg_us&& ... args) {
            ecs::allocator::pool_sequencer<exec_pol, elem_t>(*this).template emplace<order_pol>(packed + extent, e, std::forward<arg_us>(args)...);
        }

        template<policy::order order_pol = default_order_policy, policy::execution exec_pol = default_exec_policy, typename it, typename ... arg_us>
        void emplace(it first, it last, arg_us&& ... args) {
            allocator::pool_sequencer<exec_pol, elem_t>(*this).template emplace<order_pol>(packed + extent, first, last, std::forward<arg_us>(args)...);
        }

        template<policy::order order_pol = default_order_policy, policy::execution exec_pol = default_exec_policy, typename ... arg_us>
        elem_t& emplace(iterator where, entity e, arg_us&& ... args) {
            allocator::pool_sequencer<exec_pol, elem_t>(*this).template emplace<order_pol>(where, e, std::forward<arg_us>(args)...);
        }

        template<policy::order order_pol = default_order_policy, policy::execution exec_pol = default_exec_policy, typename it, typename ... arg_us>
        void emplace(iterator where, it first, it last, arg_us&& ... args) {
            allocator::pool_sequencer<exec_pol, elem_t>(*this).template emplace<order_pol>(where, first, last, std::forward<arg_us>(args)...);
        }

        template<policy::order order_pol = default_order_policy, policy::execution exec_pol = default_exec_policy>
        void erase(entity e) {
            allocator::pool_sequencer<exec_pol, elem_t>(*this).template erase<order_pol>(&operator[](e), 1);
        }

        template<policy::order order_pol = default_order_policy, policy::execution exec_pol = default_exec_policy, typename it>
        void erase(it first, it last) {
            allocator::pool_sequencer<exec_pol, elem_t>(*this).template erase<order_pol>(first, last);
        }

        template<policy::order order_pol = default_order_policy, policy::execution exec_pol = default_exec_policy, typename it>
        void erase(iterator where, size_t count) {
            allocator::pool_sequencer<exec_pol, elem_t>(*this).template erase<order_pol>(where, count);
        }

    private:
        mutable std::shared_mutex mtx;

        sparse_t sparse; 
        packed_t packed;

        size_t capacity;
        size_t extent;
    };
}
