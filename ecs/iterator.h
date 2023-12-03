#pragma once
#include "fwd.h"

namespace ecs {
    template<traits::component_class t>
    class pool_iterator {
        using base_t = traits::pool_builder<t>;
    public:
        pool_iterator(base_t* b, size_t i) : base(b), index(i) { }

        auto operator*() const { 
            if constexpr (std::is_empty_v<t>) return base->at(index); 
            else return std::tuple<entity, t&> { base->at(index), base->get_component_at(index) }; 
        }
        pool_iterator<t>& operator++() { ++index; return *this; }
        pool_iterator<t>& operator--() { --index; return *this; }
        template<std::integral int_t> pool_iterator<t>& operator+=(int_t i) { index += i; return *this; }
        template<std::integral int_t> pool_iterator<t>& operator-=(int_t i) { index -= i; return *this; }
        template<std::integral int_t> pool_iterator<t> operator+(int_t i) { return pool_iterator<t> { base, index + i }; }
        template<std::integral int_t> pool_iterator<t> operator-(int_t i) { return pool_iterator<t> { base, index - i }; }
        ptrdiff_t operator-(const pool_iterator<t>& other) { return index - other.index; }
        bool operator==(const pool_iterator<t>& other) { return index == other.index; }
        bool operator!=(const pool_iterator<t>& other) { return index != other.index; }
        bool operator<(const pool_iterator<t>& other) { return index < other.index; }
        bool operator>(const pool_iterator<t>& other) { return index > other.index; }
        bool operator<=(const pool_iterator<t>& other) { return index <= other.index; }
        bool operator>=(const pool_iterator<t>& other) { return index >= other.index; }

    private:
        base_t* base;
        size_t index;
    };
}