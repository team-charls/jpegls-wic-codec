// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#include "macros.h"
#include <CppUnitTest.h>

import errors;
import factory;
import guids;
import util;
import winrt;
import "win.h";

using namespace winrt;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;


TEST_CLASS(dllmain_test)
{
public:
    TEST_METHOD(class_factory_jpegls_decoder_lock_server) // NOLINT
    {
        const auto class_factory{factory_.get_class_factory(id::jpegls_decoder)};

        HRESULT result{class_factory->LockServer(true)};
        Assert::AreEqual(error_ok, result);

        result = class_factory->LockServer(false);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_lock_server) // NOLINT
    {
        const auto class_factory{factory_.get_class_factory(id::jpegls_encoder)};

        HRESULT result{class_factory->LockServer(true)};
        Assert::AreEqual(error_ok, result);

        result = class_factory->LockServer(false);
        Assert::AreEqual(error_ok, result);
    }

    TEST_METHOD(class_factory_unknown_id) // NOLINT
    {
        com_ptr<IClassFactory> class_factory;
        const HRESULT result{factory_.get_class_factory(id::vendor_team_charls, class_factory)};

        Assert::AreEqual(error_class_not_available, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_create_instance_bad_result) // NOLINT
    {
        const auto class_factory{factory_.get_class_factory(id::jpegls_encoder)};

        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        const HRESULT result{class_factory->CreateInstance(nullptr, id::vendor_team_charls, nullptr)};

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(class_factory_jpegls_encoder_create_instance_no_aggregation) // NOLINT
    {
        const auto class_factory{factory_.get_class_factory(id::jpegls_encoder)};

        auto* outer{reinterpret_cast<IUnknown*>(1)};
        com_ptr<IWICBitmapDecoder> decoder;
        const HRESULT result{class_factory->CreateInstance(outer, IID_PPV_ARGS(decoder.put()))};

        Assert::AreEqual(error_no_aggregation, result);
    }

    TEST_METHOD(class_factory_jpegls_decoder_create_instance_bad_result) // NOLINT
    {
        const auto class_factory{factory_.get_class_factory(id::jpegls_decoder)};

        WARNING_SUPPRESS_NEXT_LINE(6387) // don't pass nullptr
        const HRESULT result{class_factory->CreateInstance(nullptr, id::vendor_team_charls, nullptr)};

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(class_factory_jpegls_decoder_create_instance_no_aggregation) // NOLINT
    {
        const auto class_factory{factory_.get_class_factory(id::jpegls_decoder)};

        auto* outer{reinterpret_cast<IUnknown*>(1)};
        com_ptr<IWICBitmapDecoder> decoder;
        const HRESULT result{class_factory->CreateInstance(outer, IID_PPV_ARGS(decoder.put()))};

        Assert::AreEqual(error_no_aggregation, result);
    }

private:
    factory factory_;
};
