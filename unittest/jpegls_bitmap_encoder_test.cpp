// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#include "pch.h"

#include "factory.h"

#include "../src/util.h"

#include <CppUnitTest.h>

using winrt::com_ptr;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(jpegls_bitmap_encoder_test)
{
public:
    TEST_METHOD(GetContainerFormat)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.CreateEncoder();

        GUID container_format;
        const HRESULT result = encoder->GetContainerFormat(&container_format);
        Assert::AreEqual(S_OK, result);
        Assert::IsTrue(GUID_ContainerFormatJpegLS == container_format);
    }

    TEST_METHOD(GetContainerFormat_with_nullptr)
    {
        com_ptr<IWICBitmapEncoder> encoder = factory_.CreateEncoder();

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const HRESULT result = encoder->GetContainerFormat(nullptr);
        WARNING_UNSUPPRESS()

        Assert::IsTrue(FAILED(result));
    }

private:
    factory factory_;
};
