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
        uint32_t sample_shift;
        if (!try_get_pixel_format(frame_info.bits_per_sample, frame_info.component_count, pixel_format, sample_shift))
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
            if (stride < compute_stride(frame_info))
            {
                ASSERT(false);
                winrt::throw_hresult(WINCODEC_ERR_BADIMAGE);
            }

            BYTE* data_buffer;
            uint32_t data_buffer_size;
            winrt::check_hresult(bitmap_lock->GetDataPointer(&data_buffer_size, &data_buffer));
            __assume(data_buffer != nullptr);

            if (frame_info.component_count != 1 && decoder.interleave_mode() == charls::interleave_mode::none)
            {
                auto planar = decoder.decode<std::vector<std::byte>>();
                convert_planar_to_rgb(frame_info.width, frame_info.height, planar.data(), data_buffer, stride);
            }
            else
            {
                decoder.decode(data_buffer, data_buffer_size, stride);
            }

            if (sample_shift != 0)
            {
                shift_samples(data_buffer, data_buffer_size / 2, sample_shift);
            }
        }

        winrt::check_hresult(bitmap->QueryInterface(bitmap_source_.put()));
    }

    // IWICBitmapSource
    HRESULT __stdcall GetSize(uint32_t* width, uint32_t* height) noexcept override
    {
        TRACE("%p jpegls_bitmap_frame_decoder::GetSize.1, width=%p, height=%p\n", this, width, height);

        WARNING_SUPPRESS(26447) // noexcept: COM methods are not defined as noexcept
        const auto result = bitmap_source_->GetSize(width, height);
        WARNING_UNSUPPRESS()

        TRACE("%p jpegls_bitmap_frame_decoder::GetSize.2, *width=%u, *height=%u\n", this,
              width ? *width : 0, height ? *height : 0);
        return result;
    }

    HRESULT __stdcall GetPixelFormat(GUID* pixel_format) noexcept override
    {
        TRACE("%p jpegls_bitmap_frame_decoder::GetPixelFormat.1, pixel_format=%p\n", this, pixel_format);

        WARNING_SUPPRESS(26447) // noexcept: COM methods are not defined as noexcept
        const auto result = bitmap_source_->GetPixelFormat(pixel_format);
        WARNING_UNSUPPRESS()

        TRACE("%p jpegls_bitmap_frame_decoder::GetPixelFormat.2, pixel_format=%s\n", this,
              pixel_format ? pixel_format_to_string(*pixel_format) : "");
        return result;
    }

    HRESULT __stdcall GetResolution(double* dpi_x, double* dpi_y) noexcept override
    {
        TRACE("%p jpegls_bitmap_frame_decoder::GetResolution, dpi_x=%p, dpi_y=%p\n", this, dpi_x, dpi_y);

        WARNING_SUPPRESS(26447) // noexcept: COM methods are not defined as noexcept
        return bitmap_source_->GetResolution(dpi_x, dpi_y);
        WARNING_UNSUPPRESS()
    }

    HRESULT __stdcall CopyPixels(const WICRect* rectangle, const uint32_t stride, const uint32_t buffer_size, BYTE* buffer) noexcept override
    {
        TRACE("%p jpegls_bitmap_frame_decoder::CopyPixels, rectangle=%p, buffer_size=%d, buffer=%p\n", this, rectangle, buffer_size, buffer);

        WARNING_SUPPRESS(26447) // noexcept: COM methods are not defined as noexcept
        return bitmap_source_->CopyPixels(rectangle, stride, buffer_size, buffer);
        WARNING_UNSUPPRESS()
    }

    HRESULT __stdcall CopyPalette(IWICPalette*) noexcept override
    {
        TRACE("%p jpegls_bitmap_frame_decoder::CopyPalette\n", this);
        return wincodec::error_palette_unavailable;
    }

    // IWICBitmapFrameDecode : IWICBitmapSource
    HRESULT __stdcall GetThumbnail(IWICBitmapSource**) noexcept override
    {
        return wincodec::error_codec_no_thumbnail;
    }

    HRESULT __stdcall GetColorContexts(const uint32_t count, IWICColorContext** color_contexts, uint32_t* actual_count) noexcept override
    {
        TRACE("%p jpegls_bitmap_frame_decoder::GetColorContexts, count=%d, color_contexts=%p, actual_count=%p\n", this, count, color_contexts, actual_count);
        if (!actual_count)
            return error_pointer;

        *actual_count = 0;
        return error_ok;
    }

    HRESULT __stdcall GetMetadataQueryReader([[maybe_unused]] IWICMetadataQueryReader** metadata_query_reader) noexcept override
    {
        TRACE("%p jpegls_bitmap_decoder::GetMetadataQueryReader, metadata_query_reader=%p\n", this, metadata_query_reader);

        // Keep the initial design simple: no support for metadata.
        return wincodec::error_unsupported_operation;
    }

    static bool can_decode_to_wic_pixel_format(const int32_t bits_per_sample, const int32_t component_count) noexcept
    {
        GUID pixel_format_dummy;
        uint32_t dummy_sample_shift;
        return try_get_pixel_format(bits_per_sample, component_count, pixel_format_dummy, dummy_sample_shift);
    }

private:
    static bool try_get_pixel_format(const int32_t bits_per_sample, const int32_t component_count, GUID& pixel_format, uint32_t& sample_shift) noexcept
    {
        sample_shift = 0;

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

            case 10:
            case 12:
                sample_shift = 16 - bits_per_sample;
                pixel_format = GUID_WICPixelFormat16bppGray;
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

    static void shift_samples(void* buffer, const size_t pixel_count, const uint32_t sample_shift)
    {
        const auto pixels = static_cast<uint16_t*>(buffer);
        std::transform(pixels, pixels + pixel_count, pixels,
                       [sample_shift](const uint16_t pixel) -> uint16_t { return pixel << sample_shift; });
    }

    static void convert_planar_to_rgb(const size_t width, const size_t height, const std::byte* source, void* destination, const size_t rgb_stride) noexcept
    {
        const std::byte* r = source;
        const std::byte* g = r + (width * height);
        const std::byte* b = g + (width * height);

        auto rgb = static_cast<std::byte*>(destination);

        for (size_t row = 0; row < height; ++row)
        {
            for (size_t col = 0, offset = 0; col < width; ++col, offset += 3)
            {
                rgb[offset + 0] = r[col];
                rgb[offset + 1] = g[col];
                rgb[offset + 2] = b[col];
            }

            b += width;
            g += width;
            r += width;
            rgb += rgb_stride;
        }
    }

    // purpose: replacement container that doesn't initialize its content (provided in C++20)
    struct storage_buffer final
    {
        using value_type = std::byte;

        explicit storage_buffer(const size_t size) :
            size_{size},
            buffer_{std::make_unique<std::byte[]>(size)} // NOLINT(cppcoreguidelines-avoid-c-arrays)
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
        std::unique_ptr<std::byte[]> buffer_; // NOLINT(cppcoreguidelines-avoid-c-arrays)
    };

    winrt::com_ptr<IWICBitmapSource> bitmap_source_;
};
