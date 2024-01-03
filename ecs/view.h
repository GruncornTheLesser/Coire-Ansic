#pragma once
#include "fwd.h"


namespace ecs {
    template<type_traits::pipeline_class pip_t, type_traits::comp_class inc_t, type_traits::comp_class ... inc_ts, type_traits::comp_class ... exc_ts>
    class view<pip_t, inc<inc_t, inc_ts...>, exc<exc_ts...>> {
    public:
        using elem_type = util::tuple_extract<typename util::tuple_filter<std::tuple<pip_t, inc_t, inc_ts...>, util::not_empty>::type, container::view_element>::type;


        class iterator
        {
        public:
            iterator(pip_t& pip, size_t i) : pip(pip), ind(i) { }

            elem_type operator*() {
                return { pip, ind };
            }

            iterator operator++() {
                entity e;
                bool  x;
                do { 
                    if (++ind >= pip.template pool<inc_t>().size()) return *this;  
                    e = pip.template pool<const inc_t>()[ind];
                } while(!(pip.template pool<inc_ts>().contains(e) && ...) || (pip.template pool<const exc_ts>().contains(e) || ...)); 
                return *this;
            }
            ptrdiff_t operator-(const iterator& other) { return ind - other.ind; }
            bool operator==(const iterator& other) { return ind == other.ind; }
            bool operator!=(const iterator& other) { return ind != other.ind; }
            bool operator<(const iterator& other) { return ind < other.ind; }
            bool operator>(const iterator& other) { return ind > other.ind; }
            bool operator<=(const iterator& other) { return ind <= other.ind; }
            bool operator>=(const iterator& other) { return ind >= other.ind; }
        private:
            pip_t& pip;
            size_t ind;
        };

        view(pip_t& pip) : pip(pip) { }

        iterator begin() { return ++iterator{ pip, -1ull }; }
        iterator end() { return iterator{ pip, pip->template pool<std::tuple_element_t<0, std::tuple<inc_ts...>>>().count() }; }

    private:
        pip_t& pip;
    };
}