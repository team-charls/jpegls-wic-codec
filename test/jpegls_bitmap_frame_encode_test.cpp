﻿// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "factory.h"
#include "util.h"

#include "../src/errors.h"

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

        const hresult result = bitmap_frame_encoder->Initialize(nullptr);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(Initialize_twice) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        const hresult result = bitmap_frame_encoder->Initialize(nullptr);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(Initialize_after_commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        commit(bitmap_frame_encoder.get());

        const hresult result = bitmap_frame_encoder->Initialize(nullptr);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(SetSize) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        const hresult result = bitmap_frame_encoder->SetSize(512, 512);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(SetSize_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const hresult result = bitmap_frame_encoder->SetSize(512, 512);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(SetSize_after_commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        commit(bitmap_frame_encoder.get());

        const hresult result = bitmap_frame_encoder->SetSize(512, 512);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(SetResolution) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        const hresult result = bitmap_frame_encoder->SetResolution(96., 96.);
        Assert::AreEqual(error_ok, result);

        // TODO: extend this test by encode\decode a sample image.
    }

    TEST_METHOD(SetResolution_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const hresult result = bitmap_frame_encoder->SetResolution(96., 96.);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(SetResolution_after_commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        commit(bitmap_frame_encoder.get());

        const hresult result = bitmap_frame_encoder->SetResolution(96., 96.);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(SetColorContexts_is_unsupported) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        com_ptr<IWICColorContext> color_context;
        check_hresult(imaging_factory()->CreateColorContext(color_context.put()));
        array color_contexts{color_context.get()};

        const hresult result = bitmap_frame_encoder->SetColorContexts(static_cast<UINT>(color_contexts.size()), color_contexts.data());
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(GetMetadataQueryWriter_is_not_supported) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        com_ptr<IWICMetadataQueryWriter> metadata_query_writer;
        const hresult result = bitmap_frame_encoder->GetMetadataQueryWriter(metadata_query_writer.put());
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
        Assert::IsNull(metadata_query_writer.get());
    }

    TEST_METHOD(SetThumbnail_is_not_supported) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const hresult result = bitmap_frame_encoder->SetThumbnail(nullptr);
        Assert::AreEqual(wincodec::error_unsupported_operation, result);
    }

    TEST_METHOD(SetPalette_is_not_supported) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        com_ptr<IWICPalette> palette;
        check_hresult(imaging_factory()->CreatePalette(palette.put()));

        const hresult result = bitmap_frame_encoder->SetPalette(palette.get());
        Assert::AreEqual(wincodec::error_palette_unavailable, result);
    }

    TEST_METHOD(SetPixelFormat) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        const array formats{GUID_WICPixelFormat8bppGray, GUID_WICPixelFormat16bppGray, GUID_WICPixelFormat24bppRGB};
        for (const auto& format : formats)
        {
            GUID pixel_format = format;
            const hresult result = bitmap_frame_encoder->SetPixelFormat(&pixel_format);
            Assert::AreEqual(error_ok, result);
            Assert::IsTrue(pixel_format == format);
        }
    }

    TEST_METHOD(SetPixelFormat_with_nullptr) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));

        WARNING_SUPPRESS_NEXT_LINE(6387)
        const hresult result = bitmap_frame_encoder->SetPixelFormat(nullptr);
        Assert::AreEqual(error_invalid_argument, result);
    }

    TEST_METHOD(SetPixelFormat_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        WARNING_SUPPRESS_NEXT_LINE(26496) // The variable 'pixel_format' is assigned only once, mark it as const (con.4).
        GUID pixel_format = GUID_WICPixelFormat8bppGray;

        const hresult result = bitmap_frame_encoder->SetPixelFormat(&pixel_format);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(SetPixelFormat_after_commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        commit(bitmap_frame_encoder.get());

        WARNING_SUPPRESS_NEXT_LINE(26496) // The variable 'pixel_format' is assigned only once, mark it as const (con.4).
        GUID pixel_format = GUID_WICPixelFormat8bppGray;

        const hresult result = bitmap_frame_encoder->SetPixelFormat(&pixel_format);
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(Commit_not_initialized) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const hresult result = bitmap_frame_encoder->Commit();
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(Commit_no_size) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));
        set_pixel_format(bitmap_frame_encoder.get(), GUID_WICPixelFormat8bppGray);

        const hresult result = bitmap_frame_encoder->Commit();
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(Commit_no_pixel_format) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));
        check_hresult(bitmap_frame_encoder->SetSize(512, 512));

        const hresult result = bitmap_frame_encoder->Commit();
        Assert::AreEqual(wincodec::error_wrong_state, result);
    }

    TEST_METHOD(Commit) // NOLINT
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();
        check_hresult(bitmap_frame_encoder->Initialize(nullptr));
        check_hresult(bitmap_frame_encoder->SetSize(512, 512));
        set_pixel_format(bitmap_frame_encoder.get(), GUID_WICPixelFormat8bppGray);
        vector<uint8_t> source(512 * 512);
        bitmap_frame_encoder->WritePixels(512, 512, static_cast<UINT>(source.size()), source.data());

        const hresult result = bitmap_frame_encoder->Commit();
        Assert::AreEqual(error_ok, result); // TODO: commit should fail.
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
        WARNING_SUPPRESS_NEXT_LINE(26496) // The variable 'pixel_format' is assigned only once, mark it as const (con.4).
        GUID pixel_format = format;

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
