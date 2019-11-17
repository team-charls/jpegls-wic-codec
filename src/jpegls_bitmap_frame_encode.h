// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include "trace.h"
#include "util.h"

#include <charls/charls.h>

struct jpegls_bitmap_frame_encode final : winrt::implements<jpegls_bitmap_frame_encode, IWICBitmapFrameEncode>
{
    [[nodiscard]] const charls::frame_info& frame_info() const noexcept
    {
        ASSERT(state_ == state::commited);
        return frame_info_;
    }

    [[nodiscard]] const std::vector<BYTE>& source() const noexcept
    {
        ASSERT(state_ == state::commited);
        return source_;
    }

    [[nodiscard]] bool is_dpi_set() const noexcept
    {
        ASSERT(state_ == state::commited);
        return dpi_set;
    }

    [[nodiscard]] double dpi_x() const noexcept
    {
        ASSERT(dpi_set && state_ == state::commited);
        return dpi_x_;
    }

    [[nodiscard]] double dpi_y() const noexcept
    {
        ASSERT(dpi_set && state_ == state::commited);
        return dpi_y_;
    }

    HRESULT Initialize([[maybe_unused]] _In_ IPropertyBag2* encoder_options) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::Initialize.1, instance=%p, encoder_options=%p\n", this, encoder_options);

        if (state_ != state::created)
        {
            TRACE("jpegls_bitmap_frame_encode::Initialize.2, instance=%p, failed with WINCODEC_ERR_WRONGSTATE\n", this);
            return WINCODEC_ERR_WRONGSTATE;
        }

        state_ = state::initialized;
        return S_OK;
    }

    HRESULT SetSize(const uint32_t width, const uint32_t height) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetSize.1, instance=%p, width=%u, height=%u\n", this, width, height);

        if (state_ != state::initialized)
        {
            TRACE("jpegls_bitmap_frame_encode::SetSize.2, instance=%p, failed with WINCODEC_ERR_WRONGSTATE\n", this);
            return WINCODEC_ERR_WRONGSTATE;
        }

        frame_info_.width = width;
        frame_info_.height = height;
        size_set_ = true;

        return S_OK;
    }

    HRESULT SetResolution(const double dpi_x, const double dpi_y) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetResolution.1, instance=%p, dpi_x=%f, dpi_y=%f\n", this, dpi_x, dpi_y);

        if (state_ == state::commited)
        {
            TRACE("jpegls_bitmap_frame_encode::SetResolution.2, instance=%p, failed with WINCODEC_ERR_WRONGSTATE\n", this);
            return WINCODEC_ERR_WRONGSTATE;
        }

        dpi_x_ = dpi_x;
        dpi_y_ = dpi_y;
        dpi_set = true;

        return S_OK;
    }

    HRESULT SetPixelFormat(GUID* pixel_format) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetPixelFormat, instance=%p, pixel_format=%p\n", this, pixel_format);
        if (!pixel_format)
            return E_INVALIDARG;

        if (state_ != state::initialized)
            return WINCODEC_ERR_WRONGSTATE;

        swap_pixels_ = false;

        if (*pixel_format == GUID_WICPixelFormat8bppGray)
        {
            set_pixel_format(8, 1);
            return S_OK;
        }

        if (*pixel_format == GUID_WICPixelFormat16bppGray)
        {
            set_pixel_format(16, 1);
            return S_OK;
        }

        if (*pixel_format == GUID_WICPixelFormat24bppRGB)
        {
            set_pixel_format(8, 3);
            return S_OK;
        }

        if (*pixel_format == GUID_WICPixelFormat24bppBGR)
        {
            set_pixel_format(8, 3);
            swap_pixels_ = true;
            return S_OK;
        }

        *pixel_format = GUID_WICPixelFormatUndefined;
        return WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT;
    }

    HRESULT SetColorContexts([[maybe_unused]] const uint32_t count, [[maybe_unused]] IWICColorContext** color_context) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetColorContexts, instance=%p, count=%d, color_context=%p\n", this, count, color_context);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT GetMetadataQueryWriter(IWICMetadataQueryWriter** /*ppIMetadataQueryWriter*/) noexcept override
    {
        return E_FAIL;
    }

    HRESULT SetThumbnail([[maybe_unused]] _In_ IWICBitmapSource* thumbnail) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetThumbnail, instance=%p, thumbnail=%p\n", this, thumbnail);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT WritePixels(const uint32_t line_count, const uint32_t stride, UINT /*cbBufferSize*/, BYTE* pixels) noexcept override
    {
        if (!(state_ == state::initialized || state_ == state::received_pixels) || !size_set_ || !pixel_format_set_)
            return WINCODEC_ERR_WRONGSTATE;

        if (received_line_count_ + line_count > frame_info_.height)
            return WINCODEC_ERR_CODECTOOMANYSCANLINES;

        try
        {
            if (source_.empty())
            {
                source_.resize(static_cast<size_t>(frame_info_.width) * frame_info_.height * frame_info_.component_count);
            }

            BYTE* source = pixels;
            uint8_t* destination = source_.data() + (static_cast<size_t>(received_line_count_) * frame_info_.width * frame_info_.component_count);
            for (uint32_t i = 0; i < line_count; ++i)
            {
                const size_t bytes_to_copy = static_cast<size_t>(frame_info_.width) * frame_info_.component_count;
                memcpy(destination, source, bytes_to_copy);

                source += stride;
                destination += bytes_to_copy;
            }

            received_line_count_ += line_count;
            state_ = state::received_pixels;
            return S_OK;
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    HRESULT WriteSource(_In_ IWICBitmapSource* bitmap_source, [[maybe_unused]] _In_ WICRect* rectangle) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::WriteSource, instance=%p, bitmap_source=%p, rectangle=%p\n", this, bitmap_source, rectangle);
        if (!bitmap_source)
            return E_INVALIDARG;

        if (state_ != state::initialized)
            return WINCODEC_ERR_WRONGSTATE;

        try
        {
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
                source_.resize(static_cast<size_t>(frame_info_.width) * frame_info_.height * frame_info_.component_count);
            }

            winrt::check_hresult(bitmap_source->CopyPixels(nullptr, stride(), static_cast<uint32_t>(source_.size()), source_.data()));
            state_ = state::received_pixels;
            return S_OK;
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    HRESULT Commit() noexcept override
    {
        if (state_ != state::received_pixels || !size_set_ || !pixel_format_set_)
            return WINCODEC_ERR_WRONGSTATE;

        if (swap_pixels_)
        {
            convert_bgr_to_rgb();
        }

        state_ = state::commited;
        return S_OK;
    }

    HRESULT SetPalette([[maybe_unused]] _In_ IWICPalette* palette) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetPalette, instance=%p, palette=%p\n", this, palette);
        return WINCODEC_ERR_PALETTEUNAVAILABLE;
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
        return frame_info_.width * frame_info_.component_count;
    }

    void convert_bgr_to_rgb() noexcept
    {
        for (size_t i = 0; i < source_.size(); i += 3)
        {
            const BYTE b = source_[i];
            const BYTE g = source_[i + 1];
            const BYTE r = source_[i + 2];
            source_[i] = r;
            source_[i + 1] = g;
            source_[i + 2] = b;
        }
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
    bool dpi_set{};
    bool swap_pixels_{};
    uint32_t received_line_count_{};
    std::vector<BYTE> source_;
    charls::frame_info frame_info_{};
    double dpi_x_{};
    double dpi_y_{};
};
