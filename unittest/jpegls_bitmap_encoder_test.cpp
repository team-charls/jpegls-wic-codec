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
    const auto byte_count_file = static_cast<int>(input.tellg());
    input.seekg(0, ifstream::beg);

    vector<std::byte> buffer(byte_count_file);
    input.read(reinterpret_cast<char*>(buffer.data()), buffer.size());

    return buffer;
}

} // namespace


TEST_CLASS(jpegls_bitmap_encoder_test)
{
public:
    TEST_METHOD(GetContainerFormat) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        GUID container_format;
        const hresult result = encoder->GetContainerFormat(&container_format);
        Assert::AreEqual(error_ok, result);
        Assert::IsTrue(GUID_ContainerFormatJpegLS == container_format);
    }

    TEST_METHOD(GetContainerFormat_with_nullptr) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result = encoder->GetContainerFormat(nullptr);
        WARNING_UNSUPPRESS()

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(GetEncoderInfo) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        com_ptr<IWICBitmapEncoderInfo> encoder_info;
        const hresult result = encoder->GetEncoderInfo(encoder_info.put());
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
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result = encoder->GetEncoderInfo(nullptr);
        WARNING_UNSUPPRESS()

        Assert::IsTrue(failed(result));
    }

    TEST_METHOD(SetPreview) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const hresult result = encoder->SetPreview(nullptr);
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(SetThumbnail) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const hresult result = encoder->SetThumbnail(nullptr);
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(SetColorContexts) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const hresult result = encoder->SetColorContexts(0, nullptr);
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetMetadataQueryWriter) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        com_ptr<IWICMetadataQueryWriter> metadata_query_writer;
        const hresult result = encoder->GetMetadataQueryWriter(metadata_query_writer.put());
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
        Assert::IsNull(metadata_query_writer.get());
    }

    TEST_METHOD(SetPalette) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        com_ptr<IWICPalette> palette;
        check_hresult(imaging_factory()->CreatePalette(palette.put()));

        const hresult result = encoder->SetPalette(palette.get());
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(Initialize) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const hresult result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_with_nullptr) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const hresult result = encoder->Initialize(nullptr, WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(Initialize_twice) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        hresult result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(error_ok, result);

        result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(CreateNewFrame) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        hresult result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(error_ok, result);

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
        Assert::AreEqual(error_ok, result);
        Assert::IsNotNull(frame_encode.get());
    }

    TEST_METHOD(CreateNewFrame_with_nullptr) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        hresult result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(error_ok, result);

        WARNING_SUPPRESS(6387) // don't pass nullptr
        result = encoder->CreateNewFrame(nullptr, nullptr);
        WARNING_UNSUPPRESS()
        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(CreateNewFrame_while_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        const hresult result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
        Assert::AreEqual(wincodec::error_not_initialized, result);
    }

    TEST_METHOD(Commit_while_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const hresult result = encoder->Commit();
        Assert::AreEqual(wincodec::error_not_initialized, result);
    }

    TEST_METHOD(Commit_without_a_frame) // NOLINT
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        hresult result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(error_ok, result);

        result = encoder->Commit();
        Assert::AreEqual(wincodec::error_frame_missing, result);
    }

    TEST_METHOD(encode_conformance_color_lossless) // NOLINT
    {
        const wchar_t* filename = L"encode_conformance_color_lossless.jls";
        portable_anymap_file anymap_file{"test8.ppm"};

        {
            com_ptr<IStream> stream;
            check_hresult(SHCreateStreamOnFileEx(filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

            com_ptr<IWICBitmapEncoder> encoder{factory_.create_encoder()};
            check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

            const GUID pixel_format{get_pixel_format(anymap_file.bits_per_sample(), anymap_file.component_count())};

            com_ptr<IWICBitmap> bitmap;
            check_hresult(imaging_factory()->CreateBitmapFromMemory(anymap_file.width(), anymap_file.height(),
                                                                    pixel_format, anymap_file.width() * anymap_file.component_count(),
                                                                    static_cast<uint32_t>(anymap_file.image_data().size()), reinterpret_cast<BYTE*>(anymap_file.image_data().data()), bitmap.put()));

            com_ptr<IWICBitmapFrameEncode> frame_encode;
            hresult result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
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

private:
    [[nodiscard]] static com_ptr<IWICImagingFactory> imaging_factory()
    {
        com_ptr<IWICImagingFactory> imaging_factory;
        check_hresult(CoCreateInstance(CLSID_WICImagingFactory,
                                       nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, imaging_factory.put_void()));

        return imaging_factory;
    }

    static void compare(const wchar_t* filename, const vector<std::byte>& decoded_source)
    {
        const auto encoded_source = read_file(filename);

        jpegls_decoder decoder;
        decoder.source(encoded_source);
        decoder.read_header();

        vector<std::byte> destination{decoder.destination_size()};
        decoder.decode(destination);

        for (size_t i = 0; i < destination.size(); ++i)
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
