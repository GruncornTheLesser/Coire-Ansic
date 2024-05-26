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


#include "ecs/registry.h"
#include "ecs/pipeline.h"
#include "ecs/view.h"
#include "ecs/pool.h"
#include "ecs/util/paged_vector.h"
//#include "ecs/util/sparse_map2.h"
#include <vector>
#include <unordered_map>

#include <iostream>
#include <cassert>

struct A;
struct B;
struct A { int a; };
struct B { int b; };

struct C { };
struct D { };
struct E { };
struct F { };

namespace ecs2 {
	struct at { size_t index; };
	struct back { };
	struct front { };

	struct swap { };
	struct ordered { };
	struct replace { };

	struct deferred { };
	struct immediate { };

	// storage components
	struct grouped { }; // maintain groups when emplacing and erasing
	struct sorted { };  // maintain order when emplacing and erasing, insert into correct location when emplacing
}



/*
namespace ecs2::traits {
	template<typename T> struct is_position_arg : std::disjunction<
		std::is_same<T, back>, 
		std::is_same<T, at>, 
		std::is_same<T, front>> { };

	template<typename T, typename What_T, typename=void> struct is_argument_of : 
		std::is_same<T, What_T> { };
	template<typename T, typename What_T> struct is_argument_of<T, What_T, std::void_t<
		decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> : 
		std::conjunction<
		std::is_same<std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>, What_T>, 
		std::is_same<std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>, What_T>> { }; 
	
	template<typename T, typename args_T>
	struct argument_wrapper {
		argument_wrapper(args_T& args): args(args) { }
		template<typename func_T>
		void for_each(func_T func) { std::for_each(args.begin(), args.end(), func); }
		auto begin() { return args.begin(); }
		auto end() { return args.end(); }
		args_T& args;
	};

	template<typename arg_T>
	struct argument_wrapper<arg_T, arg_T> { 
		argument_wrapper(arg_T& arg): arg(arg) { }
		template<typename func_T>
		void for_each(func_T func) { func(arg); }
		arg_T* begin() { return &arg; }
		arg_T* end() { return ++(&arg); }
		arg_T& arg;
	};


	template<typename T, typename=void> struct is_sequence_arg : std::true_type { };

	template<typename T> struct is_order_arg : std::disjunction<std::is_same<T, ordered>, std::is_same<T, swap>, std::is_same<T, replace>> { };
	
	template<typename T, typename What_T> concept argument_of_class = is_argument_of<T, What_T>::value;
	template<typename T> concept position_arg_class = is_position_arg<T>::value;
	template<typename T> concept sequence_arg_class = is_sequence_arg<T>::value;
	template<typename T> concept order_arg_class = is_order_arg<T>::value;
}

template<typename T,
	ecs2::traits::sequence_arg_class seq_T = ecs2::immediate, 
	ecs2::traits::order_arg_class OrderPolicy_T = ecs2::swap,
	typename Pip_T, typename ... Arg_Ts>
void emplace_at(Pip_T& pip, ecs::pool<T>::const_iterator pos, ecs::entity e, Arg_Ts&& ... args);

template<typename T,
	ecs2::traits::sequence_arg_class seq_T = ecs2::immediate, 
	ecs2::traits::order_arg_class OrderPolicy_T = ecs2::swap,
    
	typename Pip_T, typename ... Arg_Ts>
void emplace_at(Pip_T& pip, ecs::pool<T>::const_iterator pos, it first, it last, Arg_Ts&& ... args)

template<typename T,
	ecs2::traits::sequence_arg_class seq_T = ecs2::immediate, 
	typename Pip_T>
void swap(ecs::entity e1, ecs::entity e2) {

}

template<typename T, 
	ecs2::traits::sequence_arg_class seq_T = ecs2::immediate, 
	ecs2::traits::argument_of_class<ecs::entity> ent_T = ecs::entity, 
	typename Pip_T, typename ... Arg_Ts>
void emplace(Pip_T& pip, ent_T ents, Arg_Ts&& ... args) {
	
}
*/
int main() {
    
	util::sparse_map<size_t> s_map;
    std::unordered_map<size_t, size_t> u_map;

    util::paged_vector<size_t> pv;




	/*
	ecs::registry reg;
	auto pip = reg.pipeline<A, B, C>();
	
	ecs::entity e = 0;
	std::vector<ecs::entity> e_arr;
	emplace_at<A, ecs2::immediate, ecs2::swap>(pip, e, ecs2::back{});
	emplace_at<A>(pip, e, ecs2::at{1}, 0.0f);
	emplace_at<A>(pip, e, ecs2::front{});
	emplace_at<A>(pip, e, ecs2::front{});
	emplace<A>(pip, e);
	emplace<A>(pip, e);
	emplace<A>(pip, e_arr);

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

/*
registry -> stores resources objects
pipeline -> manages resource_set acquire and release
pool -> storage set of resource
event -> has resource_set of required resources
*/
