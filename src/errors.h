// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <unknwn.h>
#include <winerror.h>
#include <winrt/base.h>

inline constexpr winrt::hresult error_ok{S_OK};
inline constexpr winrt::hresult error_fail{E_FAIL};
inline constexpr winrt::hresult error_pointer{E_POINTER};
inline constexpr winrt::hresult error_no_aggregation{static_cast<winrt::hresult>(CLASS_E_NOAGGREGATION)};
inline constexpr winrt::hresult error_class_not_available{static_cast<winrt::hresult>(CLASS_E_CLASSNOTAVAILABLE)};
inline constexpr winrt::hresult error_invalid_argument{static_cast<winrt::hresult>(E_INVALIDARG)};

namespace wincodec {

inline constexpr winrt::hresult error_palette_unavailable{static_cast<winrt::hresult>(WINCODEC_ERR_PALETTEUNAVAILABLE)};
inline constexpr winrt::hresult error_unsupported_operation{static_cast<winrt::hresult>(WINCODEC_ERR_UNSUPPORTEDOPERATION)};
inline constexpr winrt::hresult error_codec_no_thumbnail{static_cast<winrt::hresult>(WINCODEC_ERR_CODECNOTHUMBNAIL)};
inline constexpr winrt::hresult error_frame_missing{static_cast<winrt::hresult>(WINCODEC_ERR_FRAMEMISSING)};
inline constexpr winrt::hresult error_not_initialized{static_cast<winrt::hresult>(WINCODEC_ERR_NOTINITIALIZED)};
inline constexpr winrt::hresult error_wrong_state{static_cast<winrt::hresult>(WINCODEC_ERR_WRONGSTATE)};
inline constexpr winrt::hresult error_unsupported_pixel_format{static_cast<winrt::hresult>(WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT)};
inline constexpr winrt::hresult error_codec_too_many_scan_lines{static_cast<winrt::hresult>(WINCODEC_ERR_CODECTOOMANYSCANLINES)};
inline constexpr winrt::hresult error_component_not_found{static_cast<winrt::hresult>(WINCODEC_ERR_COMPONENTNOTFOUND)};

} // namespace wincodec
