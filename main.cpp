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

namespace ecs::traits {

}

int main() {
	using namespace ecs;

	ecs::registry reg;
	auto pip = reg.pipeline<const A, const B>();
	{
		std::lock_guard guard(pip);
		
	}
}

