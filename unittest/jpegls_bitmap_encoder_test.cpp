// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "factory.h"

#include "../src/util.h"

#include <CppUnitTest.h>

using winrt::com_ptr;
using winrt::check_hresult;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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
        check_hresult(SHCreateStreamOnFileEx(L"output.jls", STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

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
        check_hresult(SHCreateStreamOnFileEx(L"output.jls", STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        HRESULT result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(S_OK, result);

        result = encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(CreateNewFrame)
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(L"output.jls", STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

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
        check_hresult(SHCreateStreamOnFileEx(L"output.jls", STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

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
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(L"output.jls", STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

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

private:
    [[nodiscard]] static com_ptr<IWICImagingFactory> imaging_factory()
    {
        com_ptr<IWICImagingFactory> imaging_factory;
        check_hresult(CoCreateInstance(CLSID_WICImagingFactory,
            nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, imaging_factory.put_void()));

        return imaging_factory;
    }

    factory factory_;
};
