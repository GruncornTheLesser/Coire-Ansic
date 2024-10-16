/*
 TODO: ECS emplace/erase/etc funcs
 TODO: view where parameter implementation
 TODO: entity manager
 TODO: proper interfaces for resource types -> creating an interface would allow less confusion when creating resources that point to the same type
 TODO: component resource traits & policies: eg default use of particular policy for using grouped etc
 TODO: redo handle manager its quite bad
 TODO: ecs namespace could do with less templates -> I'm not convinced all of them are necessary
 TODO: traits classes -> proper traits class to actually implement the various attributes

*/

// TODO: guard events behind constexpr if has_event<T>() ...
// TODO: view, iteration babee
// TODO: pool, maybe all reg func should call pool func
// TODO: dynamic registry
// TODO: be more specific when calling manager, indexer and storage functions 
// * allows for wider variety of resource setups
// manager->entity_at(size_t i)
// indexer->index_of(entity e)
// storage->component_at(size_t i)
// TODO: use ecs::traits for events, 



#include "simplified\ecs.h"
#include <iostream>
#include <cassert>

struct A { };
struct B { };
struct C { };
struct D { };

static_assert(std::random_access_iterator<ecs::pool_iterator<int, ecs::registry<int>>>);

int main() {
	using namespace ecs;
	using namespace ecs::event;

	registry<A, B, C, D> reg;
	auto log_init = reg.on<init<A>>() ^= [](const init<A>& e) { std::cout << "init A fire once" << std::endl; };
	auto log_terminate = reg.on<terminate<A>>() += [](const terminate<A>& e) { std::cout << "terminate A" << std::endl; };
	auto log_create = reg.on<create<>>().listen([](const create<>& e) { std::cout << "create entity" << std::endl; });
	auto log_destroy = reg.on<destroy<>>() += [](const destroy<>& e) { std::cout << "destroy entity" << std::endl; };

	auto e = reg.create(); 			// create entity
	reg.init<A>(e);					// create component
	assert(reg.has<A>(e));			// assert entity has component
	
	assert(!reg.has<B>(e));			// assert entity does not have component
	reg.init<B>(e);					// create component
	assert(reg.has<B>(e));			// assert entity has component
	reg.terminate<B>(e);			// create component
	assert(!reg.has<B>(e));			// assert entity does not have component
	
	reg.destroy(e);					// destroy entity, also terminate component
	assert(!reg.alive(e));


	std::vector<handle> entities(64);
	for (int i = 0; i < 64; ++i)
		entities[i] = reg.create<handle>();

	for (int i = 0; i < 40; ++i)
		reg.init<A>(entities[i]);
	
	for (int i = 16; i < 48; ++i)
		reg.init<B>(entities[i]);
	
	for (int i = 24; i < 64; ++i)
		reg.init<C>(entities[i]);
	

	
	auto beg = reg.pool<A>().cbegin();
	auto end = reg.pool<A>().cend();
	
	for (auto curr=beg; curr != end; ++curr) {
		std::tuple<ecs::entity<A>, A&> rf = *curr;
		auto [e, a] = *curr;
	}

	std::cout << "---pool<A>---" << std::endl;
	for (auto [e, a] : reg.pool<A>()) { std::cout << e.value() << ","; }
	
	std::cout << "\n---pool<B>---" << std::endl;
	for (auto [e, b] : reg.pool<B>()) { std::cout << e.value() << ","; }
		
	std::cout << "\n---const pool<C>---" << std::endl;
	for (auto [e, b] : reg.pool<C>()) { std::cout << e.value() << ","; }
		
	std::cout << "\n---view<entity, A>---" << std::endl;
	for (auto [e, a] : reg.view<ecs::handle, A>()) { std::cout << e.value() << ","; }
	
	std::cout << "\n---view<entity, B>---" << std::endl;
	for (auto [e, a] : reg.view<ecs::handle, B>()) { std::cout << e.value() << ","; }

	std::cout << "\n---view<entity, C>---" << std::endl;
	for (auto [e, a] : reg.view<ecs::handle, C>()) { std::cout << e.value() << ","; }

	std::cout << "\n---view<entity, A, B>---" << std::endl;
	for (auto [e, a, b] : reg.view<handle, A, B>()) { std::cout << e.value() << ","; }
	
	std::cout << "\n---view<entity, B, C>---" << std::endl;
	for (auto [e, b, c] : reg.view<handle, B, C>()) { std::cout << e.value() << ","; }
}

