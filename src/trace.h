// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once


// The include files are not used by the macros, but using the macros require the include files.
#ifndef NDEBUG
// ReSharper disable CppUnusedIncludeDirective
#include <format>
#include <Windows.h>
// ReSharper restore CppUnusedIncludeDirective
#endif

namespace fmt {

// Copied from fmtlib as P2510 (Formatting Pointers) is not yet accepted.
template<typename T>
auto ptr(T p) -> const void*
{
    static_assert(std::is_pointer_v<T>);
    return std::bit_cast<const void*>(p);
}

// Copied from fmtlib as replacement for std::to_underlying (C++23)
template<typename Enum>
constexpr auto underlying(Enum e) noexcept -> std::underlying_type_t<Enum>
{
    return static_cast<std::underlying_type_t<Enum>>(e);
}

} // namespace fmt

// The TRACE macro can be used in debug builds to watch the behaviour of the implementation
// when used by 3rd party applications.
#ifdef NDEBUG
    #define TRACE __noop
#else
    // Use std::format directly to get compile time checking of the string arguments.
    #define TRACE(fmt, ...) OutputDebugStringA(std::format(fmt __VA_OPT__(,) __VA_ARGS__).c_str())
#endif
