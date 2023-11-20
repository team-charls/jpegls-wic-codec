// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

module;

#include <winerror.h>

export module errors;

import winrt;

export {

inline constexpr HRESULT error_ok{S_OK};
inline constexpr HRESULT error_fail{E_FAIL};
inline constexpr HRESULT error_pointer{E_POINTER};
inline constexpr HRESULT error_no_aggregation{CLASS_E_NOAGGREGATION};
inline constexpr HRESULT error_class_not_available{CLASS_E_CLASSNOTAVAILABLE};
inline constexpr HRESULT error_invalid_argument{E_INVALIDARG};

namespace wincodec {

inline constexpr HRESULT error_palette_unavailable{WINCODEC_ERR_PALETTEUNAVAILABLE};
inline constexpr HRESULT error_unsupported_operation{WINCODEC_ERR_UNSUPPORTEDOPERATION};
inline constexpr HRESULT error_codec_no_thumbnail{WINCODEC_ERR_CODECNOTHUMBNAIL};
inline constexpr HRESULT error_frame_missing{WINCODEC_ERR_FRAMEMISSING};
inline constexpr HRESULT error_not_initialized{WINCODEC_ERR_NOTINITIALIZED};
inline constexpr HRESULT error_wrong_state{WINCODEC_ERR_WRONGSTATE};
inline constexpr HRESULT error_unsupported_pixel_format{WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT};
inline constexpr HRESULT error_codec_too_many_scan_lines{WINCODEC_ERR_CODECTOOMANYSCANLINES};
inline constexpr HRESULT error_component_not_found{WINCODEC_ERR_COMPONENTNOTFOUND};
inline constexpr HRESULT error_bad_header{WINCODEC_ERR_BADHEADER};
inline constexpr HRESULT error_bad_image{WINCODEC_ERR_BADIMAGE};

} // namespace wincodec

template<typename T>
T* check_in_pointer(_In_ T* pointer)
{
    if (!pointer)
        winrt::throw_hresult(error_invalid_argument);

    return pointer;
}

template<typename T>
T* check_out_pointer(T* pointer)
{
    if (!pointer)
        winrt::throw_hresult(error_pointer);

    return pointer;
}

inline void check_condition(const bool condition, const winrt::hresult result_to_throw)
{
    if (!condition)
        throw_hresult(result_to_throw);
}

}
