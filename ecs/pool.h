#pragma once
#include <algorithm>
#include <functional>
#include <shared_mutex>
#include "fwd.h"
#include "util/sparse_map.h"
#include "util/next_pow2.h"
#include "pool_policy.h"

namespace ecs {
    template<traits::comp_class comp_t>
    class pool {
        friend struct ecs::pool_policy::where;
        friend struct ecs::pool_policy::front_policy;
        friend struct ecs::pool_policy::back_policy;
        friend struct ecs::pool_policy::swap_policy;
        friend struct ecs::pool_policy::inplace_policy;
        //friend class pool_policy::deferred_swap;
        //friend class pool_policy::deferred_inplace;


        using sparse_t = util::sparse_map<entity>;
        using return_t = std::conditional_t<std::is_empty_v<comp_t>, void, comp_t&>;    
        using default_order = ecs::pool_policy::swap_policy;
        using default_exec = ecs::pool_policy::immediate_policy;
        
    public:
        pool(size_t cap = 8) : packed(std::allocator<entity>().allocate(cap)), capacity(cap), size(0) { 
            if constexpr (!std::is_empty_v<comp_t>)
                buffer = std::allocator<comp_t>().allocate(cap);
        }

        ~pool() {
            if constexpr (!std::is_empty_v<comp_t>)
            {
                std::for_each(buffer, buffer + size, [&](comp_t& curr) { std::destroy_at(&curr); });
                std::allocator<comp_t>().deallocate(buffer, capacity);
            }
    
            std::allocator<entity>().deallocate(packed, capacity);
        }

        __attribute__ ((warning("copying constructing pool function called")))
        pool(const pool<comp_t>&) = default;

        __attribute__ ((warning("copying assigning pool function called")))
        pool<comp_t>& operator=(const pool<comp_t>&) = default;

        bool contains(entity e) const { return sparse.contains(e); }

        entity at(size_t i) const { return packed[i]; }

        comp_t& get_comp(entity e) { return buffer[sparse[e]]; }
        const comp_t& get_comp(entity e) const { return buffer[sparse[e]]; }

        comp_t& get_comp_at(size_t i) { return buffer[i]; }
        const comp_t& get_comp_at(size_t i) const { return buffer[i]; }

        size_t count() const { return size; }

        ecs::iterator<pool<comp_t>> begin() { return ecs::iterator<pool<comp_t>>{ this, 0 }; }
        ecs::iterator<pool<const comp_t>> begin() const { return ecs::iterator<pool<const comp_t>>{ this, 0 }; }

        ecs::iterator<pool<comp_t>> end() { return ecs::iterator<pool<comp_t>>{ this, size }; }
        ecs::iterator<pool<const comp_t>> end() const { return ecs::iterator<pool<const comp_t>>{ this, size }; }

        void reserve(size_t cap) {
            size_t new_capacity = util::next_pow2(cap);
                
            if constexpr (!std::is_empty_v<comp_t>)
            {
                comp_t* new_buffer = std::allocator<comp_t>().allocate(new_capacity);
                std::move(buffer, buffer + size, new_buffer);
                std::allocator<comp_t>().deallocate(buffer, capacity);

                buffer = new_buffer;
            }

            {
                entity* new_packed = std::allocator<entity>().allocate(new_capacity);
                std::move(packed, packed + size, new_packed);
                std::allocator<entity>().deallocate(packed, capacity);
                
                packed = new_packed;
            }
            
            capacity = new_capacity;
        }

        void lock() { mtx.lock(); }
        void lock() const { mtx.lock_shared(); }
        
        void unlock() { mtx.unlock(); }
        void unlock() const { mtx.unlock_shared(); }

        template<typename order_pol = default_order, typename exec_pol = default_exectypename ... arg_us>
        return_t emplace(entity e, arg_ts&& ... args) { return erase<order_pol, exec_pol>(back{} e, std::forward(args)...); }

        template<typename order_pol = default_order, typename exec_pol = default_exec, typename at_t, typename it, typename ... arg_us>
        void emplace(at_t where, it first, it last, arg_us&& ... args) { 
            size_t index = order_pol::get_index(*this, where);
            size_t count = last - first;
            
            order_pol::allocate_for(*this, where, count);

            std::copy(first, last, packed + index);

            for (int i = 0; i < count; ++i) sparse[*(first++)] = i;

            if constexpr (!std::is_empty_v<comp_t>)
                std::for_each(buffer + index, buffer + index + count, [&](comp_t& curr) { std::construct_at(&curr, std::forward<arg_us>(args)...); });
        };

        template<typename order_pol = default_order, typename exec_pol = default_exec, typename at_t, typename ... arg_us>
        return_t emplace(at_t where, entity e, arg_us&& ... args) {
            size_t index = order_pol::where::get_index(*this, where);
            size_t count = 1;

            order_pol::allocate_for(*this, where, count);

            packed[index] = e;

            sparse[e] = index;
            
            if constexpr (!std::is_empty_v<comp_t>)
                return *std::construct_at(buffer + index, std::forward<arg_us>(args)...);
        }
        
        template<typename order_pol = default_order, typename exec_pol = default_exectypename ... arg_us>
        void erase(entity e) { erase<order_pol, exec_pol>(elem{e}); }

        template<typename order_pol = default_order, typename exec_pol = default_exec, typename it>
        void erase(it first, it last) {
            while (first != last) erase<order_pol, exec_pol>(elem{*(first++)});
        }
        
        template<typename order_pol = default_order, typename exec_pol = default_exec, typename at_t>
        void erase(at_t where, size_t count = 1) {
            size_t index = order_pol::where::get_index(*this, where);
            
            if constexpr (!std::is_empty_v<comp_t>)
                std::for_each(buffer + index, buffer + index + count, [&](comp_t& curr) { std::destroy_at(&curr); });
            
            order_pol::deallocate_for(*this, where, count);
        }

    private:
        mutable std::shared_mutex mtx;

        sparse_t sparse;
        entity*  packed;
        comp_t*  buffer;

        size_t   capacity;
        size_t   size;
    };

    template<traits::comp_class t>
    class iterator<pool<t>> {
        using base_t = traits::pool_builder<t>;
    public:
        iterator(base_t* b, size_t i) : base(b), index(i) { }

        auto operator*() const { 
            if constexpr (std::is_empty_v<t>) return base->at(index); 
            else return std::tuple<entity, t&> { base->at(index), base->get_comp_at(index) }; 
        }
        
        iterator<pool<t>>& operator++() { ++index; return *this; }
        iterator<pool<t>>& operator--() { --index; return *this; }
        template<std::integral int_t> iterator<pool<t>>& operator+=(int_t i) { index += i; return *this; }
        template<std::integral int_t> iterator<pool<t>>& operator-=(int_t i) { index -= i; return *this; }
        template<std::integral int_t> iterator<pool<t>> operator+(int_t i) { return iterator<pool<t>> { base, index + i }; }
        template<std::integral int_t> iterator<pool<t>> operator-(int_t i) { return iterator<pool<t>> { base, index - i }; }
        ptrdiff_t operator-(const iterator<pool<t>>& other) { return index - other.index; }
        bool operator==(const iterator<pool<t>>& other) { return index == other.index; }
        bool operator!=(const iterator<pool<t>>& other) { return index != other.index; }
        bool operator<(const iterator<pool<t>>& other) { return index < other.index; }
        bool operator>(const iterator<pool<t>>& other) { return index > other.index; }
        bool operator<=(const iterator<pool<t>>& other) { return index <= other.index; }
        bool operator>=(const iterator<pool<t>>& other) { return index >= other.index; }

    private:
        base_t* base;
        size_t index;
    };
}
