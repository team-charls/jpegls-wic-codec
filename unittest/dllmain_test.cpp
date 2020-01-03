// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "factory.h"

#include "../src/util.h"

#include <CppUnitTest.h>

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(dllmain_test)
{
public :
    TEST_METHOD(class_factory_jpegls_decoder_lock_server)
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSDecoder);

        HRESULT result{class_factory->LockServer(true)};
        Assert::AreEqual(S_OK, result);

        result = class_factory->LockServer(false);
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_lock_server)
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSEncoder);

        HRESULT result{class_factory->LockServer(true)};
        Assert::AreEqual(S_OK, result);

        result = class_factory->LockServer(false);
        Assert::AreEqual(S_OK, result);
    }

    TEST_METHOD(class_factory_unknown_id)
    {
        winrt::com_ptr<IClassFactory> class_factory;
        const HRESULT result{factory_.get_class_factory(GUID_VendorTeamCharLS, class_factory)};

        Assert::AreEqual(CLASS_E_CLASSNOTAVAILABLE, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_create_instance_bad_result)
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSEncoder);

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const HRESULT result{class_factory->CreateInstance(nullptr, GUID_VendorTeamCharLS, nullptr)};
        WARNING_UNSUPPRESS()

        Assert::AreEqual(E_POINTER, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_create_instance_no_aggregation)
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSEncoder);

        IUnknown* outer = reinterpret_cast<IUnknown*>(1);
        winrt::com_ptr<IWICBitmapDecoder> decoder;
        const HRESULT result{class_factory->CreateInstance(outer, IID_PPV_ARGS(decoder.put()))};

        Assert::AreEqual(CLASS_E_NOAGGREGATION, result);
    }

    TEST_METHOD(class_factory_jpegls_decoder_create_instance_bad_result)
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSDecoder);

        WARNING_SUPPRESS(6387) // don't pass nullptr
        const HRESULT result{class_factory->CreateInstance(nullptr, GUID_VendorTeamCharLS, nullptr)};
        WARNING_UNSUPPRESS()

        Assert::AreEqual(E_POINTER, result);
    }

    TEST_METHOD(class_factory_jpegls_decoder_create_instance_no_aggregation)
    {
        auto class_factory = factory_.get_class_factory(CLSID_JpegLSDecoder);

        IUnknown* outer = reinterpret_cast<IUnknown*>(1);
        winrt::com_ptr<IWICBitmapDecoder> decoder;
        const HRESULT result{class_factory->CreateInstance(outer, IID_PPV_ARGS(decoder.put()))};

        Assert::AreEqual(CLASS_E_NOAGGREGATION, result);
    }

private:
    factory factory_;
};
