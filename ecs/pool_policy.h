#pragma once
#include "pool.h"
#include "util/next_pow2.h"
namespace ecs::pool_policy {
    struct back {
    public:
        template<typename comp_t>
        static inline void allocate(pool<comp_t>& pl, size_t count) {
            size_t new_size = pl.size + count;
            if (new_size >= pl.capacity)
            {
                size_t new_capacity = util::next_pow2(new_size);
                
                if constexpr (!std::is_empty_v<comp_t>)
                {
                    comp_t* new_buffer = std::allocator<comp_t>().allocate(new_capacity);
                    std::move(pl.buffer, pl.buffer + pl.size, new_buffer);
                    std::allocator<comp_t>().deallocate(pl.buffer, pl.capacity);
                    pl.buffer = new_buffer;
                }

                entity* new_packed = std::allocator<entity>().allocate(new_capacity);
                std::move(pl.packed, pl.packed + pl.size, new_packed);
                std::allocator<entity>().deallocate(pl.packed, pl.capacity);
                pl.packed = new_packed;

                pl.capacity = new_capacity;
            }
            pl.size = new_size;
        }

        template<typename comp_t>
        static inline void deallocate(pool<comp_t>& pl, size_t count) {
            size_t new_size = pl.size - count;
            pl.size = new_size;
        }
    };

    struct immediate_swap {
    public:
        template<typename comp_t>
        static inline void allocate_at(pool<comp_t>& pl, size_t index, size_t count) {
            size_t new_size = pl.size + count;
            
            size_t last = std::min(index + count, pl.size); // end of range that needs moving
            size_t back = std::max(index + count, pl.size); // new_size - number of moved elements
                
            if (pl.capacity <= new_size) 
            {
                size_t new_capacity = util::next_pow2(new_size);
                
                if constexpr (!std::is_empty_v<comp_t>)
                {
                    comp_t* new_buffer = std::allocator<comp_t>().allocate(new_capacity);
                    std::move(pl.buffer, pl.buffer + index, new_buffer);
                    std::move(pl.buffer + index, pl.buffer + last, new_buffer + back);
                    std::move(pl.buffer + last,  pl.buffer + back, new_buffer + last);
                    std::allocator<comp_t>().deallocate(pl.buffer, pl.capacity);

                    pl.buffer = new_buffer;
                }

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
                if constexpr (!std::is_empty_v<comp_t>)
                    std::move_backward(pl.buffer + index, pl.buffer + last, pl.buffer + new_size);
                std::move_backward(pl.packed + index, pl.packed + last, pl.packed + new_size);
            }

            pl.size = new_size;
        
            // update sparse map lookup
            for (size_t i = back; i < new_size; ++i)
                pl.sparse[pl.packed[i]] = i;
        }

        template<typename comp_t>
        static inline void deallocate_at(pool<comp_t>& pl, size_t index, size_t count) {
            // if deallocate range overlaps with back replacing range:
            // 

            size_t new_size = pl.size - count;
            size_t back = index + count;
            size_t src =      std::max(back, new_size);
            size_t dst_last = std::min(back, new_size);

            if constexpr (!std::is_empty_v<comp_t>)
                std::move(pl.buffer + src, pl.buffer + pl.size, pl.buffer + index);
            
            std::move(pl.packed + src, pl.packed + pl.size, pl.packed + index);
            
            for (size_t i = index; i < dst_last; ++i)
                pl.sparse[pl.packed[i]] = i;

            pl.size = new_size;

            // update moved entities' index
            
        }
    };
    
    struct immediate_inplace {
    public:
        template<typename comp_t>
        static void allocate_at(pool<comp_t>& pl, size_t index, size_t count) {
            size_t new_size = pl.size + count;
            size_t last = index + count;
            
            if (pl.capacity <= new_size) 
            {
                size_t new_capacity = util::next_pow2(new_size);
               

                if constexpr (!std::is_empty_v<comp_t>)
                {
                    comp_t* new_buffer = std::allocator<comp_t>().allocate(new_capacity);
                    std::move(pl.buffer, pl.buffer + index, new_buffer);           // move up to the index
                    std::move(pl.buffer + index, pl.buffer + pl.size, new_buffer + last);    // move after the index
                    std::allocator<comp_t>().deallocate(pl.buffer, pl.capacity);
                    
                    pl.buffer = new_buffer;
                }

                entity* new_packed = std::allocator<entity>().allocate(new_capacity);
                std::move(pl.packed, pl.packed + index, new_packed);           // move up to the index
                std::move(pl.packed + index, pl.packed + pl.size, new_packed + last);    // move after the index
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

            pl.size = new_size;

            // update sparse map lookup
            for (size_t i = last; i < new_size; ++i)
                pl.sparse[pl.packed[i]] = i;
        }
    
        template<typename comp_t>
        static void deallocate_at(pool<comp_t>& pl, size_t index, size_t count) {
            size_t last = index + count;
    
            if constexpr (!std::is_empty_v<comp_t>)
                std::move(pl.buffer + last, pl.buffer + pl.size, pl.buffer + index);
    
            std::move(pl.packed + last, pl.packed + pl.size, pl.packed + index);
            pl.size -= count;

            // update sparse map lookup
            for (size_t i = index; i < pl.size; ++i)
                pl.sparse[pl.packed[i]] = i;
        }
    };

    struct deferred_swap;
    struct deferred_inplace;
}

