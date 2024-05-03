#pragma once
#include <string_view>

#if defined(__clang__)
	#define COMPILER_PRETTY_FUNCTION __PRETTY_FUNCTION__
	#define PRETTY_FUNCTION_PREFIX "std::basic_string_view<char> util::type_name() [T = "
	#define PRETTY_FUNCTION_SUFFIX "]"
#elif defined(__GNUC__) && !defined(__clang__)
	#define COMPILER_PRETTY_FUNCTION __PRETTY_FUNCTION__
	#define PRETTY_FUNCTION_PREFIX "constexpr std::basic_string_view<char> util::type_name() [with T = "
	#define PRETTY_FUNCTION_SUFFIX "]"
#elif defined(_MSC_VER)
	#define COMPILER_PRETTY_FUNCTION __FUNCSIG__
	#define PRETTY_FUNCTION_PREFIX "struct std::basic_string_view<char> __cdecl util::type_name<"
	#define PRETTY_FUNCTION_SUFFIX ">(void)"
#else
	#error "No support for this compiler."
#endif

namespace util { // NOTE: when changing the namespace of this func you must update the macros for pretty function
	template<typename T> constexpr std::basic_string_view<char> type_name() {
		return { COMPILER_PRETTY_FUNCTION + sizeof(PRETTY_FUNCTION_PREFIX) - 1, sizeof(COMPILER_PRETTY_FUNCTION) + 1 - sizeof(PRETTY_FUNCTION_PREFIX) - sizeof(PRETTY_FUNCTION_SUFFIX) };
	}

	template<typename LHS_T, typename RHS_T> 
	struct alpha_lt : std::bool_constant<util::type_name<LHS_T>() < util::type_name<RHS_T>()> { };
}