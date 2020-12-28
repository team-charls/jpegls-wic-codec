// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "factory.h"
#include "util.h"

#include "../src/errors.h"

#include <CppUnitTest.h>

#include <array>
#include <vector>

using std::array;
using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(jpegls_bitmap_frame_decode_test)
{
public:
    TEST_METHOD(GetSize) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        uint32_t width;
        uint32_t height;

        const hresult result = bitmap_frame_decoder->GetSize(&width, &height);
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(512U, width);
        Assert::AreEqual(512U, height);
    }

    TEST_METHOD(CopyPalette) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        com_ptr<IWICPalette> palette;
        check_hresult(imaging_factory()->CreatePalette(palette.put()));

        const hresult result = bitmap_frame_decoder->CopyPalette(palette.get());
        Assert::AreEqual(wincodec::error_palette_unavailable, result);
    }

    TEST_METHOD(GetThumbnail) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        com_ptr<IWICBitmapSource> thumbnail;
        const hresult result = bitmap_frame_decoder->GetThumbnail(thumbnail.put());
        Assert::AreEqual(wincodec::error_codec_no_thumbnail, result);
    }

    TEST_METHOD(GetPixelFormat) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        GUID pixel_format;
        const hresult result = bitmap_frame_decoder->GetPixelFormat(&pixel_format);
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat8bppGray == pixel_format);
    }

    TEST_METHOD(GetPixelFormat_with_nullptr) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        WARNING_SUPPRESS_NEXT_LINE(6387)
        const hresult result = bitmap_frame_decoder->GetPixelFormat(nullptr);
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(GetResolution_jpegls_no_spiff_header) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        double dpi_x;
        double dpi_y;
        const hresult result = bitmap_frame_decoder->GetResolution(&dpi_x, &dpi_y);
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(96., dpi_x);
        Assert::AreEqual(96., dpi_y);
    }

    TEST_METHOD(GetResolution_with_nullptr) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        WARNING_SUPPRESS_NEXT_LINE(6387)
        const hresult result = bitmap_frame_decoder->GetResolution(nullptr, nullptr);
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(GetColorContexts) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        uint32_t actual_count;
        hresult result = bitmap_frame_decoder->GetColorContexts(0, nullptr, &actual_count);
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(0U, actual_count);

        array<IWICColorContext*, 1> color_contexts{};
        result = bitmap_frame_decoder->GetColorContexts(static_cast<UINT>(color_contexts.size()), color_contexts.data(),
                                                        &actual_count);
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(0U, actual_count);
    }

    TEST_METHOD(GetMetadataQueryReader) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        com_ptr<IWICMetadataQueryReader> metadata_query_reader;
        const hresult result = bitmap_frame_decoder->GetMetadataQueryReader(metadata_query_reader.put());
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(CopyPixels) // NOLINT
    {
        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decoder = create_frame_decoder(L"lena8b.jls");

        uint32_t width;
        uint32_t height;

        check_hresult(bitmap_frame_decoder->GetSize(&width, &height));
        std::vector<BYTE> buffer(static_cast<size_t>(width) * height);

        hresult result =
            bitmap_frame_decoder->CopyPixels(nullptr, width, static_cast<uint32_t>(buffer.size()), buffer.data());
        Assert::AreEqual(error_ok, result);

        result = bitmap_frame_decoder->CopyPixels(nullptr, width, static_cast<uint32_t>(buffer.size()), buffer.data());
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(IsIWICBitmapSource) // NOLINT
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
        check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
                                       imaging_factory.put_void()));

        return imaging_factory;
    }

    factory factory_;
};
