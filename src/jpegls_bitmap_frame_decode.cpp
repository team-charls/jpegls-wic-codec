// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "jpegls_bitmap_frame_decode.h"

#include "errors.h"
#include "storage_buffer.h"
#include "trace.h"
#include "util.h"

#include <charls/charls.h>

#include <Shlwapi.h>
#include <wincodec.h>

#include <span>


namespace {

[[nodiscard]]
std::optional<std::pair<GUID, uint32_t>> get_pixel_format(const int32_t bits_per_sample,
                                                          const int32_t component_count) noexcept
{
    switch (component_count)
    {
    case 1:
        switch (bits_per_sample)
        {
        case 2:
            return std::make_pair(GUID_WICPixelFormat2bppGray, 0);

        case 4:
            return std::make_pair(GUID_WICPixelFormat4bppGray, 0);

        case 8:
            return std::make_pair(GUID_WICPixelFormat8bppGray, 0);

        case 10:
        case 12:
        case 16:
            return std::make_pair(GUID_WICPixelFormat16bppGray, 16 - bits_per_sample);

        default:
            break;
        }
        break;

    case 3:
        switch (bits_per_sample)
        {
        case 8:
            return std::make_pair(GUID_WICPixelFormat24bppRGB, 0);

        case 16:
            return std::make_pair(GUID_WICPixelFormat48bppRGB, 0);

        default:
            break;
        }
        break;

    default:
        break;
    }

    return {};
}

void pack_to_crumbs(const std::span<const std::byte> byte_pixels, std::byte* crumb_pixels, const size_t width,
                    const size_t height, const size_t stride) noexcept
{
    size_t j{};
    for (size_t row{}; row != height; ++row)
    {
        std::byte* crumb_row{crumb_pixels + (row * stride)};
        size_t i{};
        for (; i != width / 4; ++i)
        {
            std::byte value{byte_pixels[j++] << 6};
            value |= byte_pixels[j++] << 4;
            value |= byte_pixels[j++] << 2;
            value |= byte_pixels[j++];
            crumb_row[i] = value;
        }

        switch (width % 4)
        {
        case 3:
            crumb_row[i] = byte_pixels[j++] << 6;
            [[fallthrough]];

        case 2:
            crumb_row[i] |= byte_pixels[j++] << 4;
            [[fallthrough]];

        case 1:
            crumb_row[i] |= byte_pixels[j++] << 2;
            break;

        default:
            break;
        }
    }
}

void pack_to_nibbles(const std::span<const std::byte> byte_pixels, std::byte* nibble_pixels, const size_t width,
                     const size_t height, const size_t stride) noexcept
{
    for (size_t j{}, row{}; row != height; ++row)
    {
        std::byte* nibble_row{nibble_pixels + (row * stride)};
        size_t i{};
        for (; i != width / 2; ++i)
        {
            nibble_row[i] = byte_pixels[j++] << 4;
            nibble_row[i] |= byte_pixels[j++];
        }
        if (width % 2)
        {
            nibble_row[i] = byte_pixels[j++] << 4;
        }
    }
}

#ifndef NDEBUG
[[nodiscard]]
uint32_t compute_minimal_stride(const charls::frame_info& frame_info) noexcept
{
    if (frame_info.bits_per_sample == 2)
    {
        const uint32_t stride{(frame_info.width + 3) / 4 * frame_info.component_count};
        return ((stride + 3) / 4) * 4;
    }

    uint32_t stride{frame_info.width * ((frame_info.bits_per_sample + 7) / 8) * frame_info.component_count};
    if (frame_info.bits_per_sample < 8)
    {
        stride /= 2;
    }

    return stride;
}
#endif

void shift_samples(void* buffer, const size_t pixel_count, const uint32_t sample_shift)
{
    auto* const pixels{static_cast<uint16_t*>(buffer)};
    std::transform(pixels, pixels + pixel_count, pixels,
                   [sample_shift](const uint16_t pixel) -> uint16_t { return pixel << sample_shift; });
}

template<typename SizeType>
void convert_planar_to_rgb(const size_t width, const size_t height, const void* source, void* destination,
                           const size_t destination_stride) noexcept
{
    const auto* r{static_cast<const SizeType*>(source)};
    const auto* g{static_cast<const SizeType*>(source) + (width * height)};
    const auto* b{static_cast<const SizeType*>(source) + (width * height * 2)};

    auto* rgb{static_cast<SizeType*>(destination)};

    for (size_t row{}; row != height; ++row)
    {
        for (size_t col{}, offset = 0; col != width; ++col, offset += 3)
        {
            rgb[offset + 0] = r[col];
            rgb[offset + 1] = g[col];
            rgb[offset + 2] = b[col];
        }

        b += width;
        g += width;
        r += width;
        rgb += destination_stride / sizeof(SizeType);
    }
}

} // namespace

jpegls_bitmap_frame_decode::jpegls_bitmap_frame_decode(_In_ IStream* stream, _In_ IWICImagingFactory* factory)
{
    ULARGE_INTEGER size;
    winrt::check_hresult(IStream_Size(stream, &size));

    const storage_buffer buffer{size.LowPart};
    winrt::check_hresult(IStream_Read(stream, buffer.data(), static_cast<ULONG>(buffer.size())));

    charls::jpegls_decoder decoder{buffer, false};

    std::error_code error;
    decoder.read_header(error);
    if (error)
        winrt::throw_hresult(wincodec::error_bad_header);

    const auto& frame_info{decoder.frame_info()};
    auto pixel_format_info{get_pixel_format(frame_info.bits_per_sample, frame_info.component_count)};
    if (!pixel_format_info)
        winrt::throw_hresult(wincodec::error_unsupported_pixel_format);

    const auto& [pixel_format, sample_shift] = pixel_format_info.value();
    winrt::com_ptr<IWICBitmap> bitmap;
    winrt::check_hresult(
        factory->CreateBitmap(frame_info.width, frame_info.height, pixel_format, WICBitmapCacheOnLoad, bitmap.put()));
    winrt::check_hresult(bitmap->SetResolution(96, 96));

    {
        winrt::com_ptr<IWICBitmapLock> bitmap_lock;
        const WICRect complete_image{0, 0, static_cast<int32_t>(frame_info.width), static_cast<int32_t>(frame_info.height)};
        winrt::check_hresult(bitmap->Lock(&complete_image, WICBitmapLockWrite, bitmap_lock.put()));

        uint32_t stride;
        winrt::check_hresult(bitmap_lock->GetStride(&stride));
        ASSERT(stride >= compute_minimal_stride(frame_info));

        std::byte* data_buffer;
        uint32_t data_buffer_size;
        winrt::check_hresult(bitmap_lock->GetDataPointer(&data_buffer_size, reinterpret_cast<BYTE**>(&data_buffer)));
        __assume(data_buffer != nullptr);

        if (frame_info.component_count != 1 && decoder.interleave_mode() == charls::interleave_mode::none)
        {
            const auto planar{decoder.decode<std::vector<std::byte>>()};
            if (frame_info.bits_per_sample > 8)
            {
                convert_planar_to_rgb<uint16_t>(frame_info.width, frame_info.height, planar.data(), data_buffer, stride);
            }
            else
            {
                convert_planar_to_rgb<std::byte>(frame_info.width, frame_info.height, planar.data(), data_buffer, stride);
            }
        }
        else if (frame_info.bits_per_sample == 2)
        {
            std::vector<std::byte> byte_pixels(static_cast<size_t>(frame_info.width) * frame_info.height);
            decoder.decode(byte_pixels);
            pack_to_crumbs(byte_pixels, data_buffer, frame_info.width, frame_info.height, stride);
        }
        else if (frame_info.bits_per_sample == 4)
        {
            std::vector<std::byte> byte_pixels(static_cast<size_t>(frame_info.width) * frame_info.height);
            decoder.decode(byte_pixels);
            pack_to_nibbles(byte_pixels, data_buffer, frame_info.width, frame_info.height, stride);
        }
        else
        {
            decoder.decode(data_buffer, data_buffer_size, stride);

            if (sample_shift != 0)
            {
                shift_samples(data_buffer, data_buffer_size / 2, sample_shift);
            }
        }
    }

    winrt::check_hresult(bitmap->QueryInterface(bitmap_source_.put()));
}

// IWICBitmapSource
HRESULT jpegls_bitmap_frame_decode::GetSize(uint32_t* width, uint32_t* height)
{
    TRACE("{} jpegls_bitmap_frame_decoder::GetSize, width={}, height={}\n", fmt::ptr(this), fmt::ptr(width),
          fmt::ptr(height));

    return bitmap_source_->GetSize(width, height);
}

HRESULT jpegls_bitmap_frame_decode::GetPixelFormat(GUID* pixel_format)
{
    TRACE("{} jpegls_bitmap_frame_decoder::GetPixelFormat, pixel_format={}\n", fmt::ptr(this), fmt::ptr(pixel_format));

    return bitmap_source_->GetPixelFormat(pixel_format);
}

HRESULT jpegls_bitmap_frame_decode::GetResolution(double* dpi_x, double* dpi_y)
{
    TRACE("{} jpegls_bitmap_frame_decoder::GetResolution, dpi_x={}, dpi_y={}\n", fmt::ptr(this), fmt::ptr(dpi_x),
          fmt::ptr(dpi_y));

    return bitmap_source_->GetResolution(dpi_x, dpi_y);
}

HRESULT jpegls_bitmap_frame_decode::CopyPixels(const WICRect* rectangle, const uint32_t stride, const uint32_t buffer_size,
                                               BYTE* buffer)
{
    TRACE("{} jpegls_bitmap_frame_decoder::CopyPixels, rectangle={}, stride={}, buffer_size={}, buffer={}\n", fmt::ptr(this),
          fmt::ptr(rectangle), stride, buffer_size, fmt::ptr(buffer));

    return bitmap_source_->CopyPixels(rectangle, stride, buffer_size, buffer);
}

HRESULT jpegls_bitmap_frame_decode::CopyPalette(IWICPalette* /*palette*/) noexcept
{
    TRACE("{} jpegls_bitmap_frame_decoder::CopyPalette\n", fmt::ptr(this));
    return wincodec::error_palette_unavailable;
}

// IWICBitmapFrameDecode : IWICBitmapSource
HRESULT jpegls_bitmap_frame_decode::GetThumbnail(IWICBitmapSource** /*source*/) noexcept
{
    return wincodec::error_codec_no_thumbnail;
}

HRESULT jpegls_bitmap_frame_decode::GetColorContexts(const uint32_t count, IWICColorContext** color_contexts,
                                                     uint32_t* actual_count) noexcept
try
{
    TRACE("{} jpegls_bitmap_frame_decoder::GetColorContexts, count={}, color_contexts={}, actual_count={}\n", fmt::ptr(this),
          count, fmt::ptr(color_contexts), fmt::ptr(actual_count));

    // Keep the initial design simple: no support for color profiles.
    // The JPEG-LS standard does support embedded color profiles.
    *check_out_pointer(actual_count) = 0;
    return error_ok;
}
catch (...)
{
    return winrt::to_hresult();
}

HRESULT __stdcall jpegls_bitmap_frame_decode::GetMetadataQueryReader(
    [[maybe_unused]] IWICMetadataQueryReader** metadata_query_reader) noexcept
{
    TRACE("{} jpegls_bitmap_decoder::GetMetadataQueryReader, metadata_query_reader={}\n", fmt::ptr(this),
          fmt::ptr(metadata_query_reader));

    // Keep the initial design simple: no support for metadata.
    return wincodec::error_unsupported_operation;
}

bool jpegls_bitmap_frame_decode::can_decode_to_wic_pixel_format(const int32_t bits_per_sample,
                                                                const int32_t component_count) noexcept
{
    return get_pixel_format(bits_per_sample, component_count).has_value();
}
