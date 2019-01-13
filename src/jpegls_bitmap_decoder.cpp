// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "trace.h"
#include "jpegls_bitmap_frame_decoder.h"
#include "guids.h"

#include <charls/jpegls_decoder.h>

using std::mutex;
using std::error_code;
using charls::decoder;
using namespace winrt;


struct jpegls_bitmap_decoder final : implements<jpegls_bitmap_decoder, IWICBitmapDecoder>
{
    // IWICBitmapDecoder
    HRESULT __stdcall QueryCapability(IStream* stream, DWORD* capability) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::Initialize, instance=%p, stream=%p, capability=%p\n", this, stream, capability);

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

            decoder decoder;
            std::error_code error;
            decoder.read_header(header, read_byte_count, error);

            if (error)
            {
                *capability = 0;
            }
            else
            {
                *capability = WICBitmapDecoderCapabilityCanDecodeAllImages;
            }

            return S_OK;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    HRESULT __stdcall Initialize(IStream* stream, [[maybe_unused]] WICDecodeOptions cache_options) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::Initialize, instance=%p, stream=%p, cache_options=%d\n", this, stream, cache_options);

        if (!stream)
            return E_INVALIDARG;

        stream_.attach(stream);
        return S_OK;
    }

    HRESULT __stdcall GetContainerFormat(GUID* container_format) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetContainerFormat, instance=%p, container_format=%p\n", this, container_format);

        if (!container_format)
            return E_POINTER;

        *container_format = GUID_ContainerFormatJpegLS;
        return S_OK;
    }

    HRESULT __stdcall GetDecoderInfo(IWICBitmapDecoderInfo** decoder_info) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetContainerFormat, instance=%p, decoder_info=%p\n", this, decoder_info);

        try
        {
            com_ptr<IWICComponentInfo> component_info;
            check_hresult(factory()->CreateComponentInfo(CLSID_JpegLSDecoder, component_info.put()));
            check_hresult(component_info->QueryInterface(IID_PPV_ARGS(decoder_info)));

            return S_OK;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    HRESULT __stdcall CopyPalette([[maybe_unused]] IWICPalette* palette) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::CopyPalette, instance=%p, palette=%p\n", this, palette);

        // Palettes are for JPEG-LS on frame level.
        return WINCODEC_ERR_PALETTEUNAVAILABLE;
    }

    HRESULT __stdcall GetMetadataQueryReader([[maybe_unused]] IWICMetadataQueryReader** metadata_query_reader) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetMetadataQueryReader, instance=%p, metadata_query_reader=%p\n", this, metadata_query_reader);

        // Keep the initial design simple: no support for container-level metadata.
        return  WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT __stdcall GetPreview([[maybe_unused]] IWICBitmapSource** bitmap_source) noexcept override
    {
        TRACE("jpegls_bitmap_decoder::GetPreview, instance=%p, bitmap_source=%p\n", this, bitmap_source);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT __stdcall GetColorContexts([[maybe_unused]] uint32_t count, [[maybe_unused]] IWICColorContext** color_contexts, [[maybe_unused]] uint32_t* actualCount) override
    {
        TRACE("jpegls_bitmap_decoder::GetColorContexts, instance=%p, color_contexts=%p\n", this, color_contexts);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT __stdcall GetThumbnail([[maybe_unused]] IWICBitmapSource** thumbnail) override
    {
        TRACE("jpegls_bitmap_decoder::GetThumbnail, instance=%p, thumbnail=%p\n", this, thumbnail);
        return WINCODEC_ERR_CODECNOTHUMBNAIL;
    }

    HRESULT __stdcall GetFrameCount(uint32_t* count) override
    {
        TRACE("jpegls_bitmap_decoder::GetFrameCount, instance=%p, count=%p\n", this, count);
        if (!count)
            return E_POINTER;

        *count = 1; // JPEG-LS format can only store 1 frame.
        return S_OK;
    }

    HRESULT __stdcall GetFrame(const uint32_t index, IWICBitmapFrameDecode** bitmap_frame_decode) override
    {
        TRACE("jpegls_bitmap_decoder::GetFrame, instance=%p, index=%d, bitmap_frame_decode=%p\n", this, index, bitmap_frame_decode);
        if (!bitmap_frame_decode)
            return E_POINTER;

        if (index != 0)
            return WINCODEC_ERR_FRAMEMISSING;

        if (!stream_)
            return WINCODEC_ERR_NOTINITIALIZED;

        try
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return make<jpegls_bitmap_frame_decoder>(*stream_)->QueryInterface(IID_PPV_ARGS(bitmap_frame_decode));
        }
        catch (const charls::jpegls_error&)
        {
            // TODO: trace.
            return WINCODEC_ERR_BADHEADER;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

private:
    IWICImagingFactory* factory()
    {
        if (!factory_)
        {
            check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, factory_.put_void()));
        }

        return factory_.get();
    }

    mutex mutex_;
    com_ptr<IWICImagingFactory> factory_;
    com_ptr<IStream> stream_;
};


struct bitmap_decoder_factory final : implements<bitmap_decoder_factory, IClassFactory>
{
    HRESULT __stdcall CreateInstance(
        IUnknown* outer,
        GUID const& interface_id,
        void** result) noexcept override
    {
        if (!result)
            return E_POINTER;

        *result = nullptr;

        if (outer)
            return CLASS_E_NOAGGREGATION;

        return make<jpegls_bitmap_decoder>()->QueryInterface(interface_id, result);
    }

    HRESULT __stdcall LockServer(BOOL) noexcept override
    {
        return S_OK;
    }
};

// Purpose: Returns a class factory to create an object of the requested type
_Check_return_
STDAPI DllGetClassObject(_In_ GUID const& class_id, _In_ GUID const& interface_id, _Outptr_ void** result)
{
    if (class_id == CLSID_JpegLSDecoder)
        return make<bitmap_decoder_factory>()->QueryInterface(interface_id, result);

    return CLASS_E_CLASSNOTAVAILABLE;
}
