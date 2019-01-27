// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include "trace.h"

struct jpegls_bitmap_frame_encode final : winrt::implements<jpegls_bitmap_frame_encode, IWICBitmapFrameEncode>
{
    // Required methods
    HRESULT Initialize([[maybe_unused]] _In_ IPropertyBag2* encoder_options) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::Initialize, instance=%p, encoder_options=%p\n", this, encoder_options);
        return S_OK;
    }

    HRESULT SetSize(const uint32_t width, const uint32_t height) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetSize, instance=%p, width=%u, height=%u\n", this, width, height);
        return E_FAIL;
    }

    HRESULT SetResolution([[maybe_unused]] double dpi_x, [[maybe_unused]] double dpi_y) noexcept override
    {
        TRACE("jpegls_bitmap_frame_encode::SetResolution, instance=%p, dpi_x=%f, dpi_y=%f\n", this, dpi_x, dpi_y);
        return E_FAIL;
    }

    HRESULT SetPixelFormat(WICPixelFormatGUID* /*pPixelFormat*/) noexcept override
    {
        return E_FAIL;
    }

    HRESULT SetColorContexts(UINT /*cCount*/,  IWICColorContext** /*ppIColorContext*/) noexcept override
    {
        return E_FAIL;
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
};
