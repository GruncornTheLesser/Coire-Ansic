#include "ecs/fwd.h"
#include "ecs/registry.h"
#include "ecs/pool.h"
#include "ecs/pipeline.h"
#include "ecs/pool_policy.h"
#include "ecs/iterator.h"
#include <iostream>

#define SET_COMPONENT_ID public: static constexpr int component_id = __COUNTER__; 

struct A { SET_COMPONENT_ID size_t a; A() {} A(A& other) { std::cout << "I've been copied!!!\n"; } A& operator=(const A& other) { std::cout << "I've been copied!!!\n"; return *this; }};
struct B { SET_COMPONENT_ID };
struct C { SET_COMPONENT_ID };
struct D { SET_COMPONENT_ID };

int main() {
	using namespace ecs;

	ecs::registry reg;
	
	auto pip = reg.pipeline<const B, A>();
	
	{
		std::lock_guard guard(pip);
		auto& const_pl = pip.pool<const A>(); 	// no error
		//auto& pl2 = pip.pool<const B>(); 		// no error
		auto& pl = pip.pool<A>(); 				// no error
		//pip.pool<B>(); 						// error

		pl.emplace_back(0);
		pl.emplace_back(1);

		for (auto [e, a] : const_pl) { std::cout << e << "\n"; }
		for (auto [e, a] : pl)       { std::cout << e << "\n"; }
		
	}
}

