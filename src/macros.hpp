// SPDX-FileCopyrightText: © 2023 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cassert>

#define SUPPRESS_WARNING_NEXT_LINE(x) \
    __pragma(warning(suppress \
                     : x)) // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)

#define ASSERT(expression) \
    __pragma(warning(push)) __pragma(warning(disable : 26493)) assert(expression) __pragma(warning(pop))

#ifdef NDEBUG
#define VERIFY(expression) static_cast<void>(expression)
#else
#define VERIFY(expression) assert(expression)
#endif

// The TRACE macro can be used in debug builds to watch the behaviour of the implementation
// when used by 3rd party applications.
#ifdef NDEBUG
#define TRACE(fmt, ...) __noop
#else
#if defined(__INTELLISENSE__)
// IntelliSense fails to parse std::format in combination with modules [VS 2022 17.13.6].
#define TRACE(fmt, ...) __noop
#else
// Use std::format directly to get compile time checking of the string arguments.
#define TRACE(fmt, ...) OutputDebugStringA(std::format(fmt __VA_OPT__(, ) __VA_ARGS__).c_str())
#endif
#endif
