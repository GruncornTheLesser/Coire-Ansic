// TODO: guard events behind constexpr if enable_events
// TODO: dynamic registry
// TODO: use any type as a resource and not require inheritance
// ! THERE IS UGLY CONST CASTING AROUND REGISTRY, POOL, POOL_ITERATOR

#include "ecs\registry.h"
#include "ecs\util\type_name.h"
#include <iostream>
#include <cassert>

using namespace ecs;

struct res_A : ecs::restricted_resource { };
struct res_B : ecs::restricted_resource { };
struct res_C : ecs::restricted_resource { };
struct res_D : ecs::restricted_resource { };


struct A { 
	using handle_type = ecs::entity;
	using manager_type = ecs::manager<A>;
	using indexer_type = ecs::indexer<A>;
	using storage_type = ecs::storage<A>;
	static constexpr bool enable_events = true;
};
struct B { using ecs_tag = tag::component; };
struct C { using ecs_tag = tag::component; };
struct D { using ecs_tag = tag::component; };


auto log_init_func = [](const init<A>& ev)            { std::cout << "init<A>{ "         << handle_traits<entity>::get_data(ev.handle) << " }\n"; };
auto log_terminate_func = [](const terminate<A>& ev)  { std::cout << "terminate<A>{ "    << handle_traits<entity>::get_data(ev.handle) << " }\n"; };
auto log_create_func = [](const create<entity>& ev)   { std::cout << "create<entity>{ "  << handle_traits<entity>::get_data(ev.handle) << " }\n"; };
auto log_destroy_func = [](const destroy<entity>& ev) { std::cout << "destroy<entity>{ " << handle_traits<entity>::get_data(ev.handle) << " }\n"; };

using reg_t = registry<A, B, C, D, res_A, res_B, res_C, res_D>;

int main() {
	util::apply<std::tuple<A, B, C, D>>([&]<typename T>(){ std::cout << util::type_name<T>() << std::endl; });

	reg_t reg;
	ecs::manager<A>{}.acquire(ecs::priority::HIGH);
	
	static_assert(ecs::traits::is_contract_v<A>);
	static_assert(ecs::traits::is_contract_v<res_A>);

	static_assert(ecs::traits::is_resource_v<ecs::manager<A>>);
	static_assert(ecs::traits::is_resource_v<ecs::indexer<A>>);
	static_assert(ecs::traits::is_resource_v<ecs::invoker<init<A>>>);
	static_assert(ecs::traits::is_resource_v<ecs::generator<entity>>);
	// pool and view range concepts
	static_assert(std::random_access_iterator<pool_iterator<A, reg_t>>);
	static_assert(std::ranges::random_access_range<pool<A, reg_t>>);
	static_assert(std::bidirectional_iterator<view_iterator<select<A>, from<A>, include<A>, reg_t>>);
	static_assert(std::ranges::bidirectional_range<view<select<A>, from<A>, include<A>, reg_t>>);
	// component access
	static_assert(std::is_same_v<decltype(reg.get<get_manager_t<A>>()), get_manager_t<A>&>);
	static_assert(std::is_same_v<decltype(std::as_const(reg).get<get_manager_t<A>>()), const get_manager_t<A>&>);

	reg.on<init<A>>() += log_init_func;
	reg.on<terminate<A>>() += log_terminate_func;
	
	// fire once
	reg.on<create<entity>>().listen(log_create_func, true);
	reg.on<destroy<entity>>() ^= log_destroy_func;
	
	std::vector<entity> ents;
	for (int i = 0; i < 32; ++i) ents.push_back(reg.create());

	entity e1 = reg.create(); 
	reg.destroy(e1);

	assert(!reg.alive(e1));

	for (int i = 0; i < 0xfff; ++i) reg.destroy(reg.create());
	entity e2 = reg.create();

	assert(reg.alive(e1)); // e1 has been resurrected for 
	assert(e1 == e2); // versions wrap around after 0xfff 4k iterations


	assert(reg.alive(e1)); 
	assert(reg.alive(e2));
	
	auto e = reg.create(); 			// create entity
	assert(!reg.has<A>(e));

	reg.init<A>(e);					// create component
	assert(reg.has<A>(e));

	reg.terminate<A>(e);			// destroy component
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
	for (int i = 0; i < 64; ++i) entities[i] = reg.create<entity>();
	for (int i = 0; i < 40; ++i) reg.init<A>(entities[i]);
	for (int i = 16; i < 48; ++i) reg.init<B>(entities[i]);
	for (int i = 24; i < 64; ++i) reg.init<C>(entities[i]);

	std::cout << "\n---pool<A>---" << std::endl;
	for (auto [e, a] : reg.pool<A>()) { std::cout << handle_traits<entity>::get_data(e) << ","; }
	
	std::cout << "\n---pool<A>---" << std::endl;
	for (auto [e, a] : reg.pool<const A>()) { std::cout << handle_traits<entity>::get_data(e) << ", "; }
	
	std::cout << "\n---pool<B>---" << std::endl;
	for (auto [e, b] : reg.pool<B>()) { std::cout << handle_traits<entity>::get_data(e) << ","; }
		
	std::cout << "\n---const pool<C>---" << std::endl;
	for (auto [e, b] : reg.pool<C>()) { std::cout << handle_traits<entity>::get_data(e) << ","; }
		
	std::cout << "\n---view<entity, A>---" << std::endl;
	for (auto [e, a] : reg.view<entity, A>()) { std::cout << handle_traits<entity>::get_data(e) << ","; }
	
	std::cout << "\n---view<entity, B>---" << std::endl;
	for (auto [e, a] : reg.view<entity, B>()) { std::cout << handle_traits<entity>::get_data(e) << ","; }

	std::cout << "\n---view<entity, C>---" << std::endl;
	for (auto [e, a] : reg.view<entity, C>()) { std::cout << handle_traits<entity>::get_data(e) << ","; }

	std::cout << "\n---view<entity, A, B>---" << std::endl;
	for (auto [e, a, b] : reg.view<entity, A, B>()) { std::cout << handle_traits<entity>::get_data(e) << ","; }
	
	std::cout << "\n---view<entity, B, C>---" << std::endl;
	for (auto [e, b, c] : reg.view<entity, B, C>()) { std::cout << handle_traits<entity>::get_data(e) << ","; }
	
	std::cout << std::endl;
}
