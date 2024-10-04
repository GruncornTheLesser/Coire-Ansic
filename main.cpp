/*
 TODO: ECS emplace/erase/etc funcs
 TODO: view where parameter implementation
 TODO: entity manager
 TODO: versioner
 TODO: dispatcher
 TODO: proper interfaces for resource types -> creating an interface would allow less confusion when creating resources that point to the same type
! THING I WANT !
 * 1->N relationship: sparse returns start, entity stores number of components, and storage dereferencing to a pointer
 * archetypes:

*/
/*
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
using test_1 = ecs::traits::get_handle_t<const comp_B>;
using test_2 = ecs::traits::get_manager_t<const comp_B>;
using test_3 = ecs::traits::get_indexer_t<const comp_B>;
using test_4 = ecs::traits::get_storage_t<const comp_B>;

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

*/

#include "util/paged_vector.h"
#include "util/chain_vector.h"
//#include "util/compressed.h"
#include <iostream>
#include <ranges>

int main() {
	using paged_vec = util::paged_vector<int, std::allocator<std::span<int, 4u>>>;
	{
		std::allocator<int> alloc;
		std::allocator<std::span<int, 4u>> page_alloc;
		paged_vec vec_a;
		paged_vec vec_b{ alloc };
		paged_vec vec_c{ alloc, page_alloc };
		paged_vec vec_d{ 13 };
		paged_vec vec_e{ 13, alloc };
		paged_vec vec_f{ 13, alloc, page_alloc };
		std::array<int, 10> arr{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		paged_vec vec_g{ arr.begin(), arr.end() };
		paged_vec vec_h{ arr.begin(), arr.end(), alloc };
		paged_vec vec_i{ arr.begin(), arr.end(), alloc, page_alloc };
		paged_vec vec_j{ vec_i };
		paged_vec vec_k{ std::move(vec_j) };
		paged_vec vec_l{ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 } };
		paged_vec vec_m{ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, alloc };
		{
			paged_vec vec_n{ { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }, alloc, page_alloc };
		} // ~paged_vector();
		vec_a = vec_b;
		vec_b = std::move(vec_k);
		vec_c = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };

		vec_a.assign(arr.begin() + 2, arr.end() - 2);
		vec_b.assign(6, -1);
		vec_c.assign({ 6, 5, 4, 3, 2, 1 });

		for (auto& num : vec_c) std::cout << num << ", "; std::cout << std::endl;
		for (auto& num : const_cast<paged_vec&>(vec_c)) std::cout << num << ", "; std::cout << std::endl;
		for (auto& num : vec_c | std::views::reverse) std::cout << num << ", "; std::cout << std::endl;

		std::ranges::sort(vec_c);

		vec_a.resize(12);
		vec_b.resize(12, -24);
		vec_c.reserve(12);
		vec_c.shrink_to_fit();

		vec_a.emplace_back(0);
		vec_b.push_back(1);
		vec_c.push_back(std::move(2));
		vec_a.pop_back();
		vec_b.emplace(vec_b.cbegin() + 7, 0);
		vec_c.insert(vec_b.cbegin() + 7, 1);
		vec_a.insert(vec_b.cbegin() + 7, std::move(2));
		vec_b.insert(vec_b.cbegin() + 7, 3, 4);
		vec_c.insert(vec_b.cbegin() + 7, arr.begin(), arr.end());
		vec_a.insert(vec_b.cbegin() + 7, { 5, 6, 7, 8, 9, });
		vec_b.erase(vec_b.cbegin() + 3, 10);
		vec_c.erase(vec_b.cbegin() + 3, vec_b.cbegin() + 11);

		vec_a.clear();
		vec_a.shrink_to_fit();
	}
	
	
	using chain_vec = util::chain_vector<int, std::allocator<int>, 4u>;
	{
		chain_vec vec
		{ 
			'a', 'b', 'c', 'd', 
			'e', 'f', 'g', 'h', 
			'i', 'j', 'k', 'l' 
		};

		auto chain_view = vec.view();
		vec.view().insert(vec.begin() + 0); // a
		vec.view().insert(vec.begin() + 8); // i
		vec.view().insert(vec.begin() + 3); // d
		vec.view().insert(vec.begin() + 3); // d
		vec.view().insert(vec.begin() + 5); // f
		vec.view().insert(vec.begin() + 10); // k
		auto beg = chain_view.begin();
		auto end = chain_view.end();
		for (char c : vec.view()) { std::cout << c << ", "; }
		
		vec.view().clear_page(vec.get_page(1));

	}
}



	/*

	*/








