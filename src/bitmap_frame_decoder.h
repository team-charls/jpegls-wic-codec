// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include "trace.h"

#include <winrt/base.h>
#include <wincodec.h>
#include <shlwapi.h>
#include <vector>

struct bitmap_frame_decoder final : winrt::implements<bitmap_frame_decoder, IWICBitmapFrameDecode>
{
    std::vector<std::byte> buffer_;
    charls::decoder decoder_;

    explicit bitmap_frame_decoder(IStream* stream)
    {
        ULARGE_INTEGER size;
        winrt::check_hresult(IStream_Size(stream, &size));

        try
        {
            buffer_.resize(size.LowPart);
            decoder_.read_header(buffer_.data(), buffer_.size());

        }
        catch (...)
        {
            throw winrt::hresult_error(E_FAIL);
        }
    }

    // IWICBitmapSource
    HRESULT GetSize(uint32_t* width, uint32_t* height) noexcept override
    {
        TRACE("(%p, %p)\n", width, height);

        if (!width || !height)
            return E_POINTER;

        *width = decoder_.metadata_info().width;
        *height = decoder_.metadata_info().height;

        return S_OK;
    }

    HRESULT GetPixelFormat(WICPixelFormatGUID* pixelFormat) noexcept override
    {
        if (!pixelFormat)
            return E_POINTER;

        *pixelFormat = GUID_WICPixelFormat16bppGray;
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
