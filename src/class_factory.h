// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

template <typename Class>
struct class_factory : winrt::implements<class_factory<Class>, IClassFactory>
{
    HRESULT __stdcall CreateInstance(
        IUnknown* outer,
        GUID const& interface_id,
        void** result) noexcept override
    {
        if (!result)
            return E_POINTER;

        *result = nullptr;

        if (outer)
            return CLASS_E_NOAGGREGATION;

        try
        {
            return winrt::make<Class>()->QueryInterface(interface_id, result);
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    HRESULT __stdcall LockServer(BOOL) noexcept override
    {
        return S_OK;
    }
};
