// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#include "pch.h"

#include "factory.h"
#include "portable_anymap_file.h"
#include "util.h"

#include "../src/errors.h"

#include <CppUnitTest.h>

#include <charls/charls.h>

using charls::jpegls_decoder;
using std::ifstream;
using std::vector;
using winrt::check_hresult;
using winrt::com_ptr;
using winrt::hresult;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace {

[[nodiscard]]
GUID get_pixel_format(const int32_t bits_per_sample, const int32_t component_count, const bool bgr_input = false)
{
    switch (component_count)
    {
    case 1:
        switch (bits_per_sample)
        {
        case 2:
            return GUID_WICPixelFormat2bppGray;
        case 4:
            return GUID_WICPixelFormat4bppGray;
        case 8:
            return GUID_WICPixelFormat8bppGray;
        case 16:
            return GUID_WICPixelFormat16bppGray;

        default:
            Assert::Fail();
        }

    case 3:
        switch (bits_per_sample)
        {
        case 8:
            return bgr_input ? GUID_WICPixelFormat24bppBGR : GUID_WICPixelFormat24bppRGB;
        case 16:
            return GUID_WICPixelFormat48bppRGB;

        default:
            Assert::Fail();
        }

    default:
        break;
    }

    Assert::Fail();
}

[[nodiscard]]
vector<std::byte> read_file(const wchar_t* filename)
{
    ifstream input;
    input.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
    input.open(filename, ifstream::in | ifstream::binary);

    input.seekg(0, ifstream::end);
    const auto byte_count_file{static_cast<int>(input.tellg())};
    input.seekg(0, ifstream::beg);

    vector<std::byte> buffer(byte_count_file);
    input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    return buffer;
}

[[nodiscard]]
uint32_t compute_stride(const charls::frame_info& frame_info) noexcept
{
    uint32_t stride;

    if (frame_info.bits_per_sample == 2)
    {
        stride = (frame_info.width + 3) / 4 * frame_info.component_count;
    }
    else if (frame_info.bits_per_sample == 4)
    {
        stride = (frame_info.width + 1) / 2 * frame_info.component_count;
    }
    else if (frame_info.bits_per_sample == 8)
    {
        stride = frame_info.width * frame_info.component_count;
    }
    else
    {
        stride = frame_info.width * 2 * frame_info.component_count;
    }

    return ((stride + 3) / 4) * 4;
}

[[nodiscard]]
vector<std::byte> pack_to_crumbs(const std::span<const std::byte> byte_pixels, const size_t width, const size_t height,
                                 const size_t stride) noexcept
{
    vector<std::byte> crumb_pixels(stride * height);

    size_t j{};
    for (size_t row{}; row != height; ++row)
    {
        std::byte* crumb_row{crumb_pixels.data() + (row * stride)};
        size_t i{};
        for (; i != width / 4; ++i)
        {
            std::byte value{byte_pixels[j++] << 6};
            value |= byte_pixels[j++] << 4;
            value |= byte_pixels[j++] << 2;
            value |= byte_pixels[j++];
            crumb_row[i] = value;
        }

        switch (width % 4)
        {
        case 3:
            crumb_row[i] = byte_pixels[j++] << 6;
            [[fallthrough]];

        case 2:
            crumb_row[i] |= byte_pixels[j++] << 4;
            [[fallthrough]];

        case 1:
            crumb_row[i] |= byte_pixels[j++] << 2;
            break;

        default:
            break;
        }
    }

    return crumb_pixels;
}

[[nodiscard]]
vector<std::byte> pack_to_nibbles(const std::span<const std::byte> byte_pixels, const size_t width, const size_t height,
                                  const size_t stride) noexcept
{
    vector<std::byte> nibble_pixels(stride * height);

    for (size_t j{}, row{}; row != height; ++row)
    {
        std::byte* nibble_row{nibble_pixels.data() + (row * stride)};
        size_t i{};
        for (; i != width / 2; ++i)
        {
            nibble_row[i] = byte_pixels[j++] << 4;
            nibble_row[i] |= byte_pixels[j++];
        }
        if (width % 2)
        {
            nibble_row[i] = byte_pixels[j++] << 4;
        }
    }

    return nibble_pixels;
}

[[nodiscard]]
vector<std::byte> pack_to_bytes_correct_stride(const std::span<const std::byte> byte_pixels, const size_t width,
                                               const size_t height, const size_t stride) noexcept
{
    vector<std::byte> pixels(stride * height);

    for (size_t j{}, row{}; row != height; ++row)
    {
        std::byte* pixel_row{pixels.data() + (row * stride)};
        for (size_t i{}; i != width; ++i)
        {
            pixel_row[i] = byte_pixels[j++];
        }
    }

    return pixels;
}

[[nodiscard]]
vector<std::byte> pack_to_bytes_correct_stride_16_bit(const std::span<const std::byte> word_pixels, const size_t width,
                                                      const size_t height, const size_t stride, const size_t component_count) noexcept
{
    vector<std::byte> pixels(stride * height);

    for (size_t j{}, row{}; row != height; ++row)
    {
        std::byte* pixel_row{pixels.data() + (row * stride)};
        for (size_t i{}; i != width * 2 * component_count; i += 2)
        {
            pixel_row[i] = word_pixels[j++];
            pixel_row[i + 1] = word_pixels[j++];
        }
    }

    return pixels;
}


constexpr void convert_rgb_to_bgr_in_place(const std::span<std::byte> pixels) noexcept
{
    for (size_t i{}; i < pixels.size(); i += 3)
    {
        std::swap(pixels[i], pixels[i + 2]);
    }
}

} // namespace


TEST_CLASS(jpegls_bitmap_encoder_test)
{
public:
    TEST_METHOD(GetContainerFormat) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        GUID container_format;
        const HRESULT result{encoder->GetContainerFormat(&container_format)};
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(id::container_format_jpegls == container_format);
    }

    TEST_METHOD(GetContainerFormat_with_nullptr) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        const hresult result{encoder->GetContainerFormat(nullptr)};

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(GetEncoderInfo) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        com_ptr<IWICBitmapEncoderInfo> encoder_info;
        const HRESULT result{encoder->GetEncoderInfo(encoder_info.put())};
        Assert::IsTrue(result == error_ok || result == wincodec::error_component_not_found);

        if (succeeded(result))
        {
            Assert::IsNotNull(encoder_info.get());
        }
        else
        {
            Assert::IsNull(encoder_info.get());
        }
    }

    TEST_METHOD(GetEncoderInfo_with_nullptr) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        const hresult result{encoder->GetEncoderInfo(nullptr)};

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(SetPreview) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        const HRESULT result{encoder->SetPreview(nullptr)};
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(SetThumbnail) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        const HRESULT result{encoder->SetThumbnail(nullptr)};
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(SetColorContexts) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        const HRESULT result{encoder->SetColorContexts(0, nullptr)};
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetMetadataQueryWriter_is_not_supported) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        com_ptr<IWICMetadataQueryWriter> metadata_query_writer;
        const HRESULT result{encoder->GetMetadataQueryWriter(metadata_query_writer.put())};
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
        Assert::IsNull(metadata_query_writer.get());
    }

    TEST_METHOD(SetPalette) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        com_ptr<IWICPalette> palette;
        check_hresult(imaging_factory()->CreatePalette(palette.put()));

        const HRESULT result{encoder->SetPalette(palette.get())};
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(Initialize) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const com_ptr encoder{factory_.create_encoder()};

        const HRESULT result{encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory)};
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_with_nullptr) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        const HRESULT result{encoder->Initialize(nullptr, WICBitmapEncoderCacheInMemory)};
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(Initialize_twice) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const com_ptr encoder{factory_.create_encoder()};

        HRESULT result{encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory)};
        Assert::AreEqual(error_ok, result);

        result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(CreateNewFrame) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        HRESULT result{encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory)};
        Assert::AreEqual(error_ok, result);

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
        Assert::AreEqual(error_ok, result);
        Assert::IsNotNull(frame_encode.get());
    }

    TEST_METHOD(CreateNewFrame_with_property_bag) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        HRESULT result{encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory)};
        Assert::AreEqual(error_ok, result);

        com_ptr<IWICBitmapFrameEncode> frame_encode;

        com_ptr<IPropertyBag2> property_bag;
        result = encoder->CreateNewFrame(frame_encode.put(), property_bag.put());
        Assert::AreEqual(error_ok, result);
        Assert::IsNotNull(frame_encode.get());
        Assert::IsNull(property_bag.get());
    }

    TEST_METHOD(CreateNewFrame_with_nullptr) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const com_ptr encoder{factory_.create_encoder()};

        HRESULT result{encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory)};
        Assert::AreEqual(error_ok, result);

        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        result = encoder->CreateNewFrame(nullptr, nullptr);

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(CreateNewFrame_while_not_initialized) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        const HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
        Assert::AreEqual(wincodec::error_not_initialized, result);
    }

    TEST_METHOD(CreateNewFrame_twice) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const com_ptr encoder{factory_.create_encoder()};

        HRESULT result{encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory)};
        Assert::AreEqual(error_ok, result);

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
        Assert::AreEqual(error_ok, result);

        com_ptr<IWICBitmapFrameEncode> frame_encode2;
        result = encoder->CreateNewFrame(frame_encode2.put(), nullptr);
        Assert::AreEqual(wincodec::error_wrong_state, result);
        Assert::IsNull(frame_encode2.get());
    }

    TEST_METHOD(Commit_while_not_initialized) // NOLINT
    {
        const com_ptr encoder{factory_.create_encoder()};

        const HRESULT result{encoder->Commit()};
        Assert::AreEqual(wincodec::error_not_initialized, result);
    }

    TEST_METHOD(Commit_without_a_frame) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        const com_ptr encoder{factory_.create_encoder()};

        HRESULT result{encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory)};
        Assert::AreEqual(error_ok, result);

        result = encoder->Commit();
        Assert::AreEqual(wincodec::error_frame_missing, result);
    }

    TEST_METHOD(Commit_twice) // NOLINT
    {
        const wchar_t* filename{L"encode_conformance_color_lossless.jls"};
        portable_anymap_file anymap_file{"test8.ppm"};

        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false,
                                             nullptr, stream.put()));

        const com_ptr encoder{factory_.create_encoder()};
        check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

        const GUID pixel_format{get_pixel_format(anymap_file.bits_per_sample(), anymap_file.component_count())};

        com_ptr<IWICBitmap> bitmap;
        check_hresult(imaging_factory()->CreateBitmapFromMemory(
            anymap_file.width(), anymap_file.height(), pixel_format, anymap_file.width() * anymap_file.component_count(),
            static_cast<uint32_t>(anymap_file.image_data().size()), reinterpret_cast<BYTE*>(anymap_file.image_data().data()),
            bitmap.put()));

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
        Assert::AreEqual(error_ok, result);

        result = frame_encode->Initialize(nullptr);
        Assert::AreEqual(error_ok, result);

        result = frame_encode->WriteSource(bitmap.get(), nullptr);
        Assert::AreEqual(error_ok, result);

        result = frame_encode->Commit();
        Assert::AreEqual(error_ok, result);

        result = encoder->Commit();
        Assert::AreEqual(error_ok, result);

        result = encoder->Commit();
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(encode_conformance_color_lossless) // NOLINT
    {
        const wchar_t* filename{L"encode_conformance_color_lossless.jls"};
        portable_anymap_file anymap_file{"test8.ppm"};

        {
            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false,
                                                 nullptr, stream.put()));

            const com_ptr encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(anymap_file.bits_per_sample(), anymap_file.component_count())};

            com_ptr<IWICBitmap> bitmap;
            check_hresult(imaging_factory()->CreateBitmapFromMemory(
                anymap_file.width(), anymap_file.height(), pixel_format, anymap_file.width() * anymap_file.component_count(),
                static_cast<uint32_t>(anymap_file.image_data().size()),
                reinterpret_cast<BYTE*>(anymap_file.image_data().data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Initialize(nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->WriteSource(bitmap.get(), nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Commit();
            Assert::AreEqual(error_ok, result);

            result = encoder->Commit();
            Assert::AreEqual(error_ok, result);
        }

        compare(filename, anymap_file.image_data());
    }

    TEST_METHOD(encode_conformance_color_lossless_bgr) // NOLINT
    {
        const wchar_t* filename{L"encode_conformance_color_lossless_bgr_input.jls"};
        portable_anymap_file anymap_file{"test8.ppm"};

        {
            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false,
                                                 nullptr, stream.put()));

            const com_ptr encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(anymap_file.bits_per_sample(), anymap_file.component_count(), true)};

            auto input_data{anymap_file.image_data()};
            convert_rgb_to_bgr_in_place(input_data);

            com_ptr<IWICBitmap> bitmap;
            check_hresult(imaging_factory()->CreateBitmapFromMemory(
                anymap_file.width(), anymap_file.height(), pixel_format, anymap_file.width() * anymap_file.component_count(),
                static_cast<uint32_t>(input_data.size()), reinterpret_cast<BYTE*>(input_data.data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Initialize(nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->WriteSource(bitmap.get(), nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Commit();
            Assert::AreEqual(error_ok, result);

            result = encoder->Commit();
            Assert::AreEqual(error_ok, result);
        }

        compare(filename, anymap_file.image_data());
    }

    TEST_METHOD(encode_monochrome_2_bit_4x1) // NOLINT
    {
        encode_monochrome_2_bit("2bit_4x1.pgm", L"2bit_4x1-wic-encoded.jls");
    }

    TEST_METHOD(encode_monochrome_2_bit_5x1) // NOLINT
    {
        encode_monochrome_2_bit("2bit_5x1.pgm", L"2bit_5x1-wic-encoded.jls");
    }

    TEST_METHOD(encode_monochrome_2_bit_6x1) // NOLINT
    {
        encode_monochrome_2_bit("2bit_6x1.pgm", L"2bit_6x1-wic-encoded.jls");
    }

    TEST_METHOD(encode_monochrome_2_bit_7x1) // NOLINT
    {
        encode_monochrome_2_bit("2bit_7x1.pgm", L"2bit_7x1-wic-encoded.jls");
    }

    TEST_METHOD(encode_monochrome_2_bit_150x200) // NOLINT
    {
        encode_monochrome_2_bit("2bit-parrot-150x200.pgm", L"2bit-parrot-150x200-wic-encoded.jls");
    }

    TEST_METHOD(encode_monochrome_4_bit_4x1) // NOLINT
    {
        encode_monochrome_4_bit("4bit_4x1.pgm", L"4bit_4x1-wic-encoded.jls");
    }

    TEST_METHOD(encode_monochrome_4_bit_5x1) // NOLINT
    {
        encode_monochrome_4_bit("4bit_5x1.pgm", L"4bit_5x1-wic-encoded.jls");
    }

    TEST_METHOD(encode_monochrome_4_bit_360x360) // NOLINT
    {
        encode_monochrome_4_bit("4bit-monochrome.pgm", L"4bit-monochrome-wic-encoded.jls");
    }

    TEST_METHOD(encode_monochrome_8_bit) // NOLINT
    {
        const char* source_filename{"8bit_2x2.pgm"};
        const wchar_t* destination_filename{L"8bit_2x2-wic-encoded.jls"};
        portable_anymap_file anymap_file{source_filename};

        {
            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(destination_filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE,
                                                 0, false, nullptr, stream.put()));

            const com_ptr encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(anymap_file.bits_per_sample(), anymap_file.component_count())};

            const uint32_t stride{compute_stride({.width = static_cast<uint32_t>(anymap_file.width()),
                                                  .height = static_cast<uint32_t>(anymap_file.height()),
                                                  .bits_per_sample = 8,
                                                  .component_count = 1})};

            auto byte_pixels{
                pack_to_bytes_correct_stride(anymap_file.image_data(), anymap_file.width(), anymap_file.height(), stride)};

            com_ptr<IWICBitmap> bitmap;
            check_hresult(imaging_factory()->CreateBitmapFromMemory(
                anymap_file.width(), anymap_file.height(), pixel_format, stride, static_cast<uint32_t>(byte_pixels.size()),
                reinterpret_cast<BYTE*>(byte_pixels.data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Initialize(nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->WriteSource(bitmap.get(), nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Commit();
            Assert::AreEqual(error_ok, result);

            result = encoder->Commit();
            Assert::AreEqual(error_ok, result);
        }

        compare(destination_filename, anymap_file.image_data());
    }

    TEST_METHOD(encode_monochrome_16_bit) // NOLINT
    {
        const wchar_t* destination_filename{L"16bit_3x2-wic-encoded.jls"};
        std::array anymap_pixels{std::byte{0}, std::byte{1}, std::byte{0}, std::byte{2}, std::byte{0}, std::byte{3},
                                 std::byte{0}, std::byte{4}, std::byte{0}, std::byte{5}, std::byte{0}, std::byte{6}};

        {
            constexpr uint32_t width{3};
            constexpr uint32_t height{2};
            constexpr uint32_t component_count{1};

            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(destination_filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE,
                                                 0, false, nullptr, stream.put()));

            const com_ptr encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(16, component_count)};

            const uint32_t stride{compute_stride(
                {.width = width, .height = height, .bits_per_sample = 16, .component_count = component_count})};

            auto byte_pixels{pack_to_bytes_correct_stride_16_bit(anymap_pixels, width, height, stride, component_count)};

            com_ptr<IWICBitmap> bitmap;
            check_hresult(imaging_factory()->CreateBitmapFromMemory(
                width, height, pixel_format, stride, static_cast<uint32_t>(byte_pixels.size()),
                reinterpret_cast<BYTE*>(byte_pixels.data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Initialize(nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->WriteSource(bitmap.get(), nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Commit();
            Assert::AreEqual(error_ok, result);

            result = encoder->Commit();
            Assert::AreEqual(error_ok, result);
        }

        compare(destination_filename, anymap_pixels);
    }

    TEST_METHOD(encode_rgb_16_bit) // NOLINT
    {
        const wchar_t* destination_filename{L"16bit_rgb_3x2-wic-encoded.jls"};
        std::array anymap_pixels{std::byte{0}, std::byte{1}, std::byte{0}, std::byte{2}, std::byte{0}, std::byte{3},
                                 std::byte{0}, std::byte{1}, std::byte{0}, std::byte{2}, std::byte{0}, std::byte{3},
                                 std::byte{0}, std::byte{1}, std::byte{0}, std::byte{2}, std::byte{0}, std::byte{3},
                                 std::byte{0}, std::byte{4}, std::byte{0}, std::byte{5}, std::byte{0}, std::byte{6},
                                 std::byte{0}, std::byte{4}, std::byte{0}, std::byte{5}, std::byte{0}, std::byte{6},
                                 std::byte{0}, std::byte{4}, std::byte{0}, std::byte{5}, std::byte{0}, std::byte{6}};

        {
            constexpr uint32_t width{3};
            constexpr uint32_t height{2};
            constexpr uint32_t component_count{3};

            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(destination_filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE,
                                                 0, false, nullptr, stream.put()));

            const com_ptr encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(16, component_count)};

            const uint32_t stride{compute_stride(
                {.width = width, .height = height, .bits_per_sample = 16, .component_count = component_count})};

            auto byte_pixels{pack_to_bytes_correct_stride_16_bit(anymap_pixels, width, height, stride, component_count)};

            com_ptr<IWICBitmap> bitmap;
            check_hresult(imaging_factory()->CreateBitmapFromMemory(
                width, height, pixel_format, stride, static_cast<uint32_t>(byte_pixels.size()),
                reinterpret_cast<BYTE*>(byte_pixels.data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Initialize(nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->WriteSource(bitmap.get(), nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Commit();
            Assert::AreEqual(error_ok, result);

            result = encoder->Commit();
            Assert::AreEqual(error_ok, result);
        }

        compare(destination_filename, anymap_pixels);
    }

    TEST_METHOD(encode_unsupported_format) // NOLINT
    {
        const wchar_t* filename{L"encode_unsupported_format.jls"};

        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false,
                                             nullptr, stream.put()));

        const com_ptr encoder{factory_.create_encoder()};
        check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

        std::vector<std::byte> input_data(4);

        com_ptr<IWICBitmap> bitmap;
        check_hresult(imaging_factory()->CreateBitmapFromMemory(32, 1, GUID_WICPixelFormatBlackWhite, 4,
                                                                static_cast<uint32_t>(input_data.size()),
                                                                reinterpret_cast<BYTE*>(input_data.data()), bitmap.put()));

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
        Assert::AreEqual(error_ok, result);

        result = frame_encode->Initialize(nullptr);
        Assert::AreEqual(error_ok, result);

        result = frame_encode->WriteSource(bitmap.get(), nullptr);
        Assert::AreEqual(wincodec::error_unsupported_pixel_format, result);
    }

private:
    void encode_monochrome_2_bit(const char* source_filename, const wchar_t* destination_filename) const
    {
        portable_anymap_file anymap_file{source_filename};

        {
            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(destination_filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE,
                                                 0, false, nullptr, stream.put()));

            const com_ptr encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(anymap_file.bits_per_sample(), anymap_file.component_count())};

            const uint32_t stride{compute_stride({.width = static_cast<uint32_t>(anymap_file.width()),
                                                  .height = static_cast<uint32_t>(anymap_file.height()),
                                                  .bits_per_sample = 2,
                                                  .component_count = 1})};

            auto nibble_pixels{pack_to_crumbs(anymap_file.image_data(), anymap_file.width(), anymap_file.height(), stride)};
            com_ptr<IWICBitmap> bitmap;
            check_hresult(imaging_factory()->CreateBitmapFromMemory(
                anymap_file.width(), anymap_file.height(), pixel_format, stride, static_cast<uint32_t>(nibble_pixels.size()),
                reinterpret_cast<BYTE*>(nibble_pixels.data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Initialize(nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->WriteSource(bitmap.get(), nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Commit();
            Assert::AreEqual(error_ok, result);

            result = encoder->Commit();
            Assert::AreEqual(error_ok, result);
        }

        compare(destination_filename, anymap_file.image_data());
    }

    void encode_monochrome_4_bit(const char* source_filename, const wchar_t* destination_filename) const
    {
        portable_anymap_file anymap_file{source_filename};

        {
            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(destination_filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE,
                                                 0, false, nullptr, stream.put()));

            const com_ptr encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(anymap_file.bits_per_sample(), anymap_file.component_count())};

            const uint32_t stride{compute_stride({.width = static_cast<uint32_t>(anymap_file.width()),
                                                  .height = static_cast<uint32_t>(anymap_file.height()),
                                                  .bits_per_sample = 4,
                                                  .component_count = 1})};

            auto nibble_pixels{pack_to_nibbles(anymap_file.image_data(), anymap_file.width(), anymap_file.height(), stride)};
            com_ptr<IWICBitmap> bitmap;
            check_hresult(imaging_factory()->CreateBitmapFromMemory(
                anymap_file.width(), anymap_file.height(), pixel_format, stride, static_cast<uint32_t>(nibble_pixels.size()),
                reinterpret_cast<BYTE*>(nibble_pixels.data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Initialize(nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->WriteSource(bitmap.get(), nullptr);
            Assert::AreEqual(error_ok, result);

            result = frame_encode->Commit();
            Assert::AreEqual(error_ok, result);

            result = encoder->Commit();
            Assert::AreEqual(error_ok, result);
        }

        compare(destination_filename, anymap_file.image_data());
    }

    [[nodiscard]]
    static com_ptr<IWICImagingFactory> imaging_factory()
    {
        com_ptr<IWICImagingFactory> imaging_factory;
        check_hresult(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory,
                                       imaging_factory.put_void()));

        return imaging_factory;
    }

    static void compare(const wchar_t* filename, const std::span<std::byte> decoded_source)
    {
        const auto encoded_source{read_file(filename)};

        jpegls_decoder decoder;
        decoder.source(encoded_source);
        decoder.read_header();

        vector<std::byte> destination{decoder.destination_size()};
        decoder.decode(destination);

        for (size_t i{}; i != destination.size(); ++i)
        {
            if (decoded_source[i] != destination[i])
            {
                Assert::IsTrue(false);
                break;
            }
        }
    }

    factory factory_;
};
