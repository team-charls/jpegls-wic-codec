// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include "trace.h"
#include "util.h"

#include <charls/charls.h>

#include <Shlwapi.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <memory>

struct jpegls_bitmap_frame_decode final : winrt::implements<jpegls_bitmap_frame_decode, IWICBitmapFrameDecode, IWICBitmapSource>
{
    explicit jpegls_bitmap_frame_decode(IStream* stream, IWICImagingFactory* factory)
    {
        ULARGE_INTEGER size;
        winrt::check_hresult(IStream_Size(stream, &size));

        const storage_buffer buffer{size.LowPart};
        winrt::check_hresult(IStream_Read(stream, buffer.data(), static_cast<ULONG>(buffer.size())));

        charls::jpegls_decoder decoder;
        decoder.source(buffer);

        try
        {
            decoder.read_header();
        }
        catch (charls::jpegls_error&)
        {
            winrt::throw_hresult(WINCODEC_ERR_BADHEADER);
        }

        const auto& frame_info = decoder.frame_info();
        GUID pixel_format;
        if (!try_get_pixel_format(frame_info.bits_per_sample, frame_info.component_count, pixel_format))
            winrt::throw_hresult(WINCODEC_ERR_UNSUPPORTEDPIXELFORMAT);

        winrt::com_ptr<IWICBitmap> bitmap;
        winrt::check_hresult(factory->CreateBitmap(frame_info.width, frame_info.height, pixel_format, WICBitmapCacheOnLoad, bitmap.put()));
        winrt::check_hresult(bitmap->SetResolution(96, 96));

        {
            winrt::com_ptr<IWICBitmapLock> bitmap_lock;
            WICRect complete_image{0, 0, static_cast<int32_t>(frame_info.width), static_cast<int32_t>(frame_info.height)};
            winrt::check_hresult(bitmap->Lock(&complete_image, WICBitmapLockWrite, bitmap_lock.put()));

            uint32_t stride;
            winrt::check_hresult(bitmap_lock->GetStride(&stride));
            if (stride != compute_stride(frame_info))
            {
                ASSERT(false);
                winrt::throw_hresult(WINCODEC_ERR_BADIMAGE);
            }

            BYTE* data_buffer;
            uint32_t data_buffer_size;
            winrt::check_hresult(bitmap_lock->GetDataPointer(&data_buffer_size, &data_buffer));

            decoder.decode(data_buffer, data_buffer_size);
        }

        winrt::check_hresult(bitmap->QueryInterface(bitmap_source_.put()));
    }

    // IWICBitmapSource
    HRESULT GetSize(uint32_t* width, uint32_t* height) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetSize, instance=%p, width=%p, height=%p\n", this, width, height);

        WARNING_SUPPRESS(26447) // noexcept: COM methods are not defined as noexcept
        return bitmap_source_->GetSize(width, height);
        WARNING_UNSUPPRESS()
    }

    HRESULT GetPixelFormat(GUID* pixel_format) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetPixelFormat, instance=%p, pixel_format=%p\n", this, pixel_format);

        WARNING_SUPPRESS(26447) // noexcept: COM methods are not defined as noexcept
        return bitmap_source_->GetPixelFormat(pixel_format);
        WARNING_UNSUPPRESS()
    }

    HRESULT GetResolution(double* dpi_x, double* dpi_y) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::GetResolution, instance=%p,  dpi_x=%p, dpi_y=%p\n", this, dpi_x, dpi_y);

        WARNING_SUPPRESS(26447) // noexcept: COM methods are not defined as noexcept
        return bitmap_source_->GetResolution(dpi_x, dpi_y);
        WARNING_UNSUPPRESS()
    }

    HRESULT CopyPixels(const WICRect* rectangle, const uint32_t stride, const uint32_t buffer_size, BYTE* buffer) noexcept override
    {
        TRACE("jpegls_bitmap_frame_decoder::CopyPixels, instance=%p, rectangle=%p, buffer_size=%d, buffer=%p\n", this, rectangle, buffer_size, buffer);

        WARNING_SUPPRESS(26447) // noexcept: COM methods are not defined as noexcept
        return bitmap_source_->CopyPixels(rectangle, stride, buffer_size, buffer);
        WARNING_UNSUPPRESS()
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
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    static bool can_decode_to_wic_pixel_format(const int32_t bits_per_sample, const int32_t component_count) noexcept
    {
        GUID pixel_format_dummy;
        return try_get_pixel_format(bits_per_sample, component_count, pixel_format_dummy);
    }

private:
    static bool try_get_pixel_format(const int32_t bits_per_sample, const int32_t component_count, GUID& pixel_format) noexcept
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
            default:
                break;
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
            default:
                break;
            }
            break;

        default:
            break;
        }

        return false;
    }

    static uint32_t compute_stride(const charls::frame_info& frame_info) noexcept
    {
        return frame_info.width * ((frame_info.bits_per_sample + 7) / 8) * frame_info.component_count;
    }

    winrt::com_ptr<IWICBitmapSource> bitmap_source_;

    // purpose: replacement container that doesn't initialize its content (provided in C++20)
    struct storage_buffer final
    {
        using value_type = std::byte;

        explicit storage_buffer(const size_t size) :
            size_{size},
            buffer_{std::make_unique<std::byte[]>(size)}
        {
        }

        ~storage_buffer() = default;
        storage_buffer(const storage_buffer&) = delete;
        storage_buffer(storage_buffer&&) = delete;
        storage_buffer& operator=(const storage_buffer&) = delete;
        storage_buffer& operator=(storage_buffer&&) = delete;

        [[nodiscard]] size_t size() const noexcept
        {
            return size_;
        }

        [[nodiscard]] std::byte* data() const noexcept
        {
            return buffer_.get();
        }

    private:
        size_t size_;
        std::unique_ptr<std::byte[]> buffer_;
    };
};
