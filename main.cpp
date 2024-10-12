/*
 TODO: ECS emplace/erase/etc funcs
 TODO: view where parameter implementation
 TODO: entity manager
 TODO: versioner
 TODO: dispatcher
 TODO: proper interfaces for resource types -> creating an interface would allow less confusion when creating resources that point to the same type
 TODO: component resource traits & policies: eg default use of particular policy for using grouped etc
 TODO: redo handle manager its quite bad
 TODO: ecs namespace could do with less templates -> I'm not convinced all of them are necessary
 TODO: traits classes -> proper traits class to actually implement the various attributes
 TODO: paged vectors could be setup to have a pointer to the next page at the end

*/

//#include "ecs/pipeline.h"
//#include "ecs/traits.h"
//#include <iostream>


/*
int main()
{

	using namespace ecs;
	
	
	resource::registry<> reg;
	pipeline<res_A, comp_A, entity> pip{ reg };
	{
		std::lock_guard lk(pip);
		entity e = pip.create(); assert(pip.alive(e));
		entity e1 = pip.create(); assert(pip.alive(e1));
		entity e2 = pip.create(); assert(pip.alive(e2));
		entity e3 = pip.create(); assert(pip.alive(e3));
		entity e4 = pip.create(); assert(pip.alive(e4));
		
		pip.destroy(e2); 
		assert(!pip.alive(e2)); 
		
		pip.destroy(e3); 
		assert(!pip.alive(e3));
		
		entity e5 = pip.create(); assert(pip.alive(e5));
		entity e6 = pip.create(); assert(pip.alive(e6));
		assert(e2 != e5); 
		assert(e3 != e6);
		entity e7 = pip.create(); assert(pip.alive(e7));
		pip.emplace<comp_A, policy::stable, policy::immediate>(e);
		pip.emplace<comp_A, policy::stable, policy::immediate>(e);
		
		pip.sync<comp_A>();
	}
}

*/

#include "simplified\ecs.h"
#include <iostream>
#include <cassert>
struct A { 
	using component_handle = ecs::handle<>;
};
struct B { };
struct C { };
struct D { };

static_assert(std::is_same_v<ecs::indexer<A>, ecs::component_indexer<A>>);

int main() {
	using namespace ecs;
	using namespace ecs::event;

	registry<A, B, C, D> reg;
	reg.on<init<A>>() += [](const init<A>& e) { std::cout << "init A" << std::endl; };
	reg.on<terminate<A>>() += [](const terminate<A>& e) { std::cout << "terminate A" << std::endl; };
	reg.on<create<>>() += [](const create<>& e) { std::cout << "create entity" << std::endl; };
	reg.on<destroy<>>() += [](const destroy<>& e) { std::cout << "destroy entity" << std::endl; };

	auto e = reg.create(); 			// create entity
	reg.init<A>(e);					// create component
	assert(reg.has_component<A>(e));
	reg.destroy(e);					// destroy entity
	//for (auto [e, a, b] : reg.view<A, B>(ecs::from<C>{}, ecs::exc<D>{})) { }
}

