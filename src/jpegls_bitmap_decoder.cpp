// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "jpegls_bitmap_decoder.h"

#include "jpegls_bitmap_frame_decode.h"
#include "class_factory.h"
#include "guids.h"

#include <charls/jpegls_decoder.h>

#include <wincodec.h>
#include <winrt/base.h>

#include <mutex>

using std::mutex;
using std::error_code;
using std::scoped_lock;
using charls::jpegls_decoder;
using winrt::implements;
using winrt::com_ptr;
using winrt::check_hresult;
using winrt::to_hresult;
using winrt::make;


struct jpegls_bitmap_decoder final : implements<jpegls_bitmap_decoder, IWICBitmapDecoder>
{
    // IWICBitmapDecoder
    HRESULT __stdcall QueryCapability(_In_ IStream* stream, _Out_ DWORD* capability) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::QueryCapability.1, instance=%p, stream=%p, capability=%p\n", this, stream, capability);

        if (!stream)
            return E_INVALIDARG;

        if (!capability)
            return E_POINTER;

        // Custom decoder implementations should save the current position of the specified IStream,
        // read whatever information is necessary in order to determine which capabilities
        // it can provide for the supplied stream, and restore the stream position.
        try
        {
            std::byte header[4 * 1024];
            unsigned long read_byte_count;

            check_hresult(stream->Read(header, sizeof header, &read_byte_count));

            LARGE_INTEGER offset;
            offset.QuadPart = -static_cast<int64_t>(read_byte_count);
            check_hresult(stream->Seek(offset, STREAM_SEEK_CUR, nullptr));

            jpegls_decoder decoder;
            error_code error;
            decoder.read_header(header, read_byte_count, error);

            if (error ||
                !jpegls_bitmap_frame_decode::can_decode_to_wic_pixel_format(decoder.metadata_info().bits_per_sample, decoder.metadata_info().component_count))
            {
                *capability = 0;
            }
            else
            {
                *capability = WICBitmapDecoderCapabilityCanDecodeAllImages;
            }
            TRACE("jpegls_bitmap_decoder::QueryCapability.2, instance=%p, stream=%p, capability=%d\n", this, stream, *capability);

            return S_OK;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    HRESULT __stdcall Initialize(_In_ IStream* stream, [[maybe_unused]] const WICDecodeOptions cache_options) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::Initialize, instance=%p, stream=%p, cache_options=%d\n", this, stream, cache_options);

        if (!stream)
            return E_INVALIDARG;

        scoped_lock lock{mutex_};

        stream_.copy_from(stream);
        bitmap_frame_decode_.attach(nullptr);

        return S_OK;
    }

    HRESULT __stdcall GetContainerFormat(_Out_ GUID* container_format) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetContainerFormat, instance=%p, container_format=%p\n", this, container_format);

        if (!container_format)
            return E_POINTER;

        *container_format = GUID_ContainerFormatJpegLS;
        return S_OK;
    }

    HRESULT __stdcall GetDecoderInfo(_Outptr_ IWICBitmapDecoderInfo** decoder_info) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetContainerFormat, instance=%p, decoder_info=%p\n", this, decoder_info);

        try
        {
            com_ptr<IWICComponentInfo> component_info;
            check_hresult(imaging_factory()->CreateComponentInfo(CLSID_JpegLSDecoder, component_info.put()));
            check_hresult(component_info->QueryInterface(IID_PPV_ARGS(decoder_info)));

            return S_OK;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    HRESULT __stdcall CopyPalette([[maybe_unused]] _In_ IWICPalette* palette) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::CopyPalette, instance=%p, palette=%p\n", this, palette);

        // Palettes are for JPEG-LS on frame level.
        return WINCODEC_ERR_PALETTEUNAVAILABLE;
    }

    HRESULT __stdcall GetMetadataQueryReader([[maybe_unused]] _Outptr_ IWICMetadataQueryReader** metadata_query_reader) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetMetadataQueryReader, instance=%p, metadata_query_reader=%p\n", this, metadata_query_reader);

        // Keep the initial design simple: no support for container-level metadata.
        return  WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT __stdcall GetPreview([[maybe_unused]] _Outptr_ IWICBitmapSource** bitmap_source) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetPreview, instance=%p, bitmap_source=%p\n", this, bitmap_source);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT __stdcall GetColorContexts([[maybe_unused]] const uint32_t count, [[maybe_unused]] IWICColorContext** color_contexts, [[maybe_unused]] uint32_t* actualCount) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetColorContexts, instance=%p, color_contexts=%p\n", this, color_contexts);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT __stdcall GetThumbnail([[maybe_unused]] _Outptr_ IWICBitmapSource** thumbnail) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetThumbnail, instance=%p, thumbnail=%p\n", this, thumbnail);
        return WINCODEC_ERR_CODECNOTHUMBNAIL;
    }

    HRESULT __stdcall GetFrameCount(_Out_ uint32_t* count) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetFrameCount, instance=%p, count=%p\n", this, count);
        if (!count)
            return E_POINTER;

        *count = 1; // JPEG-LS format can only store 1 frame.
        return S_OK;
    }

    HRESULT __stdcall GetFrame(const uint32_t index, _Outptr_ IWICBitmapFrameDecode** bitmap_frame_decode) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetFrame, instance=%p, index=%d, bitmap_frame_decode=%p\n", this, index, bitmap_frame_decode);
        if (!bitmap_frame_decode)
            return E_POINTER;

        if (index != 0)
            return WINCODEC_ERR_FRAMEMISSING;

        scoped_lock lock{mutex_};

        if (!stream_)
            return WINCODEC_ERR_NOTINITIALIZED;

        try
        {
            if (!bitmap_frame_decode_)
            {
                bitmap_frame_decode_ = make<jpegls_bitmap_frame_decode>(stream_.get(), imaging_factory());
            }

            bitmap_frame_decode_.copy_to(bitmap_frame_decode);
            return S_OK;
        }
        catch (...)
        {
            return to_hresult();
        }
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

    mutex mutex_;
    com_ptr<IWICImagingFactory> imaging_factory_;
    com_ptr<IStream> stream_;
    com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode_;
};

HRESULT jpegls_bitmap_decoder_create_factory(_In_ GUID const& interface_id, _Outptr_ void** result)
{
    return make<class_factory<jpegls_bitmap_decoder>>()->QueryInterface(interface_id, result);
}