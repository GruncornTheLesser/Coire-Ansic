#include "ecs/registry.h"
#include "ecs/pipeline.h"
#include "ecs/pool.h"
#include "ecs/view.h"
#include "ecs/pool_policy.h"

#include <iostream>

#define SET_COMPONENT_ID public: static constexpr int component_id = __COUNTER__; 

struct A { SET_COMPONENT_ID size_t a; A() {} A(A& other) { std::cout << "I've been copied!!!\n"; } A& operator=(const A& other) { std::cout << "I've been copied!!!\n"; return *this; }};
struct B { SET_COMPONENT_ID };
struct C { SET_COMPONENT_ID };
struct D { SET_COMPONENT_ID };

int main() {
	using namespace ecs;

	registry reg;

	pipeline<A, const B> pip { &reg };
	
	reg.pool<B>().emplace_back(0);
	

	{
		std::lock_guard guard(pip);

		const pool<A>& const_pl = pip.pool<const A>();
		pool<A>&             pl = pip.pool<A>();

		pip.pool<A>().emplace_back(0);
		pip.pool<A>().emplace_back(1);

		for (auto [e, a] : pip.pool<const A>()) { std::cout << e << "\n"; }
		for (auto [e, a] : pip.pool<A>())       { std::cout << e << "\n"; }
		
		for (auto [e, b] : pip.view<const B>()) { std::cout << e << "\n"; }
		for (auto [e, a] : pip.view<A>(exc<B>{})) { std::cout << e << "\n"; }

	}

	std::cout << "end...\n";
}

