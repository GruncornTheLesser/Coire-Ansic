#include <chrono>

namespace util {
	template<typename func_t, typename... arg_us>
	double performance(func_t func, arg_us&&... args) {
		std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
		func(std::forward<arg_us>(args)...);
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - t1).count();
	}
}