// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "jpegls_bitmap_encoder.h"

#include "jpegls_bitmap_frame_encode.h"
#include "class_factory.h"
#include "guids.h"

using winrt::implements;
using winrt::com_ptr;
using winrt::check_hresult;
using winrt::to_hresult;
using winrt::make;

struct jpegls_bitmap_encoder final : implements<jpegls_bitmap_encoder, IWICBitmapEncoder>
{
    // IWICBitmapEncoder
    HRESULT Initialize(_In_ IStream* destination, [[maybe_unused]] const WICBitmapEncoderCacheOption cache_option) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::Initialize, instance=%p, stream=%p, cache_option=%d\n", this, destination, cache_option);

        if (!destination)
            return E_INVALIDARG;

        if (destination_)
            return WINCODEC_ERR_WRONGSTATE;

        destination_.copy_from(destination);

        return S_OK;
    }

    HRESULT GetContainerFormat(_Out_ GUID* container_format) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::GetContainerFormat, instance=%p, container_format=%p\n", this, container_format);

        if (!container_format)
            return E_POINTER;

        *container_format = GUID_ContainerFormatJpegLS;
        return S_OK;
    }

    HRESULT GetEncoderInfo(_Outptr_ IWICBitmapEncoderInfo** encoder_info) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::GetContainerFormat, instance=%p, encoder_info=%p\n", this, encoder_info);

        try
        {
            com_ptr<IWICComponentInfo> component_info;
            check_hresult(imaging_factory()->CreateComponentInfo(CLSID_JpegLSEncoder, component_info.put()));
            check_hresult(component_info->QueryInterface(IID_PPV_ARGS(encoder_info)));

            return S_OK;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    HRESULT CreateNewFrame(_Outptr_ IWICBitmapFrameEncode** bitmap_frame_encode, IPropertyBag2** encoder_options) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::GetContainerFormat, instance=%p, bitmap_frame_encode=%p, encoder_options=%p\n", this, bitmap_frame_encode, encoder_options);

        if (!bitmap_frame_encode)
            return E_POINTER;

        if (!destination_)
            return WINCODEC_ERR_NOTINITIALIZED;

        if (bitmap_frame_encode_)
            return WINCODEC_ERR_WRONGSTATE; // Only 1 frame is supported.

        bitmap_frame_encode_ = make<jpegls_bitmap_frame_encode>(destination_.get());
        bitmap_frame_encode_.copy_to(bitmap_frame_encode);

        if (encoder_options)
        {
            *encoder_options = nullptr;
        }

        return S_OK;
    }

    HRESULT Commit() noexcept override
    {
        if (!destination_)
            return WINCODEC_ERR_NOTINITIALIZED;

        bitmap_frame_encode_.detach();
        destination_.detach();
        return E_FAIL;
    }

    // Optional methods
    HRESULT SetPreview([[maybe_unused]] _In_ IWICBitmapSource* preview) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::SetPreview, instance=%p, preview=%p\n", this, preview);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT SetThumbnail([[maybe_unused]] _In_ IWICBitmapSource* thumbnail) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::SetThumbnail, instance=%p, thumbnail=%p\n", this, thumbnail);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT SetColorContexts([[maybe_unused]] const uint32_t count, [[maybe_unused]] IWICColorContext** color_context) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::SetColorContexts, instance=%p, count=%u, color_context=%p\n", this, count, color_context);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT GetMetadataQueryWriter([[maybe_unused]] _Outptr_ IWICMetadataQueryWriter** metadata_query_writer) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::GetMetadataQueryWriter, instance=%p, metadata_query_writer=%p\n", this, metadata_query_writer);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT SetPalette(_In_ IWICPalette* palette) noexcept override
    {
        TRACE("jpegls_bitmap_encoder::SetPalette, instance=%p, palette=%p\n", this, palette);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

private:
    IWICImagingFactory* imaging_factory()
    {
        if (!imaging_factory_)
        {
            check_hresult(CoCreateInstance(CLSID_WICImagingFactory,
                nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, imaging_factory_.put_void()));
        }

        return imaging_factory_.get();
    }

    charls::jpegls_encoder jpegls_encoder_;
    com_ptr<IWICImagingFactory> imaging_factory_;
    com_ptr<IStream> destination_;
    com_ptr<IWICBitmapFrameEncode> bitmap_frame_encode_;
};

HRESULT jpegls_bitmap_encoder_create_factory(_In_ GUID const& interface_id, _Outptr_ void** result)
{
    return make<class_factory<jpegls_bitmap_encoder>>()->QueryInterface(interface_id, result);
}