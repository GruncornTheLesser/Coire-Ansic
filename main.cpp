/* 
 TODO: ECS emplace/erase/etc funcs
 TODO: view where parameter implementation
 TODO: entity manager
 TODO: versioner
 TODO: dispatcher
 TODO: proper interfaces for resource types -> creating an interface would allow less confusion when creating resources that point to the same type
*/
#include "ecs/pipeline.h"
#include "ecs/traits.h"
#include <iostream>

//#include "resource/registry.h"
//#include "resource/set.h"

#include <iostream>
#include <type_traits>


struct res_A {
	using component_tag = ecs::tags::resource;
	int a; 
};
struct res_B { using component_tag = ecs::tags::resource; int b; };

struct comp_A {
	float a; 
	using component_tag = ecs::tags::basictype<comp_A>;
};
struct comp_B { float b; };


int main() 
{
	using namespace ecs;
	resource::registry<> reg;
	pipeline<res_A, comp_A, ecs::entity> pip{ reg }; // res_A, handle<A>, indexer<A>, storage<A>
	{
		std::lock_guard lk(pip);
		res_A& rA = pip.get_resource<res_A>();
		
		ecs::entity e = pip.create();
		pip.emplace<comp_A, ecs::policy::stable, ecs::policy::immediate>(e);
	}

	std::cout << "fdsfdsfds=" << util::type_name<pipeline<res_A, comp_A, ecs::entity>::resource_lockset>() << std::endl;
}

	


	/*
	
	*/








