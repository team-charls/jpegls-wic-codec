// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include "trace.h"
#include "util.h"

#include <charls/charls.h>

struct jpegls_bitmap_frame_encode final : winrt::implements<jpegls_bitmap_frame_encode, IWICBitmapFrameEncode>
{
    const charls::frame_info& frame_info() const noexcept
    {
        return frame_info_;
    }

    const std::vector<BYTE>& source() const noexcept
    {
        return source_;
    }

    // Required methods
    HRESULT Initialize([[maybe_unused]] _In_ IPropertyBag2* encoder_options) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::Initialize, instance=%p, encoder_options=%p\n", this, encoder_options);
        initialized_called_ = true;
        return S_OK;
    }

    HRESULT SetSize(const uint32_t width, const uint32_t height) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetSize, instance=%p, width=%u, height=%u\n", this, width, height);

        frame_info_.width = width;
        frame_info_.height = height;
        size_set_ = true;

        return S_OK;
    }

    HRESULT SetResolution(const double dpi_x, const double dpi_y) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetResolution, instance=%p, dpi_x=%f, dpi_y=%f\n", this, dpi_x, dpi_y);

        dpi_x_ = dpi_x;
        dpi_y_ = dpi_y;

        return S_OK;
    }

    HRESULT SetPixelFormat(GUID* pixel_format) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetPixelFormat, instance=%p, pixel_format=%p\n", this, pixel_format);
        if (!pixel_format)
            return E_INVALIDARG;

        if (!initialized_called_)
            return WINCODEC_ERR_WRONGSTATE;

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

    HRESULT WritePixels(UINT /*lineCount*/, UINT /*cbStride*/, UINT /*cbBufferSize*/, BYTE* /*pbPixels*/) noexcept override
    {
        return E_FAIL;
    }

    HRESULT WriteSource(_In_ IWICBitmapSource* bitmap_source, [[maybe_unused]] _In_ WICRect* rectangle) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::WriteSource, instance=%p, bitmap_source=%p, rectangle=%p\n", this, bitmap_source, rectangle);
        if (!bitmap_source)
            return E_INVALIDARG;

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
            return S_OK;
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    HRESULT Commit() noexcept override
    {
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

    uint32_t stride() const noexcept
    {
        ASSERT(size_set_ && pixel_format_set_);
        return frame_info_.width * frame_info_.component_count;
    }

    bool initialized_called_{};
    bool size_set_{};
    bool pixel_format_set_{};
    std::vector<BYTE> source_;
    charls::frame_info frame_info_{};
    charls::jpegls_encoder encoder_;
    double dpi_x_{};
    double dpi_y_{};
};
