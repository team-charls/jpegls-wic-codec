// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include "errors.h"

template<typename Class>
struct class_factory : winrt::implements<class_factory<Class>, IClassFactory>
{
    HRESULT __stdcall CreateInstance(IUnknown* outer, GUID const& interface_id, void** result) noexcept override
    {
        if (!result)
            return error_pointer;

        *result = nullptr;

        if (outer)
            return error_no_aggregation;

        try
        {
            return winrt::make<Class>()->QueryInterface(interface_id, result);
        }
        catch (...)
        {
            return winrt::to_hresult();
        }
    }

    HRESULT __stdcall LockServer(BOOL /*lock*/) noexcept override
    {
        return error_ok;
    }
};
