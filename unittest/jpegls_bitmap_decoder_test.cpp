// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "factory.h"

#include "../src/util.h"

#include <CppUnitTest.h>

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


using DllGetClassObjectPtr = HRESULT (_stdcall *) (GUID const&, GUID const&, void** result);


TEST_CLASS(jpegls_bitmap_decoder_test)
{
public:
    TEST_METHOD(GetContainerFormat)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        GUID container_format;
        const HRESULT result = wic_bitmap_decoder->GetContainerFormat(&container_format);
        Assert::AreEqual(S_OK, result);
        Assert::IsTrue(GUID_ContainerFormatJpegLS == container_format);
    }

    TEST_METHOD(GetContainerFormat_with_nullptr)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const HRESULT result = wic_bitmap_decoder->GetContainerFormat(nullptr);
        WARNING_UNSUPPRESS()

        Assert::IsTrue(FAILED(result));
    }

    TEST_METHOD(GetDecoderInfo)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        com_ptr<IWICBitmapDecoderInfo> decoder_info;
        const HRESULT result = wic_bitmap_decoder->GetDecoderInfo(decoder_info.put());
        Assert::IsTrue(result == S_OK || result == WINCODEC_ERR_COMPONENTNOTFOUND);
    }

    TEST_METHOD(GetDecoderInfo_with_nullptr)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const HRESULT result = wic_bitmap_decoder->GetDecoderInfo(nullptr);
        WARNING_UNSUPPRESS()

        Assert::IsTrue(FAILED(result));
    }

    TEST_METHOD(CopyPalette)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        com_ptr<IWICPalette> palette;
        const HRESULT result = wic_bitmap_decoder->CopyPalette(palette.get());
        Assert::AreEqual(WINCODEC_ERR_PALETTEUNAVAILABLE, result);
    }

    TEST_METHOD(GetMetadataQueryReader)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        com_ptr<IWICMetadataQueryReader> metadata_query_reader;
        const HRESULT result = wic_bitmap_decoder->GetMetadataQueryReader(metadata_query_reader.put());
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(GetPreview)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        com_ptr<IWICBitmapSource> bitmap_source;
        const HRESULT result = wic_bitmap_decoder->GetPreview(bitmap_source.put());
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(GetThumbnail)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        com_ptr<IWICBitmapSource> bitmap_source;
        const HRESULT result = wic_bitmap_decoder->GetThumbnail(bitmap_source.put());
        Assert::AreEqual(WINCODEC_ERR_CODECNOTHUMBNAIL, result);
    }

    TEST_METHOD(GetFrameCount)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        uint32_t frame_count;
        const HRESULT result = wic_bitmap_decoder->GetFrameCount(&frame_count);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(1u, frame_count);
    }

    TEST_METHOD(QueryCapability_cannot_decode_empty)
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        DWORD capability;
        const HRESULT result = wic_bitmap_decoder->QueryCapability(stream.get(), &capability);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(0ul, capability);
    }

    TEST_METHOD(QueryCapability_can_decode_8bit_monochrome)
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(L"lena8b.jls", STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();

        DWORD capability;
        const HRESULT result = wic_bitmap_decoder->QueryCapability(stream.get(), &capability);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(static_cast<DWORD>(WICBitmapDecoderCapabilityCanDecodeAllImages), capability);
    }

    TEST_METHOD(GetFrame)
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(L"lena8b.jls", STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();
        HRESULT result = wic_bitmap_decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand);
        Assert::AreEqual(S_OK, result);

        uint32_t frame_count;
        result = wic_bitmap_decoder->GetFrameCount(&frame_count);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(1u, frame_count);

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        result = wic_bitmap_decoder->GetFrame(0, bitmap_frame_decode.put());
        Assert::AreEqual(S_OK, result);
        Assert::IsTrue(bitmap_frame_decode.get() != nullptr);
    }

private:
    factory factory_;
};