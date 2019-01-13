// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "factory.h"

#include "../src/util.h"

#include <CppUnitTest.h>

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(jpegls_bitmap_frame_decoder_test)
{
public:
    TEST_METHOD(GetSize)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = CreateFrameDecoder(L"lena8b.jls");

        uint32_t width;
        uint32_t height;

        const HRESULT result = bitmap_frame_decoder->GetSize(&width, &height);
        Assert::AreEqual(S_OK, result);
        Assert::AreEqual(512u, width);
        Assert::AreEqual(512u, height);
    }

    TEST_METHOD(CopyPalette)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = CreateFrameDecoder(L"lena8b.jls");

        com_ptr<IWICPalette> palette;
        const HRESULT result = bitmap_frame_decoder->CopyPalette(palette.get());
        Assert::AreEqual(WINCODEC_ERR_PALETTEUNAVAILABLE, result);
    }

    TEST_METHOD(GetThumbnail)
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = CreateFrameDecoder(L"lena8b.jls");

        com_ptr<IWICBitmapSource> thumbnail;
        const HRESULT result = bitmap_frame_decoder->GetThumbnail(thumbnail.put());
        Assert::AreEqual(WINCODEC_ERR_CODECNOTHUMBNAIL, result);
    }

private:
    com_ptr<IWICBitmapFrameDecode> CreateFrameDecoder(PCWSTR filename)
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(filename, STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        com_ptr<IWICBitmapDecoder> wic_bitmap_decoder = factory_.CreateDecoder();
        check_hresult(wic_bitmap_decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand));

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        check_hresult(wic_bitmap_decoder->GetFrame(0, bitmap_frame_decode.put()));

        return bitmap_frame_decode;
    }

    factory factory_;
};
