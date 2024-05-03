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

#include "ecs/registry.h"
#include "ecs/pipeline.h"
#include "ecs/view.h"
#include "ecs/pool.h"

struct A;
struct B;
struct A { int a; using pool = ecs::archetype<A, B>; };
struct B { int b; using pool = ecs::archetype<A, B>; };

struct C { };
struct D { };
struct E { };
struct F { };
#include "ecs/util/type_name.h"
#include <vector>
#include <iostream>

namespace ecs2 {
	struct at{ 
		template<typename T> using resource_set = std::tuple<ecs::traits::index_storage_t<T>>; 
		size_t index; 
	};
	struct back {
		template<typename T> using resource_set = std::tuple<>;
	};
	struct front { 
		template<typename T> using resource_set = std::tuple<>;
	};

	struct deferred {
		template<typename T> using resource_set = std::tuple<ecs::pool<T>>;

		template<typename Pip_T, typename T, typename Ent_T, typename Pos_T, typename ... Arg_Ts>
		auto emplace(Pip_T pip, Ent_T ent, Pos_T pos, Arg_Ts&& ... args);

		template<typename Pip_T, typename T, typename Ent_T, typename ... Arg_Ts>
		auto erase(Pip_T pip, Ent_T ent, Arg_Ts&& ... args);
	};
	struct immediate {
		template<typename T> using resource_set = std::tuple<ecs::pool<T>>;

		template<typename Pip_T, typename T, typename Ent_T, typename Pos_T, typename ... Arg_Ts>
		auto emplace(Pip_T pip, Ent_T ent, Pos_T pos, Arg_Ts&& ... args);

		template<typename Pip_T, typename T, typename Ent_T, typename ... Arg_Ts>
		auto erase(Pip_T pip, Ent_T ent, Arg_Ts&& ... args);
	};
}

namespace ecs2::traits {
	template<typename T> struct is_position_arg : std::disjunction<std::is_same<T, back>, std::is_same<T, at>, std::is_same<T, front>> { };

	template<typename T, typename What_T, typename=void> struct is_input_arg : std::is_same<T, What_T> { };
	template<typename T, typename What_T> struct is_input_arg<T, What_T, std::void_t<
		decltype(std::begin(std::declval<T&>())), decltype(std::end(std::declval<T&>()))>> : 
	std::conjunction<
		std::is_same<std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>, What_T>, 
		std::is_same<std::remove_cvref_t<decltype(*std::begin(std::declval<T>()))>, What_T>> { }; 
	
	template<typename T, typename=void> struct is_exection_arg : std::true_type { };

	template<typename T, typename What_T> concept input_arg_class = is_input_arg<T, What_T>::value;
	template<typename T> concept position_arg_class = is_position_arg<T>::value;
	template<typename T> concept exection_arg_class = is_exection_arg<T>::value;
}

template<typename T, 
	ecs2::traits::exection_arg_class exec_T = ecs2::immediate, 
	ecs2::traits::input_arg_class<ecs::entity> ent_T = ecs::entity, 
	ecs2::traits::position_arg_class pos_T = ecs2::back, 
	typename ... Arg_Ts>
void emplace_at(ent_T what, pos_T pos, Arg_Ts ... args) { }

template<typename T, 
	ecs2::traits::exection_arg_class exec_T = ecs2::immediate, 
	ecs2::traits::input_arg_class<ecs::entity> ent_T = ecs::entity, 
	typename ... Arg_Ts>
void emplace(ent_T, Arg_Ts ... args) { }

int main() {
	ecs::entity e = 0;
	std::vector<ecs::entity> e_arr;

	emplace_at<int>(e, ecs2::back{});
	emplace_at<int>(e, ecs2::at(1), 0.0f);
	emplace_at<int>(e, ecs2::front{});
	emplace_at<int>(e, ecs2::front{});
	emplace<int>(e);
	emplace<int>(e);
	emplace<int>(e_arr);

	ecs::registry reg;
	auto pip = reg.pipeline<A, B, C>();

	for (uint32_t i = 0; i < 12; ++i) {
		pip.emplace<A, ecs::immediate>(ecs::entity{i}, i); 
		pip.emplace<B, ecs::immediate>(ecs::entity{i}, i);
	}

	auto view1 = reg.view<ecs::entity, A, const B>();
	for (auto it = view1.rbegin(); it != view1.rend(); ++it) 
	{
		//auto [e, a, b] = *it;
		//std::cout << e << ", " << a.a << ", " << b.b << std::endl;
	}

	auto view2 = pip.view<ecs::entity, A, const B>();
	for (auto [e, a, b] : view2) {
		//std::cout << e << ", " << a.a << ", " << b.b << std::endl;
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