// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <winrt/base.h>

#include <CppUnitTest.h>

#define WARNING_SUPPRESS(x) __pragma(warning(push)) __pragma(warning(disable \
                                                                     : x)) // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)
#define WARNING_UNSUPPRESS() __pragma(warning(pop))                        // NOLINT(cppcoreguidelines-macro-usage)


inline constexpr bool failed(winrt::hresult const result) noexcept
{
    return result < 0;
}

inline constexpr bool succeeded(winrt::hresult const result) noexcept
{
    return result >= 0;
}

namespace Microsoft::VisualStudio::CppUnitTestFramework {

template<>
inline std::wstring ToString<winrt::hresult>(const winrt::hresult& q)
{
    RETURN_WIDE_STRING(static_cast<int>(q));
}

} // namespace Microsoft::VisualStudio::CppUnitTestFramework
