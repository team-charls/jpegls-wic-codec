// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

export module util;

import winrt;

export constexpr bool failed(winrt::hresult const result) noexcept
{
    return result < 0;
}

export constexpr bool succeeded(winrt::hresult const result) noexcept
{
    return result >= 0;
}
