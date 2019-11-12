// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "factory.h"
#include "portable_anymap_file.h"

#include "../src/util.h"

#include <charls/charls.h>
#include <CppUnitTest.h>

using charls::jpegls_decoder;
using std::ifstream;
using std::vector;
using winrt::com_ptr;
using winrt::check_hresult;
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

}


TEST_CLASS(jpegls_bitmap_encoder_test)
{
public:
    TEST_METHOD(GetContainerFormat)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        GUID container_format;
        const HRESULT result = encoder->GetContainerFormat(&container_format);
        Assert::AreEqual(S_OK, result);
        Assert::IsTrue(GUID_ContainerFormatJpegLS == container_format);
    }

    TEST_METHOD(GetContainerFormat_with_nullptr)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const HRESULT result = encoder->GetContainerFormat(nullptr);
        WARNING_UNSUPPRESS()

        Assert::IsTrue(FAILED(result));
    }

    TEST_METHOD(GetEncoderInfo)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        com_ptr<IWICBitmapEncoderInfo> encoder_info;
        const HRESULT result = encoder->GetEncoderInfo(encoder_info.put());
        Assert::IsTrue(result == S_OK || result == WINCODEC_ERR_COMPONENTNOTFOUND);

        if (SUCCEEDED(result))
        {
            Assert::IsNotNull(encoder_info.get());
        }
        else
        {
            Assert::IsNull(encoder_info.get());
        }
    }

    TEST_METHOD(GetEncoderInfo_with_nullptr)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const HRESULT result = encoder->GetEncoderInfo(nullptr);
        WARNING_UNSUPPRESS()

       Assert::IsTrue(FAILED(result));
    }

    TEST_METHOD(SetPreview)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const HRESULT result = encoder->SetPreview(nullptr);
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(SetThumbnail)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const HRESULT result = encoder->SetThumbnail(nullptr);
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(SetColorContexts)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const HRESULT result = encoder->SetColorContexts(0, nullptr);
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(GetMetadataQueryWriter)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        com_ptr<IWICMetadataQueryWriter> metadata_query_writer;
        const HRESULT result = encoder->GetMetadataQueryWriter(metadata_query_writer.put());
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
        Assert::IsNull(metadata_query_writer.get());
    }

    TEST_METHOD(SetPalette)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        com_ptr<IWICPalette> palette;
        check_hresult(imaging_factory()->CreatePalette(palette.put()));

        const HRESULT result = encoder->SetPalette(palette.get());
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(Initialize)
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const HRESULT result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(Initialize_with_nullptr)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const HRESULT result = encoder->Initialize(nullptr, WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(E_INVALIDARG, result);
    }

    TEST_METHOD(Initialize_twice)
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        HRESULT result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(S_OK, result);

        result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(CreateNewFrame)
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        HRESULT result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(S_OK, result);

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
        Assert::AreEqual(S_OK, result);
        Assert::IsNotNull(frame_encode.get());
    }

    TEST_METHOD(CreateNewFrame_with_nullptr)
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(nullptr, 0));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        HRESULT result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(S_OK, result);

        WARNING_SUPPRESS(6387) // don't pass nullptr
        result = encoder->CreateNewFrame(nullptr, nullptr);
        WARNING_UNSUPPRESS()
        Assert::AreEqual(E_POINTER, result);
    }

    TEST_METHOD(CreateNewFrame_while_not_initialized)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        const HRESULT result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
        Assert::AreEqual(WINCODEC_ERR_NOTINITIALIZED, result);
    }

    TEST_METHOD(Commit_while_not_initialized)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        const HRESULT result = encoder->Commit();
        Assert::AreEqual(WINCODEC_ERR_NOTINITIALIZED, result);
    }

    TEST_METHOD(encode_conformance_color_lossless)
    {
        const wchar_t* filename = L"encode_conformance_color_lossless.jls";
        portable_anymap_file anymap_file{ "test8.ppm" };

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
            HRESULT result = encoder->CreateNewFrame(frame_encode.put(), nullptr);
            Assert::AreEqual(S_OK, result);

            result = frame_encode->Initialize(nullptr);
            Assert::AreEqual(S_OK, result);

            result = frame_encode->WriteSource(bitmap.get(), nullptr);
            Assert::AreEqual(S_OK, result);

            result = frame_encode->Commit();
            Assert::AreEqual(S_OK, result);

            result = encoder->Commit();
            Assert::AreEqual(S_OK, result);
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
