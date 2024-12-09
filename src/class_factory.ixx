// SPDX-FileCopyrightText: © 2019 Team CharLS
// SPDX-License-Identifier: BSD-3-Clause

export module class_factory;

import winrt;
import <win.hpp>;

import hresults;
import util;

export template<typename Class>
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
            return to_hresult();
        }
    }

    HRESULT __stdcall LockServer(BOOL /*lock*/) noexcept override
    {
        return success_ok;
    }
};
