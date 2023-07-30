// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#include "pch.h"

#include "factory.h"
#include "portable_anymap_file.h"
#include "util.h"

#include "../src/errors.h"
#include "charls/charls_jpegls_encoder.h"

#include <CppUnitTest.h>

#include <array>
#include <cstddef>
#include <span>
#include <vector>


using std::array;
using std::vector;
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

        const HRESULT result{bitmap_frame_decoder->GetSize(&width, &height)};
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(512U, width);
        Assert::AreEqual(512U, height);
    }

    TEST_METHOD(CopyPalette) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        com_ptr<IWICPalette> palette;
        check_hresult(imaging_factory()->CreatePalette(palette.put()));

        const HRESULT result{bitmap_frame_decoder->CopyPalette(palette.get())};
        Assert::AreEqual(wincodec::error_palette_unavailable, result);
    }

    TEST_METHOD(GetThumbnail) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        com_ptr<IWICBitmapSource> thumbnail;
        const HRESULT result{bitmap_frame_decoder->GetThumbnail(thumbnail.put())};
        Assert::AreEqual(wincodec::error_codec_no_thumbnail, result);
    }

    TEST_METHOD(GetPixelFormat) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        GUID pixel_format;
        const HRESULT result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat8bppGray == pixel_format);
    }

    TEST_METHOD(GetPixelFormat_with_nullptr) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        WARNING_SUPPRESS_NEXT_LINE(6387)
        const HRESULT result{bitmap_frame_decoder->GetPixelFormat(nullptr)};
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(GetResolution_jpegls_no_spiff_header) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        double dpi_x;
        double dpi_y;
        const HRESULT result{bitmap_frame_decoder->GetResolution(&dpi_x, &dpi_y)};
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(96., dpi_x);
        Assert::AreEqual(96., dpi_y);
    }

    TEST_METHOD(GetResolution_dots_per_inch_in_spiff_header) // NOLINT
    {
        charls::jpegls_encoder encoder;
        encoder.frame_info({1, 1, 8, 1});
        std::vector<std::byte> destination(encoder.estimated_destination_size());
        encoder.destination(destination);

        encoder.write_standard_spiff_header(charls::spiff_color_space::grayscale,
                                            charls::spiff_resolution_units::dots_per_inch, 100, 100);

        constexpr std::array pixel_data{std::byte{0}};
        destination.resize(encoder.encode(pixel_data));

        const com_ptr bitmap_frame_decoder{create_frame_decoder(destination)};

        double dpi_x;
        double dpi_y;
        const HRESULT result{bitmap_frame_decoder->GetResolution(&dpi_x, &dpi_y)};
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(100., dpi_x);
        Assert::AreEqual(100., dpi_y);
    }

    TEST_METHOD(GetResolution_dots_per_centimeter_in_spiff_header) // NOLINT
    {
        charls::jpegls_encoder encoder;
        encoder.frame_info({1, 1, 8, 1});
        std::vector<std::byte> destination(encoder.estimated_destination_size());
        encoder.destination(destination);

        encoder.write_standard_spiff_header(charls::spiff_color_space::grayscale,
                                            charls::spiff_resolution_units::dots_per_centimeter, 100, 100);

        constexpr std::array pixel_data{std::byte{0}};
        destination.resize(encoder.encode(pixel_data));

        const com_ptr bitmap_frame_decoder{create_frame_decoder(destination)};

        double dpi_x;
        double dpi_y;
        const HRESULT result{bitmap_frame_decoder->GetResolution(&dpi_x, &dpi_y)};
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(254., dpi_x);
        Assert::AreEqual(254., dpi_y);
    }

    TEST_METHOD(GetResolution_with_nullptr) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        WARNING_SUPPRESS_NEXT_LINE(6387)
        const HRESULT result{bitmap_frame_decoder->GetResolution(nullptr, nullptr)};
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(GetColorContexts) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        uint32_t actual_count;
        HRESULT result{bitmap_frame_decoder->GetColorContexts(0, nullptr, &actual_count)};
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(0U, actual_count);

        array<IWICColorContext*, 1> color_contexts{};
        result = bitmap_frame_decoder->GetColorContexts(static_cast<UINT>(color_contexts.size()), color_contexts.data(),
                                                        &actual_count);
        Assert::AreEqual(error_ok, result);
        Assert::AreEqual(0U, actual_count);
    }

    TEST_METHOD(GetColorContexts_with_nullptr) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        WARNING_SUPPRESS_NEXT_LINE(
            6387) // parameter 3 could be 'nullptr': this does not adhere to the specification for the function
        const HRESULT result{bitmap_frame_decoder->GetColorContexts(0, nullptr, nullptr)};
        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(GetMetadataQueryReader) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        com_ptr<IWICMetadataQueryReader> metadata_query_reader;
        const HRESULT result{bitmap_frame_decoder->GetMetadataQueryReader(metadata_query_reader.put())};
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(CopyPixels) // NOLINT
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(L"tulips-gray-8bit-512-512.jls")};

        uint32_t width;
        uint32_t height;
        check_hresult(bitmap_frame_decoder->GetSize(&width, &height));
        std::vector<std::byte> buffer(static_cast<size_t>(width) * height);

        HRESULT result{bitmap_frame_decoder->CopyPixels(nullptr, width, static_cast<uint32_t>(buffer.size()),
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

    TEST_METHOD(decode_2_bit_monochrome_4_pixels) // NOLINT
    {
        decode_2_bit_monochrome(L"2bit_4x1.jls", "2bit_4x1.pgm");
    }

    TEST_METHOD(decode_2_bit_monochrome_5_pixels) // NOLINT
    {
        decode_2_bit_monochrome(L"2bit_5x1.jls", "2bit_5x1.pgm");
    }

    TEST_METHOD(decode_2_bit_monochrome_6_pixels) // NOLINT
    {
        decode_2_bit_monochrome(L"2bit_6x1.jls", "2bit_6x1.pgm");
    }

    TEST_METHOD(decode_2_bit_monochrome_7_pixels) // NOLINT
    {
        decode_2_bit_monochrome(L"2bit_7x1.jls", "2bit_7x1.pgm");
    }

    TEST_METHOD(decode_2_bit_monochrome_parrot) // NOLINT
    {
        decode_2_bit_monochrome(L"2bit-parrot-150x200.jls", "2bit-parrot-150x200.pgm");
    }

    TEST_METHOD(decode_4_bit_monochrome_4_pixels) // NOLINT
    {
        decode_4_bit_monochrome(L"4bit_4x1.jls", "4bit_4x1.pgm");
    }

    TEST_METHOD(decode_4_bit_monochrome_5_pixels) // NOLINT
    {
        decode_4_bit_monochrome(L"4bit_5x1.jls", "4bit_5x1.pgm");
    }

    TEST_METHOD(decode_monochrome_4_bit) // NOLINT
    {
        decode_4_bit_monochrome(L"4bit-monochrome.jls", "4bit-monochrome.pgm");
    }

    TEST_METHOD(decode_8_bit_monochrome) // NOLINT
    {
        decode_8bit_monochrome(L"tulips-gray-8bit-512-512.jls", "tulips-gray-8bit-512-512.pgm");
    }

    TEST_METHOD(decode_8_bit_monochrome_stride_mismatch) // NOLINT
    {
        decode_8bit_monochrome(L"8bit_2x2.jls", "8bit_2x2.pgm");
    }

    TEST_METHOD(decode_10_bit_monochrome) // NOLINT
    {
        decode_16_bit_monochrome(L"10bit_3x2.jls", "10bit_3x2.pgm");
    }

    TEST_METHOD(decode_12_bit_monochrome) // NOLINT
    {
        decode_16_bit_monochrome(L"12bit_3x2.jls", "12bit_3x2.pgm");
    }

    TEST_METHOD(decode_16_bit_monochrome) // NOLINT
    {
        decode_16_bit_monochrome(L"16bit_3x2.jls", "16bit_3x2.pgm");
    }

    TEST_METHOD(decode_8_bit_rgb_interleave_none) // NOLINT
    {
        decode_8_bit_rgb(L"8bit_rgb_interleave_none.jls", "test8.ppm");
    }

    TEST_METHOD(decode_8_bit_rgb_interleave_line) // NOLINT
    {
        decode_8_bit_rgb(L"8bit_rgb_interleave_line.jls", "test8.ppm");
    }

    TEST_METHOD(decode_8_bit_rgb_interleave_sample) // NOLINT
    {
        decode_8_bit_rgb(L"8bit_rgb_interleave_sample.jls", "test8.ppm");
    }

    TEST_METHOD(decode_16_bit_rgb_interleave_none) // NOLINT
    {
        decode_16_bit_rgb(L"16bit_3x2_rgb_interleave_none.jls", "16bit_3x2_rgb.ppm");
    }

    TEST_METHOD(decode_16_bit_rgb_interleave_line) // NOLINT
    {
        decode_16_bit_rgb(L"16bit_3x2_rgb_interleave_line.jls", "16bit_3x2_rgb.ppm");
    }

    TEST_METHOD(decode_16_bit_rgb_interleave_sample) // NOLINT
    {
        decode_16_bit_rgb(L"16bit_3x2_rgb_interleave_line.jls", "16bit_3x2_rgb.ppm");
    }

private:
    void decode_2_bit_monochrome(_Null_terminated_ const wchar_t* filename_actual,
                                 _Null_terminated_ const char* filename_expected) const
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(filename_actual)};

        const auto [width, height]{get_size(*bitmap_frame_decoder)};
        vector<std::byte> buffer(static_cast<size_t>(width) * height);

        const uint32_t stride{(width + 15) / 16 * 4};
        const HRESULT result{copy_pixels<std::byte>(*bitmap_frame_decoder.get(), stride, buffer)};
        Assert::AreEqual(error_ok, result);

        const std::vector decoded_buffer{unpack_crumbs(buffer.data(), width, height, stride)};
        compare(filename_expected, decoded_buffer);
    }

    void decode_4_bit_monochrome(_Null_terminated_ const wchar_t* filename_actual,
                                 _Null_terminated_ const char* filename_expected) const
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(filename_actual)};

        GUID pixel_format;
        HRESULT result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat4bppGray == pixel_format);

        const auto [width, height]{get_size(*bitmap_frame_decoder)};
        vector<std::byte> buffer(static_cast<size_t>(width) * height);

        const uint32_t stride{(width + 7) / 8 * 4};
        result = copy_pixels<std::byte>(*bitmap_frame_decoder.get(), stride, buffer);
        Assert::AreEqual(error_ok, result);

        const std::vector decoded_buffer{unpack_nibbles(buffer.data(), width, height, stride)};
        compare(filename_expected, decoded_buffer);
    }

    void decode_8bit_monochrome(_Null_terminated_ const wchar_t* filename_actual,
                                _Null_terminated_ const char* filename_expected) const
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(filename_actual)};

        GUID pixel_format;
        HRESULT result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat8bppGray == pixel_format);

        const auto [width, height]{get_size(*bitmap_frame_decoder)};
        vector<std::byte> buffer(static_cast<size_t>(width) * height);

        result = copy_pixels<std::byte>(*bitmap_frame_decoder.get(), width, buffer);
        Assert::AreEqual(error_ok, result);

        compare(filename_expected, buffer);
    }

    void decode_16_bit_monochrome(_Null_terminated_ const wchar_t* filename_actual,
                                  _Null_terminated_ const char* filename_expected) const
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(filename_actual)};

        GUID pixel_format;
        HRESULT result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat16bppGray == pixel_format);

        const auto [width, height]{get_size(*bitmap_frame_decoder)};
        vector<std::uint16_t> buffer(static_cast<size_t>(width) * height);

        result = copy_pixels<uint16_t>(*bitmap_frame_decoder.get(), width, buffer);
        Assert::AreEqual(error_ok, result);

        compare(filename_expected, buffer);
    }

    void decode_8_bit_rgb(_Null_terminated_ const wchar_t* filename_actual, _Null_terminated_ const char* filename_expected)
        const
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(filename_actual)};

        GUID pixel_format;
        HRESULT result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat24bppRGB == pixel_format);

        const auto [width, height]{get_size(*bitmap_frame_decoder)};
        vector<std::byte> buffer(static_cast<size_t>(width) * height * 3);

        result = copy_pixels<std::byte>(*bitmap_frame_decoder.get(), width * 3, buffer);
        Assert::AreEqual(error_ok, result);

        compare(filename_expected, buffer);
    }

    void decode_16_bit_rgb(_Null_terminated_ const wchar_t* filename_actual, _Null_terminated_ const char* filename_expected)
        const
    {
        const com_ptr bitmap_frame_decoder{create_frame_decoder(filename_actual)};

        GUID pixel_format;
        HRESULT result{bitmap_frame_decoder->GetPixelFormat(&pixel_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_WICPixelFormat48bppRGB == pixel_format);

        const auto [width, height]{get_size(*bitmap_frame_decoder)};
        vector<std::uint16_t> buffer(static_cast<size_t>(width) * height * 3);

        result = copy_pixels<std::uint16_t>(*bitmap_frame_decoder.get(), width * 3, buffer);
        Assert::AreEqual(error_ok, result);

        compare(filename_expected, buffer);
    }

    [[nodiscard]]
    static std::vector<std::byte> unpack_nibbles(const std::byte* nibble_pixels, const size_t width, const size_t height,
                                                 const size_t stride)
    {
        std::vector<std::byte> destination(static_cast<size_t>(width) * height);

        for (size_t j{}, row{}; row != height; ++row)
        {
            const std::byte* nibble_row{nibble_pixels + (row * stride)};
            size_t i{};
            for (; i != width / 2; ++i)
            {
                destination[j++] = nibble_row[i] >> 4;
                destination[j++] = nibble_row[i] & std::byte{0x0F};
            }
            if (width % 2)
            {
                destination[j++] = nibble_row[i] >> 4;
            }
        }

        return destination;
    }

    [[nodiscard]]
    com_ptr<IWICBitmapFrameDecode> create_frame_decoder(_Null_terminated_ const wchar_t* filename) const
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(filename, STGM_READ | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        const com_ptr wic_bitmap_decoder{factory_.create_decoder()};
        check_hresult(wic_bitmap_decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand));

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        check_hresult(wic_bitmap_decoder->GetFrame(0, bitmap_frame_decode.put()));

        return bitmap_frame_decode;
    }

    [[nodiscard]]
    com_ptr<IWICBitmapFrameDecode> create_frame_decoder(const std::span<const std::byte> jpegls_buffer) const
    {
        const com_ptr<IStream> stream{
            SHCreateMemStream(reinterpret_cast<const BYTE*>(jpegls_buffer.data()), static_cast<UINT>(jpegls_buffer.size())),
            take_ownership_from_abi};

        const com_ptr wic_bitmap_decoder{factory_.create_decoder()};
        check_hresult(wic_bitmap_decoder->Initialize(stream.get(), WICDecodeMetadataCacheOnDemand));

        com_ptr<IWICBitmapFrameDecode> bitmap_frame_decode;
        check_hresult(wic_bitmap_decoder->GetFrame(0, bitmap_frame_decode.put()));

        return bitmap_frame_decode;
    }

    [[nodiscard]]
    static com_ptr<IWICImagingFactory> imaging_factory()
    {
        com_ptr<IWICImagingFactory> imaging_factory;
        check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
                                       imaging_factory.put_void()));

        return imaging_factory;
    }

    [[nodiscard]]
    static std::vector<std::byte> unpack_crumbs(const std::byte* crumbs_pixels, const size_t width,
                                                const size_t height, const size_t stride)
    {
        std::vector<std::byte> destination(static_cast<size_t>(width) * height);

        for (size_t j{}, row{}; row != height; ++row)
        {
            const std::byte* crumbs_row{crumbs_pixels + (row * stride)};
            size_t i{};
            for (; i != width / 4; ++i)
            {
                destination[j] = crumbs_row[i] >> 6;
                ++j;
                destination[j] = (crumbs_row[i] & std::byte{0x30}) >> 4;
                ++j;
                destination[j] = (crumbs_row[i] & std::byte{0x0C}) >> 2;
                ++j;
                destination[j] = crumbs_row[i] & std::byte{0x03};
                ++j;
            }
            switch (width % 4)
            {
            case 3:
                destination[j] = crumbs_row[i] >> 6;
                ++j;
                [[fallthrough]];
            case 2:
                destination[j] = (crumbs_row[i] & std::byte{0x30}) >> 4;
                ++j;
                [[fallthrough]];
            case 1:
                destination[j] = (crumbs_row[i] & std::byte{0x0C}) >> 2;
                ++j;
                break;

            default:
                break;
            }
        }

        return destination;
    }

    [[nodiscard]]
    static std::pair<uint32_t, uint32_t> get_size(IWICBitmapFrameDecode& bitmap_frame_decoder)
    {
        uint32_t width;
        uint32_t height;
        check_hresult(bitmap_frame_decoder.GetSize(&width, &height));

        return {width, height};
    }

    template<typename SampleType>
    [[nodiscard]]
    static hresult copy_pixels(IWICBitmapFrameDecode& decoder, const uint32_t stride,
                               const std::span<SampleType> buffer)
    {
        void* data = buffer.data();
        return decoder.CopyPixels(nullptr, stride * sizeof(SampleType),
                                  static_cast<uint32_t>(buffer.size() * sizeof(SampleType)), static_cast<BYTE*>(data));
    }

    static void compare(const char* filename, const vector<std::byte>& pixels)
    {
        portable_anymap_file anymap_file{filename};
        const auto& expected_pixels{anymap_file.image_data()};

        for (size_t i{}; i < pixels.size(); ++i)
        {
            if (expected_pixels[i] != pixels[i])
            {
                Assert::IsTrue(false);
                break;
            }
        }
    }

    static void shift_samples(uint16_t * pixels, const size_t pixel_count, const uint32_t sample_shift)
    {
        std::transform(pixels, pixels + pixel_count, pixels,
                       [sample_shift](const uint16_t pixel) -> uint16_t { return pixel << sample_shift; });
    }

    static void compare(const char* filename, const vector<uint16_t>& pixels)
    {
        portable_anymap_file anymap_file{filename};
        const auto expected_pixels{anymap_file.image_data_as_uint16()};

        shift_samples(expected_pixels.data(), expected_pixels.size(), 16 - anymap_file.bits_per_sample());

        for (size_t i{}; i < pixels.size(); ++i)
        {
            if (expected_pixels[i] != pixels[i])
            {
                Assert::IsTrue(false);
                break;
            }
        }
    }

    factory factory_;
};
