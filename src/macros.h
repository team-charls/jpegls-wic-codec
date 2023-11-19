// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#define SUPPRESS_WARNING_NEXT_LINE(x) \
    __pragma(warning(suppress \
                     : x)) // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)

// The 2 macros below are used to disable false positives. These warnings are too useful to disable them globally.

// A false warning is generated when a noexcept methods calls a function that is not marked noexcept.
// Reported to Microsoft:
// https://developercommunity.visualstudio.com/content/problem/804372/c26447-false-positive-when-using-stdscoped-lock-ev.html
#define SUPPRESS_FALSE_WARNING_C26447_NEXT_LINE SUPPRESS_WARNING_NEXT_LINE(26447)

#define ASSERT(expression) \
    __pragma(warning(push)) __pragma(warning(disable : 26493)) assert(expression) __pragma(warning(pop))

#ifdef NDEBUG
#define VERIFY(expression) static_cast<void>(expression)
#else
#define VERIFY(expression) assert(expression)
#endif
