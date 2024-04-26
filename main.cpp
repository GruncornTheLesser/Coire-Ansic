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

struct A;
struct B;

//struct A { using pool = ecs::archetype<A, B>; };
//struct B { using pool = ecs::archetype<A, B>; };

struct C { };
struct D { };
struct E { };

//static_assert(ecs::pipeline<ecs::pool<A>>::is_accessible<ecs::pool<A>::entity>, "");
#include "ecs/util/type_name.h"
#include <vector>
#include <iostream>
template<typename T> struct get_resource_set { using type = std::tuple<int, const float>; };
template<typename T> struct is_resource : std::is_const<T> { };
template<typename T> struct get_pool { using type = std::array<T, 1>; };

int main() {
	ecs::registry reg;
	std::cout << util::type_name<ecs::pipeline_builder<const ecs::pool<int>, float>::type>() << std::endl;
	/*
	{
		auto pip = reg.pipeline<ecs::pool<int>, ecs::pool<float>, ecs::pool<char>>();
		std::lock_guard guard(pip);

		auto e = ecs::entity{0ull};
		pip.emplace<int, ecs::immediate>(e, 1);
		pip.emplace<float, ecs::immediate>(e, 1.0f);
		pip.emplace<char, ecs::immediate>(e, '1');
	}
	
	{
		using view_pip = ecs::pipeline<ecs::pool<int>, ecs::pool<float>, ecs::pool<char>>;
		using view_it = ecs::view_iterator<ecs::select<ecs::entity, int, float, char>, ecs::from<float>, 
			ecs::where<ecs::include<int, float, char>>, view_pip>;
		using view_ref = ecs::view_reference<ecs::select<int, float, char>, ecs::from<int>, view_pip>;

		auto pip = reg.pipeline<ecs::pool<int>, ecs::pool<float>, ecs::pool<char>>();
		std::lock_guard guard(pip);
		//auto [i, f, c] = view_ref{ pip, 0ull };

		//std::cout << i << f << c << std::endl;
	}
	
	auto pip = reg.pipeline<const ecs::pool<A>>();
	{
		std::lock_guard guard(pip);
		const ecs::pool<A>::entity& pl_e = pip.get_resource<const ecs::pool<A>::entity>();
	}

	//auto view = reg.view<ecs::entity, A, B>(ecs::from<C>{}, ecs::where<ecs::include<A, B>>{});
	//auto& [e, a, b] = *view.begin();
	*/
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