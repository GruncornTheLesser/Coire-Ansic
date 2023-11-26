#include <vector>
#include <cassert>
#include <iostream>
#include <random>
#include <format>
#include <execution>
#include "ecs/pool_policy.h"
#include "ecs/pool.h"

#define ASSERT(CND, MSG) if (!CND) { std::cout << MSG << std::endl; throw std::runtime_error(MSG); }

template<typename t>
bool validate_sparse_set(ecs::pool<t>& pl, std::vector<ecs::entity>& alive, std::vector<ecs::entity>& dead) {

	bool out = true;
	bool pass;

	pass = true;
	for (auto it = alive.begin(); it != alive.end(); ++it) if (!pl.sparse.contains(*it)) {
		pass = false;
		std::cout << "        " << *it << " is alive but is not contained in sparse" << std::endl;
	}
	if (!pass) { std::cout << "ERROR: alive entity not alive in sparse set" << std::endl; }

	out &= pass;

	pass = true;
	for (auto it = dead.begin(); it != dead.end(); ++it) if (pl.sparse.at(*it) != util::tombstone) {
		pass = false; 
		std::cout << "        " << *it << " is dead but is found at " << pl.sparse.at(*it) << std::endl;
	} 
	
	if (!pass) { std::cout << "ERROR: dead entity not dead in sparse set" << std::endl; }
	
	out &= pass;

	pass = true;
	size_t i = 0;
	for (size_t i = 0; i < pl.size; i++)
		if (pl.sparse[pl.packed[i]] == -1) 
		{
			pass = false;
			std::cout << "        " << "sparse set entity " << pl.packed[i] << " at index " << i << " has tombstone index" << std::endl;
		}
	if (!pass) { std::cout << "ERROR: sparse set entity with no valid index" << std::endl; }
	
	out &= pass;

	pass = true;
	size_t index = 0;
	for (auto it = pl.packed; it != pl.packed + pl.size; ++it) 
		if (pl.sparse[*it] != index++) 
		{ 
			pass = false; 
			std::cout << "(" << *it << "@" << index-1 << "!=" << pl.sparse[*it] << ")" << std::endl; 
		}
	out &= pass;


	return out;
}

void validation_test() {
	using namespace ecs;
	using namespace ecs::pool_policy;
	std::vector<entity> dead = {};
	dead.reserve(200);
	for (size_t i = 0; i < 20; ++i) dead.push_back(i);

	std::vector<entity> alive = {};
	
	ecs::pool<float> pl;

	std::mt19937 rng{};
		
	for (int i = 0; i < 2000; i++) {
		// emplace_back
		{
			size_t add_count = rand() % dead.size();
			std::shuffle(dead.begin(), dead.end(), rng);

			pl.emplace_back(dead.begin(), dead.begin() + add_count);
			
			alive.insert(alive.end(), dead.begin(), dead.begin() + add_count);
			dead.erase(dead.begin(), dead.begin() + add_count);

			ASSERT(validate_sparse_set(pl, alive, dead), "failed validation after emplace back");
		}

		// erase
		{
			size_t del_count = rand() % alive.size();
			std::shuffle(alive.begin(), alive.end(), rng);

			pl.erase<inplace>(alive.begin(), alive.begin() + del_count);
			
			dead.insert(dead.end(), alive.begin(), alive.begin() + del_count);
			alive.erase(alive.begin(), alive.begin() + del_count);

			ASSERT(validate_sparse_set(pl, alive, dead), "failed validation after inplace erase");
		}

		// inplace emplace at
		{
			size_t add_count = rand() % dead.size();
			size_t index = rand() % alive.size();
			
			std::shuffle(dead.begin(), dead.end(), rng);

			pl.emplace_at<inplace>(index, dead.begin(), dead.begin() + add_count);
			
			alive.insert(alive.begin(), dead.begin(), dead.begin() + add_count);
			dead.erase(dead.begin(), dead.begin() + add_count);

			ASSERT(validate_sparse_set(pl, alive, dead), "failed validation after inplace emplace");
		}
		
		// swap erase
		{
			size_t del_count = rand() % alive.size();
			std::shuffle(alive.begin(), alive.end(), rng);

			pl.erase<swap>(alive.begin(), alive.begin() + del_count);
			
			dead.insert(dead.end(), alive.begin(), alive.begin() + del_count);
			alive.erase(alive.begin(), alive.begin() + del_count);

			ASSERT(validate_sparse_set(pl, alive, dead), "failed validation after swap erase");
		}
		
		// swap emplace at
		{
			size_t add_count = rand() % dead.size();
			size_t index = rand() % pl.count();

			std::shuffle(dead.begin(), dead.end(), rng);
			
			pl.emplace_at<swap>(index, dead.begin(), dead.begin() + add_count);
			
			alive.insert(alive.end(), dead.begin(), dead.begin() + add_count);
			dead.erase(dead.begin(), dead.begin() + add_count);

			ASSERT(validate_sparse_set(pl, alive, dead), "failed validation after swap emplace at");
		}
	}
}

template<typename t>
bool valid2(ecs::pool<t>& pl) {
	using namespace ecs;
	
	bool test1 = std::all_of(std::execution::par, pl.packed, pl.packed + pl.size, [&](entity e) { 
		return pl.packed[pl.sparse[e]] == e;
	});
	if (!test1) std::cout << "entity in packed is not in sparse" << std::endl;
	
	bool test2 = std::all_of(std::execution::par, pl.sparse.pages, pl.sparse.pages + pl.sparse.page_count, [&](size_t* page) {
		return (page == nullptr) || std::all_of(page, page + util::sparse_map::page_size, [&](size_t i) { 
			return i == -1 || (i < pl.capacity && pl.sparse[pl.packed[i]] == i);
		});
	});
	if (!test2) std::cout << "entity in sparse not in packed" << std::endl;
	return test1 && test2;
}

template<typename func_t>
auto time_func(std::string test_name, size_t N, size_t elem_count, func_t func) {
	auto t1 = std::chrono::high_resolution_clock::now();
	for(size_t i = 0; i < N; ++i) func(i);
	auto t2 = std::chrono::high_resolution_clock::now();
	auto duration = t2 - t1;
	auto average = duration / (double)elem_count;
	std::cout << test_name << 
		" - Time Elapsed: " << 
		std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count() << "ns" <<
		" - Average: " << 
		std::chrono::duration_cast<std::chrono::nanoseconds>(average).count() << "ns" << std::endl;
}

void stress_test(size_t count) {
	using namespace ecs;
	using namespace ecs::pool_policy;

	struct Guh { };
	pool<Guh> pl{};
	pl.reserve(count);
	std::mt19937 rng{};

	std::cout << "count: " << count << std::endl;
	
	const size_t max_range_size = count / 20;
	std::cout << "max range size: " << max_range_size << std::endl;
	std::vector<entity> open{};
	std::vector<size_t> index_cache{};
	std::vector<size_t> range_cache{};
	size_t n;

	{
		for(size_t i = 0; i < count * 2; ++i) open.emplace_back(i);
		std::shuffle(open.begin(), open.end(), rng);
		pl.emplace_back(open.begin() + count, open.end());
	}
	time_func("test 1 - emplace_back()", count, count, [&](size_t i){ pl.emplace_back(open[i]); });

	assert(valid2(pl));

	time_func("test 2 - erase_back()", count, count, [&](size_t i){ pl.erase_back(); });

	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.begin() + count, rng);
		range_cache.clear();
		range_cache.push_back(0);
		n = 0;
		for (; range_cache.back() != count; ++n) 
			range_cache.push_back(std::min(range_cache.back() + (rng() % max_range_size), count)); 
	}
	time_func("test 3 - emplace_back(n)", n, count, [&](size_t i){ pl.emplace_back(open.begin() + range_cache[i], open.begin() + range_cache[i + 1]); });

	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.begin() + count, rng);
		range_cache.clear();
		n = 0;
		do {
			range_cache.push_back(std::min(range_cache.back() + (rng() % max_range_size), count));
			++n;
		} while (range_cache.back() != count);
	}
	time_func("test 4 - erase_back(n)", n, count, [&](size_t i){ pl.erase_back(range_cache[i]); });

	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.begin() + count, rng);
		index_cache.clear();
		for (size_t size = 0; size != count; ++size) index_cache.push_back(rng() % std::max(1ull, size)); 
	}
	time_func("test 5 - emplace_at<swap>()", count, count, [&](size_t i) { pl.emplace_at<immediate_swap>(index_cache[i], open[i]); });

	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.end(), rng);
	}
	time_func("test 6 - erase<swap>()", count, count, [&](size_t i){ pl.erase<immediate_swap>(open[i]); });
	
	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.begin() + count, rng);
		index_cache.clear();
		range_cache.clear();
		n = 0;
		range_cache.push_back(0);
		while (range_cache.back() != count) {
			index_cache.push_back(rng() % std::max(range_cache.back(), 1ull));
			range_cache.push_back(range_cache.back() + std::min(rng() % max_range_size, count - range_cache.back()));
			n++;
		}
	}
	time_func("test 7 - emplace_at<swap>(n)", n, count, [&](size_t i) { pl.emplace_at<immediate_swap>(index_cache[i], open.begin() + range_cache[i], open.begin() + range_cache[i + 1]); });
	
	assert(valid2(pl));

	{
		
		std::shuffle(open.begin(), open.end(), rng);
		index_cache.clear();
		range_cache.clear();
		n = 0;
		range_cache.push_back(0);
		for (size_t size = 0; size != count; ) {
			range_cache.push_back(std::min(size + rng() % max_range_size, count));
			size = range_cache.back();
			n++;
		}
	}
	time_func("test 8 - erase<swap>(n)", n, count, [&](size_t i){ pl.erase<swap>(open.begin() + range_cache[i], open.begin() + range_cache[i + 1]); });

	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.begin() + count, rng);
		index_cache.clear();
		for (size_t size = 0; size != count; ++size) index_cache.push_back(rng() % std::max(1ull, size)); 
	}
	time_func("test 9 - emplace_at<inplace>()", count, count,  [&](size_t i) { pl.emplace_at<inplace>(index_cache[i], open[i]); });
	
	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.end(), rng);
	}
	time_func("test 10 - erase<inplace>()", count, count,  [&](size_t i){ pl.erase<immediate_inplace>(open[i]); });

	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.begin() + count, rng);
		index_cache.clear();
		range_cache.clear();
		n = 0;
		range_cache.push_back(0);
		while (range_cache.back() != count) {
			index_cache.push_back(rng() % std::max(range_cache.back(), 1ull));
			range_cache.push_back(range_cache.back() + std::min(rng() % max_range_size, count - range_cache.back()));
			n++;
		}
	}
	time_func("test 11 - emplace_at<inplace>(n)", n, count,  [&](size_t i) { pl.emplace_at<immediate_inplace>(index_cache[i], open.begin() + range_cache[i], open.begin() + range_cache[i + 1]); });
	
	assert(valid2(pl));

	{
		std::shuffle(open.begin(), open.end(), rng);
		index_cache.clear();
		range_cache.clear();
		n = 0;
		range_cache.push_back(0);
		for (size_t size = 0; size != count; ) {
			range_cache.push_back(std::min(size + rng() % max_range_size, count));
			size = range_cache.back();
			n++;
		}
	}
	time_func("test 12 - erase<inplace>(n)", n, count, [&](size_t i){ pl.erase<immediate_inplace>(open.begin() + range_cache[i], open.begin() + range_cache[i + 1]); });

	assert(valid2(pl));
}

template<typename t>
bool validate_sparse_set(ecs::pool<t>& pl) {
	for (int i = 0; i < pl.size; i++)
		if (!pl.sparse.contains(pl.packed[i])) return false;
	
	for (int i = pl.size; i < pl.capacity; i++) 
		if (pl.sparse.contains(pl.packed[i])) return false;
	
	for (int i = 0; i < pl.size; i++) 
		if (pl.sparse[pl.packed[i]] == i) return false;

	return true;
}
