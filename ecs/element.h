#pragma once
#include "fwd.h"
#include <variant>

// https://dominikberner.ch/structured-bindings/
// create bindable objects that unpacks through overrides to std::tuple_size<elem_t>, std::tuple_element<N, elem_t>

// TODO: init container

namespace std {
    template<typename t> struct tuple_size<ecs::container::basictype<t>> : std::integral_constant<std::size_t, 2> { };
    template<typename t> struct tuple_element<0, ecs::container::basictype<t>> { using type = const ecs::entity; };
    template<typename t> struct tuple_element<1, ecs::container::basictype<t>> { using type = t; };

    template<typename ... ts> struct tuple_size<ecs::container::uniontype<ts...>> : std::integral_constant<std::size_t, 2> { };
    template<typename ... ts> struct tuple_element<0, ecs::container::uniontype<ts...>> { using type = const ecs::entity; };
    template<typename ... ts> struct tuple_element<1, ecs::container::uniontype<ts...>> { using type = typename ecs::container::uniontype<ts...>::compset_type; };

    template<typename ... ts>                struct tuple_size<ecs::container::archetype<ts...>> : std::integral_constant<std::size_t, std::tuple_size_v<typename ecs::container::archetype<ts...>::compset_type> + 1> { };
    template<typename ... ts>                struct tuple_element<0, ecs::container::archetype<ts...>> { using type = const ecs::entity; };
    template<std::size_t n, typename ... ts> struct tuple_element<n, ecs::container::archetype<ts...>> { using type = std::tuple_element_t<n - 1, typename ecs::container::archetype<ts...>::compset_type>; };

    template<typename pip_t, typename ... ts> struct tuple_size<ecs::container::view_element<pip_t, ts...>> : std::integral_constant<std::size_t, sizeof...(ts) + 1> { };
    template<typename pip_t, typename ... ts> struct tuple_element<0, ecs::container::view_element<pip_t, ts...>> { using type = const ecs::entity&; };
    template<std::size_t n, typename pip_t, typename ... ts> struct tuple_element<n, ecs::container::view_element<pip_t, ts...>> { using type = std::tuple_element_t<n - 1, std::tuple<ts...>>&; };
}

namespace ecs::container {
    template<type_traits::comp_class comp_t>
    struct basictype {
        using compset_type = comp_t;

        //template<typename ... arg_ts> basictype(entity e, arg_ts&& ... args) : ent(e), cmp(args...) { }
        
        template<typename u> u& get_component() { return cmp; }
        template<typename u> const u& get_component() const { return cmp; }

        template<size_t n> decltype(auto) get() {
            if constexpr (n == 0) return ent;
            else                  return (cmp);
        }

        template<size_t n> decltype(auto) get() const {
            if constexpr (n == 0) return ent;
            else                  return (cmp);
        }

        operator compset_type&() { return cmp; }
        operator const compset_type&() const { return cmp; }
        
        operator entity() { return ent; }
        operator entity() const { return ent; }

        const entity ent;
        compset_type cmp;
    };

    template<type_traits::comp_class ... ts>
    struct uniontype {
        using compset_type = std::variant<ts...>;

        template<size_t n> decltype(auto) get() {
            if constexpr (n == 0) return ent;
            else                  return (set);
        }

        template<size_t n> decltype(auto) get() const {
            if constexpr (n == 0) return ent;
            else                  return (set);
        }

        template<typename u> operator u&() requires ((std::is_same_v<u, ts> || ...)) { return std::get<u>(set); }
        template<typename u> operator const u&() const requires ((std::is_same_v<u, ts> || ...)) { return std::get<u>(set); }

        operator compset_type&() { return set; }
        operator const compset_type&() const { return set; }

        operator entity() { return ent; }
        operator entity() const { return ent; }

        const entity ent;
        compset_type set;
    };

    template<type_traits::comp_class ... ts>
    struct archetype {
        using compset_type = decltype(std::tuple_cat(std::declval<std::conditional_t<std::is_empty_v<ts>, std::tuple<>, std::tuple<ts>>>()...));

        template<size_t n> decltype(auto) get() {
            if constexpr (n == 0) return ent;
            else                  return (std::get<n - 1>(set));
        }

        template<size_t n> decltype(auto) get() const {
            if constexpr (n == 0) return ent;
            else                  return (std::get<n - 1>(set));
        }

        template<typename u> operator u&() requires ((std::is_same_v<u, ts> || ...)) { return std::get<u>(set); }
        template<typename u> operator const u&() const requires ((std::is_same_v<u, ts> || ...)) { return std::get<u>(set); }

        operator entity() { return ent; }
        operator entity() const { return ent; }

        const entity ent;
        compset_type set;
    };

    template<type_traits::pipeline_class pip_t, type_traits::comp_class ... ts>
    class view_element {
    public:
        view_element(pip_t& p, size_t i) : pip(pip), ind(i), ent(p.template pool<std::tuple_element_t<0, std::tuple<ts...>>>()[i]) { }

        template<size_t n> decltype(auto) get() {
            if constexpr (n == 0) 
                return ent; 
            else 
            {
                constexpr bool is_indexed_pool = std::is_same_v<
                    std::remove_const_t<pool<std::tuple_element_t<0, std::tuple<ts...>>>>, 
                    std::remove_const_t<pool<std::tuple_element_t<n - 1, std::tuple<ts...>>>>>;
                
                if constexpr (is_indexed_pool)
                    return pip.template pool<std::tuple_element_t<n - 1, std::tuple<ts...>>>()[ind];
                else 
                    return pip.template pool<std::tuple_element_t<n - 1, std::tuple<ts...>>>()[ent];
            }
        }

    private:
        pip_t& pip;
        const size_t ind;
        const entity ent;
    };
}

