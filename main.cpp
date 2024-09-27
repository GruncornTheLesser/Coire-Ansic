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
#include <iostream>
#include <ranges>



using paged_vec = util::paged_vector<int, std::allocator<int>, std::allocator<std::span<int, 4u>>>;
//static_assert(std::ranges::range<paged_vec>, "");



int main() {
	// paged_vector(Alloc_T&& elem_alloc={}, Page_Alloc_T&& page_alloc={}); 
	{ 
		paged_vec vec;

		auto vec_begin = std::ranges::begin(vec);
		auto vec_end = std::ranges::end(vec);

		// paged_vector(size_t n, Alloc_T&& elem_alloc={}, Page_Alloc_T&& page_alloc={});
		paged_vec vec1(12);
		// paged_vector(size_t n, const T& value, allocator_type&& elem_alloc={}, page_allocator_type&& page_alloc={});
		paged_vec vec2(12, -1); 
		// paged_vector(const paged_vector& other);
		paged_vec vec3(vec1);
		// paged_vector& operator=(const paged_vector& other);
		vec3 = vec2;
		// paged_vector(std::initializer_list<T> ilist, Alloc_T&& elem_alloc={}, Page_Alloc_T&& page_alloc={});
		paged_vec vec4{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 }; 
		// paged_vector& operator=(std::initializer_list<T> ilist);
		vec4 = { 3, 2, 1, 4, 5, 6, 7 };
		// paged_vector(paged_vector&& other);
		paged_vec vec5(std::move(vec2)); 
		// paged_vector& operator=(paged_vector&& other);
		vec5 = std::move(vec4);

		
		for (auto& x : vec5) { std::cout << x << ", "; }
		std::cout << std::endl;
		for (auto& x : vec5 | std::views::reverse) { std::cout << x << ", "; }
		std::cout << std::endl;
		std::ranges::sort(vec5);
		for (auto& x : vec5) { std::cout << x << ", "; }
		

		// shrink_to_fit();
		vec5.shrink_to_fit();
		
		// template<typename ... Arg_Ts> reference emplace_back(Arg_Ts&&... args);
		vec5.emplace_back(4);

		// template<typename ... Arg_Ts> iterator emplace(const_iterator pos, Arg_Ts&&... args);
		vec5.emplace(vec5.begin() + 4, -2);

		int data1 = 3;
		const int data2 = -123;
		int data3 = 5;
		
		// iterator insert(const_iterator pos, T&& value);
		vec5.insert(vec5.begin() + 7, std::move(data1));
		
		// iterator insert(const_iterator pos, const T& value);
		vec5.insert(vec5.begin() + 5, data2);
		
		// iterator insert(const_iterator pos, size_t n, const T& value);
		vec5.insert(vec5.begin() + 7, 4, data2);
		
		// iterator insert(const_iterator pos, size_t n, T& value);
		vec5.insert(vec5.begin() + 5, 4, data3);

		// template<typename It> iterator insert(const_iterator pos, It first, It last);
		std::array<int, 5> arr { -1, -2, -3, -4, -5 };
		vec5.insert(vec5.begin() + 1, arr.begin(), arr.end());

		// iterator insert(const_iterator pos, std::initializer_list<T> ilist);
		vec5.insert(vec5.begin() + 6, { 1, 2, 3, 4, 5, 6, 7, 8 });

		// iterator erase(const_iterator pos, size_t n=1);
		vec5.erase(vec5.begin(), 4);

		// iterator erase(const_iterator first, const_iterator last);
		vec5.erase(vec5.begin(), vec5.begin() + 3);

		// void clear();
		vec5.clear();

		// push_back(const T& value);
		vec5.push_back(data2);
		
		// push_back(T&& value);
		vec5.push_back(std::move(data3));
		
		// void pop_back();
		vec5.pop_back();
		
		// void resize(size_t n);
		vec5.resize(14);

		// void swap(paged_vector& other);
		vec5.swap(vec3);
	}
	





/*
	using chain_vec = ecsliter::chain_vector<char, std::allocator<ecsliter::chain_vector_element<char, 4u>>, std::allocator<ecsliter::chain_vector_page<char, 4u>>>;

	chain_vec vec = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l' };
	
	vec.chain().push_back(0);
	vec.chain().push_back(3);
	vec.chain().push_back(8);
	vec.chain().push_back(3);
	vec.chain().push_back(5);
	vec.chain().push_back(10);

	for (char c : vec.chain()) { std::cout << c; }
*/
}



	/*
	
	*/








