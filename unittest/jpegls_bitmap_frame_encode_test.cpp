// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "factory.h"

#include "../src/util.h"

#include <CppUnitTest.h>

#include <array>

using std::vector;
using std::array;
using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(jpegls_bitmap_frame_encode_test)
{
public:
    TEST_METHOD(Initialize) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const HRESULT result = bitmap_frame_encoder->Initialize(nullptr);
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(Initialize_twice) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        const HRESULT result = bitmap_frame_encoder->Initialize(nullptr);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(Initialize_after_commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        commit(bitmap_frame_encoder.get());

        const HRESULT result = bitmap_frame_encoder->Initialize(nullptr);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(SetSize) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        const HRESULT result = bitmap_frame_encoder->SetSize(512, 512);
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(SetSize_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const HRESULT result = bitmap_frame_encoder->SetSize(512, 512);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(SetSize_after_commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        commit(bitmap_frame_encoder.get());

        const HRESULT result = bitmap_frame_encoder->SetSize(512, 512);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(SetResolution) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        const HRESULT result = bitmap_frame_encoder->SetResolution(96., 96.);
        Assert::AreEqual(S_OK, result);

        // TODO: extend this test by encode\decode a sample image.
    }

    TEST_METHOD(SetResolution_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const HRESULT result = bitmap_frame_encoder->SetResolution(96., 96.);
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(SetResolution_after_commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        commit(bitmap_frame_encoder.get());

        const HRESULT result = bitmap_frame_encoder->SetResolution(96., 96.);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(SetColorContexts_is_unsupported) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        com_ptr<IWICColorContext> color_context;
        check_hresult(imaging_factory()->CreateColorContext(color_context.put()));
        array color_contexts{color_context.get()};

        const HRESULT result = bitmap_frame_encoder->SetColorContexts(static_cast<UINT>(color_contexts.size()), color_contexts.data());
        Assert::AreEqual(WINCODEC_ERR_UNSUPPORTEDOPERATION, result);
    }

    TEST_METHOD(SetPixelFormat) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        const array formats{GUID_WICPixelFormat8bppGray, GUID_WICPixelFormat16bppGray, GUID_WICPixelFormat24bppRGB};
        for (const auto& format : formats)
        {
            GUID pixel_format = format;
            const HRESULT result = bitmap_frame_encoder->SetPixelFormat(&pixel_format);
            Assert::AreEqual(S_OK, result);
            Assert::IsTrue(pixel_format == format);
        }
    }

    TEST_METHOD(SetPixelFormat_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        WARNING_SUPPRESS(26496) // The variable 'pixel_format' is assigned only once, mark it as const (con.4).
        GUID pixel_format = GUID_WICPixelFormat8bppGray;
        WARNING_UNSUPPRESS()

        const HRESULT result = bitmap_frame_encoder->SetPixelFormat(&pixel_format);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(SetPixelFormat_after_commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        commit(bitmap_frame_encoder.get());

        WARNING_SUPPRESS(26496) // The variable 'pixel_format' is assigned only once, mark it as const (con.4).
        GUID pixel_format = GUID_WICPixelFormat8bppGray;
        WARNING_UNSUPPRESS()

        const HRESULT result = bitmap_frame_encoder->SetPixelFormat(&pixel_format);
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(Commit_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const HRESULT result = bitmap_frame_encoder->Commit();
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(Commit_no_size) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));
        set_pixel_format(bitmap_frame_encoder.get(), GUID_WICPixelFormat8bppGray);

        const HRESULT result = bitmap_frame_encoder->Commit();
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(Commit_no_pixel_format) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));
        check_hresult(bitmap_frame_encoder->SetSize(512, 512));

        const HRESULT result = bitmap_frame_encoder->Commit();
        Assert::AreEqual(WINCODEC_ERR_WRONGSTATE, result);
    }

    TEST_METHOD(Commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));
        check_hresult(bitmap_frame_encoder->SetSize(512, 512));
        set_pixel_format(bitmap_frame_encoder.get(), GUID_WICPixelFormat8bppGray);
        vector<uint8_t> source(512 * 512);
        bitmap_frame_encoder->WritePixels(512, 512, static_cast<UINT>(source.size()), source.data());

        const HRESULT result = bitmap_frame_encoder->Commit();
        Assert::AreEqual(S_OK, result); // TODO: commit should fail.
    }

private:
    [[nodiscard]] com_ptr<IWICBitmapFrameEncode> create_frame_encoder(const wchar_t* filename = nullptr) const
    {
        com_ptr<IStream> stream;

        if (filename)
        {
            check_hresult(SHCreateStreamOnFileEx(filename, STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));
        }
        else
        {
            stream.attach(SHCreateMemStream(nullptr, 0));
        }

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        check_hresult(encoder->CreateNewFrame(frame_encode.put(), nullptr));

        return frame_encode;
    }

    static void commit(IWICBitmapFrameEncode * bitmap_frame_encoder)
    {
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));
        check_hresult(bitmap_frame_encoder->SetSize(512, 512));
        set_pixel_format(bitmap_frame_encoder, GUID_WICPixelFormat8bppGray);

        vector<uint8_t> source(512 * 512);
        bitmap_frame_encoder->WritePixels(512, 512, static_cast<UINT>(source.size()), source.data());

        check_hresult(bitmap_frame_encoder->Commit());
    }

    static void set_pixel_format(IWICBitmapFrameEncode * bitmap_frame_encoder, const GUID& format)
    {
        WARNING_SUPPRESS(26496) // The variable 'pixel_format' is assigned only once, mark it as const (con.4).
        GUID pixel_format = format;
        WARNING_UNSUPPRESS()
        check_hresult(bitmap_frame_encoder->SetPixelFormat(&pixel_format));
    }

    [[nodiscard]] IWICImagingFactory* imaging_factory()
    {
        if (!imaging_factory_)
        {
            check_hresult(CoCreateInstance(CLSID_WICImagingFactory,
                                           nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(imaging_factory_.put())));
        }

        return imaging_factory_.get();
    }

    com_ptr<IWICImagingFactory> imaging_factory_;
    factory factory_;
};
