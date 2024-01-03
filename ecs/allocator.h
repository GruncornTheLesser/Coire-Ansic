#pragma once
#include "fwd.h"
#include "pool.h"

namespace ecs::allocator {
    template<typename elem_t>
    struct pool_allocator<policy::order::swap, elem_t> {
        using it = storage<elem_t>::iterator;
    public:
        pool_allocator(storage<elem_t>& pl) : pl(pl) { }

        size_t allocate(it& where, size_t count) {
            size_t index = where - pl.packed;
            size_t new_size = pl.size + count;
            
            size_t last = std::min(index + count, pl.size); // end of range that needs moving
            size_t back = std::max(index + count, pl.size); // new_size - number of moved elements
            
            if (pl.capacity <= new_size) 
            {
                size_t new_capacity = util::next_pow2(new_size);
                entity* new_packed = std::allocator<entity>().allocate(new_capacity);

                std::move(pl.packed, pl.packed + index, new_packed);
                std::move(pl.packed + index, pl.packed + last, new_packed + back);
                std::move(pl.packed + last,  pl.packed + back, new_packed + last);

                std::allocator<entity>().deallocate(pl.packed, pl.capacity);
                
                pl.packed = new_packed;
                pl.capacity = new_capacity;
            }
            else
            {
                std::move_backward(pl.packed + index, pl.packed + last, pl.packed + new_size);
            }
        
            // update sparse map lookup
            for (size_t i = back; i < new_size; ++i)
                pl.sparse[pl.packed[i]] = i;

            return new_size;
        }

        size_t deallocate(storage<elem_t>::iterator& where, size_t count) {
            size_t new_size = pl.size - count;
            size_t last = index + count;

            size_t src = std::max(last, new_size);
            size_t dst = std::min(last, new_size);

            if constexpr (!std::is_empty_v<comp_t>)
                std::move(pl.buffer + src, pl.buffer + pl.size, pl.buffer + index);
            
            std::move(pl.packed + src, pl.packed + pl.size, pl.packed + index);
            
            for (size_t i = index; i < dst; ++i)
                pl.sparse[pl.packed[i]] = i;
            
            return new_size;
        }
    private:
        storage<elem_t>& pl;
    };

    template<typename comp_t>
    struct pool_allocator<policy::order::inplace, comp_t> {
    public:
        pool_allocator(storage<comp_t>& pl) : pl(pl) { }

        size_t allocate(size_t index, size_t count) {
            size_t new_size = pl.size + count;
            size_t last = index + count;
            
            if (pl.capacity <= new_size) 
            {
                size_t new_capacity = util::next_pow2(new_size);
               
                if constexpr (!std::is_empty_v<comp_t>)
                {
                    comp_t* new_buffer = std::allocator<comp_t>().allocate(new_capacity);
                    std::move(pl.buffer, pl.buffer + index, new_buffer);                     // move up to the index
                    std::move(pl.buffer + index, pl.buffer + pl.size, new_buffer + last);    // move after the index
                    std::allocator<comp_t>().deallocate(pl.buffer, pl.capacity);
                    
                    pl.buffer = new_buffer;
                }

                entity* new_packed = std::allocator<entity>().allocate(new_capacity);
                std::move(pl.packed, pl.packed + index, new_packed);                        // move up to the index
                std::move(pl.packed + index, pl.packed + pl.size, new_packed + last);       // move after the index
                std::allocator<entity>().deallocate(pl.packed, pl.capacity);
                
                pl.packed = new_packed;

                pl.capacity = new_capacity;
            }
            else
            {
                if constexpr (!std::is_empty_v<comp_t>)
                    std::move_backward(pl.buffer + index, pl.buffer + pl.size, pl.buffer + new_size);
    
                std::move_backward(pl.packed + index, pl.packed + pl.size, pl.packed + new_size);
            }

            // update sparse map lookup
            for (size_t i = last; i < new_size; ++i)
                pl.sparse[pl.packed[i]] = i;

            return new_size;
        }

        size_t deallocate(size_t index, size_t count) {
            size_t new_size = pl.size - count;
            size_t last = index + count;
    
            if constexpr (!std::is_empty_v<comp_t>)
                std::move(pl.buffer + last, pl.buffer + pl.size, pl.buffer + index);
    
            std::move(pl.packed + last, pl.packed + pl.size, pl.packed + index);
            
            // update sparse map lookup
            for (size_t i = index; i < new_size; ++i)
                pl.sparse[pl.packed[i]] = i;
            
            return new_size;
        }
    private:
        storage<comp_t>& pl;
    };

    template<typename elem_t>
    struct pool_sequencer<policy::execution::immediate, elem_t> {
    public:
        pool_sequencer(storage<elem_t>& pl) : pl(pl) { }
        template<policy::order order_pol, typename ... arg_us>
        void emplace(size_t i, size_t n, entity* e, arg_us&& ... args)
        {
            pool_allocator<order_pol, elem_t>(pl).allocate(i, n);
            pool_constructor<elem_t>(pl).construct(i, e, std::forward<arg_us>(args)...);
        }

        template<policy::order order_pol>
        void erase(size_t i, size_t n)
        {
            traits::pool_constructor<elem_t>(pl).destroy();
            traits::pool_allocator<order_pol, elem_t>(pl).deallocate(i, n);
            
        }

    private:
        storage<elem_t>& pl;
    };


    // NOTE: plan for deferred sequence
    //  write entity to back of packed memory block, write index to sparse
    //  must encode index in sparse
    //  must also encode erasure in sparce
    //  must differentiate between alive and dead entities
    //      -1=erasured -> needs to know where to erase it from
    //      0=front
    //      index=emplace
    // in deferred execute
    //  should increment any that are placed on the same index
    // mulitple deferred operations?
    // 
    // 2 operations 
    // create, erase, 
    // add size_t

    // keep destroy list and create list
    // 
}