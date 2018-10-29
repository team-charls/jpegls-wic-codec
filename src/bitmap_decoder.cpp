// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "trace.h"
#include "decoder.h"
#include "bitmap_frame_decoder.h"

using namespace winrt;
using std::mutex;


// {E57DC18B-019C-47F2-8ED0-BF587BE4FF1B}
static const GUID JpegLsDecoder
{ 0xe57dc18b, 0x19c, 0x47f2, { 0x8e, 0xd0, 0xbf, 0x58, 0x7b, 0xe4, 0xff, 0x1b } };

// {52C25458-282D-4EF4-A69F-021BB2984543}
static const GUID ContainerFormat
{ 0x52c25458, 0x282d, 0x4ef4, { 0xa6, 0x9f, 0x2, 0x1b, 0xb2, 0x98, 0x45, 0x43 } };

struct bitmap_decoder : implements<bitmap_decoder, IWICBitmapDecoder>
{
    // IWICBitmapDecoder
    HRESULT __stdcall QueryCapability(IStream* stream, DWORD* capability) noexcept override
    {
        TRACE("(%p)\n", stream);

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

            // TODO ReadHeader.
            *capability = WICBitmapDecoderCapabilityCanDecodeAllImages;

            //WICBitmapDecoderCapabilitySameEncoder

            return S_OK;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    HRESULT __stdcall Initialize(IStream* stream, WICDecodeOptions /*cacheOptions*/) override
    {
        if (!stream)
            return E_INVALIDARG;

        stream_.attach(stream);
        return S_OK;
    }

    HRESULT __stdcall GetContainerFormat(GUID* containerFormat) override
    {
        TRACE("(%p)\n", containerFormat);
        if (!containerFormat)
            return E_POINTER;

        *containerFormat = ContainerFormat;
        return S_OK;
    }

    HRESULT __stdcall GetDecoderInfo(IWICBitmapDecoderInfo **ppIDecoderInfo) override
    {
        TRACE("(%p)\n", ppIDecoderInfo);

        try
        {
            com_ptr<IWICComponentInfo> compInfo;
            check_hresult(factory()->CreateComponentInfo(JpegLsDecoder, compInfo.put()));
            check_hresult(compInfo->QueryInterface(IID_IWICBitmapDecoderInfo, reinterpret_cast<void**>(ppIDecoderInfo)));

            return S_OK;
        }
        catch (...)
        {
            return to_hresult();
        }
    }

    HRESULT __stdcall CopyPalette([[maybe_unused]] IWICPalette* palette) override
    {
        TRACE("bitmap_decoder::CopyPalette, this=%p, palette=%p\n", this, palette);
        return WINCODEC_ERR_PALETTEUNAVAILABLE;
    }

    HRESULT __stdcall GetMetadataQueryReader([[maybe_unused]] IWICMetadataQueryReader** ppIMetadataQueryReader) override
    {
        return S_OK;
    }

    HRESULT __stdcall GetPreview([[maybe_unused]] IWICBitmapSource** bitmapSource) override
    {
        TRACE("bitmap_decoder::GetPreview, this=%p, palette=%p\n", this, bitmapSource);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT __stdcall GetColorContexts([[maybe_unused]] uint32_t count, [[maybe_unused]] IWICColorContext** colorContexts, [[maybe_unused]] uint32_t* actualCount) override
    {
        TRACE("bitmap_decoder::GetPreview, this=%p, palette=%p\n", this, colorContexts);
        return WINCODEC_ERR_UNSUPPORTEDOPERATION;
    }

    HRESULT __stdcall GetThumbnail(IWICBitmapSource** ppIThumbnail) override
    {
        TRACE("(%p)\n", ppIThumbnail);
        return WINCODEC_ERR_CODECNOTHUMBNAIL;
    }

    HRESULT __stdcall GetFrameCount(uint32_t* count) override
    {
        TRACE("bitmap_decoder::GetFrameCount, this=%p, count=%p\n", this, count);
        if (!count)
            return E_POINTER;

        *count = 1; // JPEG-LS format can only store 1 frame.
        return S_OK;
    }

    HRESULT __stdcall GetFrame(const uint32_t index, IWICBitmapFrameDecode** bitmapFrameDecode) override
    {
        TRACE("(%d, %p)\n", index, bitmapFrameDecode);
        if (!bitmapFrameDecode)
            return E_POINTER;

        if (index != 0)
            return WINCODEC_ERR_FRAMEMISSING;

        if (!stream_)
            return WINCODEC_ERR_NOTINITIALIZED;

        try
        {
            std::lock_guard<std::mutex> lock(mutex_);
            return make<bitmap_frame_decoder>(stream_.get())->QueryInterface(__uuidof(IWICBitmapFrameDecode),
                reinterpret_cast<void**>(bitmapFrameDecode));
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


struct jpegls_container_factory final : implements<jpegls_container_factory, IClassFactory>
{
    HRESULT __stdcall CreateInstance(
        IUnknown* outer,
        GUID const& iid,
        void** result) noexcept override
    {
        *result = nullptr;

        if (outer)
        {
            return CLASS_E_NOAGGREGATION;
        }

        return make<bitmap_decoder>()->QueryInterface(iid, result);
    }

    HRESULT __stdcall LockServer(BOOL) noexcept override
    {
        return S_OK;
    }
};

// Purpose: Returns a class factory to create an object of the requested type
_Check_return_
STDAPI DllGetClassObject(_In_ GUID const& /*classId*/, _In_ GUID const& interfaceId, _Outptr_ void** result)
{
    return make<jpegls_container_factory>()->QueryInterface(interfaceId, result);
}
