#include <iostream>
#include <span>
#include <tuple>
#include <type_traits>

#include "ecs/fwd.h"

#include "ecs/registry.h"
#include "ecs/pipeline.h"
#include "ecs/pool.h"
#include "ecs/view.h"
#include "ecs/element.h"

//#include "ecs/registry.h"
//TODO:
//	HANDLE MANAGER!!!
//  add some asserts/error checking
//		adding an entity that pool already contains
//		removing an entity that pool doesnt contain
// 		static validation on view condition erase contradictory statements
//	expand view condition to allow "any_of" -> maybe update language
//		DO NOT FALL DOWN RABBIT HOLE OF CONDITION EVALUALTION OPTIMISATION
//		limit the scope to something more achievable 

struct A { char a; };
struct B { };

struct C;
struct D;

struct C { char c[32]; using container = ecs::container::uniontype<C, D>; };
struct D { char d[32]; using container = ecs::container::uniontype<C, D>; };

struct F;
struct G;

struct F { char c[32]; using container = ecs::container::archetype<F, G>; };
struct G { char c[32]; using container = ecs::container::archetype<F, G>; };

int main() {
    ecs::registry reg;

    auto  pip = reg.pipeline<A, B, C, F, G>();
    {
        { auto& [e, a] = *pip.pool<const A>().begin(); }
        { auto& [e, cd] = *pip.pool<const C>().begin(); }
        { auto& [e, f, g] = *pip.pool<const F>().begin(); }

        { auto& [e, a] = *pip.pool<A>().begin(); }
        { auto& [e, cd] = *pip.pool<C>().begin(); }
        { auto& [e, f, g] = *pip.pool<F>().begin(); }

        { auto [e, f, c] = *pip.view<F, C>().begin(); }

        { ecs::entity e7 = *pip.pool<A>().begin(); }
        { ecs::entity e8 = *pip.pool<C>().begin(); }
        { ecs::entity e9 = *pip.pool<F>().begin(); }
        { ecs::entity e7 = *pip.pool<const A>().begin(); }
        { ecs::entity e8 = *pip.pool<const C>().begin(); }
        { ecs::entity e9 = *pip.pool<const F>().begin(); }
        
        { A& e7 = *pip.pool<A>().begin(); }
        { C& e8 = *pip.pool<C>().begin(); }
        { F& e9 = *pip.pool<F>().begin(); }
        { const A& e7 = *pip.pool<const A>().begin(); }
        { const C& e8 = *pip.pool<const C>().begin(); }
        { const F& e9 = *pip.pool<const F>().begin(); }



        
    }
}

