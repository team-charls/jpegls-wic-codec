// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include "errors.h"
#include "trace.h"
#include "util.h"

#include <winrt/base.h>

#include <charls/charls.h>

class jpegls_bitmap_frame_encode final : public winrt::implements<jpegls_bitmap_frame_encode, IWICBitmapFrameEncode>
{
public:
    [[nodiscard]]
    const charls::frame_info& frame_info() const noexcept
    {
        ASSERT(state_ == state::commited);
        return frame_info_;
    }

    [[nodiscard]]
    const std::vector<std::byte>& source() const noexcept
    {
        ASSERT(state_ == state::commited);
        return source_;
    }

    [[nodiscard]]
    bool is_dpi_set() const noexcept
    {
        ASSERT(state_ == state::commited);
        return dpi_set_;
    }

    [[nodiscard]]
    double dpi_x() const noexcept
    {
        ASSERT(dpi_set_ && state_ == state::commited);
        return dpi_x_;
    }

    [[nodiscard]]
    double dpi_y() const noexcept
    {
        ASSERT(dpi_set_ && state_ == state::commited);
        return dpi_y_;
    }

    HRESULT __stdcall Initialize(_In_ IPropertyBag2* encoder_options) noexcept override;
    HRESULT __stdcall SetSize(uint32_t width, uint32_t height) noexcept override;
    HRESULT __stdcall SetResolution(double dpi_x, double dpi_y) noexcept override;
    HRESULT __stdcall SetPixelFormat(GUID* pixel_format) noexcept override;
    HRESULT __stdcall SetColorContexts(uint32_t count, IWICColorContext** color_context) noexcept override;
    HRESULT __stdcall GetMetadataQueryWriter(IWICMetadataQueryWriter** /* writer */) noexcept override;
    HRESULT __stdcall SetThumbnail(_In_ IWICBitmapSource* thumbnail) noexcept override;
    HRESULT __stdcall WritePixels(uint32_t line_count, uint32_t source_stride, uint32_t buffer_size,
                                  BYTE* pixels) noexcept override;
    HRESULT __stdcall WriteSource(_In_ IWICBitmapSource* bitmap_source, _In_ WICRect* rectangle) noexcept override;
    HRESULT __stdcall Commit() noexcept override;
    HRESULT __stdcall SetPalette(_In_ IWICPalette* palette) noexcept override;

private:
    void set_pixel_format(const int32_t bits_per_sample, const int32_t component_count) noexcept
    {
        frame_info_.bits_per_sample = bits_per_sample;
        frame_info_.component_count = component_count;
        pixel_format_set_ = true;
    }

    [[nodiscard]]
    uint32_t stride() const noexcept
    {
        ASSERT(size_set_ && pixel_format_set_);
        return frame_info_.width * frame_info_.component_count; // TODO: update for 16 bit images.
    }

    void convert_bgr_to_rgb() noexcept
    {
        for (size_t i{}; i < source_.size(); i += 3)
        {
            std::swap(source_[i], source_[i + 2]);
        }
    }

    void unpack_nibbles()
    {
        std::vector<std::byte> unpacked(source_.size() * 2);

        size_t j{};
        for (const auto nibble_pixel : source_)
        {
            unpacked[j] = nibble_pixel >> 4;
            ++j;
            unpacked[j] = nibble_pixel | static_cast<std::byte>(0xF);
            ++j;
        }

        source_.swap(unpacked);
    }

    enum class state
    {
        created,
        initialized,
        received_pixels,
        commited
    };

    state state_{};
    bool size_set_{};
    bool pixel_format_set_{};
    bool dpi_set_{};
    bool swap_pixels_{};
    uint32_t received_line_count_{};
    std::vector<std::byte> source_;
    charls::frame_info frame_info_{};
    double dpi_x_{};
    double dpi_y_{};
};
