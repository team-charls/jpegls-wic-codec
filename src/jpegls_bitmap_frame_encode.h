// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include "errors.h"
#include "trace.h"
#include "util.h"

#include <charls/charls.h>

#include <mfapi.h>

class jpegls_bitmap_frame_encode final : public winrt::implements<jpegls_bitmap_frame_encode, IWICBitmapFrameEncode>
{
public:
    [[nodiscard]] const charls::frame_info& frame_info() const noexcept
    {
        ASSERT(state_ == state::commited);
        return frame_info_;
    }

    [[nodiscard]] const std::vector<std::byte>& source() const noexcept
    {
        ASSERT(state_ == state::commited);
        return source_;
    }

    [[nodiscard]] bool is_dpi_set() const noexcept
    {
        ASSERT(state_ == state::commited);
        return dpi_set_;
    }

    [[nodiscard]] double dpi_x() const noexcept
    {
        ASSERT(dpi_set_ && state_ == state::commited);
        return dpi_x_;
    }

    [[nodiscard]] double dpi_y() const noexcept
    {
        ASSERT(dpi_set_ && state_ == state::commited);
        return dpi_y_;
    }

    HRESULT __stdcall Initialize([[maybe_unused]] _In_ IPropertyBag2* encoder_options) noexcept override
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

    HRESULT __stdcall SetSize(const uint32_t width, const uint32_t height) noexcept override
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

    HRESULT __stdcall SetResolution(const double dpi_x, const double dpi_y) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_frame_encode::SetResolution.1, dpi_x={}, dpi_y={}\n", fmt::ptr(this), dpi_x, dpi_y);

        check_condition(!(state_ == state::commited), wincodec::error_wrong_state);
        dpi_x_ = dpi_x;
        dpi_y_ = dpi_y;
        dpi_set_ = true;

        return error_ok;
    }
    catch (...)
    {
        return winrt::to_hresult();
    }

    HRESULT __stdcall SetPixelFormat(GUID* pixel_format) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_frame_encode::SetPixelFormat, pixel_format={}\n", fmt::ptr(this), fmt::ptr(pixel_format));

        check_in_pointer(pixel_format);
        check_condition(state_ == state::initialized, wincodec::error_wrong_state);

        swap_pixels_ = false;

        if (*pixel_format == GUID_WICPixelFormat2bppGray)
        {
            set_pixel_format(2, 1);
            return wincodec::error_unsupported_pixel_format;
        }

        if (*pixel_format == GUID_WICPixelFormat4bppGray)
        {
            set_pixel_format(4, 1);
            return wincodec::error_unsupported_pixel_format;
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
            return wincodec::error_unsupported_pixel_format;
        }

        *pixel_format = GUID_WICPixelFormatUndefined;
        return wincodec::error_unsupported_pixel_format;
    }
    catch (...)
    {
        return winrt::to_hresult();
    }

    HRESULT __stdcall SetColorContexts([[maybe_unused]] const uint32_t count,
                                       [[maybe_unused]] IWICColorContext** color_context) noexcept override
    {
        TRACE("{} jpegls_bitmap_frame_encode::SetColorContexts, count={}, color_context={}\n", fmt::ptr(this), count,
              fmt::ptr(color_context));
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall GetMetadataQueryWriter(IWICMetadataQueryWriter** /* writer */) noexcept override
    {
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall SetThumbnail([[maybe_unused]] _In_ IWICBitmapSource* thumbnail) noexcept override
    {
        TRACE("{} jpegls_bitmap_frame_encode::SetThumbnail, thumbnail={}\n", fmt::ptr(this), fmt::ptr(thumbnail));
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall WritePixels(const uint32_t line_count, const uint32_t source_stride, uint32_t /* buffer_size */,
                                  BYTE* pixels) noexcept override
    try
    {
        check_condition((state_ == state::initialized || state_ == state::received_pixels) && size_set_ && pixel_format_set_,
                        wincodec::error_wrong_state);
        check_condition(received_line_count_ + line_count <= frame_info_.height, wincodec::error_codec_too_many_scan_lines);

        if (source_.empty())
        {
            source_.resize(static_cast<size_t>(frame_info_.width) * frame_info_.height * frame_info_.component_count);
        }

        const size_t destination_stride{stride()};
        std::byte* destination{source_.data() + (received_line_count_ * destination_stride)};
        winrt::check_hresult(MFCopyImage(reinterpret_cast<BYTE*>(destination), static_cast<LONG>(destination_stride), pixels,
                                         static_cast<LONG>(source_stride), static_cast<DWORD>(destination_stride),
                                         line_count));

        received_line_count_ += line_count;
        state_ = state::received_pixels;
        return error_ok;
    }
    catch (...)
    {
        return winrt::to_hresult();
    }

    HRESULT __stdcall WriteSource(_In_ IWICBitmapSource* bitmap_source,
                                  [[maybe_unused]] _In_ WICRect* rectangle) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_frame_encode::WriteSource, bitmap_source={}, rectangle={}\n", fmt::ptr(this),
              fmt::ptr(bitmap_source), fmt::ptr(rectangle));

        check_in_pointer(bitmap_source);
        check_condition(state_ == state::initialized, wincodec::error_wrong_state);

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

        if (source_.empty())
        {
            size_t size{static_cast<size_t>(frame_info_.width) * frame_info_.height * frame_info_.component_count};
            if (frame_info_.bits_per_sample < 8)
            {
                // In a WIC bitmap pixels are packed, if bits per sample allows it.
                size /= 2;
            }

            source_.resize(size);
        }

        uint32_t s{stride()};
        if (frame_info_.bits_per_sample < 8)
        {
            s /= 2;
        }

        winrt::check_hresult(bitmap_source->CopyPixels(nullptr, s, static_cast<uint32_t>(source_.size()),
                                                       reinterpret_cast<BYTE*>(source_.data())));
        state_ = state::received_pixels;
        return error_ok;
    }
    catch (...)
    {
        return winrt::to_hresult();
    }

    HRESULT __stdcall Commit() noexcept override
    try
    {
        check_condition(state_ == state::received_pixels && size_set_ && pixel_format_set_, wincodec::error_wrong_state);

        if (swap_pixels_)
        {
            convert_bgr_to_rgb();
        }
        else if (frame_info_.bits_per_sample < 8)
        {
            unpack_nibbles();
        }

        state_ = state::commited;
        return error_ok;
    }
    catch (...)
    {
        return winrt::to_hresult();
    }

    HRESULT __stdcall SetPalette([[maybe_unused]] _In_ IWICPalette* palette) noexcept override
    {
        TRACE("{} jpegls_bitmap_frame_encode::SetPalette, palette={}\n", fmt::ptr(this), fmt::ptr(palette));
        return wincodec::error_palette_unavailable;
    }

private:
    void set_pixel_format(const int32_t bits_per_sample, const int32_t component_count) noexcept
    {
        frame_info_.bits_per_sample = bits_per_sample;
        frame_info_.component_count = component_count;
        pixel_format_set_ = true;
    }

    [[nodiscard]] uint32_t stride() const noexcept
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
