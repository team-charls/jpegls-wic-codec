// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "factory.h"

#include "../src/util.h"

#include <CppUnitTest.h>

#include <vector>

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(jpegls_bitmap_frame_decode_test)
{
public:
    TEST_METHOD(GetSize)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        uint32_t width;
        uint32_t height;

        const HRESULT result = bitmap_frame_decoder->GetSize(&width, &height);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(512U, width);
        Assert::AreEqual(512U, height);
    }

    TEST_METHOD(CopyPalette)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        com_ptr<IWICPalette> palette;
        check_hresult(imaging_factory()->CreatePalette(palette.put()));

        const HRESULT result = bitmap_frame_decoder->CopyPalette(palette.get());
        Assert::AreEqual(WINCODEC_ERR_PALETTEUNAVAILABLE, result);
    }

    TEST_METHOD(GetThumbnail)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        com_ptr<IWICBitmapSource> thumbnail;
        const HRESULT result = bitmap_frame_decoder->GetThumbnail(thumbnail.put());
        Assert::AreEqual(WINCODEC_ERR_CODECNOTHUMBNAIL, result);
    }

    TEST_METHOD(GetPixelFormat)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        GUID pixel_format;
        const HRESULT result = bitmap_frame_decoder->GetPixelFormat(&pixel_format);
        Assert::AreEqual(S_OK, result);
        Assert::IsTrue(GUID_WICPixelFormat8bppGray == pixel_format);
    }

    TEST_METHOD(GetColorContexts)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        uint32_t actual_count;
        HRESULT result = bitmap_frame_decoder->GetColorContexts(0, nullptr, &actual_count);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(0U, actual_count);

        IWICColorContext* color_contexts[1]{};
        result = bitmap_frame_decoder->GetColorContexts(1, color_contexts, &actual_count);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(0U, actual_count);
    }

    TEST_METHOD(GetMetadataQueryReader)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        com_ptr<IWICMetadataQueryReader> metadata_query_reader;
        const HRESULT result = bitmap_frame_decoder->GetMetadataQueryReader(metadata_query_reader.put());
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(CopyPixels)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        uint32_t width;
        uint32_t height;

        check_hresult(bitmap_frame_decoder->GetSize(&width, &height));
        std::vector<BYTE> buffer(static_cast<size_t>(width) * height);

        HRESULT result = bitmap_frame_decoder->CopyPixels(nullptr, width, static_cast<uint32_t>(buffer.size()), buffer.data());
        Assert::AreEqual(S_OK, result);

        result = bitmap_frame_decoder->CopyPixels(nullptr, width, static_cast<uint32_t>(buffer.size()), buffer.data());
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(IsIWICBitmapSource)
    {
        const com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        const com_ptr<IWICBitmapSource> bitmap_source(bitmap_frame_decoder);
        Assert::IsTrue(bitmap_source.get() != nullptr);
    }

private:
    [[nodiscard]] com_ptr<IWICBitmapFrameDecode> create_frame_decoder(const PCWSTR filename) const
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(filename, STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.create_decoder();
        check_hresult(wic_bitmap_decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand));

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        check_hresult(wic_bitmap_decoder->GetFrame(0, bitmap_frame_decode.put()));

        return bitmap_frame_decode;
    }

    [[nodiscard]] static com_ptr<IWICImagingFactory> imaging_factory()
    {
        com_ptr<IWICImagingFactory> imaging_factory;
        check_hresult(CoCreateInstance(CLSID_WICImagingFactory,
            nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, imaging_factory.put_void()));

        return imaging_factory;
    }

    factory factory_;
};
