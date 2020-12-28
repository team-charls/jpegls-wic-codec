// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <winrt/base.h>

#include <CppUnitTest.h>

#define WARNING_SUPPRESS_NEXT_LINE(x) \
    __pragma(warning(suppress \
                     : x)) // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)


constexpr bool failed(winrt::hresult const result) noexcept
{
    return result < 0;
}

constexpr bool succeeded(winrt::hresult const result) noexcept
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
