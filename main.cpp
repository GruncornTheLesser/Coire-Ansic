/* Design:
class registry: a type erased resource storage map. only one resource of a resource type is stored in the registry. has 2 functions:
	get_resource<T>() - returns the resource, if not initialized, initializes resource
	erase_resource<T>() - if a resource is stored in a resource_container, erases the containers and all associated resources
	view() - creates a view

interface resource: a resource is a lockable set of data. it has 2 required functions:
	acquire() - locks mutex/s for access
	release() - unlocks mutex/s for access
	resource_container[optional] - the resource_container that stores this resource. by default a resource is it's own container

storage: the resources owned by archetype

interface resource_container: a class containing one or more resources. it has 2 required functions:
	get_resource<T>() - where T is a resource contained by the resource storage class
	sync<Ts...>() - where Ts is a subset of the synchronization_set, that are being resynced after an update.
	synchronization_set - a tuple of resources that if sync'd together are passed as types when this resource is sync'd 
		defaults to std::tuple<T>

class pipeline: a class containing a pointers to resource containers, initialized from registry. 
	collects resources from arguments's resource_set. the pipeline is the main point of contact 
	has 2 mutex functions:
	lock() - acquires all resource, ensures no deadlocks
	unlock() - calls sync<Ts...>() on each of resource_containers, releases all resources 
	emplace
	erase
	view - creates a view from the pip

class view: a class for iterating through pools

class archetype<Ts...>: a component pool, stores 2 resources plus 1 resource per component. 


entity: a unique ID representing a set of components
component: 
pool type-attribute which defines which pool stores this component. defaults to pool archetype<T>
pool: 
*/

// TODO: pool view
// a pool view would simplify the pool access and mean pipeline<...>().pool<T>() can return a coherent class
// a pool view would basically be a pipeline but for a specific class. it could simplify the synchronization

// TODO: sub-pipeline - I need a way to request further resources for synchronisation etc. This breaks the 
// system for avoiding deadlocks. currently all resources are acquired in a standardised order in the pipeline. 
// if I want to acquire further resources some of those might be resources that should've been acquired first 
// before the first. 
// add parent type to pipeline, eg ecs::pipeline<ecs::registry, Ts...>, ecs::pipeline<ecs::pipeline<T, Ts...>, Us...>
// allows specific locking of only associated components
// doesnt solve problem of 
//		ecs::pipeline<ecs::registry, A, B, C> // acquires A, B, C
//		ecs::pipeline<ecs::pipeline<ecs::registry, C>, A, B> // acquires C, tries to acquire A, B
// dont allow acquiring resources out of scope, rely on lock_priority trait. problem - traits currently doesnt allow specify per resource

// TODO: traits classes
//		ecs::resource_traits<T>
//			lock_priority = get_lock_priority_v<T>;
//			resource_set = get_resource_set_t<T>;
//			resource_container = get_resource_container_t<T>;
//			
//		ecs::component_traits<T>
//			lock_priority
//			pool
//			index_storage
//			entity_storage
//			component_storage

// TODO: resource_alias type - resource_alias would change the type stored in the registry, but not its ID, 
// This could increase template compile time as less instances of templates would exist, eg pool<T>::entity -> entity_array
// TODO: further: could change pool<T> to be a alias type for ecs::archetype<T...>
// pool<T> is empty type with navigation types to entity, index, component storage which are also all aliases
// that would simplify the call from ecs::pool<T>::comp<T> to ecs::pool<T>::comp
// a bit simpler
// might complicate references to resource, would become resource_alias_t<T> which would often be wrapped in a get_resource_set_t
// would have to work out what to do with resource_set, resource_set<alias_T>, 
// might be a bit silly for simply referencing a member of class


// NEW PLAN:
//	objects:
//		keep registry
//		keep pipeline
//		keep view
//		change pool resources -> currently pool resources are pool<T>::resource, instead I would prefer:
//			indexer<T> = get_attrib<T, indexer>;
//			storage<T> = get_attrib<T, storage>;
//			handler<T> = get_attrib<T, handler>;
//			dispatcher<T> = get_attrib<T, dispatcher>;
//		add pool_view -> a pool view would be a subset of a pipeline without the resource acquisition
//						 would be encoded with the resource_set and allow access to pool functions
//						 I want this because all naming conventions are subject to a collection. 
//						 by separating the pool into resoufces I have lost that understanding
//	attributes:
//		keep resource_set
//		remove resource container -> provides no caching improvement given that its all stored in registry anyway
//		remove synchronisation_set -> This was intended to work so when a resource is release it acquires the set of these resources to update itself
//		add resource_alias -> changes the type stored for the given key, defaults to pool
//		add resource_shared_set -> when this resource is registered -> shared set adds 
//		add resource_syncer -> a tuple of on synchronization functors, replacing the sync func on the pipeline container
//		
//		



#include "ecs/registry.h"
#include "ecs/pipeline.h"
#include "ecs/view.h"
#include "ecs/pool.h"
#include "ecs/util/paged_vector.h"
#include <iostream>
#include <cassert>

struct A { int a; };
struct B { int b; };
struct C { };
struct D { };
struct E { };
struct F { };
/*
namespace ecs2 {
	struct swap { };
	struct ordered { };
	struct replace { };

	struct deferred { };
	struct immediate { };

	// storage components
	struct grouped { }; // maintain groups when emplacing and erasing
	struct sorted { };  // maintain order when emplacing and erasing, insert into correct location when emplacing
}
*/
struct res_A;
struct res_C;
struct res_D;

struct res_A { };
struct res_B { using resource_type = res_A; };
struct res_C { using resource_alias = res_B; };
struct res_D { using resource_alias = res_A; };

static_assert(std::is_same_v<ecs::traits::get_resource_alias_t<res_A>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_type_t<res_A>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_alias_t<res_B>, res_B>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_type_t<res_B>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_alias_t<res_C>, res_B>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_type_t<res_C>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_alias_t<res_D>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_type_t<res_D>, res_A>, "");

int main() {
	ecs::registry reg;
	auto x = ecs::traits::is_resource_key<ecs::handler<A>>::value;
	std::cout << util::type_name<ecs::pipeline_builder<A, B, C>::type>() << std::endl;
	auto pip = reg.pipeline<A, B, C>();
	
	res_A* r1 = &reg.get_resource<res_A>(); // default behaviour, unique resource of type T
	res_A* r2 = &reg.get_resource<res_B>(); // resource_type=res_A, unique resource of type resource_type<T> 
	res_A* r3 = &reg.get_resource<res_C>(); // resource_alias=res_B, shares resource with r2
	res_A* r4 = &reg.get_resource<res_D>(); // resource_alias=res_A, shares resource with r1
	
	assert(r2 == r3); // resource_alias
	assert(r1 == r4);

	ecs::entity e;
	pip.emplace_back<A>(e);
	pip.erase<A>(e);
	
	/*

	

	for (uint32_t i = 0; i < 12; ++i) {
		pip.emplace<A, ecs::immediate>(ecs::entity{i}, i);
		pip.emplace<B, ecs::immediate>(ecs::entity{i}, i);
	}

	auto view1 = reg.view<ecs::entity, A, const B>();
	for (auto it = view1.rbegin(); it != view1.rend(); ++it) 
	{
		auto [e, a, b] = *it;
		std::cout << e << ", " << a.a << ", " << b.b << std::endl;
	}

	auto view2 = pip.view<ecs::entity, A, const B>();
	for (auto [e, a, b] : view2) {
		std::cout << e << ", " << a.a << ", " << b.b << std::endl;
	}
	*/
}

