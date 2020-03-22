// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <unknwn.h>
#include <winrt/base.h>

inline constexpr winrt::hresult error_ok{S_OK};
inline constexpr winrt::hresult error_pointer{E_POINTER};
inline constexpr winrt::hresult error_no_aggregation{static_cast<winrt::hresult>(CLASS_E_NOAGGREGATION)};
inline constexpr winrt::hresult error_invalid_argument{static_cast<winrt::hresult>(E_INVALIDARG)};
