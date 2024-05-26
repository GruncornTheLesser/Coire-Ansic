#pragma once
#include <string_view>

#if defined(__clang__)
	#define PF_CMD __PRETTY_FUNCTION__
	#define PF_PREFIX "std::basic_string_view<char> util::type_name() [T = "
	#define PF_SUFFIX "]"
#elif defined(__GNUC__) && !defined(__clang__)
	#define PF_CMD __PRETTY_FUNCTION__
	#define PF_PREFIX "constexpr std::basic_string_view<char> util::type_name() [with T = "
	#define PF_SUFFIX "]"
#elif defined(_MSC_VER)
	#define PF_CMD __FUNCSIG__
	#define PF_PREFIX "struct std::basic_string_view<char> __cdecl util::type_name<"
	#define PF_SUFFIX ">(void)"
#else
	#error "No support for this compiler."
#endif

namespace util { // NOTE: when changing the namespace of this func you must update the macros for pretty function
	template<typename T> constexpr std::basic_string_view<char> type_name() {
		return { PF_CMD + sizeof(PF_PREFIX) - 1, sizeof(PF_CMD) + 1 - sizeof(PF_PREFIX) - sizeof(PF_SUFFIX) };
	}

	template<typename LHS_T, typename RHS_T> 
	struct alpha_lt : std::bool_constant<util::type_name<LHS_T>() < util::type_name<RHS_T>()> { };
}

#undef PF_CMD
#undef PF_PREFIX
#undef PF_SUFFIX