#include "ecs.h"
#include <iostream>
//TODO:
//	HANDLE MANAGER!!!
//  add some asserts/error checking
//		adding an entity that pool already contains
//		removing an entity that pool doesnt contain
// 		static validation on view condition erase contradictory statements
//	expand view condition to allow "any_of" -> maybe update language
//		DO NOT FALL DOWN RABBIT HOLE OF CONDITION EVALUALTION OPTIMISATION
//		limit the scope to something more achievable 
//  better comp pool collection type control
//	 	either dont sort ts in pipeline or double down and expand it...
//		expand pool collection types such as 
//			archetype, -> pool<std::tuple<ts...>>
//			uniontype, -> pool<std::variant<ts...>>
//			basictype  -> pool<t>
//  
// NOTE:
// maybe I could better increase parallel control by introducing a new constantness parameter
// eg 
//		const pool<const comp>		// comp position wont change and comp value wont change
// 		const pool<comp> 			// comp position can change but comp value wont change
//		pool<comp>					// comp position can change and comp value can change
// 		pool<const comp>			// comp position can change but comp value wont change -> not useful

// 		a system could iterate through the values but not change their position???
// 		there is no nice way to give syntax to that...
// 			could wrap comp for pipeline eg const, modify, write pipeline<read<comp>, modify<comp>>


#define SET_comp_ID public: static constexpr int comp_id = __COUNTER__; 

struct A { SET_comp_ID size_t a; A() {} A(A& other) { std::cout << "I've been copied!!!\n"; } A& operator=(const A& other) { std::cout << "I've been copied!!!\n"; return *this; }};
struct B { SET_comp_ID size_t b; };
struct C { SET_comp_ID };
struct D { SET_comp_ID };

int main() {
	using namespace ecs;
	auto x = std::is_same_v<unsigned long int, entity>;
	registry reg;
	pipeline<A, B> pip { &reg };
	reg.pool<B>().emplace(back{}, 0);
	reg.pool<A>().emplace(at{0}, 0);
	reg.pool<A>().emplace(front{}, 1);

	{
		std::lock_guard guard(pip);

		for (auto [e, a] : pip.pool<const A>())   { std::cout << e << "\n"; }
		for (auto [e, a] : pip.pool<A>())         { std::cout << e << "\n"; }
		for (auto [e, b] : pip.view<const B>())   { std::cout << e << "\n"; }
		for (auto [e, a] : pip.view<A>(exc<B>{})) { std::cout << e << "\n"; }
	}

	std::cout << "end...\n";
}

