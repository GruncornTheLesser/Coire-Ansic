#pragma once
#include <chrono>

namespace util {
	template<typename func_t, typename... Arg_Us>
	double performance(func_t func, Arg_Us&&... args) 
	{
		std::chrono::high_resolution_clock::time_point start_time = std::chrono::high_resolution_clock::now();
		func(std::forward<Arg_Us>(args)...);
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start_time).count();
	}
}