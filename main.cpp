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

// ! THERE IS UGLY CONST CASTING AROUND REGISTRY, POOL, POOL_ITERATOR



// TODO: make dispatcher use handle<>

#include "inc\registry.h"
#include "inc\util\type_name.h"
#include <iostream>
#include <cassert>

struct A { using component_tag = ecs::tag::component::basictype; };
struct B { using component_tag = ecs::tag::component::basictype; };
struct C { using component_tag = ecs::tag::component::basictype; };
struct D { using component_tag = ecs::tag::component::basictype; };

static_assert(ecs::traits::is_event_v<ecs::init<A>>);
static_assert(ecs::traits::is_resource_v<ecs::invoker_t<ecs::init<A>>>);

auto log_init_func = [](const ecs::init<A>& e) { std::cout << "init A, "; };
auto log_terminate_func = [](const ecs::terminate<A>& e) { std::cout << "terminate A, "; };
auto log_create_func = [](const ecs::create<ecs::entity>& e) { std::cout << "create e, "; };
auto log_destroy_func = [](const ecs::destroy<ecs::entity>& e) { std::cout << "destroy e, "; };

int main() {
	using namespace ecs;
	
	registry<A, B, C, D> reg;

	invoker_t<init<A>>& inv = reg.on<init<A>>();

	auto log_init = reg.on<init<A>>() ^= log_init_func;
	auto log_terminate = reg.on<terminate<A>>() += log_terminate_func;
	auto log_create = reg.on<create<entity>>().listen(log_create_func, false);
	auto log_destroy = reg.on<destroy<entity>>() += log_destroy_func;
	
	std::vector<ecs::entity> ents;
	for (int i = 0; i < 32; ++i) ents.push_back(reg.create());
	
	entity e1 = reg.create(); 
	reg.destroy(e1);
	assert(!reg.alive(e1));

	for (int i = 0; i < 0xfff; ++i) reg.destroy(reg.create());
	entity e2 = reg.create();
	
	assert(e1 == e2); // versions wrap around after 0xfff 65k iterations
	assert(reg.alive(e1)); 
	assert(reg.alive(e2));
	
	auto e = reg.create(); 			// create entity
	assert(!reg.has<A>(e));

	reg.init<A>(e);					// create component
	assert(reg.has<A>(e));

	reg.terminate<A>(e);			// create component
	assert(!reg.has<A>(e));
	
	reg.destroy(e);					// destroy entity, also terminate component
	assert(!reg.alive(e));
	
	for (auto [e, d] : reg.pool<D>()) {
		static_assert(std::is_same_v<decltype(e), entity>);
		static_assert(std::is_same_v<decltype(d), D&>);
	}
	for (auto [e, d] : reg.pool<const D>()) {
		static_assert(std::is_same_v<decltype(e), const entity>);
		static_assert(std::is_same_v<decltype(d), const D&>);
	}

	for (auto [e, d] : reg.view<entity, D>()) {
		static_assert(std::is_same_v<decltype(e), entity>);
		static_assert(std::is_same_v<decltype(d), D&>);
	}
	for (auto [e, d] : reg.view<entity, const D>()) {
		static_assert(std::is_same_v<decltype(e), entity>);
		static_assert(std::is_same_v<decltype(d), const D&>);
	}
	
	std::vector<entity> entities(64);
	for (int i = 0; i < 64; ++i)
		entities[i] = reg.create<entity>();

	for (int i = 0; i < 40; ++i)
		reg.init<A>(entities[i]);
	
	for (int i = 16; i < 48; ++i)
		reg.init<B>(entities[i]);
	
	for (int i = 24; i < 64; ++i)
		reg.init<C>(entities[i]);
	

	std::cout << "---pool<A>---" << std::endl;
	for (auto [e, a] : reg.pool<A>()) { 
		std::cout << "entity " << ecs::handle_traits<entity>::get_index(e) << ",";
	}
	
	std::cout << "---pool<A>---" << std::endl;
	for (auto [e, a] : reg.pool<const A>());
	
	std::cout << "\n---pool<B>---" << std::endl;
	for (auto [e, b] : reg.pool<B>()) { std::cout << handle_traits<entity>::get_index(e) << ","; }
		
	std::cout << "\n---const pool<C>---" << std::endl;
	for (auto [e, b] : reg.pool<C>()) { std::cout << handle_traits<entity>::get_index(e) << ","; }
		
	std::cout << "\n---view<entity, A>---" << std::endl;
	for (auto [e, a] : reg.view<ecs::entity, A>()) { std::cout << handle_traits<entity>::get_index(e) << ","; }
	
	std::cout << "\n---view<entity, B>---" << std::endl;
	for (auto [e, a] : reg.view<ecs::entity, B>()) { std::cout << handle_traits<entity>::get_index(e) << ","; }

	std::cout << "\n---view<entity, C>---" << std::endl;
	for (auto [e, a] : reg.view<ecs::entity, C>()) { std::cout << handle_traits<entity>::get_index(e) << ","; }

	std::cout << "\n---view<entity, A, B>---" << std::endl;
	for (auto [e, a, b] : reg.view<entity, A, B>()) { std::cout << handle_traits<entity>::get_index(e) << ","; }
	
	std::cout << "\n---view<entity, B, C>---" << std::endl;
	for (auto [e, b, c] : reg.view<entity, B, C>()) { std::cout << handle_traits<entity>::get_index(e) << ","; }
	std::cout << std::endl;

}
