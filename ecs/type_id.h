#pragma once
#include <string_view>

#if defined(__clang__)
    #define COMPILER_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #define PRETTY_FUNCTION_PREFIX (sizeof("std::string_view ecs::type_name() [T = ") - 1)
    #define PRETTY_FUNCTION_SUFFIX (sizeof("]") - 1)
#elif defined(__GNUC__) && !defined(__clang__)
    #define COMPILER_PRETTY_FUNCTION __PRETTY_FUNCTION__
    #define PRETTY_FUNCTION_PREFIX "constexpr std::basic_string_view<char> ecs::type_name() [with t = "
    #define PRETTY_FUNCTION_SUFFIX "]"
#elif defined(_MSC_VER)
    #define COMPILER_PRETTY_FUNCTION __FUNCSIG__
    #define PRETTY_FUNCTION_PREFIX (sizeof("struct std::string_view __cdecl ecs::type_name<") - 1)
    #define PRETTY_FUNCTION_SUFFIX (sizeof(">(void)") - 1)
#else
    #error "No support for this compiler."
#endif

namespace ecs {
    template<typename t> constexpr std::basic_string_view<char> type_name() {
        return { COMPILER_PRETTY_FUNCTION + sizeof(PRETTY_FUNCTION_PREFIX) - 1, sizeof(COMPILER_PRETTY_FUNCTION) + 1 - sizeof(PRETTY_FUNCTION_PREFIX) - sizeof(PRETTY_FUNCTION_SUFFIX) };
    }
}