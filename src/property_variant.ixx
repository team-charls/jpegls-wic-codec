// SPDX-FileCopyrightText: © 2021 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

export module property_variant;

import std;
import winrt;
import <win.hpp>;

import "macros.hpp";


export struct property_variant final : PROPVARIANT
{
    property_variant() noexcept
    {
        PropVariantInit(this);
    }

    explicit property_variant(const std::uint16_t value) noexcept
    {
        VERIFY(SUCCEEDED(InitPropVariantFromUInt16(value, this)));
    }

    explicit property_variant(const std::uint32_t value) noexcept
    {
        VERIFY(SUCCEEDED(InitPropVariantFromUInt32(value, this)));
    }

    explicit property_variant(_In_ _Null_terminated_ const wchar_t* value)
    {
        winrt::check_hresult(InitPropVariantFromString(value, this));
    }

    void copy(_Out_ PROPVARIANT* destination) const
    {
        winrt::check_hresult(PropVariantCopy(destination, this));
    }

    property_variant(const property_variant&) = delete;

    property_variant(property_variant&& other) noexcept
    {
        memcpy(this, &other, sizeof(PROPVARIANT));
        PropVariantInit(&other);
    }

    property_variant& operator=(const property_variant&) = delete;

    property_variant& operator=(property_variant&& other) noexcept
    {
        memcpy(this, &other, sizeof(PROPVARIANT));
        PropVariantInit(&other);
        return *this;
    }

    ~property_variant()
    {
        VERIFY(SUCCEEDED(PropVariantClear(this)));
    }
};
