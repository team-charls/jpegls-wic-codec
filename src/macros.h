// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

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
// To use it, import also std.h and win.h.
#ifdef NDEBUG
#define TRACE __noop
#else
// Use std::format directly to get compile time checking of the string arguments.
#define TRACE(fmt, ...) OutputDebugStringA(std::format(fmt __VA_OPT__(, ) __VA_ARGS__).c_str())
#endif
