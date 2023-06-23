﻿// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

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

    [[nodiscard]]
    uint32_t source_stride() const noexcept
    {
        ASSERT(state_ == state::commited);
        return source_stride_;
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

    void allocate_pixel_buffer_if_needed(const size_t stride)
    {
        ASSERT(size_set_ && pixel_format_set_);
        if (source_.empty())
        {
            source_.resize(stride * frame_info_.height);
        }
    }

    [[nodiscard]]
    uint32_t compute_stride() const noexcept
    {
        ASSERT(size_set_ && pixel_format_set_);
        return compute_stride(frame_info_);
    }

    [[nodiscard]]
    static uint32_t compute_stride(const charls::frame_info& frame_info) noexcept
    {
        uint32_t stride;
        if (frame_info.bits_per_sample <= 8)
        {
            const uint32_t samples_per_byte{8U / frame_info.bits_per_sample};
            stride = ((frame_info.width * frame_info.component_count) + (samples_per_byte - 1)) / samples_per_byte;
        }
        else
        {
            stride = frame_info.width * 2 * frame_info.component_count;
        }

        // Windows bitmaps are always DWORD (4 bytes) aligned per scan line.
        constexpr uint32_t alignment{4};
        return ((stride + (alignment - 1)) / alignment) * alignment;
    }

    void convert_bgr_to_rgb() noexcept
    {
        for (size_t i{}; i < source_.size(); i += 3)
        {
            std::swap(source_[i], source_[i + 2]);
        }
    }

    void unpack_crumbs()
    {
        const std::byte* crumbs_pixels{source_.data()};
        const size_t stride{compute_stride()};
        const size_t width{frame_info_.width};
        const size_t height{frame_info_.height};
        std::vector<std::byte> unpacked(width * height);

        for (size_t j{}, row{}; row != height; ++row)
        {
            const std::byte* crumbs_row{crumbs_pixels + (row * stride)};
            size_t i{};
            for (; i != width / 4; ++i)
            {
                unpacked[j++] = crumbs_row[i] >> 6;
                unpacked[j++] = (crumbs_row[i] & std::byte{0x30}) >> 4;
                unpacked[j++] = (crumbs_row[i] & std::byte{0x0C}) >> 2;
                unpacked[j++] = crumbs_row[i] & std::byte{0x03};
            }
            switch (width % 4)
            {
            case 3:
                unpacked[j++] = crumbs_row[i] >> 6;
                [[fallthrough]];
            case 2:
                unpacked[j++] = (crumbs_row[i] & std::byte{0x30}) >> 4;
                [[fallthrough]];
            case 1:
                unpacked[j++] = (crumbs_row[i] & std::byte{0x0C}) >> 2;
                break;

            default:
                break;
            }
        }

        source_.swap(unpacked);
    }

    void unpack_nibbles()
    {
        const std::byte* nibble_pixels{source_.data()};
        const size_t stride{compute_stride()};
        const size_t width{frame_info_.width};
        const size_t height{frame_info_.height};
        std::vector<std::byte> unpacked(width * height);

        for (size_t j{}, row{}; row != height; ++row)
        {
            const std::byte* nibble_row{nibble_pixels + (row * stride)};
            size_t i{};
            for (; i != width / 2; ++i)
            {
                unpacked[j++] = nibble_row[i] >> 4;
                unpacked[j++] = nibble_row[i] & std::byte{0x0F};
            }
            if (width % 2)
            {
                unpacked[j++] = nibble_row[i] >> 4;
            }
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
    uint32_t source_stride_{};
    std::vector<std::byte> source_;
    charls::frame_info frame_info_{};
    double dpi_x_{};
    double dpi_y_{};
};
