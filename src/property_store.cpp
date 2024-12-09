// SPDX-FileCopyrightText: © 2024 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

module;

#include "intellisense.hpp"

module property_store;

import std;
import winrt;
import charls;
import <win.hpp>;

import hresults;
import util;
import class_factory;
import property_variant;
import storage_buffer;
import "macros.hpp";

using std::pair;
using std::span;
using std::to_wstring;
using std::uint16_t;
using std::uint32_t;
using winrt::check_hresult;

namespace {

template<typename K, typename V, std::size_t Extent>
[[nodiscard]]
const V* find(span<pair<K, V>, Extent> values, const K& key)
{
    for (const auto& pair : values)
    {
        if (pair.first == key)
        {
            return &pair.second;
        }
    }
    return nullptr;
}

struct property_store : winrt::implements<property_store, IInitializeWithStream, IPropertyStoreCapabilities, IPropertyStore>
{
    // IInitializeWithStream
    HRESULT __stdcall Initialize(_In_ IStream* stream, [[maybe_unused]]
                                                       const DWORD access_mode) noexcept override
    try
    {
        TRACE("{} property_store::Initialize, stream address={}, access_mode={}\n", fmt::ptr(this), fmt::ptr(stream),
              static_cast<int>(access_mode));

        if (initialized_)
            return error_already_initialized;

        ULARGE_INTEGER size;
        check_hresult(IStream_Size(stream, &size));

        const storage_buffer buffer{size.LowPart};
        check_hresult(IStream_Read(stream, buffer.data(), static_cast<ULONG>(buffer.size())));

        charls::jpegls_decoder decoder{buffer, false};
        std::error_code error;
        decoder.read_header(error);
        if (error)
            winrt::throw_hresult(wincodec::error_bad_header);

        const auto& [width, height, bits_per_sample, component_count]{decoder.frame_info()};

        properties_[0].first = PKEY_Image_HorizontalSize;
        properties_[0].second = property_variant{width};
        properties_[1].first = PKEY_Image_VerticalSize;
        properties_[1].second = property_variant{height};
        properties_[2].first = PKEY_Image_BitDepth;
        properties_[2].second = property_variant{static_cast<uint32_t>(bits_per_sample * component_count)};
        properties_[3].first = PKEY_Image_Dimensions;
        properties_[3].second = property_variant{(to_wstring(width) + L"x" + to_wstring(height)).c_str()};
        properties_[4].first = PKEY_Image_Compression;
        properties_[4].second = property_variant{static_cast<uint16_t>(IMAGE_COMPRESSION_JPEG)};
        initialized_ = true;

        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    // IPropertyStoreCapabilities
    HRESULT __stdcall IsPropertyWritable(const PROPERTYKEY& /*key*/) noexcept override
    {
        TRACE("{} property_store::IsPropertyWritable\n", fmt::ptr(this));
        return success_false;
    }

    // IPropertyStore Interface
    HRESULT __stdcall GetCount(DWORD* count) noexcept override
    try
    {
        TRACE("{} property_store::GetCount\n", fmt::ptr(this));

        // Implementation recommendation is to set count to zero (when possible) in an error condition.
        *check_out_pointer(count) = static_cast<DWORD>(initialized_ ? properties_.size() : 0U);
        check_state();

        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall GetAt(const DWORD index, PROPERTYKEY* key) noexcept override
    try
    {
        TRACE("{} property_store::GetAt\n", fmt::ptr(this));
        check_state();

        if (index >= properties_.size())
            return error_invalid_argument;

        *check_out_pointer(key) = properties_[index].first;
        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall GetValue(const PROPERTYKEY& key, PROPVARIANT* value) noexcept override
    try
    {
        TRACE("{} property_store::GetValue\n", fmt::ptr(this));
        check_state();

        if (const auto* property_value{find(span{properties_}, key)}; property_value)
        {
            property_value->copy(value);
        }
        else
        {
            PropVariantInit(check_out_pointer(value));
        }

        return success_ok;
    }
    catch (...)
    {
        return to_hresult();
    }

    HRESULT __stdcall SetValue(const PROPERTYKEY& /*key*/, const PROPVARIANT& /*value*/) noexcept override
    {
        TRACE("{} property_store::SetValue\n", fmt::ptr(this));
        return error_access_denied;
    }

    HRESULT __stdcall Commit() noexcept override
    {
        TRACE("{} property_store::Commit\n", fmt::ptr(this));
        return error_access_denied;
    }

private:
    void check_state() const
    {
        if (!initialized_)
            winrt::throw_hresult(error_not_valid_state);
    }

    std::array<pair<PROPERTYKEY, property_variant>, 5> properties_; // Use std::array as a small map.
    std::atomic<bool> initialized_{};
};

} // namespace

HRESULT create_property_store_class_factory(GUID const& interface_id, void** result)
{
    return winrt::make<class_factory<property_store>>()->QueryInterface(interface_id, result);
}
