// SPDX-FileCopyrightText: © 2018 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

module;

#include "intellisense.hpp"

module jpegls_bitmap_decoder;

import std;
import winrt;
import charls;
import <win.hpp>;

import class_factory;
import guids;
import util;
import hresults;
import jpegls_bitmap_frame_decode;
import "macros.hpp";

using charls::jpegls_category;
using charls::jpegls_decoder;
using charls::jpegls_error;
using std::array;
using std::error_code;
using std::int64_t;
using std::scoped_lock;
using std::uint32_t;
using winrt::check_hresult;
using winrt::com_ptr;
using winrt::make;


namespace {

struct jpegls_bitmap_decoder : winrt::implements<jpegls_bitmap_decoder, IWICBitmapDecoder>
{
    // IWICBitmapDecoder
    HRESULT __stdcall QueryCapability(_In_ IStream* stream, _Out_ DWORD* capability) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_decoder::QueryCapability.1, stream={}, capability={}\n", fmt::ptr(this), fmt::ptr(stream),
              fmt::ptr(capability));

        check_in_pointer(stream);
        *check_out_pointer(capability) = 0;

        // Custom decoder implementations should save the current position of the specified IStream,
        // read whatever information is necessary in order to determine which capabilities
        // it can provide for the supplied stream, and restore the stream position.
        array<std::byte, static_cast<size_t>(4) * 1024> header{};
        unsigned long read_byte_count;

        check_hresult(stream->Read(header.data(), static_cast<ULONG>(header.size()), &read_byte_count));

        LARGE_INTEGER offset;
        offset.QuadPart = -static_cast<int64_t>(read_byte_count);
        check_hresult(stream->Seek(offset, STREAM_SEEK_CUR, nullptr));

        jpegls_decoder decoder;
        decoder.source(header.data(), read_byte_count);

        error_code error;
        decoder.read_header(error);
        if (error)
        {
            TRACE("{} jpegls_bitmap_decoder::QueryCapability.2, capability=0, ec={}, (reason={})\n", fmt::ptr(this),
                  error.value(), jpegls_category().message(error.value()));
            return success_ok;
        }

        if (const auto& [width, height, bits_per_sample, component_count]{decoder.frame_info()};
            jpegls_bitmap_frame_decode::can_decode_to_wic_pixel_format(bits_per_sample, component_count))
        {
            *capability = WICBitmapDecoderCapabilityCanDecodeAllImages;
        }

        TRACE("{} jpegls_bitmap_decoder::QueryCapability.3, stream={}, *capability={}\n", fmt::ptr(this), fmt::ptr(stream),
              *capability);
        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall Initialize(_In_ IStream* stream, [[maybe_unused]]
                                                       const WICDecodeOptions cache_options) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_decoder::Initialize, stream={}, cache_options={}\n", fmt::ptr(this), fmt::ptr(stream),
              fmt::underlying(cache_options));

        scoped_lock lock{mutex_};

        source_stream_.copy_from(check_in_pointer(stream));
        bitmap_frame_decode_.attach(nullptr);

        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall GetContainerFormat(_Out_ GUID* container_format) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_decoder::GetContainerFormat, container_format={}\n", fmt::ptr(this),
              fmt::ptr(container_format));

        *check_out_pointer(container_format) = id::container_format_jpegls;
        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall GetDecoderInfo(_Outptr_ IWICBitmapDecoderInfo** decoder_info) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_decoder::GetDecoderInfo, decoder_info={}\n", fmt::ptr(this), fmt::ptr(decoder_info));

        com_ptr<IWICComponentInfo> component_info;
        check_hresult(imaging_factory()->CreateComponentInfo(id::jpegls_decoder, component_info.put()));
        check_hresult(component_info->QueryInterface(IID_PPV_ARGS(decoder_info)));

        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall CopyPalette([[maybe_unused]]
                                  _In_ IWICPalette* palette) noexcept override
    {
        TRACE("{} jpegls_bitmap_decoder::CopyPalette, palette={}\n", fmt::ptr(this), fmt::ptr(palette));

        // Palettes are for JPEG-LS on frame level.
        return wincodec::error_palette_unavailable;
    }

    HRESULT __stdcall GetMetadataQueryReader([[maybe_unused]]
                                             _Outptr_ IWICMetadataQueryReader** metadata_query_reader) noexcept override
    {
        TRACE("{} jpegls_bitmap_decoder::GetMetadataQueryReader, metadata_query_reader=%p\n", fmt::ptr(this),
              fmt::ptr(metadata_query_reader));

        // Keep the initial design simple: no support for container-level metadata.
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall GetPreview([[maybe_unused]]
                                 _Outptr_ IWICBitmapSource** bitmap_source) noexcept override
    {
        TRACE("{} jpegls_bitmap_decoder::GetPreview, bitmap_source={}\n", fmt::ptr(this), fmt::ptr(bitmap_source));

        // The JPEG-LS standard does not define a preview image.
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall GetColorContexts([[maybe_unused]] const uint32_t count,
                                       [[maybe_unused]] IWICColorContext** color_contexts,
                                       [[maybe_unused]]
                                       uint32_t* actual_count) noexcept override
    {
        TRACE("{} jpegls_bitmap_decoder::GetColorContexts, count={}, color_contexts={}, actual_count={}\n", fmt::ptr(this),
              count, fmt::ptr(color_contexts), fmt::ptr(actual_count));

        // The JPEG-LS standard does not define an overall color profiles. Color profiles are defined on frame level.
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall GetThumbnail([[maybe_unused]]
                                   _Outptr_ IWICBitmapSource** thumbnail) noexcept override
    {
        TRACE("{} jpegls_bitmap_decoder::GetThumbnail, thumbnail={}\n", fmt::ptr(this), fmt::ptr(thumbnail));

        return wincodec::error_codec_no_thumbnail;
    }

    HRESULT __stdcall GetFrameCount(_Out_ uint32_t* count) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_decoder::GetFrameCount, count={}\n", fmt::ptr(this), fmt::ptr(count));

        *check_out_pointer(count) = 1; // The JPEG-LS format can only store 1 frame.
        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall GetFrame(const uint32_t index, _Outptr_ IWICBitmapFrameDecode** bitmap_frame_decode) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_decoder::GetFrame, index={}, bitmap_frame_decode={}\n", fmt::ptr(this), index,
              fmt::ptr(bitmap_frame_decode));

        check_condition(index == 0, wincodec::error_frame_missing);

        scoped_lock lock{mutex_};

        check_condition(static_cast<bool>(source_stream_), wincodec::error_not_initialized);

        if (!bitmap_frame_decode_)
        {
            bitmap_frame_decode_ = make<jpegls_bitmap_frame_decode>(source_stream_.get(), imaging_factory());
        }

        bitmap_frame_decode_.copy_to(check_out_pointer(bitmap_frame_decode));
        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

private:
    IWICImagingFactory* imaging_factory()
    {
        if (!imaging_factory_)
        {
            check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                           IID_PPV_ARGS(imaging_factory_.put())));
        }

        return imaging_factory_.get();
    }

    std::mutex mutex_;
    com_ptr<IWICImagingFactory> imaging_factory_;
    com_ptr<IStream> source_stream_;
    com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode_;
};

} // namespace

HRESULT create_jpegls_bitmap_decoder_factory(_In_ GUID const& interface_id, _Outptr_ void** result)
{
    return make<class_factory<jpegls_bitmap_decoder>>()->QueryInterface(interface_id, result);
}
