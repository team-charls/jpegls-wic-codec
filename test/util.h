// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <winrt/base.h>

#include <CppUnitTest.h>

#define WARNING_SUPPRESS_NEXT_LINE(x) \
    __pragma(warning(suppress \
                     : x)) // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)

// A false warning is generated when a type is returned that can converted to a HRESULT but is not a HRESULT.
// The HRESULT type is tagged with a SAL annotation that indicates when the function returns an error or not.
// Reported to Microsoft:
// https://developercommunity.visualstudio.com/content/problem/804429/static-analysis-emits-a-false-positive-c6101-error.html
#define SUPPRESS_FALSE_WARNING_C6101_NEXT_LINE WARNING_SUPPRESS_NEXT_LINE(6101)


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
