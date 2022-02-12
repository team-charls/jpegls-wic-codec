// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "factory.h"
#include "portable_anymap_file.h"
#include "util.h"

#include "../src/errors.h"

#include <CppUnitTest.h>

#include <array>
#include <vector>
#include <cstddef>

using std::array;
using std::byte;
using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(jpegls_bitmap_frame_decode_test)
{
public:
    TEST_METHOD(GetSize) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        uint32_t width;
        uint32_t height;

        const hresult result{bitmap_frame_decoder->GetSize(&width, &height)};
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(512U, width);
        Assert::AreEqual(512U, height);
    }

    TEST_METHOD(CopyPalette) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        com_ptr<IWICPalette> palette;
        check_hresult(imaging_factory()->CreatePalette(palette.put()));

        const hresult result{bitmap_frame_decoder->CopyPalette(palette.get())};
        Assert::AreEqual(wincodec::error_palette_unavailable, result);
    }

    TEST_METHOD(GetThumbnail) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        com_ptr<IWICBitmapSource> thumbnail;
        const hresult result{bitmap_frame_decoder->GetThumbnail(thumbnail.put())};
        Assert::AreEqual(wincodec::error_codec_no_thumbnail, result);
    }

    TEST_METHOD(GetPixelFormat) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        GUID pixel_format;
        const hresult result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat8bppGray == pixel_format);
    }

    TEST_METHOD(GetPixelFormat_with_nullptr) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        WARNING_SUPPRESS_NEXT_LINE(6387)
        const hresult result{bitmap_frame_decoder->GetPixelFormat(nullptr)};
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(GetResolution_jpegls_no_spiff_header) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        double dpi_x;
        double dpi_y;
        const hresult result{bitmap_frame_decoder->GetResolution(&dpi_x, &dpi_y)};
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(96., dpi_x);
        Assert::AreEqual(96., dpi_y);
    }

    TEST_METHOD(GetResolution_with_nullptr) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        WARNING_SUPPRESS_NEXT_LINE(6387)
        const hresult result{bitmap_frame_decoder->GetResolution(nullptr, nullptr)};
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(GetColorContexts) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        uint32_t actual_count;
        hresult result{bitmap_frame_decoder->GetColorContexts(0, nullptr, &actual_count)};
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
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        com_ptr<IWICMetadataQueryReader> metadata_query_reader;
        const hresult result{bitmap_frame_decoder->GetMetadataQueryReader(metadata_query_reader.put())};
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(CopyPixels) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        uint32_t width;
        uint32_t height;
        check_hresult(bitmap_frame_decoder->GetSize(&width, &height));
        std::vector<byte> buffer(static_cast<size_t>(width) * height);

        hresult result{bitmap_frame_decoder->CopyPixels(nullptr, width, static_cast<uint32_t>(buffer.size()),
                                                        reinterpret_cast<BYTE*>(buffer.data()))};
        Assert::AreEqual(error_ok, result);

        result = bitmap_frame_decoder->CopyPixels(nullptr, width, static_cast<uint32_t>(buffer.size()),
                                                  reinterpret_cast<BYTE*>(buffer.data()));
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(IsIWICBitmapSource) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        const com_ptr<IWICBitmapSource> bitmap_source(bitmap_frame_decoder);
        Assert::IsTrue(bitmap_source.get() != nullptr);
    }

    TEST_METHOD(decode_monochrome_2_bit) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"2bit-parrot-150x200.jls")};

        GUID pixel_format;
        hresult result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat2bppGray == pixel_format);

        uint32_t width;
        uint32_t height;
        result = bitmap_frame_decoder->GetSize(&width, &height);
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(150U, width);
        Assert::AreEqual(200U, height);

        constexpr uint32_t stride{40};

        std::vector<byte> buffer(static_cast<size_t>(stride) * height);
        result = bitmap_frame_decoder->CopyPixels(nullptr, stride, static_cast<uint32_t>(buffer.size()),
                                                  reinterpret_cast<BYTE*>(buffer.data()));
        Assert::AreEqual(error_ok, result);

        //std::vector<byte> decoded_buffer{unpack_nibbles(buffer.data(), width, height, stride)};
        //portable_anymap_file anymap_file{"4bit-monochrome.pgm"};

        //for (size_t i{}; i != decoded_buffer.size(); ++i)
        //{
        //    if (anymap_file.image_data()[i] != decoded_buffer[i])
        //    {
        //        Assert::IsTrue(false);
        //        break;
        //    }
        //}
    }

    TEST_METHOD(decode_monochrome_4_bit) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"4bit-monochrome.jls")};

        GUID pixel_format;
        hresult result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat4bppGray == pixel_format);

        uint32_t width;
        uint32_t height;
        result = bitmap_frame_decoder->GetSize(&width, &height);
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(360U, width);
        Assert::AreEqual(360U, height);
        const uint32_t stride{width / 2};

        std::vector<byte> buffer(static_cast<size_t>(stride) * height);
        result = bitmap_frame_decoder->CopyPixels(nullptr, stride, static_cast<uint32_t>(buffer.size()),
                                                  reinterpret_cast<BYTE*>(buffer.data()));
        Assert::AreEqual(error_ok, result);

        std::vector<byte> decoded_buffer{unpack_nibbles(buffer.data(), width, height, stride)};
        portable_anymap_file anymap_file{"4bit-monochrome.pgm"};

        for (size_t i{}; i != decoded_buffer.size(); ++i)
        {
            if (anymap_file.image_data()[i] != decoded_buffer[i])
            {
                Assert::IsTrue(false);
                break;
            }
        }
    }

private:
    [[nodiscard]] static std::vector<byte> unpack_nibbles(const std::byte* nibble_pixels, const size_t width,
                                                          const size_t height, const size_t stride)
    {
        std::vector<byte> destination(static_cast<size_t>(width) * height);

        for (size_t j{}, row{}; row != height; ++row)
        {
            const std::byte* nibble_row{nibble_pixels + (row * stride)};
            for (size_t i{}; i != width / 2; ++i)
            {
                destination[j] = nibble_row[i] >> 4;
                ++j;
                constexpr byte mask{0x0F};
                destination[j] = (nibble_row[i] & mask);
                ++j;
            }
        }

        return destination;
    }

    [[nodiscard]] com_ptr<IWICBitmapFrameDecode> create_frame_decoder(_Null_terminated_ const wchar_t* filename) const
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(filename, STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        const com_ptr wic_bitmap_decoder{factory_.create_decoder()};
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
