#include "ecs/registry.h"
#include "ecs/pool.h"
#include <iostream>

struct A;
struct B;
struct C;

struct A { 
	using pool_tag = ecs::archetype<A, B>;
	char a; 
};
struct B { 
	using pool_tag = ecs::archetype<A, B>;
	char b; 
};
struct C { 
	char c; 
};

int main() {
	using namespace ecs;

	registry reg;
	
	using pip_t = pipeline<pool<A>>;
	auto pip = reg.pipeline<pool<A>>();
	{
		pip.lock();
		pool<A>::index& pl_i = pip.get_resource<pool<A>::index>();
		
		// synchronization???
		//		index always matches component, array
		// 		const
		//		entity does not always match index
		//			loop through entity by index, get prev index of entity, swap components, update index

		//		can be acquired seperately from component or index for gets and contains operations
		// 
		
		pip.unlock();
	}


	//using namespace ecs;
	//registry reg;
	//pool<A>& pA = reg.pool<A>();
	//auto pl_v = pA.view<A>();

	//pipeline<select<A, B>, from<A>> pip = reg.pipeline<A, B>();
	
	//auto pip_v = pip.view<entity, A, B>();

/*
	auto  pip = reg.pipeline<A, B, C, F, G>();
	{
		{ auto& [e, a] = *pip.pool<const A>().begin(); }
		{ auto& [e, cd] = *pip.pool<const C>().begin(); }
		{ auto& [e, f, g] = *pip.pool<const F>().begin(); }

		{ auto& [e, a] = *pip.pool<A>().begin(); }
		{ auto& [e, cd] = *pip.pool<C>().begin(); }
		{ auto& [e, f, g] = *pip.pool<F>().begin(); }

		{ auto [e, f, c] = *pip.view<F, C>().begin(); }

		{ ecs::entity e = *pip.pool<A>().begin(); }
		{ ecs::entity e = *pip.pool<C>().begin(); }
		{ ecs::entity e = *pip.pool<F>().begin(); }
		{ ecs::entity e = *pip.pool<const A>().begin(); }
		{ ecs::entity e = *pip.pool<const C>().begin(); }
		{ ecs::entity e = *pip.pool<const F>().begin(); }

		{ A& a = *pip.pool<A>().begin(); }
		{ C& c = *pip.pool<C>().begin(); }
		{ F& f = *pip.pool<F>().begin(); }
		{ const A& a = *pip.pool<const A>().begin(); }
		{ const C& c = *pip.pool<const C>().begin(); }
		{ const F& f = *pip.pool<const F>().begin(); }
	}
*/
	return 0;
}
