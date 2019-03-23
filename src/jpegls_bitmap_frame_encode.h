// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include "trace.h"

#include <charls/jpegls_encoder.h>

struct jpegls_bitmap_frame_encode final : winrt::implements<jpegls_bitmap_frame_encode, IWICBitmapFrameEncode>
{
    explicit jpegls_bitmap_frame_encode(const winrt::com_ptr<IStream>& destination) noexcept :
        destination_{destination}
    {
    }

    // Required methods
    HRESULT Initialize([[maybe_unused]] _In_ IPropertyBag2* encoder_options) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::Initialize, instance=%p, encoder_options=%p\n", this, encoder_options);
        return S_OK;
    }

    HRESULT SetSize(const uint32_t width, const uint32_t height) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetSize, instance=%p, width=%u, height=%u\n", this, width, height);

        metadata_.width = width;
        metadata_.height = height;

        return S_OK;
    }

    HRESULT SetResolution([[maybe_unused]] const double dpi_x, [[maybe_unused]] const double dpi_y) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetResolution, instance=%p, dpi_x=%f, dpi_y=%f\n", this, dpi_x, dpi_y);
        return E_FAIL;
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
            metadata_.bits_per_sample = 8;
            metadata_.component_count = 1;
            return S_OK;
        }

        if (*pixel_format == GUID_WICPixelFormat16bppGray)
        {
            metadata_.bits_per_sample = 16;
            metadata_.component_count = 1;
            return S_OK;
        }

        if (*pixel_format == GUID_WICPixelFormat24bppRGB)
        {
            metadata_.bits_per_sample = 8;
            metadata_.component_count = 3;
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

    HRESULT WritePixels(UINT /*lineCount*/,  UINT /*cbStride*/,  UINT /*cbBufferSize*/,  BYTE* /*pbPixels*/) noexcept override
    {
        return E_FAIL;
    }

    HRESULT WriteSource(IWICBitmapSource* /*pIWICBitmapSource*/,  WICRect* /*prc*/) noexcept override
    {
        return E_FAIL;
    }

    HRESULT Commit() noexcept override
    {
        return E_FAIL;
    }

    HRESULT SetPalette([[maybe_unused]] _In_ IWICPalette* palette) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetPalette, instance=%p, palette=%p\n", this, palette);
        return WINCODEC_ERR_PALETTEUNAVAILABLE;
    }

private:
    bool initialized_called_{};
    winrt::com_ptr<IStream> destination_;
    charls::metadata metadata_{};
    charls::jpegls_encoder encoder_;
};
