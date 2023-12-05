#pragma once
#include "fwd.h"

namespace ecs {
    template<traits::pipeline_class pip_t, traits::component_class ... inc_ts, traits::component_class ... exc_ts>
    class view<pip_t, inc<inc_ts...>, exc<exc_ts...>> {
    public:
        using view_iterator = ecs::iterator<view<pip_t, inc<inc_ts...>, exc<exc_ts...>>>;

        using return_t = decltype(std::tuple_cat(std::declval<std::tuple<entity>>(), 
            std::declval<std::conditional_t<std::is_empty_v<inc_ts>, std::tuple<entity>, std::tuple<inc_ts&>>>()...));
        view(pip_t* pip) : base(pip) { }

        view_iterator begin() const { return ++view_iterator{ base, -1ull }; }
        view_iterator end() const { return view_iterator{ base, base->template pool<std::tuple_element_t<0, std::tuple<inc_ts...>>>().count() }; }

    private:
        pip_t* base;
    };

    template<typename ... pip_ts, traits::component_class inc_t, traits::component_class ... inc_ts, traits::component_class ... exc_ts>
    class iterator<view<pipeline<pip_ts...>, inc<inc_t, inc_ts...>, exc<exc_ts...>>> {
    using pip_t = pipeline<pip_ts...>;
    public:
        iterator(pip_t* pip, size_t i) : base(pip), index(i) { }

        auto operator*() const { 
            entity e = base->template pool<inc_t>().at(index);
            return std::tuple_cat(std::tuple<entity, inc_t&>(e, base->template pool<inc_t>().get_component_at(index)), get_or_empty<inc_ts>()...);
        }
        iterator<view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>>>& operator++() { 
            entity e;
            bool  x;
            do { 
                if (++index >= base->template pool<inc_t>().count()) return *this;  
                e = base->template pool<inc_t>().at(index);
            } while(!(base->template pool<inc_ts>().contains(e) && ...) || (base->template pool<const exc_ts>().contains(e) || ...)); 
            return *this;
        }
        ptrdiff_t operator-(const iterator<view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>>>& other) { return index - other.index; }
        bool operator==(const iterator<view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>>>& other) { return index == other.index; }
        bool operator!=(const iterator<view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>>>& other) { return index != other.index; }
        bool operator<(const iterator<view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>>>& other) { return index < other.index; }
        bool operator>(const iterator<view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>>>& other) { return index > other.index; }
        bool operator<=(const iterator<view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>>>& other) { return index <= other.index; }
        bool operator>=(const iterator<view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>>>& other) { return index >= other.index; }
    private:
        template<typename u>
        auto get_or_empty(entity e) {
            if constexpr(std::is_empty_v<u>) return std::tuple<>{};
            else return std::tuple<u&>(base->template pool<u>().get_component(e));
        }
        pip_t* base;
        size_t index;
    };
}


