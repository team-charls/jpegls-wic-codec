// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

module;

#include "macros.h"

module jpegls_bitmap_encoder;

import "win.h";
import "std.h";
import class_factory;
import guids;
import errors;
import jpegls_bitmap_frame_encode;
import winrt;
import charls;
import util;

using charls::interleave_mode;
using charls::jpegls_encoder;
using charls::spiff_color_space;
using charls::spiff_resolution_units;
using std::vector;
using winrt::check_hresult;
using winrt::com_ptr;
using winrt::implements;
using winrt::make;
using winrt::to_hresult;

namespace {

void write_spiff_header(jpegls_encoder& encoder, const jpegls_bitmap_frame_encode& bitmap_frame_encode)
{
    const auto color_space{bitmap_frame_encode.frame_info().component_count == 1 ? spiff_color_space::grayscale
                                                                                 : spiff_color_space::rgb};

    if (const auto& resolution{bitmap_frame_encode.resolution()}; resolution.has_value())
    {
        encoder.write_standard_spiff_header(color_space, spiff_resolution_units::dots_per_inch, resolution->second,
                                            resolution->first);
    }
    else
    {
        encoder.write_standard_spiff_header(color_space);
    }
}

struct jpegls_bitmap_encoder : implements<jpegls_bitmap_encoder, IWICBitmapEncoder>
{
    // IWICBitmapEncoder
    HRESULT __stdcall Initialize(_In_ IStream* destination,
                                 [[maybe_unused]] const WICBitmapEncoderCacheOption cache_option) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_encoder::Initialize, stream={}, cache_option={}\n", fmt::ptr(this), fmt::ptr(destination),
              fmt::underlying(cache_option));

        check_condition(!static_cast<bool>(destination_), wincodec::error_wrong_state);
        destination_.copy_from(check_in_pointer(destination));
        return error_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall GetContainerFormat(_Out_ GUID* container_format) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_encoder::GetContainerFormat, container_format={}\n", fmt::ptr(this),
              fmt::ptr(container_format));

        *check_out_pointer(container_format) = id::container_format_jpegls;
        return error_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall GetEncoderInfo(_Outptr_ IWICBitmapEncoderInfo** encoder_info) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_encoder::GetEncoderInfo, encoder_info={}\n", fmt::ptr(this), fmt::ptr(encoder_info));

        com_ptr<IWICComponentInfo> component_info;
        check_hresult(imaging_factory()->CreateComponentInfo(id::jpegls_encoder, component_info.put()));
        check_hresult(component_info->QueryInterface(IID_PPV_ARGS(encoder_info)));

        return error_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall CreateNewFrame(_Outptr_ IWICBitmapFrameEncode** bitmap_frame_encode,
                                     IPropertyBag2** encoder_options) noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_encoder::CreateNewFrame, bitmap_frame_encode={}, encoder_options={}\n", fmt::ptr(this),
              fmt::ptr(bitmap_frame_encode), fmt::ptr(encoder_options));

        check_condition(static_cast<bool>(destination_), wincodec::error_not_initialized);
        check_condition(!static_cast<bool>(bitmap_frame_encode_), wincodec::error_wrong_state); // Only 1 frame is supported.

        bitmap_frame_encode_ = winrt::make_self<jpegls_bitmap_frame_encode>();

        *check_out_pointer(bitmap_frame_encode) = bitmap_frame_encode_.get();
        (*bitmap_frame_encode)->AddRef();

        if (encoder_options)
        {
            *encoder_options = nullptr;
        }

        return error_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall Commit() noexcept override
    try
    {
        TRACE("{} jpegls_bitmap_encoder::Commit\n", fmt::ptr(this));

        check_condition(!committed_, wincodec::error_wrong_state);
        check_condition(static_cast<bool>(destination_), wincodec::error_not_initialized);
        check_condition(static_cast<bool>(bitmap_frame_encode_), wincodec::error_frame_missing);

        jpegls_encoder encoder;
        encoder.frame_info(bitmap_frame_encode_->frame_info());

        vector<std::byte> destination(encoder.estimated_destination_size());
        encoder.destination(destination);

        if (bitmap_frame_encode_->frame_info().component_count > 1)
        {
            encoder.interleave_mode(interleave_mode::sample);
        }

        write_spiff_header(encoder, *bitmap_frame_encode_.get());

        const auto bytes_written{encoder.encode(bitmap_frame_encode_->source(), bitmap_frame_encode_->source_stride())};
        bitmap_frame_encode_ = nullptr;

        check_hresult(destination_->Write(destination.data(), static_cast<ULONG>(bytes_written), nullptr));
        check_hresult(destination_->Commit(STGC_DEFAULT));
        destination_ = nullptr;

        committed_ = true;
        return error_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    // Optional methods
    HRESULT __stdcall SetPreview([[maybe_unused]] _In_ IWICBitmapSource* preview) noexcept override
    {
        TRACE("{} jpegls_bitmap_encoder::SetPreview, preview={}\n", fmt::ptr(this), fmt::ptr(preview));
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall SetThumbnail([[maybe_unused]] _In_ IWICBitmapSource* thumbnail) noexcept override
    {
        TRACE("{} jpegls_bitmap_encoder::SetThumbnail, thumbnail={}\n", fmt::ptr(this), fmt::ptr(thumbnail));
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall SetColorContexts([[maybe_unused]] const uint32_t count,
                                       [[maybe_unused]] IWICColorContext** color_context) noexcept override
    {
        TRACE("{} jpegls_bitmap_encoder::SetColorContexts, count={}, color_context={}\n", fmt::ptr(this), count,
              fmt::ptr(color_context));

        // Note: the current implementation doesn't support color contexts.
        //       Normally ICC color context profiles can be stored in the JPEG APP2 marker section.
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall GetMetadataQueryWriter(
        [[maybe_unused]] _Outptr_ IWICMetadataQueryWriter** metadata_query_writer) noexcept override
    {
        TRACE("{} jpegls_bitmap_encoder::GetMetadataQueryWriter, metadata_query_writer={}\n", fmt::ptr(this),
              fmt::ptr(metadata_query_writer));

        // Note: the current implementation doesn't write metadata to the JPEG-LS stream.
        //       The SPIFF header can be used to store metadata items.
        constexpr HRESULT result = wincodec::error_unsupported_operation;
        return result;
    }

    HRESULT __stdcall SetPalette(_In_ IWICPalette* palette) noexcept override
    {
        TRACE("{} jpegls_bitmap_encoder::SetPalette, palette={}\n", fmt::ptr(this), fmt::ptr(palette));

        // Note: the current implementation doesn't support storing a palette to the JPEG-LS stream.
        //       The JPEG-LS standard does support it.
        return wincodec::error_unsupported_operation;
    }

private:
    [[nodiscard]]
    IWICImagingFactory* imaging_factory()
    {
        if (!imaging_factory_)
        {
            check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                           IID_PPV_ARGS(imaging_factory_.put())));
        }

        return imaging_factory_.get();
    }

    bool committed_{};
    com_ptr<IWICImagingFactory> imaging_factory_;
    com_ptr<IStream> destination_;
    com_ptr<jpegls_bitmap_frame_encode> bitmap_frame_encode_;
};

} // namespace

HRESULT create_jpegls_bitmap_encoder_factory(GUID const& interface_id, void** result)
{
    return make<class_factory<jpegls_bitmap_encoder>>()->QueryInterface(interface_id, result);
}
