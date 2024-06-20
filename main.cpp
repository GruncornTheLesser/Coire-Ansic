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
/*
 TODO: pool view
 * a pool view would simplify the pool access and mean pipeline<...>().pool<T>() can return a coherent class
 a pool view would basically be a pipeline but for a specific class. it could simplify the synchronization
*/
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

// TODO: NEW PLAN:
//	objects:
//		keep registry
//		keep pipeline
//		keep view
//		change pool resources -> currently pool resources are pool<T>::resource, instead I would prefer:
//			indexer<T> = GET_ATTRIB<T, indexer>;
//			storage<T> = GET_ATTRIB<T, storage>;
//			handler<T> = GET_ATTRIB<T, handler>;
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

// improvements to resource acquisition:
// TODO: more advanced dead lock avoidance

// INFO: dump - https://www.cs.uic.edu/~jbell/CourseNotes/OperatingSystems/7_Deadlocks -> incase site disappears
#pragma region DEADLOCK PREVENTION
// achieve deadlock prevention by preventing at least one of the four conditions:

// Mutual Exclusion - Shared resources such as read-only files. ie pure read only
// 
// Hold and Wait - up front allocation of resources
//    To prevent this condition processes must be prevented from holding one or more resources while simultaneously waiting for one or more others. 
//    Possible Implementation:
//   	- Require that all processes request all resources at one time. This can be wasteful of system resources if a process needs one resource early in 
//       its execution and doesn't need some other resource until much later.
//   	- Require that processes holding resources must release them before requesting new resources, and then re-acquire the released resources along with 
//       the new ones in a single new request. 
//    This can be a problem if a process has partially completed an operation using a resource and then fails to get it re-allocated after releasing it.
//    Either of the methods described above can lead to starvation if a process requires one or more popular resources.

// No Preemption - release before acquire
//    Preemption of process resource allocations can prevent this condition of deadlocks, when it is possible. 
//    Possible Implementation:
//     - if a process is forced to wait, when requesting a new resource, then all other resources previously held by this process are implicitly released, ( preempted ), 
//       forcing this process to re-acquire the old resources along with the new resources in a single request, similar to the previous discussion.
//     - Another approach is that when a resource is requested and not available, then the system looks to see what other processes currently have those resources and
//       are themselves blocked waiting for some other resource. If such a process is found, then some of their resources may get preempted and added to the list of resources 
//       for which the process is waiting.
//    Either of these approaches may be applicable for resources whose states are easily saved and restored, such as registers and memory, but are generally not applicable to other 
//    devices such as printers and tape drives.

// Circular Wait - sorted acquisiton of resources
//    One way to avoid circular wait is to number all resources, and to require that processes request resources only in strictly increasing ( or decreasing ) order.
//    In other words, in order to request resource Rj, a process must first release all Ri such that i >= j.
//    One big challenge in this scheme is determining the relative ordering of the different resources
#pragma endregion
#pragma region SAFE STATE
// A state is safe if the system can allocate all resources requested by all processes (up to their stated maximums) without entering a deadlock state.
// More formally, a state is safe if there exists a safe sequence of processes { P0, P1, P2, ..., PN } such that all of the resource requests for Pi can be granted 
// using the resources currently allocated to Pi and all processes Pj where j < i. (I.e. if all the processes prior to Pi finish and free up their resources, then Pi will be able to finish also, using the resources that they have freed up.)
// If a safe sequence does not exist, then the system is in an unsafe state, which MAY lead to deadlock. (All safe states are deadlock free, but not all unsafe states lead to deadlocks.)
#pragma endregion
#pragma region GRAPH ALGORITHM
// If resource categories have only single instances of their resources, then deadlock states can be detected by cycles in the resource-allocation graphs.
// In this case, unsafe states can be recognized and avoided by augmenting the resource-allocation graph with claim edges, noted by dashed lines, which point from a process to a resource that it may request in the future.
// In order for this technique to work, all claim edges must be added to the graph for any particular process before that process is allowed to request any resources. (Alternatively, processes may only make requests for 
// resources for which they have already established claim edges, and claim edges cannot be added to any process that is currently holding resources.)
// When a process makes a request, the claim edge Pi->Rj is converted to a request edge. Similarly when a resource is released, the assignment reverts back to a claim edge.
// This approach works by denying requests that would produce cycles in the resource-allocation graph, taking claim edges into effect.
#pragma endregion

// resource defines: 
// resource type - the type when acquiring this resource
// resource alias - the key type used when acquiring this resource
// resource set - the collection of additional resources

// process/pipeline define:
// max_allocations - 
// need_allocation

// TODO: pipeline funcs
// TODO: view where parameter implementation
// TODO: entity manager
// TODO: versioner 
// TODO: dispatcher
// TODO: 

#include "ecs/registry.h"
#include "ecs/pipeline.h"
#include "ecs/view.h"
#include "ecs/util/paged_vector.h"
#include <iostream>
#include <cassert>
#include "ecs/handler.h"

template<typename ... Ts> 
struct archetype_element
{ 

	std::tuple<Ts...> data;

	template<typename ... Us> 
	archetype_element(Us&& ... component) { } // FIXME: archetype constructor and also just archetype and the rest

	template<typename U> operator U&() { return std::get<U>(data); }
	template<typename U> operator const U&() const { return std::get<U>(data); }
};

template<typename ... Ts> using archetype = util::sort_t<archetype_element<Ts...>, util::cmp::less_<util::get_type_ID>::type>;

struct A { int a; };
struct B;
struct C;
struct B { using component_type = archetype<B, C>; int b; };
struct C { using component_type = archetype<C, B>; int c; };

static_assert(std::is_same_v<B::component_type, C::component_type>, "");

struct D { int d; };
struct E { int e; };
struct F { int f; };

struct res_A;
struct res_B;
struct res_C;
struct res_D;

struct res_A { using resource_set = std::tuple<res_A, res_B, res_C, res_D>; };
// res_A is requesting itself -> causes get_resource_set to break
// res_A::resource_set = tuple<res_A, ...>
// res_A::resource_set
struct res_B { using resource_type = res_A; };
struct res_C { using resource_alias = res_A; };
struct res_D { using resource_alias = res_B; };
struct res_E { };
struct res_F { using resource_alias = res_E; };
struct res_G { using resource_set = std::tuple<res_E>; };
struct res_H { using resource_set = std::tuple<res_A, res_G>; };

static_assert(std::is_same_v<ecs::traits::get_resource_alias_t<res_A>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_type_t<res_A>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_alias_t<res_B>, res_B>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_type_t<res_B>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_alias_t<res_C>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_type_t<res_C>, res_A>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_alias_t<res_D>, res_B>, "");
static_assert(std::is_same_v<ecs::traits::get_resource_type_t<res_D>, res_A>, "");


int main() {
	ecs::registry reg;
	{
		auto pip = reg.pipeline<const ecs::storage<B>, const ecs::handler<B>>();
		for (auto [e, b] : pip.view<ecs::entity, const B>()) { }
		for (const B& b : pip.view<const B>()) { }

		//required from 'class ecs::view_reference<
		//	ecs::select<ecs::entity, const B>, 
		//	ecs::from<const ecs::handler_t<archetype_element<B, C> > >, 
		// ecs::pipeline_t<const ecs::handler_t<archetype_element<B, C> >, const ecs::storage_t<archetype_element<B, C> > >&>'
	}
		
		//{
		//	std::cout << util::type_name<decltype(e)>() << " e = " << e << ";" << std::endl;
		//	std::cout << util::type_name<decltype(b)>() << " b = " << b.b << ";" << std::endl;
		//}

	{
//		auto pip = reg.pipeline<ecs::handler<A>>();
//		for (auto [e] : pip.view<ecs::entity>(ecs::from<A>{})) { }
//			std::cout << util::type_name<decltype(e)>() << " e" << "=" << e << std::endl;
	}

	{
//		auto pip = reg.pipeline<ecs::handler<A>, ecs::indexer<C>, ecs::storage<C>>();
//		for (auto [c] : pip.view<C>(ecs::from<A>{})) { }
	}

	{	
//		auto pip = reg.pipeline<ecs::storage<C>>();
//		using v = ecs::traits::get_resource_set_t<std::tuple<ecs::select<C>, ecs::from<void>, ecs::where<>>>;
		//std::cout << util::type_name<v>();
//		for (auto [c] : pip.view<C>(ecs::from<void>{}, ecs::where<>{})) { } // this will acquire handler -> it doesnt necessarily need to
	}


	
	//pip.template emplace_back<A>(e);
	//pip.template erase<A>(e);
	
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

