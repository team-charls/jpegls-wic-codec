// SPDX-FileCopyrightText: © 2024 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

#include <CppUnitTest.h>

import std;
import test.winrt;
import <win.hpp>;

import hresults;

import com_factory;
import test.util;
import "macros.hpp";

using std::span;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using winrt::com_ptr;

TEST_CLASS(property_store_test)
{
public:
    TEST_METHOD(GetCount_not_initialized) // NOLINT
    {
        DWORD count;
        const auto result{com_factory_.create_property_store()->GetCount(&count)};

        Assert::AreEqual(error_not_valid_state, result);
        Assert::AreEqual(0UL, count);
    }

    TEST_METHOD(GetCount_nullptr) // NOLINT
    {
        WARNING_SUPPRESS_NEXT_LINE(6387)
        const auto result{com_factory_.create_property_store()->GetCount(nullptr)};

        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(Initialize) // NOLINT
    {
        auto property_store{com_factory_.create_property_store()};
        auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        auto result{
            initialize_with_stream->Initialize(create_stream_on_file(L"tulips-gray-8bit-512-512.jls").get(), STGM_READ)};
        Assert::AreEqual(success_ok, result);

        PROPVARIANT prop_variant;
        PropVariantInit(&prop_variant);
        result = property_store->GetValue(PKEY_Image_HorizontalSize, &prop_variant);
        Assert::AreEqual(success_ok, result);
        Assert::IsTrue(VT_UI4 == prop_variant.vt);
        Assert::AreEqual(512U, prop_variant.uintVal);
        std::ignore = PropVariantClear(&prop_variant);

        PropVariantInit(&prop_variant);
        result = property_store->GetValue(PKEY_Image_VerticalSize, &prop_variant);
        Assert::AreEqual(success_ok, result);
        Assert::IsTrue(VT_UI4 == prop_variant.vt);
        Assert::AreEqual(512U, prop_variant.uintVal);
        std::ignore = PropVariantClear(&prop_variant);

        PropVariantInit(&prop_variant);
        result = property_store->GetValue(PKEY_Image_BitDepth, &prop_variant);
        Assert::AreEqual(success_ok, result);
        Assert::IsTrue(VT_UI4 == prop_variant.vt);
        Assert::AreEqual(8U, prop_variant.uintVal);
        std::ignore = PropVariantClear(&prop_variant);

        Assert::AreEqual(success_ok, result);
    }

    TEST_METHOD(Initialize_twice) // NOLINT
    {
        const auto property_store{com_factory_.create_property_store()};
        const auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        const auto source{create_stream_on_file(L"tulips-gray-8bit-512-512.jls")};
        auto result{
            initialize_with_stream->Initialize(source.get(), STGM_READ)};
        Assert::AreEqual(success_ok, result);

        result = initialize_with_stream->Initialize(source.get(), STGM_READ);
        Assert::AreEqual(error_already_initialized, result);
    }

    TEST_METHOD(Initialize_bad_input) // NOLINT
    {
        const auto property_store{com_factory_.create_property_store()};
        const auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        std::vector<char> source;
        const std::string bad_header{"NOT_A_JPEG-LS_FILE"};
        source.insert(source.end(), bad_header.cbegin(), bad_header.cend());
        const auto result{initialize_with_stream->Initialize(create_memory_stream(source).get(), STGM_READ)};
        Assert::AreEqual(wincodec::error_bad_header, result);
    }

    TEST_METHOD(IsPropertyWritable)
    {
        const auto property_store{com_factory_.create_property_store()};
        const auto property_store_capabilities{property_store.as<IPropertyStoreCapabilities>()};

        const auto result{property_store_capabilities->IsPropertyWritable(PKEY_Image_BitDepth)};

        Assert::AreEqual(success_false, result);
    }

    TEST_METHOD(GetValue_unknown_key)
    {
        auto property_store{com_factory_.create_property_store()};
        auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        auto result{
            initialize_with_stream->Initialize(create_stream_on_file(L"tulips-gray-8bit-512-512.jls").get(), STGM_READ)};
        Assert::AreEqual(success_ok, result);

        PROPVARIANT prop_variant;
        PropVariantInit(&prop_variant);
        result = property_store->GetValue(PKEY_Image_CompressionText, &prop_variant);
        Assert::AreEqual(success_ok, result);
        Assert::IsTrue(VT_EMPTY == prop_variant.vt);
        std::ignore = PropVariantClear(&prop_variant);
    }

    TEST_METHOD(GetValue_bitmap)
    {
        auto property_store{com_factory_.create_property_store()};
        auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        auto result{
            initialize_with_stream->Initialize(create_stream_on_file(L"tulips-gray-8bit-512-512.jls").get(), STGM_READ)};
        Assert::AreEqual(success_ok, result);

        PROPVARIANT prop_variant;
        PropVariantInit(&prop_variant);
        result = property_store->GetValue(PKEY_Image_BitDepth, &prop_variant);
        Assert::AreEqual(success_ok, result);
        Assert::IsTrue(VT_UI4 == prop_variant.vt);
        Assert::AreEqual(8U, prop_variant.uintVal);
        std::ignore = PropVariantClear(&prop_variant);
    }

    TEST_METHOD(GetValue_grayscale) // NOLINT
    {
        auto property_store{com_factory_.create_property_store()};
        auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        auto result{
            initialize_with_stream->Initialize(create_stream_on_file(L"tulips-gray-8bit-512-512.jls").get(), STGM_READ)};
        Assert::AreEqual(success_ok, result);

        PROPVARIANT prop_variant;
        PropVariantInit(&prop_variant);
        result = property_store->GetValue(PKEY_Image_BitDepth, &prop_variant);
        Assert::AreEqual(success_ok, result);
        Assert::IsTrue(VT_UI4 == prop_variant.vt);
        Assert::AreEqual(8U, prop_variant.uintVal);
        std::ignore = PropVariantClear(&prop_variant);
    }

    TEST_METHOD(GetAt)
    {
        const auto property_store{com_factory_.create_property_store()};
        const auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        auto result{
            initialize_with_stream->Initialize(create_stream_on_file(L"tulips-gray-8bit-512-512.jls").get(), STGM_READ)};
        Assert::AreEqual(success_ok, result);

        PROPERTYKEY property_key;
        result = property_store->GetAt(0, &property_key);
        Assert::AreEqual(success_ok, result);
        Assert::IsTrue(static_cast<bool>(PKEY_Image_HorizontalSize == property_key));
    }

    TEST_METHOD(GetAt_not_initialized)
    {
        const auto property_store{com_factory_.create_property_store()};

        PROPERTYKEY property_key;
        const auto result{property_store->GetAt(0, &property_key)};
        Assert::AreEqual(error_not_valid_state, result);
    }

    TEST_METHOD(GetAt_nullptr)
    {
        const auto property_store{com_factory_.create_property_store()};
        const auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        auto result{
            initialize_with_stream->Initialize(create_stream_on_file(L"tulips-gray-8bit-512-512.jls").get(), STGM_READ)};
        Assert::AreEqual(success_ok, result);

        WARNING_SUPPRESS_NEXT_LINE(6387)
        result = property_store->GetAt(0, nullptr);
        Assert::AreEqual(error_pointer, result);
    }

    TEST_METHOD(GetValue_null_pointer) // NOLINT
    {
        auto property_store{com_factory_.create_property_store()};
        auto initialize_with_stream{property_store.as<IInitializeWithStream>()};

        auto result{
            initialize_with_stream->Initialize(create_stream_on_file(L"tulips-gray-8bit-512-512.jls").get(), STGM_READ)};
        Assert::AreEqual(success_ok, result);

        PROPVARIANT prop_variant;
        PropVariantInit(&prop_variant);

        WARNING_SUPPRESS_NEXT_LINE(6387)
        result = property_store->GetValue(PKEY_Image_HorizontalSize, nullptr);

        Assert::AreEqual(error_invalid_argument, result);
        std::ignore = PropVariantClear(&prop_variant);
    }

    TEST_METHOD(SetValue) // NOLINT
    {
        const auto property_store{com_factory_.create_property_store()};

        PROPVARIANT prop_variant;
        PropVariantInit(&prop_variant);
        const auto result{property_store->SetValue(PKEY_Image_HorizontalSize, prop_variant)};

        Assert::AreEqual(error_access_denied, result);
        std::ignore = PropVariantClear(&prop_variant);
    }

    TEST_METHOD(Commit) // NOLINT
    {
        const auto property_store{com_factory_.create_property_store()};

        PROPVARIANT prop_variant;
        PropVariantInit(&prop_variant);
        const auto result{property_store->Commit()};

        Assert::AreEqual(error_access_denied, result);
        std::ignore = PropVariantClear(&prop_variant);
    }

private:
    static com_ptr<IStream> create_memory_stream(span<char> source) noexcept
    {
        com_ptr<IStream> stream;
        stream.attach(SHCreateMemStream(reinterpret_cast<BYTE*>(source.data()), static_cast<UINT>(source.size())));

        return stream;
    }

    com_factory com_factory_;
};
