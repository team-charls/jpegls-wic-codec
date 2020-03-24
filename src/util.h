// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#pragma once

#include "errors.h"

#include <winrt/base.h>

#include <cassert>
#include <string>

#define WARNING_SUPPRESS(x) __pragma(warning(push)) __pragma(warning(disable \
                                                                     : x)) // NOLINT(misc-macro-parentheses, bugprone-macro-parentheses, cppcoreguidelines-macro-usage)
#define WARNING_UNSUPPRESS() __pragma(warning(pop))                        // NOLINT(cppcoreguidelines-macro-usage)


#ifdef NDEBUG

#define ASSERT(expression) static_cast<void>(0)
#define VERIFY(expression) static_cast<void>(expression)

#else

#define ASSERT(expression) assert(expression)
#define VERIFY(expression) assert(expression)

#endif


[[nodiscard]] inline HMODULE get_current_module() noexcept
{
    HMODULE module;
    GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                      reinterpret_cast<PCTSTR>(get_current_module),
                      &module);

    return module;
}

[[nodiscard]] inline std::wstring get_module_path()
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

[[nodiscard]] inline std::wstring guid_to_string(const GUID& guid)
{
    std::wstring guid_text;

    guid_text.resize(39);
    VERIFY(StringFromGUID2(guid, guid_text.data(), static_cast<int>(guid_text.size())) != 0);

    // Remove the double null terminator.
    guid_text.resize(guid_text.size() - 1);

    return guid_text;
}

[[nodiscard]] inline const char* pixel_format_to_string(const GUID& guid) noexcept
{
    if (guid == GUID_WICPixelFormat2bppGray)
        return "GUID_WICPixelFormat2bppGray";

    if (guid == GUID_WICPixelFormat4bppGray)
        return "GUID_WICPixelFormat4bppGray";

    if (guid == GUID_WICPixelFormat8bppGray)
        return "GUID_WICPixelFormat8bppGray";

    if (guid == GUID_WICPixelFormat16bppGray)
        return "GUID_WICPixelFormat16bppGray";

    if (guid == GUID_WICPixelFormat24bppRGB)
        return "GUID_WICPixelFormat24bppRGB";

    if (guid == GUID_WICPixelFormat48bppRGB)
        return "GUID_WICPixelFormat48bppRGB";

    return "Unknown";
}

inline constexpr bool failed(winrt::hresult const result) noexcept
{
    return result < 0;
}

namespace registry {

WARNING_SUPPRESS(26493) // Don't use C-style casts
const HKEY hkey_local_machine{HKEY_LOCAL_MACHINE}; // NOLINT
WARNING_UNSUPPRESS()

inline void set_value(const PCWSTR sub_key, const PCWSTR value_name, const PCWSTR value)
{
    const auto length = wcslen(value) + 1;
    winrt::check_win32(RegSetKeyValue(hkey_local_machine, sub_key, value_name, REG_SZ, value, static_cast<DWORD>(length * sizeof(wchar_t))));
}

inline void set_value(const std::wstring& sub_key, const PCWSTR value_name, const PCWSTR value)
{
    set_value(sub_key.c_str(), value_name, value);
}

inline void set_value(const PCWSTR sub_key, const PCWSTR value_name, uint32_t value)
{
    winrt::check_win32(RegSetKeyValue(hkey_local_machine, sub_key, value_name, REG_DWORD, &value, sizeof value));
}

inline void set_value(const std::wstring& sub_key, const PCWSTR value_name, uint32_t value)
{
    set_value(sub_key.c_str(), value_name, value);
}

inline void set_value(const PCWSTR sub_key, const PCWSTR value_name, const void* value, const DWORD value_size_in_bytes)
{
    winrt::check_win32(RegSetKeyValue(hkey_local_machine, sub_key, value_name, REG_BINARY, value, value_size_in_bytes));
}

inline void set_value(const std::wstring& sub_key, const PCWSTR value_name, const void* value, const DWORD value_size_in_bytes)
{
    set_value(sub_key.c_str(), value_name, value, value_size_in_bytes);
}

inline HRESULT delete_tree(const PCWSTR sub_key) noexcept
{
    const LSTATUS result = RegDeleteTreeW(hkey_local_machine, sub_key);
    if (result != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(result);
    }

    return error_ok;
}

} // namespace registry
