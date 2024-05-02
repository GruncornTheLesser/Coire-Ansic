// view must be included first, 
// view includes pipeline and so includes it before beginning, 

/* Design:
class registry: a type erased resource storage map. only one resource of a resource type is stored in the registry. has 2 functions:
	get<resource>() - returns the resource, if not initialized, initializes resource
	erase<resource>() - if a resource is stored in a resource_container, erases the containers and all associated resources

interface resource countainer: a resource is a lockable set of data. it has 2 required functions:
	acquire() - locks mutex/s for access
	release() - unlocks mutex/s for access
	resource_container - the resource_container that stores this resource. by default a resource is it's own container

interface resource_container: a class containing one or more resources. it has 2 required functions:
	get_resource<T>() - where T is a resource contained by the resource storage class
	sync<Ts...>() - where Ts is a subset of the synchronization_set, that are being resynced after an update.
	synchronization_set - a tuple of resources that if sync'd together are passed as types when this resource is sync'd 
		defaults to std::tuple<T>

class pipeline: a class containing a pointers to resource containers, initialized from registry. has 2 functions:
	lock() - acquires all resource, without causing a deadlock
	unlock() - calls sync<Ts...>() on each of resource_containers, releases all resources 

class archetype<Ts...>: a resource_container, stores 2 resources plus 1 resource per component. 

class view: a class for iterating through pools


entity: a unique ID representing a set of components
component: 
pool type-attribute which defines which pool stores this component. defaults to pool archetype<T>
pool: 
*/

// TODO: fuck around with adding serialization options... on pool

// TODO: sort out actual pool implementations, and interface mechanic. 
// a pool could have a function set which is actually a class and associates a resource with an operator()
// I would love to be able to dynamically inherit the functionality them but that's maybe over complicated.
// could be done with a diamond inheritance structure but those are kinda grim and stinky.
// alternatively I could keep it simple and just declare a mirror set of functions in the pipeline.
// this would be repated again in registry. suddenly becomes all a bit grim...

#include "ecs/registry.h"
#include "ecs/pipeline.h"
#include "ecs/view.h"
#include "ecs/pool.h"

struct A { int a; };
struct B { int b; };

//struct A { using pool = ecs::archetype<A, B>; };
//struct B { using pool = ecs::archetype<A, B>; };

struct C { };
struct D { };
struct E { };
struct F { };
//static_assert(ecs::pipeline<ecs::pool<A>>::is_accessible<ecs::pool<A>::entity>, "");
#include "ecs/util/type_name.h"
#include <vector>
#include <iostream>

struct ContainerTest;
struct ResourceTest : ecs::resource { 
	ResourceTest(size_t data) : data(data) { }
	using resource_container = ContainerTest; size_t data; };

struct ContainerTest {
	using resource_set = std::tuple<ResourceTest>;
	using synchronization_set = std::tuple<ResourceTest>;

	template<typename T>
	T& get_resource() { return std::get<T>(set); };
	
	template<typename ... Us>
	void sync() { }

	resource_set set;
};


int main() {
	{
		ecs::registry reg;
		auto& reg_res = reg.get_resource<ResourceTest>(54534ull);
		auto pip = reg.pipeline<ResourceTest>();
		auto& pip_res = pip.get_resource<ResourceTest>();
		
		assert(reg_res.data == 54534);
		assert(pip_res.data == 54534);
	}
	{
		ecs::registry reg;
		auto pip = reg.pipeline<A, B, C>();

		for (uint32_t i = 0; i < 12; ++i) {
			pip.emplace<A, ecs::immediate>(ecs::entity{i}, i); 
			pip.emplace<B, ecs::immediate>(ecs::entity{i}, i);
		}

		auto view1 = reg.view<ecs::entity, A, const B>();
		auto it = view1.rbegin();
		auto end = view1.rend();
		while(it != end) {
			auto [e, a, b] = *it++;
			std::cout << e << ", " << a.a << ", " << b.b << std::endl;
		}
		
		auto view2 = pip.view<ecs::entity, A, const B>();
		for (auto [e, a, b] : view2) {
			std::cout << e << ", " << a.a << ", " << b.b << std::endl;
		}
		
		
	}
}

/*
registry -> stores resources objects
pipeline -> manages resource_set acquire and release
pool -> storage set of resource
event -> has resource_set of required resources
*/



	/*
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