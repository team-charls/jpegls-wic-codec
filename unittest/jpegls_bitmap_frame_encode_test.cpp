// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "factory.h"

#include "../src/util.h"

#include <CppUnitTest.h>

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(jpegls_bitmap_frame_encode_test)
{
public:
    TEST_METHOD(Initialize)
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const HRESULT result = bitmap_frame_encoder->Initialize(nullptr);
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(SetSize)
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const HRESULT result = bitmap_frame_encoder->SetSize(512, 512);
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(SetResolution)
    {
        com_ptr<IWICBitmapFrameEncode> bitmap_frame_encoder = create_frame_encoder();

        const HRESULT result = bitmap_frame_encoder->SetResolution(96., 96.);
        Assert::AreEqual(S_OK, result);
    }

private:
    [[nodiscard]] com_ptr<IWICBitmapFrameEncode> create_frame_encoder() const
    {
        com_ptr<IStream> stream;
        check_hresult(SHCreateStreamOnFileEx(L"output.jls", STGM_READWRITE | STGM_CREATE | STGM_SHARE_DENY_WRITE, 0, false, nullptr, stream.put()));

        com_ptr<IWICBitmapEncoder> encoder = factory_.create_encoder();

        check_hresult(encoder->Initialize(stream.get(), WICBitmapEncoderCacheInMemory));

        com_ptr<IWICBitmapFrameEncode> frame_encode;
        check_hresult(encoder->CreateNewFrame(frame_encode.put(), nullptr));

        return frame_encode;
    }

    factory factory_;
};
