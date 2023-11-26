#include "ecs/fwd.h"
#include "ecs/registry.h"
#include "ecs/pool.h"
#include "ecs/pipeline.h"
#include "ecs/pool_policy.h"

struct A { };
struct B { };
struct C { };
struct D { };

int main() {
	using namespace ecs;

	ecs::registry<A, B, C> reg;
	pipeline<const A, const B> pip{ &reg };
	pipeline<const A, const B> pip{ &reg };

}
