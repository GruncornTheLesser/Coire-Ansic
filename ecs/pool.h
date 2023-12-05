#pragma once
#include <algorithm>
#include <functional>
#include "fwd.h"
#include "util/sparse_map.h"
#include "util/next_pow2.h"
#include "resource.h"

namespace ecs {
    template<traits::component_class comp_t>
    class pool : public resource {
        friend class pool_policy::immediate_swap;
        friend class pool_policy::immediate_inplace;
        friend class pool_policy::deferred_swap;
        friend class pool_policy::deferred_inplace;
        friend class pool_policy::back;

        using sparse_t = util::sparse_map<entity>;
        using return_t = std::conditional_t<std::is_empty_v<comp_t>, void, comp_t&>;    
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

        comp_t& get_component(entity e) { return buffer[sparse[e]]; }
        const comp_t& get_component(entity e) const { return buffer[sparse[e]]; }

        comp_t& get_component_at(size_t i) { return buffer[i]; }
        const comp_t& get_component_at(size_t i) const { return buffer[i]; }

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

        template<typename pol = pool_policy::back, typename ... arg_ts>
        return_t emplace_back(entity e, arg_ts&& ... args) {
            size_t back = size;
            pol::allocate(*this, 1ul);

            packed[back] = e;

            sparse[e] = back;

            if constexpr (!std::is_empty_v<comp_t>)
                return *std::construct_at(buffer + back, std::forward<arg_ts>(args)...);
        }

        template<typename pol = pool_policy::back, typename it, typename ... arg_ts>
        void emplace_back(it first, it last, arg_ts&& ... args) {
            size_t back = size;
            size_t count = last - first;
            pol::allocate(*this, count);

            std::copy(first, last, packed + back);

            size_t index = back;
            std::for_each(first, last, [&](entity e) { sparse[e] = index++; });
            
            if constexpr (!std::is_empty_v<comp_t>)
                std::for_each(buffer + back, buffer + size, [&](comp_t& curr) { std::construct_at(&curr, std::forward<arg_ts>(args)...); });
        }

        template<typename pol = pool_policy::back>
        void erase_back(size_t count = 1) {
            size_t back = size;
            size_t new_back = back - count;

            if constexpr (!std::is_empty_v<comp_t>)
                std::for_each(buffer + new_back, buffer + back, [&](comp_t& curr) { std::destroy_at(&curr); });
            
            std::for_each(packed + new_back, packed + back, [&](entity e) { sparse[e] = util::tombstone; });

            pol::deallocate(*this, count);
        }

        template<traits::policy_class<comp_t> pol = pool_policy::swap, typename ... arg_ts> 
        return_t emplace_at(size_t i, entity e, arg_ts&& ... args) {
            pol::allocate_at(*this, i, 1);
            packed[i] = e;
            sparse[e] = i;
            if constexpr (!std::is_empty_v<comp_t>)
                return *std::construct_at(buffer + i, std::forward<arg_ts>(args)...);
        }

        template<traits::policy_class<comp_t> pol = pool_policy::swap, typename it, typename ... arg_ts>
        void emplace_at(size_t i, it first, it last, arg_ts&& ... args) {
            size_t b = std::is_same_v<pool_policy::immediate_swap, pol>;
            size_t count = last - first;
            pol::allocate_at(*this, i, count);
            
            std::copy(first, last, packed + i);

            size_t index = i;
            std::for_each(first, last, [&](entity e) { sparse[e] = index++; });

            if constexpr (!std::is_empty_v<comp_t>)
                std::for_each(buffer + i, buffer + i + count, [&](comp_t& curr) { std::construct_at(&curr, std::forward<arg_ts>(args)...); });
        }

        template<traits::policy_class<comp_t> pol = pool_policy::immediate_swap> 
        void erase(entity e) {
            size_t index = sparse[e];
            
            if constexpr (!std::is_empty_v<comp_t>)
                std::destroy_at(buffer + index);
            
            sparse[e] = util::tombstone;

            pol::deallocate_at(*this, index, 1);
        }

        template<traits::policy_class<comp_t> pol = pool_policy::swap, typename it> 
        void erase(it first, it last) {
            //NOTE: immediate_inplace erase will be hugely inefficient and should prefer deferred_erase
            //NOTE: custom implementation that calls a deferred_erase pass and deferred_execute?
            for (it curr = first; curr != last; ++curr) erase<pol>(*curr);
        }

        template<traits::policy_class<comp_t> pol = pool_policy::swap> 
        void erase_at(size_t i, size_t count = 1) {
            if constexpr (!std::is_empty_v<comp_t>)
                std::for_each(buffer + i, buffer + i + count, [&](comp_t& curr) { std::destroy_at(&curr); });
            
            sparse[packed[i]] = util::tombstone;

            pol::deallocate_at(*this, i, count);
        }

    private:
        sparse_t sparse;
        entity*  packed;
        comp_t*  buffer;

        size_t   capacity;
        size_t   size;
    };

    template<traits::component_class t>
    class iterator<pool<t>> {
        using base_t = traits::pool_builder<t>;
    public:
        iterator(base_t* b, size_t i) : base(b), index(i) { }

        auto operator*() const { 
            if constexpr (std::is_empty_v<t>) return base->at(index); 
            else return std::tuple<entity, t&> { base->at(index), base->get_component_at(index) }; 
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
