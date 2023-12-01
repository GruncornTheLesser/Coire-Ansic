#include "ecs/fwd.h"
#include "ecs/registry.h"
#include "ecs/pool.h"
#include "ecs/pipeline.h"
#include "ecs/pool_policy.h"

#define SET_COMPONENT_ID public: static constexpr int component_id = __COUNTER__; 

struct A { SET_COMPONENT_ID };
struct B { SET_COMPONENT_ID };
struct C { SET_COMPONENT_ID };
struct D { SET_COMPONENT_ID };

int main() {
	using namespace ecs;

	ecs::registry reg;

	auto pip = reg.pipeline<const B, A>();
	{
		std::lock_guard guard(pip);
		auto& pl1 = pip.pool<const A>(); 	// no error
		auto& pl2 = pip.pool<const B>(); 	// no error
		auto& pl3 = pip.pool<A>(); 			// no error
		//pip.pool<B>(); 					// error
		
	}
}

