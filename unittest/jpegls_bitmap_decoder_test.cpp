// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include <CppUnitTest.h>
#include "../src/guids.h"

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


using DllGetClassObjectPtr = HRESULT (_stdcall *) (GUID const&, GUID const&, void** result);

// {E57DC18B-019C-47F2-8ED0-BF587BE4FF1B}
static const GUID JpegLsDecoder
{ 0xe57dc18b, 0x19c, 0x47f2, { 0x8e, 0xd0, 0xbf, 0x58, 0x7b, 0xe4, 0xff, 0x1b } };



TEST_CLASS(jpegls_bitmap_decoder_test)
{
public:
    jpegls_bitmap_decoder_test() noexcept(false)
    {
        library_ = LoadLibrary(L"jpegls-wic-codec.dll");
    }

    ~jpegls_bitmap_decoder_test()
    {
        if (library_)
        {
            FreeLibrary(library_);
        }
    }

    jpegls_bitmap_decoder_test(const jpegls_bitmap_decoder_test&) = delete;
    jpegls_bitmap_decoder_test(jpegls_bitmap_decoder_test&&) = delete;
    jpegls_bitmap_decoder_test& operator=(const jpegls_bitmap_decoder_test&) = delete;
    jpegls_bitmap_decoder_test& operator=(jpegls_bitmap_decoder_test&&) = delete;

    TEST_METHOD(GetContainerFormat)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        GUID container_format;
        const HRESULT result = wic_bitmap_decoder->GetContainerFormat(&container_format);
        Assert::AreEqual(S_OK, result);
        Assert::IsTrue(GUID_ContainerFormatJpegLS == container_format);
    }

    TEST_METHOD(GetContainerFormat_with_nullptr)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        const HRESULT result = wic_bitmap_decoder->GetContainerFormat(nullptr);
        Assert::IsTrue(FAILED(result));
    }

    TEST_METHOD(GetDecoderInfo)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        com_ptr<IWICBitmapDecoderInfo> decoder_info;
        const HRESULT result = wic_bitmap_decoder->GetDecoderInfo(decoder_info.put());
        Assert::IsTrue(result == S_OK || result == WINCODEC_ERR_COMPONENTNOTFOUND);
    }

    TEST_METHOD(GetDecoderInfo_with_nullptr)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        const HRESULT result = wic_bitmap_decoder->GetDecoderInfo(nullptr);
        Assert::IsTrue(FAILED(result));
    }

    TEST_METHOD(CopyPalette)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        com_ptr<IWICPalette> palette;
        const HRESULT result = wic_bitmap_decoder->CopyPalette(palette.get());
        Assert::AreEqual(WINCODEC_ERR_PALETTEUNAVAILABLE, result);
    }

    TEST_METHOD(GetMetadataQueryReader)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        com_ptr<IWICMetadataQueryReader> metadata_query_reader;
        const HRESULT result = wic_bitmap_decoder->GetMetadataQueryReader(metadata_query_reader.put());
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(GetPreview)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        com_ptr<IWICBitmapSource> bitmap_source;
        const HRESULT result = wic_bitmap_decoder->GetPreview(bitmap_source.put());
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(GetThumbnail)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        com_ptr<IWICBitmapSource> bitmap_source;
        const HRESULT result = wic_bitmap_decoder->GetThumbnail(bitmap_source.put());
        Assert::AreEqual(WINCODEC_ERR_CODECNOTHUMBNAIL, result);
    }

    TEST_METHOD(GetFrameCount)
    {
        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = CreateDecoder();

        uint32_t frame_count;
        const HRESULT result = wic_bitmap_decoder->GetFrameCount(&frame_count);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(1u, frame_count);
    }

private:
    com_ptr<IWICBitmapDecoder> CreateDecoder()
    {
        com_ptr<IWICBitmapDecoder> decoder;

        DllGetClassObjectPtr dll_get_class_object = reinterpret_cast<DllGetClassObjectPtr>(GetProcAddress(library_, "DllGetClassObject"));
        if (!dll_get_class_object)
            throw_last_error();

        com_ptr<IClassFactory> class_factory;
        check_hresult(dll_get_class_object(JpegLsDecoder, IID_PPV_ARGS(class_factory.put())));

        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder;
        check_hresult(class_factory->CreateInstance(nullptr, IID_PPV_ARGS(wic_bitmap_decoder.put())));

        return wic_bitmap_decoder;
    }

    HINSTANCE library_{};
};
