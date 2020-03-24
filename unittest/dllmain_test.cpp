// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "factory.h"
#include "util.h"

#include "../src/errors.h"

#include <CppUnitTest.h>

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(dllmain_test)
{
public :
    TEST_METHOD(class_factory_jpegls_decoder_lock_server) // NOLINT
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSDecoder);

        hresult result{class_factory->LockServer(true)};
        Assert::AreEqual(error_ok, result);

        result = class_factory->LockServer(false);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_lock_server) // NOLINT
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSEncoder);

        hresult result{class_factory->LockServer(true)};
        Assert::AreEqual(error_ok, result);

        result = class_factory->LockServer(false);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(class_factory_unknown_id) // NOLINT
    {
        winrt::com_ptr<IClassFactory> class_factory;
        const hresult result{factory_.get_class_factory(GUID_VendorTeamCharLS, class_factory)};

        Assert::AreEqual(error_class_not_available, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_create_instance_bad_result) // NOLINT
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSEncoder);

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result{class_factory->CreateInstance(nullptr, GUID_VendorTeamCharLS, nullptr)};
        WARNING_UNSUPPRESS()

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_create_instance_no_aggregation) // NOLINT
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSEncoder);

        auto outer = reinterpret_cast<IUnknown*>(1);
        winrt::com_ptr<IWICBitmapDecoder> decoder;
        const hresult result{class_factory->CreateInstance(outer, IID_PPV_ARGS(decoder.put()))};

        Assert::AreEqual(error_no_aggregation, result);
    }

    TEST_METHOD(class_factory_jpegls_decoder_create_instance_bad_result) // NOLINT
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSDecoder);

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const hresult result{class_factory->CreateInstance(nullptr, GUID_VendorTeamCharLS, nullptr)};
        WARNING_UNSUPPRESS()

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(class_factory_jpegls_decoder_create_instance_no_aggregation) // NOLINT
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSDecoder);

        auto outer = reinterpret_cast<IUnknown*>(1);
        winrt::com_ptr<IWICBitmapDecoder> decoder;
        const hresult result{class_factory->CreateInstance(outer, IID_PPV_ARGS(decoder.put()))};

        Assert::AreEqual(error_no_aggregation, result);
    }

private:
    factory factory_;
};
