// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include "trace.h"
#include "util.h"

#include <winrt/base.h>
#include <charls/jpegls_decoder.h>

#include <wincodec.h>
#include <shlwapi.h>
#include <memory>

struct jpegls_bitmap_frame_decoder final : winrt::implements<jpegls_bitmap_frame_decoder, IWICBitmapFrameDecode>
{
    explicit jpegls_bitmap_frame_decoder(IStream& stream)
    {
        ULARGE_INTEGER size;
        winrt::check_hresult(IStream_Size(&stream, &size));

        WARNING_SUPPRESS(26409) // use std::make_unique (explictly not used to prevent unneeded initialization of the buffer)
        buffer_size_ = size.LowPart;
        buffer_ = std::unique_ptr<std::byte[]>(new std::byte[buffer_size_]);
        WARNING_UNSUPPRESS()

        winrt::check_hresult(IStream_Read(&stream, buffer_.get(), static_cast<ULONG>(buffer_size_)));

        decoder_.read_header(buffer_.get(), buffer_size_);
    }

    // IWICBitmapSource
    HRESULT GetSize(uint32_t* width, uint32_t* height) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetSize, instance=%p, width=%p, height=%p\n", this, width, height);

        if (!width || !height)
            return E_POINTER;

        *width = decoder_.metadata_info().width;
        *height = decoder_.metadata_info().height;

        return S_OK;
    }

    HRESULT GetPixelFormat(WICPixelFormatGUID* pixel_format) noexcept override
    {
        if (!pixel_format)
            return E_POINTER;

        *pixel_format = GUID_WICPixelFormat16bppGray;
        return S_OK;
    }

    HRESULT GetResolution(double* dpi_x, double* dpi_y) noexcept override
    {
        TRACE("(%p, %p)\n", dpi_x, dpi_y);

        // TODO: check JPEG-LS standard if dpi is in header.

        // Let's assume square pixels. 96dpi seems to be a reasonable default.
        *dpi_x = 96;
        *dpi_y = 96;
        return S_OK;
    }

    HRESULT CopyPixels(const WICRect* /*rectangle*/, uint32_t /*stride*/, uint32_t /*buffer_size*/, BYTE* /*buffer*/) noexcept override
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

    HRESULT GetColorContexts(const uint32_t count, IWICColorContext** color_contexts, uint32_t* actual_count) noexcept override
    {
        TRACE("(%d, %p, %p)\n", count, color_contexts, actual_count);
        if (!actual_count)
            return E_POINTER;

        *actual_count = 0;
        return S_OK;
    }

    HRESULT GetMetadataQueryReader(IWICMetadataQueryReader** /*metadata_query_reader*/) noexcept override
    {
        return S_OK;
    }

private:
    size_t buffer_size_;
    std::unique_ptr<std::byte[]> buffer_;
    charls::decoder decoder_;
    std::mutex mutex_;
};
