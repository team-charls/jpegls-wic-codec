// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

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

        return winrt::make<Class>()->QueryInterface(interface_id, result);
    }

    HRESULT __stdcall LockServer(BOOL) noexcept override
    {
        return S_OK;
    }
};