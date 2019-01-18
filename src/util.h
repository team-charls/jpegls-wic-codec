// Copyright (c) Team CharLS. All rights reserved. See the accompanying "LICENSE.md" for licensed use.

#pragma once

#include <winrt/base.h>

#include <string>

#define WARNING_SUPPRESS(x) __pragma(warning(push)) __pragma(warning(disable : x))  // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses)
#define WARNING_UNSUPPRESS() __pragma(warning(pop))

inline HMODULE get_current_module()
{
    HMODULE module;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<PCTSTR>(get_current_module),
        &module);

    return module;
}

inline std::wstring get_module_path()
{
    std::wstring path(100, L'?');
    size_t path_size{};
    size_t actual_size{};

    do
    {
        path_size = path.size();
        actual_size = ::GetModuleFileName(get_current_module(), path.data(), static_cast<DWORD>(path_size));

        if (actual_size + 1 > path_size)
        {
            path.resize(path_size * 2, L'?');
        }
    } while (actual_size + 1 > path_size);

    path.resize(actual_size);
    return path;
}

inline std::wstring guid_to_string(const GUID& guid)
{
    std::wstring guid_text{L"{????????-????-????-????-????????????}"};
    WINRT_VERIFY(StringFromGUID2(guid, guid_text.data(), 39) != 0);

    return guid_text;
}


struct registry
{
    static void set_value(PCWSTR sub_key, PCWSTR value_name, PCWSTR value)
    {
        const auto length = wcslen(value) + 1;
        const auto result = RegSetKeyValue(HKEY_LOCAL_MACHINE, sub_key, value_name, REG_SZ, value, static_cast<DWORD>(length * sizeof(wchar_t)));
        if (result != ERROR_SUCCESS)
            throw std::exception();
    }

    static void set_value(PCWSTR sub_key, PCWSTR value_name, uint32_t value)
    {
        const auto retCode = RegSetKeyValue(HKEY_LOCAL_MACHINE, sub_key, value_name, REG_DWORD, &value, sizeof value);
        if (retCode != ERROR_SUCCESS)
            throw std::exception();
    }

    static void set_value(PCWSTR sub_key, PCWSTR value_name, const void* value, DWORD value_size_in_bytes)
    {
        const auto retCode = RegSetKeyValue(HKEY_LOCAL_MACHINE, sub_key, value_name, REG_BINARY, value, value_size_in_bytes);
        if (retCode != ERROR_SUCCESS)
            throw std::exception();
    }
};
