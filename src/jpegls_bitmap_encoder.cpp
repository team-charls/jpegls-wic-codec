// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "jpegls_bitmap_encoder.h"

#include "class_factory.h"
#include "errors.h"
#include "guids.h"
#include "jpegls_bitmap_frame_encode.h"

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

struct jpegls_bitmap_encoder final : implements<jpegls_bitmap_encoder, IWICBitmapEncoder>
{
    // IWICBitmapEncoder
    HRESULT __stdcall Initialize(_In_ IStream* destination, [[maybe_unused]] const WICBitmapEncoderCacheOption cache_option) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::Initialize, stream=%p, cache_option=%d\n", this, destination, cache_option);

        if (!destination)
            return error_invalid_argument;

        if (destination_)
            return wincodec::error_wrong_state;

        destination_.copy_from(destination);

        return error_ok;
    }

    HRESULT __stdcall GetContainerFormat(_Out_ GUID* container_format) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::GetContainerFormat, container_format=%p\n", this, container_format);

        if (!container_format)
            return error_pointer;

        *container_format = GUID_ContainerFormatJpegLS;
        return error_ok;
    }

    HRESULT __stdcall GetEncoderInfo(_Outptr_ IWICBitmapEncoderInfo** encoder_info) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::GetContainerFormat, encoder_info=%p\n", this, encoder_info);

        try
        {
            com_ptr<IWICComponentInfo> component_info;
            check_hresult(imaging_factory()->CreateComponentInfo(CLSID_JpegLSEncoder, component_info.put()));
            check_hresult(component_info->QueryInterface(IID_PPV_ARGS(encoder_info)));

            return error_ok;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    HRESULT __stdcall CreateNewFrame(_Outptr_ IWICBitmapFrameEncode** bitmap_frame_encode, IPropertyBag2** encoder_options) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::GetContainerFormat, bitmap_frame_encode=%p, encoder_options=%p\n", this, bitmap_frame_encode, encoder_options);

        if (!bitmap_frame_encode)
        {
            const HRESULT result = error_pointer;
            return result;
        }

        if (!destination_)
        {
            const HRESULT result = wincodec::error_not_initialized;
            return result;
        }

        if (bitmap_frame_encode_)
        {
            const HRESULT result = wincodec::error_wrong_state; // Only 1 frame is supported.
            return result;
        }

        try
        {
            bitmap_frame_encode_ = winrt::make_self<jpegls_bitmap_frame_encode>();

            *bitmap_frame_encode = bitmap_frame_encode_.get();
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
    }

    HRESULT __stdcall Commit() noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::Commit\n", this);

        try
        {
            if (!destination_)
                return wincodec::error_not_initialized;

            if (!bitmap_frame_encode_)
                return wincodec::error_frame_missing;

            if (committed_)
                return wincodec::error_wrong_state;

            jpegls_encoder encoder;
            encoder.frame_info(bitmap_frame_encode_->frame_info());

            vector<std::byte> destination(encoder.estimated_destination_size());
            encoder.destination(destination);

            if (bitmap_frame_encode_->frame_info().component_count > 1)
            {
                encoder.interleave_mode(interleave_mode::sample);
            }

            const auto color_space = bitmap_frame_encode_->frame_info().component_count == 1 ? spiff_color_space::grayscale : spiff_color_space::rgb;
            if (bitmap_frame_encode_->is_dpi_set())
            {
                encoder.write_standard_spiff_header(color_space, spiff_resolution_units::dots_per_inch,
                                                    lround(bitmap_frame_encode_->dpi_y()), lround(bitmap_frame_encode_->dpi_x()));
            }
            else
            {
                encoder.write_standard_spiff_header(color_space);
            }

            const auto bytes_written = encoder.encode(bitmap_frame_encode_->source());
            check_hresult(destination_->Write(destination.data(), static_cast<ULONG>(bytes_written), nullptr));
            check_hresult(destination_->Commit(STGC_DEFAULT));

            bitmap_frame_encode_ = nullptr;
            destination_ = nullptr;

            committed_ = true;
            return error_ok;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    // Optional methods
    HRESULT __stdcall SetPreview([[maybe_unused]] _In_ IWICBitmapSource* preview) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::SetPreview, preview=%p\n", this, preview);
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall SetThumbnail([[maybe_unused]] _In_ IWICBitmapSource* thumbnail) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::SetThumbnail, thumbnail=%p\n", this, thumbnail);
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall SetColorContexts([[maybe_unused]] const uint32_t count, [[maybe_unused]] IWICColorContext** color_context) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::SetColorContexts, count=%u, color_context=%p\n", this, count, color_context);

        // Note: the current implementation doesn't support color contexts.
        //       Normally ICC color context profiles can be stored in the JPEG APP2 marker section.
        return wincodec::error_unsupported_operation;
    }

    HRESULT __stdcall GetMetadataQueryWriter([[maybe_unused]] _Outptr_ IWICMetadataQueryWriter** metadata_query_writer) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::GetMetadataQueryWriter, metadata_query_writer=%p\n", this, metadata_query_writer);

        // Note: the current implementation doesn't writing metadata to the JPEG-LS stream.
        //       The SPIFF header can be used to store metadata items.
        const HRESULT result = wincodec::error_unsupported_operation;
        return result;
    }

    HRESULT __stdcall SetPalette(_In_ IWICPalette* palette) noexcept override
    {
        TRACE("%p jpegls_bitmap_encoder::SetPalette, palette=%p\n", this, palette);

        // Note: the current implementation doesn't support storing a palette to the JPEG-LS stream.
        //       The JPEG-LS standard does support it.
        return wincodec::error_unsupported_operation;
    }

private:
    [[nodiscard]] IWICImagingFactory* imaging_factory()
    {
        if (!imaging_factory_)
        {
            check_hresult(CoCreateInstance(CLSID_WICImagingFactory,
                                           nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(imaging_factory_.put())));
        }

        return imaging_factory_.get();
    }

    bool committed_{};
    com_ptr<IWICImagingFactory> imaging_factory_;
    com_ptr<IStream> destination_;
    com_ptr<jpegls_bitmap_frame_encode> bitmap_frame_encode_;
};

HRESULT create_jpegls_bitmap_encoder_factory(_In_ GUID const& interface_id, _Outptr_ void** result)
{
    return make<class_factory<jpegls_bitmap_encoder>>()->QueryInterface(interface_id, result);
}
