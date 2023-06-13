// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

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

GUID get_pixel_format(const int32_t bits_per_sample, const int32_t component_count)
{
    switch (component_count)
    {
    case 1:
        switch (bits_per_sample)
        {
        case 4:
            return GUID_WICPixelFormat4bppGray;
        case 8:
            return GUID_WICPixelFormat8bppGray;
        default:
            Assert::Fail();
        }

    case 3:
        switch (bits_per_sample)
        {
        case 8:
            return GUID_WICPixelFormat24bppRGB;
        default:
            Assert::Fail();
        }

    default:
        break;
    }

    Assert::Fail();
}

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
vector<std::byte> pack_to_nibbles(const std::span<const std::byte> byte_pixels, const size_t width,
                                  const size_t height, const size_t stride) noexcept
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
        const com_ptr<IWICBitmapEncoder> encoder{factory_.create_encoder()};

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        const HRESULT result{encoder->CreateNewFrame(frame_encode.put(), nullptr)};
        Assert::AreEqual(wincodec::error_not_initialized, result);
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

    TEST_METHOD(encode_conformance_monochrome_4_bit_4x1) // NOLINT
    {
        encode_conformance_monochrome_4_bit("4bit_4x1.pgm", L"4bit_4x1-wic-encoded.jls");
    }

    TEST_METHOD(encode_conformance_monochrome_4_bit_5x1) // NOLINT
    {
        encode_conformance_monochrome_4_bit("4bit_5x1.pgm", L"4bit_5x1-wic-encoded.jls");
    }

    TEST_METHOD(encode_conformance_monochrome_4_bit_360x360) // NOLINT
    {
        encode_conformance_monochrome_4_bit("4bit-monochrome.pgm", L"4bit-monochrome-wic-encoded.jls");
    }

private:
    void encode_conformance_monochrome_4_bit(const char* source_filename, const wchar_t* destination_filename) const
    {
        portable_anymap_file anymap_file{source_filename};

        {
            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(destination_filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE,
                                                 0, false,
                                                 nullptr, stream.put()));

            const com_ptr encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(anymap_file.bits_per_sample(), anymap_file.component_count())};

            const uint32_t stride = compute_stride({.width = static_cast<uint32_t>(anymap_file.width()),
                                                    .height = static_cast<uint32_t>(anymap_file.height()),
                                                    .bits_per_sample = 4,
                                                    .component_count = 1});

            com_ptr<IWICBitmap> bitmap;
            auto nibble_pixels =
                pack_to_nibbles(anymap_file.image_data(), anymap_file.width(), anymap_file.height(), stride);
            check_hresult(imaging_factory()->CreateBitmapFromMemory(
                anymap_file.width(), anymap_file.height(), pixel_format, stride, static_cast<uint32_t>(nibble_pixels.size()),
                reinterpret_cast<BYTE*>(nibble_pixels.data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            HRESULT result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
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

    static void compare(const wchar_t* filename, const vector<std::byte>& decoded_source)
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
