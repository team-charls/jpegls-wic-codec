// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include "trace.h"
#include <winrt/base.h>
#include <wincodec.h>

struct bitmap_frame_decoder final : winrt::implements<bitmap_frame_decoder, IWICBitmapFrameDecode>
{
    // IWICBitmapSource
    HRESULT GetSize(UINT *puiWidth, UINT *puiHeight) noexcept override
    {
        return S_OK;
    }

    HRESULT GetPixelFormat(WICPixelFormatGUID *pPixelFormat) noexcept override
    {
        return S_OK;
    }

    HRESULT GetResolution(double* pDpiX, double* pDpiY) noexcept override
    {
        TRACE("(%p, %p)\n", pDpiX, pDpiY);

        // TODO: check JPEG-LS standard if dpi is in header.

        // Let's assume square pixels. 96dpi seems to be a reasonable default.
        *pDpiX = 96;
        *pDpiY = 96;
        return S_OK;
    }

    HRESULT CopyPixels(const WICRect* prc, UINT cbStride, UINT cbBufferSize, BYTE* pbBuffer) noexcept override
    {
        return S_OK;
    }

    HRESULT CopyPalette(IWICPalette*) noexcept override
    {
        return WINCODEC_ERR_PALETTEUNAVAILABLE;
    }

    // IWICBitmapFrameDecode : IWICBitmapSource
    HRESULT GetThumbnail(IWICBitmapSource**) noexcept override
    {
        return WINCODEC_ERR_CODECNOTHUMBNAIL;
    }

    HRESULT GetColorContexts(const uint32_t count, IWICColorContext** colorContexts, UINT* actualCount) noexcept override
    {
        TRACE("(%d, %p, %p)\n", count, colorContexts, actualCount);
        if (!actualCount)
            return E_POINTER;

        *actualCount = 0;
        return S_OK;
    }

    HRESULT GetMetadataQueryReader(IWICMetadataQueryReader **ppIMetadataQueryReader) noexcept override
    {
        return S_OK;
    }

private:
    std::mutex mutex_;
};
