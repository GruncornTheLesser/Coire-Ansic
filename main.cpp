/* 
 TODO: ECS emplace/erase/etc funcs
 TODO: view where parameter implementation
 TODO: entity manager
 TODO: versioner
 TODO: dispatcher
 TODO: 
*/

#include "resource/registry.h"
#include "resource/set.h"
#include "util/type_name.h"
#include "ecs/registry.h"

struct res_A { int a; };
struct res_B { int b; };

int main() 
{
	using namespace resource;
	static_registry<res_A, res_B> reg; // dynamic_registry reg;

	resource::set<res_A, const res_B> resources(reg);
	{
		std::lock_guard lk(resources);
		const res_B& rb = resources.get_resource<const res_B>();
	}

	{
		auto pa = reg.get_resource<const res_A>(); // auto locks
	}

	resource_ptr<res_A> ptr_s{ &reg.get_resource_pool<res_A>(), priority::MEDIUM };


}

