// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include "trace.h"
#include "util.h"

#include <charls/jpegls_decoder.h>

#include <wincodec.h>
#include <shlwapi.h>
#include <winrt/base.h>

#include <memory>

struct jpegls_bitmap_frame_decode final : winrt::implements<jpegls_bitmap_frame_decode, IWICBitmapFrameDecode>
{
    explicit jpegls_bitmap_frame_decode(IStream* stream, IWICImagingFactory* factory)
    {
        ULARGE_INTEGER size;
        winrt::check_hresult(IStream_Size(stream, &size));

        WARNING_SUPPRESS(26409) // use std::make_unique (explictly not used to prevent unneeded initialization of the buffer)
        buffer_size_ = size.LowPart;
        buffer_ = std::unique_ptr<std::byte[]>(new std::byte[buffer_size_]);
        WARNING_UNSUPPRESS()

        winrt::check_hresult(IStream_Read(stream, buffer_.get(), static_cast<ULONG>(buffer_size_)));

        std::error_code error;
        decoder_.read_header(buffer_.get(), buffer_size_, error);
        if (error)
            winrt::throw_hresult(WINCODEC_ERR_BADHEADER);

        const charls::metadata_info_t& metadata_info = decoder_.metadata_info();
        GUID pixel_format;
        if (!try_get_get_pixel_format(metadata_info.bits_per_sample, metadata_info.component_count, pixel_format))
            winrt::throw_hresult(WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT);

        winrt::com_ptr<IWICBitmap> bitmap;
        winrt::check_hresult(factory->CreateBitmap(metadata_info.width, metadata_info.height, pixel_format, WICBitmapCacheOnLoad, bitmap.put()));
        winrt::check_hresult(bitmap->SetResolution(96, 96));

        {
            winrt::com_ptr<IWICBitmapLock> bitmap_lock;
            WICRect complete_image{ 0, 0, metadata_info.width, metadata_info.height };
            winrt::check_hresult(bitmap->Lock(&complete_image, WICBitmapLockWrite, bitmap_lock.put()));

            uint32_t stride;
            bitmap_lock->GetStride(&stride);
            if (stride != compute_stride(metadata_info))
            {
                WINRT_ASSERT(false);
                winrt::throw_hresult(WINCODEC_ERR_BADIMAGE);
            }

            BYTE* data_buffer;
            uint32_t data_buffer_size;
            winrt::check_hresult(bitmap_lock->GetDataPointer(&data_buffer_size, &data_buffer));

            decoder_.decode(data_buffer, data_buffer_size, error);
            if (error)
                winrt::throw_hresult(WINCODEC_ERR_BADIMAGE);
        }

        winrt::check_hresult(bitmap->QueryInterface(bitmap_source_.put()));
    }

    // IWICBitmapSource
    HRESULT GetSize(uint32_t* width, uint32_t* height) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetSize, instance=%p, width=%p, height=%p\n", this, width, height);
        return bitmap_source_->GetSize(width, height);
    }

    HRESULT GetPixelFormat(GUID* pixel_format) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetPixelFormat, instance=%p, pixel_format=%p\n", this, pixel_format);
        return bitmap_source_->GetPixelFormat(pixel_format);
    }

    HRESULT GetResolution(double* dpi_x, double* dpi_y) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetResolution, instance=%p,  dpi_x=%p, dpi_y=%p\n", this, dpi_x, dpi_y);
        return bitmap_source_->GetResolution(dpi_x, dpi_y);
    }

    HRESULT CopyPixels(const WICRect* rectangle, uint32_t stride, uint32_t buffer_size, BYTE* buffer) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::CopyPixels, instance=%p, rectangle=%p, buffer_size=%d, buffer=%p\n", this, rectangle, buffer_size, buffer);
        return bitmap_source_->CopyPixels(rectangle, stride, buffer_size, buffer);
    }

    HRESULT CopyPalette(IWICPalette*) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::CopyPalette, instance=%p\n", this);
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

private:
    static bool try_get_get_pixel_format(int32_t bits_per_sample, int32_t component_count, GUID& pixel_format) noexcept
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
            break;

        case 3:
            switch (bits_per_sample)
            {
            case 8:
                pixel_format = GUID_WICPixelFormat24bppRGB;
                return true;
            case 16:
                pixel_format = GUID_WICPixelFormat48bppRGB;
                return true;
            }
        }

        return false;
    }

    static uint32_t compute_stride(const charls::metadata_info_t& metadata_info) noexcept
    {
        return metadata_info.width * ((metadata_info.bits_per_sample + 7) / 8) * metadata_info.component_count;
    }

    size_t buffer_size_;
    std::unique_ptr<std::byte[]> buffer_;
    charls::decoder decoder_;
    winrt::com_ptr<IWICBitmapSource> bitmap_source_;
};
