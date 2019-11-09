// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include <winrt/base.h>

#include <string>

#define WARNING_SUPPRESS(x) __pragma(warning(push)) __pragma(warning(disable : x))  // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)
#define WARNING_UNSUPPRESS() __pragma(warning(pop))  // NOLINT(cppcoreguidelines-macro-usage)

inline HMODULE get_current_module() noexcept
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
    size_t path_size;
    size_t actual_size;

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
	std::wstring guid_text;

	guid_text.resize(39);
    WINRT_VERIFY(StringFromGUID2(guid, guid_text.data(), static_cast<int>(guid_text.size())) != 0);

	// Remove the double null terminator.
	guid_text.resize(guid_text.size() - 1);

    return guid_text;
}


struct registry
{
    static void set_value(const PCWSTR sub_key, const PCWSTR value_name, const PCWSTR value)
    {
        const auto length = wcslen(value) + 1;
        winrt::check_win32(RegSetKeyValue(HKEY_LOCAL_MACHINE, sub_key, value_name, REG_SZ, value, static_cast<DWORD>(length * sizeof(wchar_t))));
    }

    static void set_value(const PCWSTR sub_key, const PCWSTR value_name, uint32_t value)
    {
        winrt::check_win32(RegSetKeyValue(HKEY_LOCAL_MACHINE, sub_key, value_name, REG_DWORD, &value, sizeof value));
    }

    static void set_value(const PCWSTR sub_key, const PCWSTR value_name, const void* value, const DWORD value_size_in_bytes)
    {
        winrt::check_win32(RegSetKeyValue(HKEY_LOCAL_MACHINE, sub_key, value_name, REG_BINARY, value, value_size_in_bytes));
    }

    static HRESULT delete_tree(const PCWSTR sub_key) noexcept
    {
        const LSTATUS result = RegDeleteTreeW(HKEY_LOCAL_MACHINE, sub_key);
        if (result != ERROR_SUCCESS)
        {
            return HRESULT_FROM_WIN32(result);
        }

        return S_OK;
    }
};
