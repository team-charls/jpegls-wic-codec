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

        if (!can_decode_to_wic_pixel_format(decoder_.metadata_info().bits_per_sample, decoder_.metadata_info().component_count))
            winrt::throw_hresult(WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT);
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

    HRESULT GetPixelFormat(GUID* pixel_format) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetPixelFormat, instance=%p, pixel_format=%p\n", this, pixel_format);

        if (!pixel_format)
            return E_POINTER;

        WINRT_VERIFY(try_get_get_pixel_format(decoder_.metadata_info().bits_per_sample,
            decoder_.metadata_info().component_count, *pixel_format));
        return S_OK;
    }

    HRESULT GetResolution(double* dpi_x, double* dpi_y) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetResolution, instance=%p,  dpi_x=%p, dpi_y=%p\n", this, dpi_x, dpi_y);

        if (!dpi_x || !dpi_y)
            return E_POINTER;

        // TODO: check JPEG-LS standard if dpi is in header.

        // Let's assume square pixels. 96dpi seems to be a reasonable default.
        *dpi_x = 96;
        *dpi_y = 96;
        return S_OK;
    }

    HRESULT CopyPixels(const WICRect* /*rectangle*/, uint32_t /*stride*/, uint32_t buffer_size, BYTE* buffer) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::CopyPixels, instance=%p, buffer_size=%d, buffer=%p\n", this, buffer_size, buffer);

        std::error_code error;
        decoder_.decode(buffer, buffer_size, error);
        if (error)
            return WINCODEC_ERR_BADIMAGE;

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
        TRACE("jpegls_bitmap_frame_decoder::GetColorContexts, instance=%p, count=%d, color_contexts=%p, actual_count=%p\n", this, count, color_contexts, actual_count);
        if (!actual_count)
            return E_POINTER;

        *actual_count = 0;
        return S_OK;
    }

    HRESULT GetMetadataQueryReader([[maybe_unused]] IWICMetadataQueryReader** metadata_query_reader) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetMetadataQueryReader, instance=%p, metadata_query_reader=%p\n", this, metadata_query_reader);

        // Keep the initial design simple: no support for metadata.
        return  WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    static bool can_decode_to_wic_pixel_format(int32_t bits_per_sample, int32_t component_count)
    {
        GUID pixel_format_dummy;
        return try_get_get_pixel_format(bits_per_sample, component_count, pixel_format_dummy);
    }

    static bool try_get_get_pixel_format(int32_t bits_per_sample, int32_t component_count, GUID& pixel_format)
    {
        switch (component_count)
        {
        case 1:
            switch (bits_per_sample)
            {
            case 2:
                pixel_format = GUID_WICPixelFormat2bppGray;
                return true;
            case 4:
                pixel_format = GUID_WICPixelFormat4bppGray;
                return true;
            case 8:
                pixel_format = GUID_WICPixelFormat8bppGray;
                return true;
            case 16:
                pixel_format = GUID_WICPixelFormat16bppGray;
                return true;
            }
        }

        return false;
    }

private:
    size_t buffer_size_;
    std::unique_ptr<std::byte[]> buffer_;
    charls::decoder decoder_;
    std::mutex mutex_;
};
