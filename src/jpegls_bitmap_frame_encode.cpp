// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

module;

#include "macros.h"

module jpegls_bitmap_frame_encode;

import "win.h";
import "std.h";
import errors;
import winrt;
import util;

HRESULT __stdcall jpegls_bitmap_frame_encode::Initialize([[maybe_unused]] IPropertyBag2* encoder_options) noexcept
try
{
    TRACE("{} jpegls_bitmap_frame_encode::Initialize, encoder_options={}\n", fmt::ptr(this), fmt::ptr(encoder_options));

    check_condition(state_ == state::created, wincodec::error_wrong_state);
    state_ = state::initialized;
    return error_ok;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall jpegls_bitmap_frame_encode::SetSize(const uint32_t width, const uint32_t height) noexcept
try
{
    TRACE("{} jpegls_bitmap_frame_encode::SetSize.1, width={}, height={}\n", fmt::ptr(this), width, height);

    check_condition(state_ == state::initialized, wincodec::error_wrong_state);
    frame_info_.width = width;
    frame_info_.height = height;
    size_set_ = true;

    return error_ok;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall jpegls_bitmap_frame_encode::SetResolution(const double dpi_x, const double dpi_y) noexcept
try
{
    TRACE("{} jpegls_bitmap_frame_encode::SetResolution.1, dpi_x={}, dpi_y={}\n", fmt::ptr(this), dpi_x, dpi_y);

    check_condition(state_ != state::commited, wincodec::error_wrong_state);

    const auto resolution_x{lround(dpi_x)};
    const auto resolution_y{lround(dpi_y)};
    check_condition(resolution_x > 0 && resolution_y > 0, error_invalid_argument);

    resolution_ = {resolution_x, resolution_y};
    return error_ok;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall jpegls_bitmap_frame_encode::SetPixelFormat(GUID* pixel_format) noexcept
try
{
    TRACE("{} jpegls_bitmap_frame_encode::SetPixelFormat, pixel_format={}\n", fmt::ptr(this), fmt::ptr(pixel_format));

    check_in_pointer(pixel_format);
    check_condition(state_ == state::initialized, wincodec::error_wrong_state);

    swap_pixels_ = false;

    if (*pixel_format == GUID_WICPixelFormat2bppGray)
    {
        set_pixel_format(2, 1);
        return error_ok;
    }

    if (*pixel_format == GUID_WICPixelFormat4bppGray)
    {
        set_pixel_format(4, 1);
        return error_ok;
    }

    if (*pixel_format == GUID_WICPixelFormat8bppGray)
    {
        set_pixel_format(8, 1);
        return error_ok;
    }

    if (*pixel_format == GUID_WICPixelFormat16bppGray)
    {
        set_pixel_format(16, 1);
        return error_ok;
    }

    if (*pixel_format == GUID_WICPixelFormat24bppRGB)
    {
        set_pixel_format(8, 3);
        return error_ok;
    }

    if (*pixel_format == GUID_WICPixelFormat24bppBGR)
    {
        set_pixel_format(8, 3);
        swap_pixels_ = true;
        return error_ok;
    }

    if (*pixel_format == GUID_WICPixelFormat48bppRGB)
    {
        set_pixel_format(16, 3);
        return error_ok;
    }

    *pixel_format = GUID_WICPixelFormatUndefined;
    return wincodec::error_unsupported_pixel_format;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall jpegls_bitmap_frame_encode::SetColorContexts([[maybe_unused]] const uint32_t count,
                                                               [[maybe_unused]] IWICColorContext** color_context) noexcept
{
    TRACE("{} jpegls_bitmap_frame_encode::SetColorContexts, count={}, color_context={}\n", fmt::ptr(this), count,
          fmt::ptr(color_context));
    return wincodec::error_unsupported_operation;
}

HRESULT __stdcall jpegls_bitmap_frame_encode::GetMetadataQueryWriter(IWICMetadataQueryWriter** /* writer */) noexcept
{
    return wincodec::error_unsupported_operation;
}

HRESULT __stdcall jpegls_bitmap_frame_encode::SetThumbnail([[maybe_unused]] _In_ IWICBitmapSource* thumbnail) noexcept
{
    TRACE("{} jpegls_bitmap_frame_encode::SetThumbnail, thumbnail={}\n", fmt::ptr(this), fmt::ptr(thumbnail));
    return wincodec::error_unsupported_operation;
}

HRESULT __stdcall jpegls_bitmap_frame_encode::WritePixels(const uint32_t line_count, const uint32_t source_stride,
                                                          uint32_t /* buffer_size */, BYTE* pixels) noexcept
try
{
    using enum state;
    check_condition((state_ == initialized || state_ == received_pixels) && size_set_ && pixel_format_set_,
                    wincodec::error_wrong_state);
    check_condition(received_line_count_ + line_count <= frame_info_.height, wincodec::error_codec_too_many_scan_lines);

    const size_t destination_stride{compute_stride()};
    allocate_pixel_buffer_if_needed(destination_stride);

    std::byte* destination{source_.data() + (received_line_count_ * destination_stride)};
    winrt::check_hresult(MFCopyImage(reinterpret_cast<BYTE*>(destination), static_cast<LONG>(destination_stride), pixels,
                                     static_cast<LONG>(source_stride), static_cast<DWORD>(destination_stride), line_count));

    received_line_count_ += line_count;
    state_ = received_pixels;
    return error_ok;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall jpegls_bitmap_frame_encode::WriteSource(_In_ IWICBitmapSource* bitmap_source,
                                                          [[maybe_unused]] _In_ WICRect* rectangle) noexcept
try
{
    TRACE("{} jpegls_bitmap_frame_encode::WriteSource, bitmap_source={}, rectangle={}\n", fmt::ptr(this),
          fmt::ptr(bitmap_source), fmt::ptr(rectangle));

    check_condition(state_ == state::initialized, wincodec::error_wrong_state);
    check_in_pointer(bitmap_source);

    if (!size_set_)
    {
        uint32_t width;
        uint32_t height;
        winrt::check_hresult(bitmap_source->GetSize(&width, &height));
        winrt::check_hresult(SetSize(width, height));
    }

    if (!pixel_format_set_)
    {
        GUID pixel_format;
        winrt::check_hresult(bitmap_source->GetPixelFormat(&pixel_format));
        winrt::check_hresult(SetPixelFormat(&pixel_format));
    }

    const size_t stride{compute_stride()};
    allocate_pixel_buffer_if_needed(stride);

    winrt::check_hresult(bitmap_source->CopyPixels(nullptr, static_cast<uint32_t>(stride), static_cast<uint32_t>(source_.size()),
                                                   reinterpret_cast<BYTE*>(source_.data())));
    state_ = state::received_pixels;
    return error_ok;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall jpegls_bitmap_frame_encode::Commit() noexcept
try
{
    TRACE("{} jpegls_bitmap_frame_encode::Commit\n", fmt::ptr(this));
    check_condition(state_ == state::received_pixels, wincodec::error_wrong_state);
    ASSERT(size_set_ && pixel_format_set_);

    if (swap_pixels_)
    {
        convert_bgr_to_rgb();
    }
    else if (frame_info_.bits_per_sample == 2)
    {
        unpack_crumbs();
    }
    else if (frame_info_.bits_per_sample == 4)
    {
        unpack_nibbles();
    }
    else
    {
        source_stride_ = compute_stride(); // TODO: move to allocate_pixel_buffer_if_needed
    }

    state_ = state::commited;
    return error_ok;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall jpegls_bitmap_frame_encode::SetPalette([[maybe_unused]] _In_ IWICPalette* palette) noexcept
{
    TRACE("{} jpegls_bitmap_frame_encode::SetPalette, palette={}\n", fmt::ptr(this), fmt::ptr(palette));
    return wincodec::error_palette_unavailable;
}
